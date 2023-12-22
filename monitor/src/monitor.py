#!/usr/bin/env python

from pymongo import MongoClient
import logging
import config as cfg
import serial
import datetime
import sys
import numpy as np
import csv
import struct
from crccheck.crc import Crc16Xmodem
from struct import *
from sympy import *
import random
import binascii

import json
import base64

from flask_socketio import SocketIO
from flask import Flask
import eventlet

USBPORT0 = "COM4"
USBPORT1 = "COM5"

logging.basicConfig(filename=cfg.LOGGING_FILE, level=logging.DEBUG)

class VladModule:
    def __init__(self):
        self.toneLevel = 0
        self.baseCurrent = 0
        self.agc152m = 0
        self.agc172m = 0
        self.level152m = 0
        self.level172m = 0
        self.ref152m = 0
        self.ref172m = 0
        self.ucTemperature = 0
        self.v_5v = 0.0
        self.inputVoltage = 0.0
        self.current = 0
        self.isRemoteAttenuation = False
        self.isSmartTune = False
        self.isReverse = False
        self.attenuation = 0

class MasterModule:
    def __init__(self):
        self.deviceTemperature = 0
        self.inputVoltage = 0.0
        self.current = 0

app = Flask(__name__)

# socket = SocketIO(app, cors_allowed_origins='*',
#                   logger=True, engineio_logger=True)
socket = SocketIO(app, cors_allowed_origins='*')

database = None
client = None
ser = None
f_agc_convert = None


class CircularBuffer:
    def __init__(self, size):
        self.size = size
        self.buffer = [0] * size
        self.index = 0
        self.is_full = False
    
    def add(self, value):
        self.buffer[self.index] = value
        self.index = (self.index + 1) % self.size
        if not self.is_full and self.index == 0:
            self.is_full = True
    
    def get(self):
        if self.is_full:
            return self.buffer
        else:
            return self.buffer[:self.index]
    def __str__(self):
            return str(self.buffer)


window_size = 5  # Number of samples to consider for the moving average
downlinkPowerOutputSamples = CircularBuffer(window_size)
def getCsvData(data):
    x_vals = []
    y_vals = []

    try:
        with open(data) as file:
            reader = csv.reader(file)
            next(reader) # saltar la fila de encabezado
            for row in reader:
                a = row[0]
                x_vals.append(float(row[0]))
                y_vals.append(float(row[1]))

        # Convertir los datos a arrays de valores de x y y
        x = np.array(x_vals)
        y = np.array(y_vals)

    except Exception as e:
        logging.exception(e)
    
    return x,y

x,y = getCsvData(cfg.AGC_DATA)
coeffs = np.polyfit(x, y, 8)
f_agc_convert = np.poly1d(coeffs)
x,y = getCsvData(cfg.POWER_DATA)
coeffs = np.polyfit(x, y, 8)
f_power_convert = np.poly1d(coeffs)
x,y = getCsvData(cfg.VOLTAGE_DATA)
coeffs = np.polyfit(x, y, 8)
f_voltage_convert = np.poly1d(coeffs)
x,y = getCsvData(cfg.CURRENT_DATA)
coeffs = np.polyfit(x, y, 8)
f_current_convert = np.poly1d(coeffs)

def moving_average(new_sample, buffer):
    buffer.add(new_sample)
    samples = buffer.get()
    return sum(samples) / len(samples)

def dbConnect():
    """
    Connects to DB
    """
    global database
    global client
    logging.debug("Conneting to database...")
    try:
        client = MongoClient(
            "mongodb://"+cfg.db["user"]+":"+cfg.db["passwd"] +
            "@"+cfg.db["host"]+":"+cfg.db["port"]+"/"+cfg.db["dbname"])
        database = client["rdss"]
    except Exception as e:
        logging.exception(e)


def convert(x,in_min,in_max,out_min,out_max):
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

def getProvisionedDevices():
    """
    Gets provisioned devices from DB
    Provisioned devices has status.provisioned attribute setting to True
    """
    try:
        collection_name = database["devices"]
        devices = list(collection_name.find(
            {"status.provisioned": True}, {"id": 1, "type": 1, "name": 1,"attenuation":1,"changed":1, "_id": 0}).limit(cfg.MAX_DEVICES))
    except Exception as e:
        logging.exception(e)
    return devices


