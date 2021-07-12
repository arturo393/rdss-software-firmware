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


def dbConnect():
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


def getActiveDevices():
    logging.debug("Getting active devices...")
    try:
        collection_name = database["devices"]
        devices = list(collection_name.find(
            {"status.provisioned": True}, {"id": 1, "_id": 0}).limit(cfg.MAX_DEVICES))
    except Exception as e:
        logging.exception(e)
    return devices


def updateDeviceStatus(device, status):
    database.devices.update_one(
        {"id": device}, {"$set": {"status.connected": status}})


def insertDeviceData(device, SampleTime, Voltage, Current, Gupl, Gdwl, Power):
    SampleTime = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%d %H:%M:%S')

    if(Voltage > cfg.MAX_VOLTAGE):
        Voltage = cfg.MAX_VOLTAGE

    logging.debug("Data written to DB: {} {}: {}, {}, {}, {}, {}".format(
        device, timeNow, Voltage, Current, Gupl, Gdwl, Power))

    try:
        database.devices.update(
            {"id": device}, {"$addToSet": {"data": {"SampleTime": SampleTime, "Voltage": Voltage, "Current": Current, "Gupl": Gupl, "Gdwl": Gdwl, "Power": Power}}})
    except Exception as e:
        logging.exception(e)


def openSerialPort(port=""):
    global ser
    logging.debug("Open port %s" % port)
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


def sendCmd(ser, cmd, createdevice):
    """
    -Description: This functionsend a cmd, wait one minute if we have data write to the databse, if not write time out
    -param text: ser serial oject, devicecount actuals device in the network, cmd command to send, cursor is the database object
    -return:
    """

    haveData = False
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

        haveData = True
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
        tranformData.append((deviceData[7] / 1.0) - 255)
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
        if(createdevice == True):
            pass
        else:
            SampleTime = datetime.datetime.now()
            insertDeviceData(
                hexResponse[2], SampleTime, tranformData[0], tranformData[3], solutionAgcDwl, solutionAgcUpl, tranformData[7])

        ser.flushInput()
        ser.flushOutput()
        return haveData

    except Exception as e:
        logging.error(e)

    return haveData


def calculateAlerts():
    pass


def sendStatus(rtData):
    # "DEVICES: PROVISIONED: " +str(total)+", ACTIVE:"+str(actived)
    socket.emit('set_name_event', {"event": 'set_name_event', "leave": False,
                                   "handle": 'SET_NAME_EVENT', "name": rtData})


def t1():
    rtData = []

    activeDevicesArr = getActiveDevices()

    SampleTime = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%d %H:%M:%S')
    logging.debug(
        "-------------------------------------------------------------------------------------------")
    logging.debug("Starting Polling - TimeStamp: %s ", timeNow)

    activeDevicesArr = getActiveDevices()
    logging.debug("Devices Count:"+str(len(activeDevicesArr)))

    deviceActiv = 0

    if (len(activeDevicesArr) > 0):
        for x in activeDevicesArr:
            device = int(x["id"])
            logging.debug("ID: %s", device)

            response = sendCmd(ser, device, False)

            if (response):
                deviceActiv += 1
                updateDeviceStatus(device, True)
            else:
                logging.debug("No response from device")
                updateDeviceStatus(device, False)

    else:
        logging.debug("No provisioned devices found in the DB")

    # # --------------------------------------------------
    logging.debug("Connected devices: %s", deviceActiv)
    sendStatus(rtData)


def listen():
    openSerialPort()
    while True:
        if database is None:
            dbConnect()
        t1()
        eventlet.sleep(cfg.POLLING_SLEEP)


eventlet.spawn(listen)

if __name__ == '__main__':
    socket.run(app, host='0.0.0.0', port=4200)
