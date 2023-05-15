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

import json
import base64

from flask_socketio import SocketIO
from flask import Flask
import eventlet


logging.basicConfig(filename=cfg.LOGGING_FILE, level=logging.DEBUG)


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
    Provisioned devices has status.provisioned attribute setting to true
    """
    try:
        collection_name = database["devices"]
        devices = list(collection_name.find(
            {"status.provisioned": True}, {"id": 1, "type": 1, "name": 1, "_id": 0}).limit(cfg.MAX_DEVICES))
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
            isRemoteAttenuation = bool(data[1]) 
            lineVoltage = ( data[2] | (data[3] << 8) ) /10.0
            lineCurrent = ( data[4] | (data[5] << 8) ) /1000.0
            remoteAttenuation = data[6]
            rotarySwitchAttenuation = data[7]
            downlinkAgcValue = data[8] / 10.0
            downlinkOutputPower =  data[9] if  data[9] < 128 else data[9] - 256
            uplinkAgcValue = data[10] / 10.0
            uplinkOutputPower = data[11]if  data[11] < 128 else data[11] - 256
            # print the decoded values

            logging.debug(f"Voltage: {lineVoltage:.2f}[V]")
            logging.debug(f"Line Current: {lineCurrent:.3f}[A]")
            logging.debug(f"Software Attenuation: {remoteAttenuation:.1f}[dB]")
            logging.debug(f"Rotary Switch Attenuation: {rotarySwitchAttenuation:.1f}[dB]")
            logging.debug(f"Is Software Attenuation : {isRemoteAttenuation}")
            logging.debug(f"AGC Uplink: {uplinkAgcValue:.1f}[dB]")
            logging.debug(f"Downlink Output Power: {downlinkOutputPower}[dBm]")
            logging.debug(f"AGC Downlink: {downlinkAgcValue:.1f}[dB]")
            logging.debug(f"Uplink Output Power:    {uplinkOutputPower}[dBm]")
            
            SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
            timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')
            finalData = {
                "sampleTime": timeNow,
                "voltage": lineVoltage,
                "current": lineCurrent,
                "gupl": uplinkAgcValue,
                "gdwl": downlinkAgcValue,
                "power": downlinkOutputPower
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
    SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
    timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')
    showBanner(provisionedDevicesArr, timeNow)

    if (len(provisionedDevicesArr) > 0):
        for x in provisionedDevicesArr:
            device = int(x["id"])
            deviceData = {}
            deviceData["rtData"] = {}
            logging.debug("ID: %s", device)

            response = sendCmd(ser, device, False)

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

            if (response):
                connectedDevices += 1
                deviceData["connected"] = True
                deviceData["rtData"] = response
                alerts = evaluateAlerts(response)
                deviceData["rtData"]["alerts"] = alerts
                deviceData["alerts"] = alerts
                updateDeviceConnectionStatus(device, True)
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