def getConfigParams():
    """
    Gets configured config params
    """
    try:
        collection_name = database["config"]
        config = collection_name.find(
            {}, {"_id": 0, "image": 0}).limit(1)
    except Exception as e:
        logging.exception(e)
    return config


def updateDeviceConnectionStatus(device, status):
    """
    Updates device status.connected attribute
    """
    database.devices.update_one(
        {"id": device}, {"$set": {"status.connected": status}})

def updateDeviceChangedFlag(device, changed):
    """
    Updates device changed attribute
    """
    database.devices.update_one(
        {"id": device}, {"$set": {"changed": changed}})
        
def insertDevicesDataIntoDB(rtData):
    """
    Saves collected devices data into DB collection
    """
    for device in rtData:
        d = json.loads(device)
        if "rtData" in d:
            logging.debug("Data written to DB: {}".format(d))

            # Patches
            if "voltage" in d["rtData"]:
                if(d["rtData"]["voltage"] > cfg.MAX_VOLTAGE):
                    d["rtData"]["voltage"] = cfg.MAX_VOLTAGE

            try:
                if "alerts" in d:
                    database.devices.update_one(
                        {"id": d["id"]}, {"$set": {"alerts": d["alerts"]}})

                # DELETEME
                # database.devices.update_one(
                #     {"id": d["id"]}, {"$addToSet": {"rtData": d["rtData"]}})
                # END OF DELETEME

                d["rtData"]["metaData"] = {"deviceId": d["id"]}
                d["rtData"]["sampleTime"] = datetime.datetime.now().replace(
                    microsecond=0)
                database.rtData.insert_one(d["rtData"])

            except Exception as e:
                logging.exception(e)


def openSerialPort(port=""):
    global ser
    logging.debug("Open Serial port %s" % port)
    try:
        ser = serial.Serial(
            port=port,
            baudrate=cfg.serial["baudrate"],
            parity=cfg.serial["parity"],
            stopbits=cfg.serial["stopbits"],
            bytesize=cfg.serial["bytesize"],
            timeout=cfg.serial["timeout"],
            write_timeout=cfg.serial["write_timeout"],
            inter_byte_timeout=cfg.serial["inter_byte_timeout"]
        )
    except serial.SerialException as msg:
        logging.exception("Error opening serial port %s" % msg)
        logging.exception("Trying to open " + port)
        openSerialPort(USBPORT0)
    except:
        exctype, errorMsg = sys.exc_info()[:2]
        logging.exception("%s  %s" % (errorMsg, exctype))
        openSerialPort(USBPORT1)


def getChecksum(cmd):
    """
    -Description: this fuction calculate the checksum for a given comand
    -param text: string with the data, ex device = 03 , id = 03 cmd = 0503110000
    -return: cheksum for the given command
    """
    data = bytearray.fromhex(cmd)

    crc = hex(Crc16Xmodem.calc(data))
    if (len(crc) == 5):
        checksum = crc[3:5] + '0' + crc[2:3]
    elif(len(crc) == 4):
        checksum = crc[2:4].zfill(2) + crc[4:6].zfill(2)
    else:
        checksum = crc[4:6] + crc[2:4]
    return checksum


def evaluateAlerts(response):
    """
    Check if the data collected is within the parameters configured in the DB
    """
    alerts = {}

    params = getConfigParams()[0]

    if (response["voltage"] < float(params["minVoltage"])) or (response["voltage"] > float(params["maxVoltage"])):
        alerts["voltage"] = True
    if (response["current"] < float(params["minCurrent"])) or (response["current"] > float(params["maxCurrent"])):
        alerts["current"] = True
    if (response["gupl"] < float(params["minUplink"])) or (response["gupl"] > float(params["maxUplink"])):
        alerts["gupl"] = True
    if (response["gdwl"] < float(params["minDownlink"])) or (response["gdwl"] > float(params["maxDownlink"])):
        alerts["gdwl"] = True
    if (response["power"] < float(params["minDownlinkOut"])) or (response["power"] > float(params["maxDownlinkOut"])):
        alerts["power"] = True
    return alerts

