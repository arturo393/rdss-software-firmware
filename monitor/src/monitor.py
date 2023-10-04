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
            write_timeout=cfg.serial["write_timeout"]
        )
    except serial.SerialException as msg:
        logging.exception("Error opening serial port %s" % msg)
        logging.exception("Trying to open " + port)
        openSerialPort("/dev/ttyUSB1")
    except:
        exctype, errorMsg = sys.exc_info()[:2]
        logging.exception("%s  %s" % (errorMsg, exctype))
        openSerialPort("/dev/ttyUSB0")


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

def sendCmd(ser, cmd, createdevice):
    """
    -Description: This functionsend a cmd, wait one minute if we have data write to the databse, if not write time out
    -param text: ser serial oject, devicecount actuals device in the network, cmd command to send, cursor is the database object
    -return:
    """

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
        cmd_string = '05' + '0' + cmd[2:3] + '110000'
    else:
        cmd_string = '05' + cmd[2:4] + '110000'

    checksum = getChecksum(cmd_string)
    command = '7E' + cmd_string + checksum + '7F'

    logging.debug("SENT: "+command)

    cmd_bytes = bytearray.fromhex(command)
    hex_byte = ''

    try:
        for cmd_byte in cmd_bytes:
            hex_byte = ("{0:02x}".format(cmd_byte))
            ser.write(bytes.fromhex(hex_byte))

        # ---- Read from serial
        hexResponse = ser.read(21)

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
            hexResponse[2] != int(cmd,16)
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
            # decode the frame according to the specified format
            unsigned_byte = data[0] 
            isReverse = "ON" if(bool(data[1] & 0x01)) else "OFF"
            isSmartTune = "ON" if (bool(data[1] & (0x1<<1))) else "OFF" 
            isRemoteAttenuation = bool(data[1] & (0x1<<2))
            attenuation = (data[1]>>3) & 0x1F
            lineVoltage = ( data[2] | (data[3] << 8) ) /10.0
            lineCurrent = ( data[4] | (data[5] << 8) ) /1000.0
            toneLevel = ((data[6] | (data[7] << 8)) - 400) * (0 - -35) / (4095 - 400) + -35
            downlinkAgcValue = data[8] / 10.0
            downlinkOutputPower =  data[9] if  data[9] < 128 else data[9] - 256
            uplinkAgcValue = data[10] / 10.0
            uplinkOutputPower = data[11]if  data[11] < 128 else data[11] - 256
            # print the decoded values

            logging.debug(f"Voltage: {lineVoltage:.2f}[V]")
            logging.debug(f"Line Current: {lineCurrent:.3f}[A]")
            logging.debug(f"Attenuation: {attenuation:.1f}[dB]")
            logging.debug(f"Tone Level: {toneLevel:.1f}[dBm]")
            logging.debug(f"AGC Uplink: {uplinkAgcValue:.1f}[dB]")
            logging.debug(f"Downlink Output Power: {downlinkOutputPower}[dBm]")
            logging.debug(f"AGC Downlink: {downlinkAgcValue:.1f}[dB]")
            logging.debug(f"Uplink Output Power:    {uplinkOutputPower}[dBm]")
            logging.debug(f"Is Software Attenuation : {isRemoteAttenuation}")
            logging.debug(f"Is Reverse : {isReverse}")
            logging.debug(f"Is SmartTune : {isSmartTune}")
            
            SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
            timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')
            finalData = {
                "sampleTime": timeNow,
                "voltage": lineVoltage,
                "current": lineCurrent,
                "gupl": uplinkAgcValue,
                "gdwl": downlinkAgcValue,
                "power": downlinkOutputPower,
                "smartTune": isSmartTune,
                "reverse": isReverse,
                "attenuation":attenuation
            }
        else:
        
            deviceData = list()
            for i in range(0, len(data)):

                if (i == 0 or i == 2 or i == 4 or i == 6):
                    if(data[i+1] == 255):
                        deviceData.append(data[i] * -1)
                    else:
                        deviceData.append(int.from_bytes(
                            ((data[i+1]).to_bytes(1, "little")) + ((data[i]).to_bytes(1, "little")), "big"))

                elif(i == 8 or i == 9 or i == 10 or i == 11):
                    deviceData.append(data[i])
                else:
                    pass

            logging.debug("Data read from Serial: {}".format(deviceData))
            lineVoltage = deviceData[0] / 10.0
            baseCurrent = deviceData[1] /10000.
            tunnelCurrent = deviceData[2] /1000.0
            unitCurrent = deviceData[3] / 1000.0
            uplinkAgcValue = deviceData[4] / 10.0
            downlinkInputPower = deviceData[5]  if  deviceData[5] < 128 else deviceData[5] - 256
            downlinkAgcValue = deviceData[6] / 10.0
            downlinkOutputPower = deviceData[7] if  deviceData[7] < 128 else deviceData[7] - 256

            downlinkOutputPowerConverted = moving_average(downlinkOutputPower,downlinkPowerOutputSamples)
            downlinkOutputPowerAvg = round(f_power_convert(downlinkOutputPowerConverted),2)
            downlinkInputPowerConverted = round(f_power_convert(downlinkInputPower),2)
            uplinkAgcValueConverted =  round(f_agc_convert(uplinkAgcValue),1)
            downlinkAgcValueConverted = round(f_agc_convert(downlinkAgcValue),1)
            lineVoltageConverted = round(f_voltage_convert(lineVoltage),2)
            unitCurrentConverted = round(f_current_convert(unitCurrent),3)

            logging.debug(f"Voltage: {lineVoltage} {lineVoltageConverted:.2f}[V]")
            logging.debug(f"Unit Current: {unitCurrent} {unitCurrentConverted:.3f}[A]")
            logging.debug(f"AGC Uplink: {uplinkAgcValue} {uplinkAgcValueConverted:.1f}[dB]")
            logging.debug(f"AGC Downlink: {downlinkAgcValue} {downlinkAgcValueConverted:.1f}[dB]")
            logging.debug(f"Downlink Output Power: {downlinkOutputPower} {downlinkOutputPowerAvg:.2f}[dBm] {downlinkPowerOutputSamples}")
            logging.debug(f"Downlink Input Power: {downlinkInputPower} {downlinkInputPowerConverted:.2f}[dBm]")
            
            SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
            timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')
            finalData = {
                "sampleTime": timeNow,
                "voltage": lineVoltageConverted,
                "current": unitCurrentConverted,
                "gupl": uplinkAgcValueConverted,
                "gdwl": downlinkAgcValueConverted,
                "power": downlinkOutputPowerAvg
            }
        # -----------------------------------------------------
        # if(createdevice == True):
        #     pass
        # else:
        #     SampleTime = datetime.datetime.now()

        ser.flushInput()
        ser.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    return finalData

