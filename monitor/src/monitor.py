#!/usr/bin/env python
from dataclasses import field

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
import time

import json
import base64

from flask_socketio import SocketIO
from flask import Flask
import eventlet
import  platform

if platform.system() == "Windows":
    # Windows-specific port names
    USBPORTTX = "COM6"
    USBPORTRX = "COM7"
    USBPORTAUX = "COM4"
else:
    # Linux-specific port names
    USBPORTTX = "/dev/ttyUSB0"
    USBPORTRX = "/dev/ttyUSB1"
    USBPORTAUX = "/dev/ttyUSB3"

# ------

logging.basicConfig(filename=cfg.LOGGING_FILE,
                    level=logging.DEBUG,
                    format='%(asctime)s %(levelname)s %(message)s')


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
serTx = None
serRx = None
f_agc_convert = None


def dbConnect():
    """
    Connects to DB
    """
    global database
    global client
    logging.debug("Conneting to database...")
    try:
        client = MongoClient(
            "mongodb://" + cfg.db["user"] + ":" + cfg.db["passwd"] +
            "@" + cfg.db["host"] + ":" + cfg.db["port"] + "/" + cfg.db["dbname"])
        database = client["rdss"]
    except Exception as e:
        logging.exception(e)


def convert(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min


def getFieldsDefinitions():
    try:
        collection_name = database["fields"]
        fields = list(collection_name.find())
            #            {"$or": [{"set": True}, {"query": True}]},
          #  {},
          #  {"_id": 1, "name": 1, "group_id": 1, "default_value": 1, "query": 1, "set": 1}))
        return fields
    except Exception as e:
        logging.exception(e)


def getFieldsGroup():
    try:
        collection_name = database["fields_group"]
        fields_group = list(collection_name.find())
        return fields_group
    except Exception as e:
        logging.exception(e)


def getProvisionedDevices():
    """
    Gets provisioned devices from DB
    Provisioned devices has status.provisioned attribute setting to True
    """
    try:
        collection_name = database["devices"]
        devices = list(collection_name.find(
            {"status.provisioned": True},
            {"id": 1, "type": 1, "name": 1, "changed": 1, "_id": 0, "fields_values": 1}).limit(cfg.MAX_DEVICES))
        return devices
    except Exception as e:
        logging.exception(e)


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
        logging.debug("---------------------------------")
        logging.debug("Data written to DB: {}".format(d))
        try:
            database.rtData.insert_one(d)

        except Exception as e:
            logging.exception(e)


def openSerialPort(port="", function=""):
    """
    Opens rx or tx serial port to correpsonding USB/Serial port
    Args:
        port: Port to be opened.
        function: Type of port to be opened: "rx" or "tx".
    """

    global serTx, serRx
    logging.debug("Opening serial port " + port + " as " + function)
    try:
        if function == "tx":
            serTx = serial.Serial(
                port=port,
                baudrate=cfg.serial["baudrate"],
                parity=cfg.serial["parity"],
                stopbits=cfg.serial["stopbits"],
                bytesize=cfg.serial["bytesize"],
                timeout=0.5,
                write_timeout=cfg.serial["write_timeout"]
                # inter_byte_timeout=cfg.serial["inter_byte_timeout"]
            )
        elif function == "rx":
            serRx = serial.Serial(
                port=port,
                baudrate=cfg.serial["baudrate"],
                parity=cfg.serial["parity"],
                stopbits=cfg.serial["stopbits"],
                bytesize=cfg.serial["bytesize"],
                timeout=cfg.serial["timeout"],
                write_timeout=cfg.serial["write_timeout"]
                # inter_byte_timeout=cfg.serial["inter_byte_timeout"]
            )
    except serial.SerialException as msg:
        logging.exception("Error opening serial port %s" % msg)
        logging.exception("Trying to open " + port)
        openSerialPort(USBPORTTX)
    except:
        exc_type, error_msg = sys.exc_info()[:2]
        logging.exception("%s  %s" % (error_msg, exc_type))
        openSerialPort(USBPORTRX)


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
    elif (len(crc) == 4):
        checksum = crc[2:4].zfill(2) + crc[4:6].zfill(2)
    else:
        checksum = crc[4:6] + crc[2:4]
    return checksum


def get_field_group_data(fields_group_arr, fields_arr, target_field_group_name):
    """Retrieves field group data from the specified arrays."""

    frame = {}
    try:
        for field_group in fields_group_arr:
            if field_group["name"] == target_field_group_name:
                for field in fields_arr:
                    if str(field_group["_id"]) == field["group_id"]:
                        frame[field["name"]] = field["default_value"]
                return frame  # Exit the function as soon as data is found

        logging.warning("Field group not found: %s", target_field_group_name)
        return {}  # Return an empty frame if the field group is not found

    except Exception as e:
        logging.error("Error retrieving field group data: %s", e)
        return {}


def construct_sniffer_query_status_frame(device_id, frame):
    """Constructs a sniffer query status frame."""

    try:
        if not frame:
            logging.error("Empty frame packet")
            return {}
        base_frame_data = (
                frame["device_function"]
                + f"{device_id:02x}"
                + frame["command"]
                + frame["blank"]
        )
        checksum = getChecksum(base_frame_data)
        sniffer_query_status_frame = (
                frame["start_byte"] + base_frame_data + checksum + frame["end_byte"]
        )
        return sniffer_query_status_frame

    except Exception as e:
        logging.error(e)
        return {}


def response_validate(hex_response, device_id, frame):
    response_size = int(frame['response_size'], 16)
    command = int(frame['command'], 16)
    start_byte = int(frame['start_byte'], 16)
    end_byte = int(frame['end_byte'], 16)
    data_start_index = 6
    data_end_index = 21
    id_index = 2
    command_index = 3

    # Validations
    if not hex_response.strip():
        logging.debug("Query reception failed: Response empty")
        return False

    if len(hex_response) != response_size:
        logging.debug("Query reception failed: Incorrect response length: {}".format(len(hex_response)))
        return False

    if hex_response[0] != start_byte or hex_response[-1] != end_byte:
        logging.debug("Query reception failed: Incorrect start or end byte")
        return False

    if hex_response[id_index] != device_id:
        logging.debug("Query reception failed: Incorrect ID received: {}".format(device_id))
        return False

    if hex_response[command_index] != command:
        logging.debug("Query reception failed: Incorrect command received: {}".format(hex_response[command_index]))
        return False


def decode_data(data):
    """Decodes received data from a byte array and returns a dictionary.

  Args:
      data: A byte array containing the received data.

  Returns:
      A dictionary containing the decoded values.
  """
    decoded_data = {
        "aIn_1_10V": (data[0] | data[1] << 8),  # byte 1-2
        "aOut_1_10V": (data[2] | data[3] << 8),  # byte 3-4
        "aIn_x_20mA": (data[4] | data[5] << 8),  # byte 5-6
        "aOut_x_20mA": (data[6] | data[7] << 8),  # byte 7-8
        "swIn_x_20mA": data[8],  # byte 9 (more succinct)
        "swOut_x_20mA": data[9],  # byte 10 (more succinct)
        "dIn1": data[10],  # byte 11 (more succinct)
        "dIn2": data[11],  # byte 12 (more succinct)
        "dOut1": data[12],  # byte 13 (more succinct)
        "dOut2": data[13],  # byte 14 (more succinct)
        "swSerial": data[14]  # byte 15 (more succinct)
    }
    return decoded_data


def linear_map(decoded_data: dict, field_group: list,device: dict) -> dict:
    """Performs linear mapping on decoded data from a byte array.

    Args:
        decoded_data: A dictionary containing decoded values from a byte array.
        field_group: A list of dictionaries containing field definitions.

    Returns:
        A dictionary containing the decoded values with applied linear mapping.
    """

    mapped_data = {}  # Use a dictionary for mapped values
    device_conv_values = get_conv_values(device)
    logging.warning(f"device_conv_values: {device_conv_values}'")
    for field in field_group:
        name = field.get("name")
        field_id = str(field.get("_id"))
        if field_id in device_conv_values:
            conv_min = device_conv_values[field_id]["min"]
            conv_max = device_conv_values[field_id]["max"]
        else:
            conv_min = field.get("conv_min")
            conv_max = field.get("conv_max")

        try:
            # Attempt to convert conv_min and conv_max to integers first
            conv_min = int(conv_min)
            conv_max = int(conv_max)
            raw = decoded_data.get(name, 0)
            mapped = arduino_map(raw, 0, 4095, conv_min, conv_max)  # Use 0 if missing
            rounded = round(mapped, 2)
            mapped_data[name] = rounded

        except Exception:

            if (isinstance(conv_max, str) or isinstance(conv_min, str))  and  (conv_max != '' or conv_min != '') :
                mapped_data[name] = conv_max if decoded_data.get(name) else conv_min
            else:
                logging.warning(f"Mapping variables not defined for field '{name}'")
                mapped_data[name] = decoded_data.get(name)  # Use original value if missing
    return mapped_data


def linear_map_old(decoded_data: dict, field_group: list) -> dict:
    """Performs linear mapping on decoded data from a byte array.

    Args:
        decoded_data: A dictionary containing decoded values from a byte array.

    Returns:
        A dictionary containing the decoded values with applied linear mapping.
    """
    MAX_2BYTE = 4095
    I_MAX = 20
    V_MAX = 10

    # Linear mapping for analog values
    a_in_1_10v_linear = round(arduino_map(decoded_data["aIn_1_10V"], 0, MAX_2BYTE, 0, V_MAX), 2)
    a_out_1_10v_linear = round(arduino_map(decoded_data["aOut_1_10V"], 0, MAX_2BYTE, 0, V_MAX), 2)

    # Determine current range based on switch states (assuming "ON" means 4-20mA)
    current_range = (0, I_MAX) if decoded_data.get("swIn_x_20mA") == decoded_data.get("swOut_x_20mA") == "ON" else (
        4, I_MAX)

    # Apply linear mapping for current values using the determined range
    a_in_x_20mA_linear = round(arduino_map(decoded_data["aIn_x_20mA"], 0, MAX_2BYTE, *current_range), 2)
    a_out_x_20mA_linear = round(arduino_map(decoded_data["aOut_x_20mA"], 0, MAX_2BYTE, *current_range), 2)

    # Update dictionary with mapped values
    mapped_data = {
        "aIn_1_10V": a_in_1_10v_linear,
        "aOut_1_10V": a_out_1_10v_linear,
        "aIn_x_20mA": a_in_x_20mA_linear,
        "aOut_x_20mA": a_out_x_20mA_linear,
        "swIn_x_20mA": "ON" if decoded_data["swIn_x_20mA"] else "OFF",
        "swOut_x_20mA": "ON" if decoded_data["swOut_x_20mA"] else "OFF",
        "dIn1": "HIGH" if decoded_data["dIn1"] else "LOW",  # Assuming HIGH/LOW logic for inputs
        "dIn2": "HIGH" if decoded_data["dIn2"] else "LOW",
        "dOut1": "HIGH" if decoded_data["dOut1"] else "LOW",
        "dOut2": "HIGH" if decoded_data["dOut2"] else "LOW",
        "swSerial": "RS485" if decoded_data["swSerial"] else "RS232"
    }
    return mapped_data


def extract_relevant_data(frame: dict, hex_response: str) -> list:
    """
    Extracts relevant data from the response based on frame indices.

    Args:
      frame: A dictionary containing frame information.
      hex_response: The hexadecimal string representing the response data.

    Returns:
      A list containing the extracted relevant data.
    """

    response_size = int(frame['response_size'], 16)
    data_start_index = int(frame['data_start_index'], 16)
    data_end_index = int(frame['data_end_index'], 16)
    extracted_data = []

    for i in range(response_size):
        if data_start_index <= i < data_end_index:
            extracted_data.append(hex_response[i])

    return extracted_data


def get_field_group(
        fields_group_arr: list[dict],
        fields_arr: list[dict],
        field_group_name: str,
        state: str,
) -> list:
    """
    Associates data points with relevant field groups and identifies potential alerts.

    Args:
        fields_group_arr: A list of dictionaries representing field groups.
        field_group_name: The name of the specific field group to associate with.
        fields_arr: A list of dictionaries representing individual fields.
        data: A dictionary containing data points for each field.
        device: (Optional) A dictionary containing device information, potentially including "fields_values".

    Returns:
        A dictionary associating field IDs with their corresponding data and alert status.
    """

    if not any(group.get("name") == "status" for group in fields_group_arr):
        logging.error("Missing 'status' field group")
        return []

    result = []

    for field_group in fields_group_arr:

        for field in fields_arr:
            if str(field_group["_id"]) == field["group_id"] and field_group["name"] == field_group_name:
                if field[state]:
                    result.append(field)

    if not result:
        logging.error(f"No fields found in '{field_group_name}' group with state '{state}'")

    return result


def get_conv_values(device: dict = None) -> dict:
    """
    Retrieves alert thresholds (min and max) from the device information.

    Args:
      device: (Optional) A dictionary containing device information, potentially including "fields_values".

    Returns:
      A dictionary containing alert thresholds for each field ID (key)
      with values being another dictionary containing 'min' and 'max' keys.
    """
    conv_values = {}
    if device:
        fields_values = device.get("fields_values", {})
        for field_id, field_data in fields_values.items():
            conv_values[field_id] = {
                "min": field_data.get("conv_min"),
                "max": field_data.get("conv_max"),
            }
    return conv_values


def get_alert_thresholds(device: dict = None) -> dict:
    """
    Retrieves alert thresholds (min and max) from the device information.

    Args:
      device: (Optional) A dictionary containing device information, potentially including "fields_values".

    Returns:
      A dictionary containing alert thresholds for each field ID (key)
      with values being another dictionary containing 'min' and 'max' keys.
    """
    alert_thresholds = {}
    if device:
        fields_values = device.get("fields_values", {})
        for field_id, field_data in fields_values.items():
            alert_thresholds[field_id] = {
                "min": field_data.get("alert_min"),
                "max": field_data.get("alert_max"),
            }
    return alert_thresholds


def process_field_data(
    field: dict, value: int, alert_thresholds: dict
) -> dict:
    """
    Processes a single field, evaluates its data against alert thresholds, and creates a dictionary with its value and alert status.

    Args:
        field: A dictionary representing a field.
        value: The integer value of the field's data (expected to be an integer).
        alert_thresholds: A dictionary containing alert thresholds for each field ID.

    Returns:
        A dictionary containing the field ID (key) and a dictionary with 'value' and 'alert' (True/False) keys.

    Raises:
        TypeError: If the `value` is not an integer.
    """
    field_id = {}
    try:
        field_id = str(field["_id"])

        if field_id in alert_thresholds:
            alert_min = alert_thresholds[field_id]["min"]
            alert_max = alert_thresholds[field_id]["max"]
            alert = value < alert_min or value > alert_max
        else:
            alert_min = field.get("alert_min")
            alert_max = field.get("alert_max")

        alert = value < alert_min or value > alert_max
        try:
            # Attempt to convert conv_min and conv_max to integers first
            alert_min = int(alert_min)
            alert_max = int(alert_max)
            alert = value < alert_min or value > alert_max

        except Exception as e:
            logging.warning(f"Mapping variables not defined for field '{e}'")
            alert = False
    except TypeError as e:
        logging.error(f"Failed to convert field {field['name']} value {value} to integer: {e}")
        alert = False
    except Exception as e:  # Catch other unexpected exceptions
        logging.error(f"Unexpected error processing field data: {e}")
        return {}  # Return an error dictionary for informative handling

    return {field_id: {"value": value, "alert": alert}}


def build_field_associations(
        fields_arr: list[dict],
        data: dict,
        device: dict = None,
) -> dict:
    """
    Associates data points with relevant fields and identifies potential alerts.

    Args:
      fields_arr: A list of dictionaries representing fields.
      data: A dictionary containing data points for each field.
      device: (Optional) A dictionary containing device information, potentially including "fields_values".

    Returns:
      A dictionary associating field IDs with their corresponding data and alert status.
    """
    alert_thresholds = get_alert_thresholds(device)
    final_data = {}
    for field in fields_arr:
        try:
            value = data.get(field["name"], 0)
        except ValueError:
            logging.warning(f"Failed to convert value for field '{field['name']}' to integer, using 0")
            value = 0
        field_data = process_field_data(field, value, alert_thresholds)
        final_data.update(field_data)
    return final_data


def getSnifferStatus(serTx, serRx, device, fieldsArr, fieldsGroupArr):
    """
    Sends request to sniffer to obtain values of analog and digital i/o
    Args:
        serTx: Transmission serial port.
        serRx: Reception serial port.
        id: id number of sniffer that will respond.
    Returns:
        boolean: True if valid segment is received.
    """

    trama = ""



    # Verificar si 'status_query' está en el diccionario
    if not any(item.get('name') == 'status_query' for item in fieldsGroupArr):
        logging.error("no status_query group created")
        return {}

    device_id = int(device['id'])
    frame = get_field_group_data(fieldsGroupArr, fieldsArr, "status_query")

    if len(frame) == 0:
        return {}
    query = construct_sniffer_query_status_frame(device_id, frame)

    message = f"SENT: {query}"
    cmd_bytes = bytearray.fromhex(query)
    startTime = time.time()
    try:
        for cmd_byte in cmd_bytes:
            hex_byte = ("{0:02x}".format(cmd_byte))
            serTx.write(bytes.fromhex(hex_byte))

        response_size = int(frame['response_size'], 16)
        hexResponse = serRx.read(response_size)

        responseTime = round(time.time() - startTime, 2)
        message += " --> GET: " + hexResponse.hex()
        message += f" / Response time:{responseTime}"
        logging.debug(message)

        if response_validate(hexResponse, device_id, frame) is False:
            return False

        extracted_data = extract_relevant_data(frame, hexResponse)

        decoded_data = decode_data(extracted_data)
        logging.info(f"Decoded data: {decoded_data}")

        status_field_group = get_field_group(fieldsGroupArr, fieldsArr, 'status', 'query')

        mapped_data = linear_map(decoded_data, status_field_group, device)
        logging.info(f"Mapped data: {mapped_data}")

        finalData = build_field_associations(status_field_group, mapped_data, device)

        serTx.flushInput()
        serTx.flushOutput()
        serRx.flushInput()
        serRx.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    logging.debug("Query reception succesfull")
    return finalData


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
    master.inputVoltage = round(measurement[Measurements.index('vin')] * ADC_VOLTAGE_FACTOR, 2)
    master.current = round(measurement[Measurements.index('current')] * ADC_CONSUMPTION_CURRENT_FACTOR / 1000, 3)
    master.deviceTemperature = buffer[bufferIndex]
    return master


def isCrcOk(hexResponse, size):
    dataEnd = size - 3
    crcEnd = size - 1
    dataBytes = hexResponse[1:dataEnd]
    checksumBytes = hexResponse[dataEnd:crcEnd]
    checksumString = binascii.hexlify(checksumBytes).decode('utf-8')
    dataString = binascii.hexlify(dataBytes).decode('utf-8')
    calculatedChecksum = getChecksum(dataString)
    crcMessage = "CRC Calculated: " + calculatedChecksum + " CRC Received: " + checksumString + " - D"
    if (calculatedChecksum == checksumString):
        logging.debug(crcMessage + "OK: " + calculatedChecksum + " == " + checksumString)
        return True
    else:
        logging.debug(crcMessage + "ERROR: " + calculatedChecksum + " != " + checksumString)
        return False


def sendMasterQuery(serTx, serRx, times):
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
        cmdString = '05' + '0' + cmd[2:3] + QUERY_MASTER_STATUS + '0000'
    else:
        cmdString = '05' + cmd[2:4] + QUERY_MASTER_STATUS + '0000'

    checksum = getChecksum(cmdString)
    command = '7E' + cmdString + checksum + '7F'
    logging.debug("Attempt: " + str(times))
    logging.debug("SENT: " + command)

    try:
        cmdBytes = bytearray.fromhex(command)
        for cmdByte in cmdBytes:
            hexByte = "{0:02x}".format(cmdByte)
            serTx.write(bytes.fromhex(hexByte))

        hexResponse = serRx.read(14)
        response = hexResponse.hex('-')
        logging.debug("GET: " + hexResponse.hex('-'))
        message = ""
        for byte in hexResponse:
            decimal_value = byte
            message += str(byte).zfill(2) + "-"
        logging.debug("GET: " + message)

        responseLen = len(hexResponse)
        logging.debug("receive len: " + str(responseLen))
        if responseLen != 14:
            sendMasterQuery(serTx, serRx, times - 1)
            return False
        if hexResponse[0] != 126:
            sendMasterQuery(serTx, serRx, times - 1)
            return False
        if hexResponse[responseLen - 1] != 127:
            sendMasterQuery(serTx, serRx, times - 1)
            return False
        if hexResponse[2] != int(cmd, 16):
            sendMasterQuery(serTx, serRx, times - 1)
            return False
        if hexResponse[3] != 19:
            sendMasterQuery(serTx, serRx, times - 1)
            return False
        if hexResponse[4] in [2, 3, 4]:
            sendMasterQuery(serTx, serRx, times - 1)
            return False
        if isCrcOk(hexResponse, responseLen) == False:
            sendMasterQuery(serTx, serRx, times - 1)
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

        serTx.flushInput()
        serTx.flushOutput()
        serRx.flushInput()
        serRx.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    return finalData


def sendStatusToFrontEnd(rtData):
    """
    Sends via SocketIO the real-time provisioned devices status
    This updates frontend interface
    """
    logging.debug("-----------------")
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
    logging.debug("Devices Count:" + str(len(provisionedDevicesArr)) + "\n")


def sendModbus(uartCmd, snifferAddress, data, serTx, serRx):
    """
    Sends variable length segment.
    Args:
        uartcmd: Command to send.
        sniffer_address: Address of target sniffer.
        data: Data to be added to modBus segment.
        serTx: Transmission serial port.
        serRx: Reception serial port.
    Returns:
        boolean: true if answer segment from sniffer is valid.
    """
    SNIFFER = "0A"
    SEGMENT_START = '7E'
    SEGMENT_END = '7F'
    ID_INDEX = 2
    COMMAND_INDEX = 3
    DATA_START_INDEX = 6

    # ---- Build segment
    intLen = int(len(data) / 2)
    dataLen = format(intLen, '04x')
    formatLen = dataLen[2:] + dataLen[0:2]
    cmdString = f"{SNIFFER}{snifferAddress}{uartCmd}{formatLen}{data}"
    checksum = getChecksum(cmdString)
    command = SEGMENT_START + cmdString + checksum + SEGMENT_END
    cmdLen = int(len(command) / 2)
    logging.debug("SENT: " + command)

    cmd_bytes = bytearray.fromhex(command)
    hex_byte = ''

    startTime = time.time()

    try:
        # ---- Send via serial
        for cmd_byte in cmd_bytes:
            hex_byte = ('{0:02x}'.format(cmd_byte))
            serTx.write(bytes.fromhex(hex_byte))

        # ---- Read serial
        hexResponse = serRx.read(cmdLen)

        # ---DEBUG---
        if len(hexResponse) == 0:
            logging.debug("Reading from Tx USB")
            auxResponse = serTx.read_until()
            logging.debug("Data read: " + auxResponse.hex('-'))
            if (auxResponse == 3):
                logging.debug("NOT_VALID_FRAME")
            if (auxResponse == 4):
                logging.debug("WRONG_MODULE_FUNCTION")
            if (auxResponse == 6):
                logging.debug("CRC_ERROR")

        logging.debug("GET: " + hexResponse.hex('-'))

        responseTime = str(time.time() - startTime)

        logging.debug("Response time: " + responseTime)

        # ---- Validations
        responseLen = len(hexResponse)

        if (hexResponse == None or hexResponse == "" or hexResponse == " " or hexResponse == b'' or responseLen == 0):
            logging.debug("Modbus reception failed: Response empty")
            return False
        if (hexResponse[0] != int(SEGMENT_START, 16) or hexResponse[responseLen - 1] != int(SEGMENT_END, 16)):
            logging.debug("Modbus reception failed: Incorrect start or end byte")
            return False
        if (hexResponse[ID_INDEX] != int(snifferAddress, 16)):
            logging.debug("Modbus reception failed: Incorrect ID: " + str(hexResponse[ID_INDEX]))
            return False
        if (hexResponse[COMMAND_INDEX] != int(uartCmd, 16)):
            logging.debug("Modbus reception failed: Incorrect command: " + str(hexResponse[COMMAND_INDEX]))
            return False

        # ---- Extract data / remove headers
        dataReceived = []

        for i in range(0, responseLen):
            if (DATA_START_INDEX <= i < DATA_START_INDEX + intLen):
                dataReceived.append(hexResponse[i])

        serTx.flushInput()
        serTx.flushOutput()
        serRx.flushInput()
        serRx.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    logging.debug("Modbus reception succesful")
    return True


def setSnifferData(serTx, serRx, id, data):
    """Sets device downlink attenuation

    Args:
        serTx: serial port for transmission
        serRx: serial port for reception
        id: device ID
        data: values to set formatted as f"{aout1:04X}{aout2:04X}{dOut1:02X}{dOut2:02X}{serialSW:02X}"

    Returns:
        boolean: if changed was applied or error
    """
    DATALEN = 7
    SNIFFER = 10
    SEGMENT_START = '7E'
    SEGMENT_END = '7F'
    RESPONSE_LEN = 16

    # ---- Build segment
    data_len = f"{DATALEN:02x}{0:02x}"
    id = f"{id:02x}"
    set_out_cmd = "B6"
    device = f"{SNIFFER:02x}"
    cmd_string = f"{device}{id}{set_out_cmd}{data_len}{data}"
    checksum = getChecksum(cmd_string)
    command = SEGMENT_START + cmd_string + checksum + SEGMENT_END

    logging.debug("data: " + str(data))
    logging.debug("SENT: " + command)

    cmd_bytes = bytearray.fromhex(command)
    hex_byte = ''

    startTime = time.time()
    try:
        # ---- Send via serial
        for cmd_byte in cmd_bytes:
            hex_byte = ("{0:02x}".format(cmd_byte))
            serTx.write(bytes.fromhex(hex_byte))

        # ---- Read from serial
        hexResponse = serRx.read(RESPONSE_LEN)

        responseTime = str(time.time() - startTime)
        logging.debug("GET: " + hexResponse.hex('-'))
        logging.debug("Response time: " + responseTime)

        # ---- Validations (response == query)
        if (hexResponse == None or hexResponse == "" or hexResponse == " " or len(hexResponse) == 0):
            logging.debug("Set reception failed: " + "Response empty")
            return False
        if ((len(hexResponse) > RESPONSE_LEN) or (len(hexResponse) < RESPONSE_LEN)):
            logging.debug(
                "Set reception failed: " + "Incorrect response length: " + str(len(hexResponse)) + ", expected " + str(
                    RESPONSE_LEN))
            return False
        if (hexResponse != cmd_bytes):
            logging.debug("Set reception failed: " + "Unexpected response: " + hexResponse.hex('-'))
            return False
        # ------------------------

        serTx.flushInput()
        serTx.flushOutput()
        serRx.flushInput()
        serRx.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    logging.debug("changing attenuation")
    return True


def getRealValues(deviceData):
    """
    Obtains real values from data extracted from database
    Args:
        deviceData: json with data to analyze
    Returns:
        out: string with converted values: aout1+aout2+dout1+dout2+swSerial
    """
    # el argumento recibido es toda la data del sniffer (deviceData del runmonitor)
    # asumiendo que el orden de las variables siempre es aout1, aout2, dout1, dout2, swSerial
    out = ""
    for var in deviceData:
        varType = var["tipo"]
        if (varType == "aout"):
            value = var["value"]
            minAnalog = var["minimumAnalog"]
            maxAnalog = var["maximumAnalog"]
            minConverted = var["minimumconverted"]
            maxConverted = var["maximumconverted"]
            if value > maxConverted:
                value = maxConverted
            if value < minConverted:
                value = minConverted
            mappedValue = round(arduino_map(value, minAnalog, maxAnalog, minConverted,
                                            maxConverted))  # solo se pueden programar numeros enteros
            mappedValue = ((mappedValue >> 8) & 0xFF) | ((mappedValue << 8) & 0xFF00)
            out = out + f"{mappedValue:02x}"

        elif (varType == "dout"):
            value = var["valor"]
            on = var["stateon"]
            off = var["stateoff"]
            if type(value) is not bool:
                value = off
            if value == on:
                out = out + f"{1:02x}"
            else:
                out = out + f"{0:02x}"
    return out


def sendTxQuery(serTx):
    """
    Sends query to master at TX port, to determine if its RX or TX master
    Args:
        serTx: Tx serial port object
    Returns:
        2 if connected master is RX, or 3 if connected master is TX
    """
    DATALEN = 1
    RESPONSE_LEN = 7
    MASTER = 0
    ID = 0
    QUERY_CMD = '16'
    SEGMENT_START = '7E'
    SEGMENT_END = '7F'

    # ---- Build segment
    dataLenStr = f"{DATALEN:02x}{0:02x}"
    data = "00"
    cmd_string = f"{MASTER:02x}{ID:02x}{QUERY_CMD}{dataLenStr}{data}"
    checksum = getChecksum(cmd_string)
    command = SEGMENT_START + cmd_string + checksum + SEGMENT_END

    logging.debug("SENT: " + command)
    cmd_bytes = bytearray.fromhex(command)
    hex_byte = ''

    startTime = time.time()
    try:
        # ---- Send via serial
        for cmd_byte in cmd_bytes:
            hex_byte = ("{0:02x}".format(cmd_byte))
            serTx.write(bytes.fromhex(hex_byte))

        # ---- Read from serial
        hexResponse = serTx.read(RESPONSE_LEN)

        responseTime = str(time.time() - startTime)
        logging.debug("GET: " + hexResponse.hex('-'))
        logging.debug("Response time: " + responseTime)

        # ---- Validations
        if (hexResponse == None or hexResponse == "" or hexResponse == " " or len(hexResponse) == 0):
            logging.debug("TxQuery reception failed: " + "Response empty")
            return False
        if ((len(hexResponse) > RESPONSE_LEN) or (len(hexResponse) < RESPONSE_LEN)):
            logging.debug("TxQuery reception failed: " + "Incorrect response length: " + str(
                len(hexResponse)) + ", expected " + str(RESPONSE_LEN))
            return False
        # ------------------------

        serTx.flushInput()
        serTx.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    if (str(hexResponse[3]) == "2"):
        logging.debug("USB0 is TX")
    else:
        logging.debug("USB0 is RX")
    return str(hexResponse[3])


def setMasterPorts():
    """
    Assigns TX and RX port to correponding masters. 
    """
    openSerialPort(USBPORTTX, "tx")
    status = sendTxQuery(serTx)
    # TX master at correct port
    if status == "2":
        openSerialPort(USBPORTRX, "rx")
        logging.debug("Ports opened")
        return
    # TX master at incorrect port
    elif status == "3":
        serTx.close()
        openSerialPort(USBPORTRX, "tx")
        openSerialPort(USBPORTTX, "rx")
        logging.debug("Ports opened")
        return
    else:
        logging.debug("Unrecognized lora mode")


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
    fieldsArr = getFieldsDefinitions()
    fieldsGroupArr = getFieldsGroup()

    connectedDevices = 0
    times = 3
    SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
    timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')
    showBanner(provisionedDevicesArr, timeNow)
    if len(provisionedDevicesArr) > 0:
        for device in provisionedDevicesArr:
            device_data = dict()
            logging.debug("-----------------------------------------------------")
            logging.debug(f"Device ID:{device['id']} name:{device['name']} START")
            SNIFFERID = 8
            SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
            response = getSnifferStatus(serTx, serRx, device, fieldsArr, fieldsGroupArr)
            logging.debug(f"Device Response: {response}")
            if response:
                connectedDevices += 1
                device_data["id"] = device["id"]
                device_data["name"] = device["name"]
                device_data["connected"] = True
                device_data["sampleTime"] = SampleTime
                device_data["field_values"] = response
                updateDeviceConnectionStatus(device["id"], True)
                data = packet_sniffer_output()
                setSnifferData(serTx, serRx, int(device["id"]), data)
                # data, uart_cmd = get_example_variable_frame(data)
                # sendModbus(uart_cmd, f"{SNIFFERID:02x}", data, serTx, serRx)
            else:
                logging.debug("No response from device")
                device_data["id"] = device["id"]
                device_data["name"] = device["name"]
                device_data["connected"] = False
                device_data["sampleTime"] = datetime.datetime.now().replace(microsecond=0)
                device_data["field_values"] = {}
                updateDeviceConnectionStatus(int(device["id"]), False)

            rtData.append(json.dumps(device_data, default=defaultJSONconverter))
            logging.debug(f"Device ID:{device['id']} name:{device['name']} END\n\n")
        logging.debug("Connected devices: %s", connectedDevices)
        insertDevicesDataIntoDB(rtData)
        logging.debug("Inserted")

        sendStatusToFrontEnd(rtData)
    else:
        # sendStatusToFrontEnd([])
        logging.debug("No provisioned devices found in the DB")


def get_example_variable_frame(data):
    uart_cmd = "14"  # comando para que el sniffer envie el paquete via serial
    sniffer_add = "08"
    data = ""
    MAXDATA = 255
    contador = 20
    i = 1
    if contador * 5 < MAXDATA - 10 - 1 - 5:
        while i <= contador * 5:
            if i == 127:
                data = f"{data}{0:02x}"
            else:
                data = f"{data}{i:02X}"
            i += 1
        data = data + "FF"
    else:
        contador = 0
    return data, uart_cmd


def packet_sniffer_output():
    aOut1_0_10V = 1000
    aOut2_x_20mA = 500
    dOut1 = 0
    dOut2 = 0
    # serialSW = 0 #seteaer a rs232
    serialSW = 1  # setear a rs485
    # Invertir los bytes de aout1
    aout1 = ((aOut1_0_10V >> 8) & 0xFF) | ((aOut1_0_10V << 8) & 0xFF00)
    # Invertir los bytes de aout2
    aout2 = ((aOut2_x_20mA >> 8) & 0xFF) | ((aOut2_x_20mA << 8) & 0xFF00)
    data = f"{aout1:04X}{aout2:04X}{dOut1:02X}{dOut2:02X}{serialSW:02X}"
    return data


def listen():
    """
    Listens frontend connection to emit socketio events
    """
    while True:
        if database is None:
            dbConnect()
        if serTx is None:
            try:
                logging.debug("Opening ports...")
                setMasterPorts()
            finally:
                openSerialPort(USBPORTAUX)

        run_monitor()
        eventlet.sleep(cfg.POLLING_SLEEP)


eventlet.spawn(listen)

if __name__ == '__main__':
    socket.run(app, host='0.0.0.0', port=4200)