def setAttenuation(ser,device,attenuation):
    """Sets device downlink attenuation

    Args:
        ser: serial port
        device: device ID
        attenuation: integer between 0 and 32

    Returns:
        boolean: if changed was applied or error
    """
    # 7e 05 05 12 01 17 f6e3 7f#

    # Convert the integer to a hexadecimal string
    hex_string = hex(attenuation)[2:]
    # Pad the hexadecimal string with zeros to ensure it has at least two digits
    hex_string_padded = hex_string.zfill(2)
    hex_attenuation = hex_string_padded
    cmd = hex(device)
    if(len(cmd) == 3):
        cmd_string = '05' + '0' + cmd[2:3] + '1201'+hex_attenuation
    else:
        cmd_string = '05' + cmd[2:4] + '1201'+hex_attenuation

    checksum = getChecksum(cmd_string)
    command = '7E' + cmd_string + checksum + '7F'

    logging.debug("Attenuation:"+str(attenuation))
    logging.debug("SENT: "+command)

    cmd_bytes = bytearray.fromhex(command)
    hex_byte = ''

    try:
        for cmd_byte in cmd_bytes:
            hex_byte = ("{0:02x}".format(cmd_byte))
            ser.write(bytes.fromhex(hex_byte))

        # ---- Read from serial
        hexResponse = ser.read(100)

        logging.debug("GET: "+hexResponse.hex('-'))

        # ---- Validations
        if ((
            (len(hexResponse) > 21)
            or (len(hexResponse) < 21)
            or hexResponse == None
            or hexResponse == ""
            or hexResponse == " "
        ) or (
            hexResponse[0] != 126
            and hexResponse[20] != 127
        ) or (
            (hexResponse[3] != 17)
        ) or (
            (hexResponse[4] == 2 or hexResponse[4]
                == 3 or hexResponse[4] == 4)
        )):
            return False
        # ------------------------

        data = list()

        for i in range(0, 21):
            if(6 <= i < 18):
                data.append(hexResponse[i])

        vladRev23Id = 0xff
        if(data[0] == vladRev23Id):
            data0 = data[0]

        ser.flushInput()
        ser.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    logging.debug("changing attenuation")
    return True