def arduino_map(value, in_min, in_max, out_min, out_max):
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

def decodeVlad(buffer):
    vlad = VladModule()
    bufferIndex = 0
    Measurements = (
        'vin',
        'v5v',
        'current',
        'agc152m',
        'ref152m',
        'level152m',
        'agc172m',
        'ref172m',
        'level172m',
        'toneLevel',
        'ucTemperature',
        'baseCurrent'
    )
    
    measurement = [0] * len(Measurements)

    for i in range(len(Measurements)):
        measurement[i] = buffer[bufferIndex] | (buffer[bufferIndex + 1] << 8)
        bufferIndex += 2

    state = buffer[bufferIndex]

    state = buffer[bufferIndex]
    vlad.isReverse = "ON" if (bool(state & 0x01)) else "OFF"
    vlad.isSmartTune = "ON" if(bool((state >> 1) & 0x01)) else "OFF"
    vlad.isRemoteAttenuation = bool((state >> 2) & 0x01)
    vlad.attenuation = (state >> 3) & 0xFF
    bufferIndex += 1
    vlad.isRemoteAttenuation = bool((state >> 2) & 0x01)
    vlad.attenuation = (state >> 3) & 0xFF
    bufferIndex += 1

    TEMP30_CAL_ADDR = 1781  # Adjust the address accordingly
    TEMP110_CAL_ADDR = 1327  # Adjust the address accordingly
    ADC_CONSUMPTION_CURRENT_FACTOR = 0.06472492
    ADC_LINE_CURRENT_FACTOR = 0.0010989
    ADC_VOLTAGE_FACTOR = 0.01387755
    ADC_V5V_FACTOR = 0.00161246
    VREF = 5 
    RESOLUTION = 12 

    logging.debug(str(measurement[Measurements.index('current')]))
    CURRENT_MIN = 170.0
    CURRENT_MAX = 260.0
    ADC_CURRENT_MIN = 2567.0
    ADC_CURRENT_MAX = 3996.0
    MAX4003_DBM_MAX = 0   
    MAX4003_DBM_MIN = -45
    MAX4003_AGC_MIN = 30
    MAX4003_AGC_MAX = 0
    #MAX4003_ADC_MAX = 1888
    MAX4003_ADC_MAX_152M = measurement[Measurements.index('ref152m')]
    MAX4003_ADC_MAX_172M = measurement[Measurements.index('ref172m')]
    MAX4003_ADC_MIN = 487

    vlad.ucTemperature = int(measurement[Measurements.index('ucTemperature')])
    vlad.v_5v = round(measurement[Measurements.index('v5v')] * ADC_V5V_FACTOR,1)
    vlad.inputVoltage = round(measurement[Measurements.index('vin')] * ADC_VOLTAGE_FACTOR,2)
    
    vlad.current = arduino_map(measurement[Measurements.index('current')],ADC_CURRENT_MIN,ADC_CURRENT_MAX,CURRENT_MIN,CURRENT_MAX)
    vlad.current = round(vlad.current,3)/1000

    vlad.agc152m = arduino_map(measurement[Measurements.index('agc152m')],0,4095,MAX4003_AGC_MIN,MAX4003_AGC_MAX)
    vlad.agc152m = int(vlad.agc152m)

    vlad.agc172m = arduino_map(measurement[Measurements.index('agc172m')],0,4095,MAX4003_AGC_MIN,MAX4003_AGC_MAX)
    vlad.agc172m = int(vlad.agc172m)

    vlad.level152m = arduino_map(measurement[Measurements.index('level152m')],MAX4003_ADC_MIN,MAX4003_ADC_MAX_152M,MAX4003_DBM_MIN,MAX4003_DBM_MAX)
    vlad.level152m = int(vlad.level152m)

    vlad.level172m = arduino_map(measurement[Measurements.index('level172m')],MAX4003_ADC_MIN,MAX4003_ADC_MAX_172M,MAX4003_DBM_MIN,MAX4003_DBM_MAX) 
    vlad.level172m = int(vlad.level172m)

    vlad.ref152m = arduino_map(measurement[Measurements.index('ref152m')],MAX4003_ADC_MIN,MAX4003_ADC_MAX_152M,MAX4003_DBM_MIN,MAX4003_DBM_MAX)
    vlad.ref152m = int(vlad.ref152m)

    vlad.ref172m = arduino_map(measurement[Measurements.index('ref172m')],MAX4003_ADC_MIN,MAX4003_ADC_MAX_172M,MAX4003_DBM_MIN,MAX4003_DBM_MAX) 
    vlad.ref172m = int(vlad.ref172m)
    

    vlad.toneLevel = arduino_map(measurement[Measurements.index('toneLevel')],MAX4003_ADC_MIN,MAX4003_ADC_MAX_152M,MAX4003_DBM_MIN,MAX4003_DBM_MAX)
    vlad.toneLevel = int(vlad.toneLevel)
    
    vlad.baseCurrent = (measurement[Measurements.index('baseCurrent')]  * VREF) / (1 << (RESOLUTION - 0x00))
    vlad.baseCurrent = round(vlad.baseCurrent,3)          
                        
    return vlad

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
        logging.debug(crcMessage+"OK: "+calculatedChecksum + " == " +checksumString )
        return True
    else:
        logging.debug(crcMessage+"ERROR: "+calculatedChecksum + " != " +checksumString )
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
    logging.debug("Attempt: " + str(times))
    logging.debug("SENT: " + command)

    try:
        cmdBytes = bytearray.fromhex(command)
        for cmdByte in cmdBytes:
            hexByte = "{0:02x}".format(cmdByte)
            ser.write(bytes.fromhex(hexByte))

        hexResponse = ser.read(14)
        response = hexResponse.hex('-')
        logging.debug("GET: " + hexResponse.hex('-'))
        message =""
        for byte in hexResponse:
          decimal_value = byte
          message+=str(byte).zfill(2)+"-"
        logging.debug("GET: "+ message)

        responseLen = len(hexResponse)
        logging.debug("receive len: " + str(responseLen))
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
    
        logging.debug(f"Input Voltage: {master.inputVoltage:.2f}[V]")
        logging.debug(f"Current Consumption: {master.current:.3f}[mA]")
        logging.debug(f"Device Temperature: {master.deviceTemperature}[°C]]")   

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

