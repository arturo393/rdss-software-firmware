#!/usr/bin/env python

from pymongo import MongoClient
import logging
import config as cfg
import serial
import datetime
import sys
from crccheck.crc import Crc16Xmodem
from struct import *
from sympy import *

import json

from flask_socketio import SocketIO
from flask import Flask
import eventlet


logging.basicConfig(filename=cfg.LOGGING_FILE, level=logging.DEBUG)


# eventlet.monkey_patch()


app = Flask(__name__)

socket = SocketIO(app, cors_allowed_origins='*')

database = None
client = None
ser = None


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


def getProvisionedDevices():
    """
    Gets provisioned devices from DB
    Provisioned devices has status.provisioned attribute setting to true
    """
    try:
        collection_name = database["devices"]
        devices = list(collection_name.find(
            {"status.provisioned": True}, {"id": 1, "type": 1, "_id": 0}).limit(cfg.MAX_DEVICES))
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
            if(d["rtData"]["voltage"] > cfg.MAX_VOLTAGE):
                d["rtData"]["voltage"] = cfg.MAX_VOLTAGE
            try:
                if "alerts" in d:
                    database.devices.update_one(
                        {"id": d["id"]}, {"$set": {"alerts": d["alerts"]}})
                database.devices.update(
                    {"id": d["id"]}, {"$addToSet": {"rtData": d["rtData"]}})
            except Exception as e:
                logging.exception(e)


def openSerialPort(port=""):
    global ser
    logging.debug("Open Serial port %s" % port)
    try:
        ser = serial.Serial(
            port=cfg.serial["port"],
            baudrate=cfg.serial["baudrate"],
            parity=cfg.serial["parity"],
            stopbits=cfg.serial["stopbits"],
            bytesize=cfg.serial["bytesize"],
            timeout=cfg.serial["timeout"],
            write_timeout=cfg.serial["write_timeout"]
        )
    except serial.SerialException as msg:
        logging.exception("Error opening serial port %s" % msg)
    except:
        exctype, errorMsg = sys.exc_info()[:2]
        logging.exception("%s  %s" % (errorMsg, exctype))


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
        hexResponse = ser.readline()

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

        tranformData = list()

        logging.debug("Data read from Serial: {}".format(deviceData))

        tranformData.append(((deviceData[0] / 10.0)))
        tranformData.append(deviceData[1] / 1000.0)
        tranformData.append(deviceData[2] / 1000.0)
        tranformData.append(deviceData[3] / 1000.0)
        tranformData.append(deviceData[4] / 10.0)
        tranformData.append(deviceData[5] / 1.0)
        tranformData.append(deviceData[6] / 10.0)

        if (deviceData[7] >= cfg.RANGE_MIN_PTX and deviceData[7] <= cfg.RANGE_MAX_PTX):
            tranformData.append((deviceData[7] / 1.0) - 255)
        else:
            tranformData.append((deviceData[7] / 1.0))

        # -----------------------------------------------------
        tranformData[0] = round(
            (tranformData[0] * 1.0075) + 0.108, 2)
        tranformData[3] = round(
            (tranformData[3] * 0.99535) - 0.05815, 2)
        # -----------------------------------------------------

        x, y = symbols('x y')
        if (tranformData[4] >= 3.8):
            solutionAgcUpl = 0
        else:
            solutionAGCUPL = solveset(
                Eq(-0.8728*x**6 + 14.702*x**5 - 99.306*x**4 + 341.63*x**3 - 624.11*x**2 + 561.72*x - 180.8, tranformData[4]), x)
            solutionAgcUpl = str(solutionAGCUPL.args[1])
            solutionAgcUpl = float(solutionAgcUpl[:6])

        if(tranformData[6] >= 3.8):
            solutionAgcDwl = 0
        else:
            solutionAGCDWL = solveset(
                Eq(-0.8728*x**6 + 14.702*x**5 - 99.306*x**4 + 341.63*x**3 - 624.11*x**2 + 561.72*x - 180.8, tranformData[4]), x)
            solutionAgcDwl = str(solutionAGCDWL.args[1])
            solutionAgcDwl = float(solutionAgcDwl[:6])

        # -----------------------------------------------------
        # if(createdevice == True):
        #     pass
        # else:
        #     SampleTime = datetime.datetime.now()

        ser.flushInput()
        ser.flushOutput()

        SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
        timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')

        finalData = {
            "sampleTime": timeNow,
            "voltage": tranformData[0],
            "current": tranformData[3],
            "gupl": solutionAgcUpl,
            "gdwl": solutionAgcDwl,
            "power": tranformData[7]
        }

        return(finalData)

    except Exception as e:
        logging.error(e)

    return finalData


def sendStatusToFrontEnd(rtData):
    """
    Sends via SocketIO the real-time provisioned devices status
    This updates frontend interface
    """
    logging.debug("Emiting event...")
    data = {
        "event": 'set_rtdata_event',
        "leave": False,
        "handle": 'SET_MONITOR_DATA_EVENT',
        "data": rtData
    }
    socket.emit('set_rtdata_event', data)


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
            logging.debug("ID: %s", device)

            response = sendCmd(ser, device, False)
            deviceData["id"] = device
            deviceData["type"] = x["type"]

            if (response):
                connectedDevices += 1
                deviceData["connected"] = True
                deviceData["rtData"] = response
                deviceData["alerts"] = evaluateAlerts(response)

                updateDeviceConnectionStatus(device, True)
            else:
                logging.debug("No response from device")
                deviceData["connected"] = False
                updateDeviceConnectionStatus(device, False)

            rtData.append(json.dumps(deviceData, default=defaultJSONconverter))
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
            openSerialPort()

        run_monitor()
        eventlet.sleep(cfg.POLLING_SLEEP)


eventlet.spawn(listen)

if __name__ == '__main__':
    socket.run(app, host='0.0.0.0', port=4200)