def getSnifferStatus(ser, cmd):
    """
    -Description: This functionsend a cmd, wait one minute if we have data write to the databse, if not write time out
    -param text: ser serial oject, devicecount actuals device in the network, cmd command to send, cursor is the database object
    -return:
    """

    SEGMENT_START = 126
    SEGMENT_END = 127
    SEGMENT_LEN = 22
    DATA_START_INDEX = 6
    DATA_END_INDEX = 19
    MAX_2BYTE = 4095
    I_MAX = 20
    V_MAX = 10
    STATUS_QUERY = 17
    ID_INDEX = 2
    COMMAND_INDEX = 3
    SNIFFER = 10

    haveData = False
    finalData = {}

    # return({
    #     "voltage": 12,
    #     "current": 50,
    #     "gupl": 23,
    #     "gdwl": 70,
    #     "power": 100
    # })

    cmd = hex(cmd)
    if(len(cmd) == 3):
        cmd_string = f'{SNIFFER:02x}' + '0' + cmd[2:3] + f'{STATUS_QUERY:02x}'+'0000'
    else:
        cmd_string = f'{SNIFFER:02x}' + '0' + cmd[2:3] + f'{STATUS_QUERY:02x}'+'0000'
    checksum = getChecksum(cmd_string)
    command = f"{SEGMENT_START:02x}" + cmd_string + checksum + f"{SEGMENT_END:02x}"

    print("SENT: " + command)

    cmd_bytes = bytearray.fromhex(command)
    hex_byte = ''

    try:
        for cmd_byte in cmd_bytes:
            hex_byte = ("{0:02x}".format(cmd_byte))
            ser.write(bytes.fromhex(hex_byte))

        # ---- Read from serial
        hexResponse = ser.read(100)
        print("GET: " + hexResponse.hex('-'))

        # ---- Validations
        if ((
            (len(hexResponse) > SEGMENT_LEN)
            or (len(hexResponse) < SEGMENT_LEN)
            or hexResponse == None
            or hexResponse == ""
            or hexResponse == " "
        ) or (
            hexResponse[0] != SEGMENT_START
            and hexResponse[SEGMENT_LEN - 1] != SEGMENT_END
        ) or ( 
            hexResponse[ID_INDEX] != int(cmd,16)
        ) or (
            (hexResponse[COMMAND_INDEX] != STATUS_QUERY)
        )):
            return False
        # ------------------------

        data = list()

        for i in range(0, SEGMENT_LEN):  #DATALEN
            if(DATA_START_INDEX <= i < DATA_END_INDEX):
                data.append(hexResponse[i])

        #Decodificaion con datos ordenados con aout2+swin+swout segun codificacion nueva
        aIn1_1_10V = ( data[0] | data[1] << 8) # byte 1-2
        aOut1_1_10V = ( data[2] | data[3] << 8 ) # byte 3-4 
        aIn2_x_20mA = ( data[4] | data [5] << 8) # byte 5-6
        aOut2_x_20mA = ( data[6] | data[7] << 8) # byte 7-8
        swIn_x_20mA = "ON" if (bool (data[8]) == 0x01) else "OFF" # byte 9
        swOut_x_20mA = "ON" if (bool (data[9]) == 0x01) else "OFF" # byte 10
        dIn1 = "ON" if (bool (data[10]) == 0x01) else "OFF" # byte 11
        dIn2 = "ON" if (bool (data[11]) == 0x01) else "OFF" # byte 12
        swSerial = "R485" if (bool (data[12]) == 0x01) else "RS232" # byte 13 (default: 0/rs232)
            
        print(f"ain1: {aIn1_1_10V}")
        print(f"aout1: {aOut1_1_10V}")
        print(f"ain2: {aIn2_x_20mA}")
        print(f"aout2: {aOut2_x_20mA}")
        print(f"din_sw: {swIn_x_20mA}") #digital
        print(f"dout_sw: {swOut_x_20mA}") #digital
        print(f"din_1: {dIn1}")
        print(f"din_2: {dIn2}")
        print(f"swSerial: {swSerial}")
        
        vIn1_linear = round(arduino_map(aIn1_1_10V, 0, MAX_2BYTE, 0, V_MAX), 2)
        vOut1_linear = round(arduino_map(aOut1_1_10V, 0, MAX_2BYTE, 0, V_MAX), 2)
        if(swIn_x_20mA == swOut_x_20mA == "ON"):
            #4-20mA
            iIn2_linear = round(arduino_map(aIn2_x_20mA, 0, MAX_2BYTE, 0, I_MAX), 2)
            iOut2_linear = round(arduino_map(aOut2_x_20mA, 0, MAX_2BYTE, 0, I_MAX), 2)
        else: 
            #0-20mA
            iIn2_linear = round(arduino_map(aIn2_x_20mA, 0, MAX_2BYTE, 4, I_MAX), 2)
            iOut2_linear = round(arduino_map(aOut2_x_20mA, 0, MAX_2BYTE, 4, I_MAX), 2)

        print("Datos escalados: ")
        print(f"Analog1 Input Voltage: {vIn1_linear} V")
        print(f"Analog1 Output Voltage: {vOut1_linear} V")
        print(f"Analog2 Input Current: {iIn2_linear} mA")
        print(f"Analog2 Output Current: {iOut2_linear} mA")
            
        SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
        timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')
        #finalData = {
        #    "sampleTime": timeNow,
        #    "voltage": lineVoltageConverted,
        #    "current": unitCurrentConverted,
        #    "gupl": uplinkAgcValueConverted,
        #    "gdwl": downlinkAgcValueConverted,
        #    "power": downlinkOutputPowerAvg
        #}
        # -----------------------------------------------------
        ser.flushInput()
        ser.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    return {}

def arduino_map(value, in_min, in_max, out_min, out_max):
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min



def decodeMaster(buffer):
    bufferIndex = 0
    master = MasterModule()
    Measurements = (
        'current',
        'vin'
    )
    
    measurement = [0] * len(Measurements)

    for i in range(len(Measurements)):
        measurement[i] = buffer[bufferIndex] | (buffer[bufferIndex + 1] << 8)
        bufferIndex += 2

    ADC_CONSUMPTION_CURRENT_FACTOR = 0.06472492
    ADC_LINE_CURRENT_FACTOR = 0.0010989
    ADC_VOLTAGE_FACTOR = 0.01387755
    ADC_V5V_FACTOR = 0.00161246
    VREF = 5 
    RESOLUTION = 12 
 #   temperature = float((float(measurement[Measurements.index('ucTemperature')]) - float(TEMP30_CAL_ADDR)) * (110.0 - 30.0) / (float(TEMP110_CAL_ADDR) - float(TEMP30_CAL_ADDR)))
    master.inputVoltage = round(measurement[Measurements.index('vin')] * ADC_VOLTAGE_FACTOR,2)
    master.current = round(measurement[Measurements.index('current')] * ADC_CONSUMPTION_CURRENT_FACTOR/1000,3)
    master.deviceTemperature = buffer[bufferIndex]
    return master