def sendVladRev23Query(ser, deviceID,times):
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
    cmd = hex(deviceID)
    if len(cmd) == 3:
        cmdString = '05' + '0' + cmd[2:3] + '110000'
    else:
        cmdString = '05' + cmd[2:4] + '110000'

    checksum = getChecksum(cmdString)
    command = '7E' + cmdString + checksum + '7F'
    logging.debug("Attempt: " + str(times))
    logging.debug("SENT: " + command)

    try:
        cmdBytes = bytearray.fromhex(command)
        for cmdByte in cmdBytes:
            hexByte = "{0:02x}".format(cmdByte)
            ser.write(bytes.fromhex(hexByte))

        hexResponse = ser.read(100)
        response = hexResponse.hex('-')
        logging.debug("GET: " + hexResponse.hex('-'))
        message =""
        for byte in hexResponse:
          decimal_value = byte
          message+=str(byte).zfill(2)+"-"
        logging.debug("GET: "+ message)

        responseLen = len(hexResponse)
        logging.debug("receive len: " + str(responseLen))
        if responseLen != 34:
            return sendVladRev23Query(ser, deviceID,times-1)
        if  hexResponse[0] != 126:
            return sendVladRev23Query(ser, deviceID,times-1)
        if hexResponse[responseLen - 1 ] != 127:
            return sendVladRev23Query(ser, deviceID,times-1)
        if hexResponse[2] != int(cmd, 16):
            return sendVladRev23Query(ser, deviceID,times-1)
        if  hexResponse[3] != 17:
            return sendVladRev23Query(ser, deviceID,times-1)
        if hexResponse[4] in [2, 3, 4]:
            return sendVladRev23Query(ser, deviceID,times-1)
        if isCrcOk(hexResponse,responseLen) == False:
            return sendVladRev23Query(ser, deviceID,times-1)
        
        data = list(hexResponse[i] for i in range(6, responseLen - 3))
        

        vlad = decodeVlad(data)
    
        logging.debug(f"Voltage: {vlad.inputVoltage:.2f}[V]")
        logging.debug(f"Voltage reference: {vlad.v_5v}[V]]")   
        logging.debug(f"Current: {vlad.current:.3f}[mA]")
        logging.debug(f"Base Current: {vlad.baseCurrent:.3f}[mA]")
        logging.debug(f"Attenuation: {vlad.attenuation:}[dB]")
        logging.debug(f"Tone Level: {vlad.toneLevel:}[dBm]")
        logging.debug(f"Downlink - AGC {vlad.agc152m:}[dB] Reference: {vlad.ref152m} [dBm] Output Power: {vlad.level152m} [dBm]") 
        logging.debug(f"Uplink   - AGC {vlad.agc172m:}[dB] Reference: {vlad.ref172m} [dBm] Output Power: {vlad.level172m} [dBm]") 
        logging.debug(f"Is Software Attenuation: {vlad.isRemoteAttenuation}") 
        logging.debug(f"Is Reverse: {vlad.isReverse}")
        logging.debug(f"Is SmartTune: {vlad.isSmartTune}")
        logging.debug(f"uC Temperature: {vlad.ucTemperature}[°C]")

        sampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
        timeNow = datetime.datetime.strptime(sampleTime, "%Y-%m-%dT%H:%M:%SZ")
        finalData = {
            "sampleTime": timeNow,
            "voltage": vlad.inputVoltage,
            "current": vlad.current,
            "gupl": vlad.agc172m,
            "gdwl": vlad.agc152m,
            "power": vlad.level152m,
            "smartTune": vlad.isSmartTune,
            "reverse": vlad.isReverse,
            "attenuation": vlad.attenuation
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

            if(deviceData["type"] == "vlad-rev23"):
                response = sendVladRev23Query(ser, device,times)
            elif(deviceData["type"] == "master"):
                response = sendMasterQuery(ser,times)
            else:
                response = sendCmd(ser, device, False)

            if (response):
                connectedDevices += 1
                deviceData["connected"] = True
                deviceData["rtData"] = response
                alerts = evaluateAlerts(response)
                deviceData["rtData"]["alerts"] = alerts
                deviceData["alerts"] = alerts
                updateDeviceConnectionStatus(device, True)
                #-------------------------------------------------
                # Leyendo atenuación y estado de cambio
                #-------------------------------------------------
                attenuation = 0
                if "attenuation" in x:
                    value = x["attenuation"]
                    if value is not None:
                         attenuation = int(value)
                attenuationChanged = bool(x["changed"]) if ('changed' in x) else False
                # TODO: Setear atenuación
                if (attenuationChanged):
                    setAttenuation(ser,device,attenuation)
                    # TODO: actualizar registro en DB para que no vuelva a setear la atenuación a menos que el usuario vuelva a cambiarla  manualmente
                    updateDeviceChangedFlag(device, False)
     
            else:
                logging.debug("No response from device")
                deviceData["connected"] = False
                deviceData["rtData"]["sampleTime"] = {"$date": SampleTime}
                deviceData["rtData"]["alerts"] = {"connection": True}
                updateDeviceConnectionStatus(device, False)

            rtData.append(json.dumps(deviceData, default=defaultJSONconverter))
            # rtData.append(json.dumps(deviceData))
            # END FOR X
        logging.debug("Connected devices: %s", connectedDevices)
        insertDevicesDataIntoDB(rtData)
        sendStatusToFrontEnd(rtData)
    else:
        sendStatusToFrontEnd([])
        logging.debug("No provisioned devices found in the DB")

def listen():
    """
    Listens frontend connection to emit socketio events
    """
    while True:
        if database is None:
            dbConnect()
        if ser is None:
            try:
                openSerialPort("/dev/ttyUSB0")
            except:
                openSerialPort("/dev/ttyUSB1")

        run_monitor()
        eventlet.sleep(cfg.POLLING_SLEEP)


eventlet.spawn(listen)

if __name__ == '__main__':
    socket.run(app, host='0.0.0.0', port=4200)
