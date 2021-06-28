#!/usr/bin/env python3
# Project Sigma-telecom

import datetime
import os
# from serial import Serial
import time
import sys
import pyodbc
from crccheck.crc import Crc16Xmodem
import binascii
import serial
import random
from struct import *
import math
from sympy import *
import logging

import config as cfg

# --- GLOBALS ----------------------------------------------------------------
logging.basicConfig(filename=cfg.LOGGING_FILE, level=logging.DEBUG)
global ser, cursor, activeVlads
# --- END GLOBAL VARS ----------------------------------------------------------------


def openSerialPort(port=""):
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

    return ser


def writeToDB(cursor, Vlad, SampleTime, Voltage, Current, Gupl, Gdwl, Power):
    """
    -Description: This function take the input parameters and pass the values to the store procedure
    -parameters:
        -dbConnection/cursor: database interface.
        -Vlad: Id of the current Vlad.
        -Voltage: actual voltage after a responce from the DAQ.
        -Current: actual current after a responce from the DAQ.
        -Gupl: actual Gupl after a responce from the DAQ.
        -Gdwl: actual Gdwl after a responce from the DAQ.
        -Power: actual Power after a responce from the DAQ.
    -return: a boolean expression if the store procedure is executed succesfully
    """
    iswritten = False

    try:
        SampleTime = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%d %H:%M:%S')

        if(Voltage > cfg.MAX_VOLTAGE):
            Voltage = cfg.MAX_VOLTAGE

        cursor.execute('exec SP_writeVladData ?,?,?,?,?,?,?',
                       Vlad, timeNow, Voltage, Current, Gupl, Gdwl, Power)

        iswritten = True
        logging.debug("Data written to DB: {} {}: {}, {}, {}, {}, {}".format(
            Vlad, SampleTime, Voltage, Current, Gupl, Gdwl, Power))

    except Exception as e:
        logging.exception(e)
        iswritten = False
    return iswritten


def getChecksum(cmd):
    """
    -Description: this fuction calculate the checksum for a given comand
    -param text: string with the data, ex VLAD = 03 , id = 03 cmd = 0503110000
    -return: cheksum for the given command
    """
    data = bytearray.fromhex(cmd)

    crc = hex(Crc16Xmodem.calc(data))

    if (len(crc) == 5):
        checksum = crc[3:5] + '0' + crc[2:3]
    else:
        checksum = crc[4:6] + crc[2:4]
    return checksum


def sendCmd(ser, cmd, cursor, createVlad):
    """
    -Description: This functionsend a cmd, wait one minute if we have data write to the databse, if not write time out
    -param text: ser serial oject, VLADcount actuals VLAD in the network, cmd command to send, cursor is the database object
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
            (hexResponse[4] == 2 or hexResponse[4] == 3 or hexResponse[4] == 4)
        )):
            return False
        # ------------------------

        haveData = True
        data = list()
        for i in range(0, 21):
            if(6 <= i < 18):
                data.append(hexResponse[i])

        vladData = list()

        for i in range(0, len(data)):

            if (i == 0 or i == 2 or i == 4 or i == 6):
                if(data[i+1] == 255):
                    vladData.append(data[i] * -1)
                else:
                    vladData.append(int.from_bytes(
                        ((data[i+1]).to_bytes(1, "little")) + ((data[i]).to_bytes(1, "little")), "big"))

            elif(i == 8 or i == 9 or i == 10 or i == 11):
                vladData.append(data[i])
            else:
                pass

        tranformData = list()

        logging.debug("Data read from Serial: {}".format(vladData))

        tranformData.append(((vladData[0] / 10.0)))
        tranformData.append(vladData[1] / 1000.0)
        tranformData.append(vladData[2] / 1000.0)
        tranformData.append(vladData[3] / 1000.0)
        tranformData.append(vladData[4] / 10.0)
        tranformData.append(vladData[5] / 1.0)
        tranformData.append(vladData[6] / 10.0)
        tranformData.append((vladData[7] / 1.0) - 255)
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
        if(createVlad == True):
            pass
        else:
            SampleTime = datetime.datetime.now()
            DbIsWrite = writeToDB(
                cursor, hexResponse[2], SampleTime, tranformData[0], tranformData[3], solutionAgcDwl, solutionAgcUpl, tranformData[7])

            if (DbIsWrite == False):
                logging.debug("cannot write to DDBB")
                return False
            else:
                logging.debug("Saved to DDBB")
                pass
        ser.flushInput()
        ser.flushOutput()
        return haveData

    except Exception as e:
        logging.error(e)

    return haveData


def getActiveVlads(cursor):
    vladSearch = cursor.execute(
        'SELECT Id FROM dbo.Monitor WHERE Activo=1')
    lista = vladSearch.fetchall()
    return lista


def t1():
    cnxn = pyodbc.connect('DRIVER={ODBC Driver 17 for SQL Server};SERVER=' +
                          cfg.mssql["host"]+';DATABASE='+cfg.mssql["db"]+';UID='+cfg.mssql["user"]+';PWD=' + cfg.mssql["passwd"], autocommit=True)
    cursor = cnxn.cursor()  # get the database object
    ser = openSerialPort(cfg.serial["port"])

    while True:
        activeVlads = 0
        SampleTime = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%d %H:%M:%S')
        logging.debug(
            "-------------------------------------------------------------------------------------------")
        logging.debug("Starting Polling - TimeStamp: %s ", timeNow)

        activeVladsArr = getActiveVlads(cursor)
        logging.debug("VLAD Count:"+str(len(activeVladsArr)))

        vladActiv = 0

        if (len(activeVladsArr) > 0):
            for x in activeVladsArr:
                logging.debug("ID: %s", x[0])
                vlad = int(x[0])

                response = sendCmd(ser, vlad, cursor, False)

                if (response):
                    vladActiv += 1
                    cursor.execute(
                        'UPDATE dbo.Detected SET Status = ? WHERE id ='+str(vlad), 1)
                else:
                    logging.debug("No response from vlad")
                    cursor.execute(
                        'UPDATE dbo.Detected SET Status = ? WHERE id ='+str(vlad), 0)
        else:
            logging.debug("No active VLADs found in the DB:Monitor table")

        # --------------------------------------------------
        logging.debug("Connected VLADs: %s", vladActiv)
        cursor.execute(
            'UPDATE dbo.Testing2 SET network = ? ,  detected= ? WHERE Id = 1', len(activeVladsArr), vladActiv)
        # logging.debug("Actual Vlad Count is : %s ", len(activeVladsArr))
        cursor.execute(
            'UPDATE dbo.Testing2 SET timealert= ? WHERE Id = 1', timeNow)

        time.sleep(cfg.POLLING_SLEEP)


# ---- MAIN ----------------------------------------------------------------
try:
    t1()
except Exception as e:
    logging.exception(e)
    t1()
# ---------------------------------------------------------------------------
except (KeyboardInterrupt, SystemExit):
    sys.exit(2)
except Exception as e:
    logging.exception(e)