def isCrcOk(hexResponse,size):
    dataEnd = size-3
    crcEnd = size-1
    dataBytes = hexResponse[1:dataEnd]
    checksumBytes = hexResponse[dataEnd:crcEnd]
    checksumString = binascii.hexlify(checksumBytes).decode('utf-8')
    dataString = binascii.hexlify(dataBytes).decode('utf-8')
    calculatedChecksum = getChecksum(dataString)
    crcMessage = "CRC Calculated: " + calculatedChecksum + " CRC Received: " +checksumString+ " - D"
    if(calculatedChecksum == checksumString):
        print(crcMessage+"OK: "+calculatedChecksum + " == " +checksumString )
        return True
    else:
        print(crcMessage+"ERROR: "+calculatedChecksum + " != " +checksumString )
        return False
    
def sendMasterQuery(ser,times):
    """
    Sends a command, waits for one minute if data is received, and returns the data.
    Args:
        ser: Serial object.
        cmd: Command to send.
    Returns:
        Dictionary containing the received data, or False if an error occurs.
    """
    if times == 0:
        return False
    
    finalData = {}
    cmd = hex(0)
    QUERY_MASTER_STATUS = '13'
    if len(cmd) == 3:
        cmdString = '05' + '0' + cmd[2:3] + QUERY_MASTER_STATUS+'0000'
    else:
        cmdString = '05' + cmd[2:4] + QUERY_MASTER_STATUS+'0000'

    checksum = getChecksum(cmdString)
    command = '7E' + cmdString + checksum + '7F'
    print("Attempt: " + str(times))
    print("SENT: " + command)

    try:
        cmdBytes = bytearray.fromhex(command)
        for cmdByte in cmdBytes:
            hexByte = "{0:02x}".format(cmdByte)
            ser.write(bytes.fromhex(hexByte))

        hexResponse = ser.read(14)
        response = hexResponse.hex('-')
        print("GET: " + hexResponse.hex('-'))
        message =""
        for byte in hexResponse:
          decimal_value = byte
          message+=str(byte).zfill(2)+"-"
        print("GET: "+ message)

        responseLen = len(hexResponse)
        print("receive len: " + str(responseLen))
        if responseLen != 14:
            sendMasterQuery(ser,times-1)
            return False
        if  hexResponse[0] != 126:
            sendMasterQuery(ser,times-1)
            return False
        if hexResponse[responseLen - 1 ] != 127:
            sendMasterQuery(ser,times-1)
            return False
        if hexResponse[2] != int(cmd, 16):
            sendMasterQuery(ser,times-1)
            return False
        if  hexResponse[3] != 19:
            sendMasterQuery(ser,times-1)
            return False
        if hexResponse[4] in [2, 3, 4]:
            sendMasterQuery(ser,times-1)
            return False
        if isCrcOk(hexResponse,responseLen) == False:
            sendMasterQuery(ser,times-1)
            return False
        
        data = list(hexResponse[i] for i in range(6, responseLen - 3))
        

        master = decodeMaster(data)
    
        print(f"Input Voltage: {master.inputVoltage:.2f}[V]")
        print(f"Current Consumption: {master.current:.3f}[mA]")
        print(f"Device Temperature: {master.deviceTemperature}[°C]]")   

        sampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
        timeNow = datetime.datetime.strptime(sampleTime, "%Y-%m-%dT%H:%M:%SZ")
        finalData = {
            "sampleTime": timeNow,
            "voltage": master.inputVoltage,
            "current": master.current,
            "gupl": 0,
            "gdwl": 0,
            "power": master.deviceTemperature
        }

        ser.flushInput()
        ser.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    return finalData


def sendStatusToFrontEnd(rtData):
    """
    Sends via SocketIO the real-time provisioned devices status
    This updates frontend interface
    """
    logging.debug("Emiting event...")

    eventMessage = {
        "event": 'set_rtdata_event',
        "leave": False,
        "handle": 'SET_MONITOR_DATA_EVENT',
        "data": rtData
    }
    socket.emit('set_rtdata_event', base64.b64encode(
        json.dumps(eventMessage).encode('utf-8')))
    eventlet.monkey_patch()
    socket.emit('set_rtdata_event', eventMessage)


def defaultJSONconverter(o):
    """
    Converter function that stringifies our datetime object.
    """
    if isinstance(o, datetime.datetime):
        return o.__str__()


def showBanner(provisionedDevicesArr, timeNow):
    """
    Just shows a message to console
    """
    logging.debug("-------------------------------------------------------")
    logging.debug("Starting Polling - TimeStamp: %s ", timeNow)
    logging.debug("Devices Count:"+str(len(provisionedDevicesArr)))




def sendModbus(uartCmd, snifferAddress, data, ser):
    """
    Sends variable length segment.
    Args:
        uartcmd: Command to send.
        sniffer_address: Address of target sniffer.
        data: Data to be added to modBus segment.
        ser: Serial port.
    Returns true if answer segment from sniffer is valid.
    """
    SNIFFER = "0A"
    SEGMENT_START = '7E'
    SEGMENT_END = '7F'
    ID_INDEX = 2
    COMMAND_INDEX = 3
    SERIAL_RESPONSE_CMD = 204 #CC
    DATA_START_INDEX = 6


    intLen = int(len(data)/2)
    dataLen = format(intLen, '04x')
    formatLen = dataLen[2:] + dataLen[0:2]
    cmdString = f"{SNIFFER}{snifferAddress}{uartCmd}{formatLen}{data}"
    checksum = getChecksum(cmdString)
    command = SEGMENT_START + cmdString + checksum + SEGMENT_END
    cmdLen = int(len(command)/2)
    #logging.debug("SENT: " + command)
    print("SENT: " + command)

    cmd_bytes = bytearray.fromhex(command)
    hex_byte = ''

    try:
        for cmd_byte in cmd_bytes:
            hex_byte = ('{0:02x}'.format(cmd_byte))
            ser.write(bytes.fromhex(hex_byte))

        hexResponse = ser.read(cmdLen + 10)
        #hexResponse = ser.read(100)
        
        print("GET: "+hexResponse.hex('-'))

        # ---- Validations
        responseLen = len(hexResponse)

        if ((hexResponse == None
            or hexResponse == ""
            or hexResponse == " "
            or hexResponse == b''
        ) or (
            hexResponse[0] != int(SEGMENT_START, 16)
            and hexResponse[responseLen - 1] != int(SEGMENT_END, 16)
        ) or ( 
            hexResponse[ID_INDEX] != int(snifferAddress,16)
        ) or (
            (hexResponse[COMMAND_INDEX] != SERIAL_RESPONSE_CMD
             and hexResponse[COMMAND_INDEX] != int(uartCmd, 16))
        )):
            return False
        
        # ----Extract data
        # Los datos recibidos no tienen formato conocido, solo se quitan los bytes de formato/validacion que agrega el sniffer
        dataReceived = []

        for i in range(0, responseLen):
            if(DATA_START_INDEX <= i < DATA_START_INDEX + intLen):
                dataReceived.append(hexResponse[i])

        ser.flushInput()
        ser.flushOutput()
    except Exception as e:
        logging.error(e)
        sys.exit()
    
    print("Modbus sent")
    return True


def run_monitor():
    """
    run_monitor(): Main process
    1. Gets provisioned devices from DB
    2. For each device gets status from serial port
    3. Calculate if status variables are alerted
    4. Send real-time status to frontend
    5. Save real-time status to DB
    """
    rtData = []
    provisionedDevicesArr = getProvisionedDevices()
    connectedDevices = 0
    times = 3
    SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
    timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')
    showBanner(provisionedDevicesArr, timeNow)

    if (len(provisionedDevicesArr) > 0):
        for x in provisionedDevicesArr:
            device = int(x["id"])
            deviceData = {}
            deviceData["rtData"] = {}
            logging.debug("ID: %s", device)

            
            deviceData["id"] = device
            # deviceData["type"] = x["type"]
            # deviceData["name"] = x["name"]
            if ('type' in x):
                deviceData["type"] = x["type"]
            else:
                deviceData["type"] = ''
            if ('name' in x):
                deviceData["name"] = x["name"]
            else:
                deviceData["name"] = ''

            #if(deviceData["type"] == "vlad-rev23"):
                #response = sendVladRev23Query(ser, device,times)
            #elif(deviceData["type"] == "master"):
                #response = sendMasterQuery(ser,times)
            #else:
            response = getSnifferStatus(ser, device)

            
            if (deviceData["type"] == "vlad"):
                aOut1_0_10V = 1000
                aOut2_x_20mA = 500
                dOut1 = 0
                dOut2 = 0
                serialSW = 0 #seteaer a rs232
                #serialSW = 1 #setear a rs485

                # Invertir los bytes de aout1
                aout1 = ((aOut1_0_10V >> 8) & 0xFF) | ((aOut1_0_10V << 8) & 0xFF00)

                # Invertir los bytes de aout2
                aout2 = ((aOut2_x_20mA >> 8) & 0xFF) | ((aOut2_x_20mA << 8) & 0xFF00)

                data = f"{aout1:04X}{aout2:04X}{dOut1:02X}{dOut2:02X}{serialSW:02X}"
                setSnifferData(ser, device, data)

                ### MODBUS TEST ###
                uart_cmd = "14" #comando para que el sniffer envie el paquete via serial
                sniffer_add = "08"
                data = ""
                MAXDATA = 45
                i = 0
                while i <= MAXDATA-10-1:
                    aux_hex = format(i, '02x')
                    data = data + aux_hex
                    i+=1
                data = data + 'FF' #para indicar el fin de la data en el dispositivo que recibe y reenvia serial
                sendModbus(uart_cmd, sniffer_add, data, ser)
                ### END TEST ###
                
            else:
                logging.debug("No response from device")
                deviceData["connected"] = False
                deviceData["rtData"]["sampleTime"] = {"$date": SampleTime}
                deviceData["rtData"]["alerts"] = {"connection": True}
                #updateDeviceConnectionStatus(device, False)

            #rtData.append(json.dumps(deviceData, default=defaultJSONconverter))
            # rtData.append(json.dumps(deviceData))
            # END FOR X
        logging.debug("Connected devices: %s", connectedDevices)
        #insertDevicesDataIntoDB(rtData)
        #sendStatusToFrontEnd(rtData)
    else:
        #sendStatusToFrontEnd([])
        logging.debug("No provisioned devices found in the DB")

def setSnifferData(ser,id,data):
    """Sets device downlink attenuation

    Args:
        ser: serial port
        id: device ID
        attenuation: integer between 0 and 32

    Returns:
        boolean: if changed was applied or error
    """
    DATALEN = 7
    SNIFFER = 10
    SEGMENT_START = '7E'
    SEGMENT_END = '7F'
    RESPONSE_LEN = 16

    data_len = f"{DATALEN:02x}{0:02x}"
    id = f"{id:02x}"
    set_out = "B6" #nombre del comando
    device = f"{SNIFFER:02x}"
    cmd_string = f"{device}{id}{set_out}{data_len}{data}"
    checksum = getChecksum(cmd_string)
    command = SEGMENT_START + cmd_string + checksum + SEGMENT_END

    print("data: " + str(data))
    print("SENT: " + command)

    cmd_bytes = bytearray.fromhex(command)
    hex_byte = ''

    try:
        for cmd_byte in cmd_bytes:
            hex_byte = ("{0:02x}".format(cmd_byte))
            ser.write(bytes.fromhex(hex_byte))

        # ---- Read from serial
        hexResponse = ser.read(100) #la resupesta es el mismo query

        print("GET: "+hexResponse.hex('-'))

        # ---- Validations
        if ((
            (len(hexResponse) > RESPONSE_LEN)
            or (len(hexResponse) < RESPONSE_LEN)
            or hexResponse == None
            or hexResponse == ""
            or hexResponse == " "
        ) or (
            hexResponse != cmd_bytes
        )):
            return False
        # ------------------------

        ser.flushInput()
        ser.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    print("changing attenuation")
    return True

def listen():
    """
    Listens frontend connection to emit socketio events
    """
    while True:
        if database is None:
            dbConnect()
        if ser is None:
            try:
                openSerialPort(USBPORT0)
            except:
                openSerialPort(USBPORT1)

        run_monitor()
        eventlet.sleep(cfg.POLLING_SLEEP)


eventlet.spawn(listen)

if __name__ == '__main__':
    socket.run(app, host='0.0.0.0', port=4200)
