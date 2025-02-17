#!/usr/bin/env python
from dataclasses import field

from pymongo import MongoClient
from pymongo import DESCENDING
import logging
import config as cfg
import serial
import datetime
import sys
from crccheck.crc import Crc16Xmodem
import binascii
import time

import json
import base64
import eventlet
from flask_socketio import SocketIO
from flask import Flask
import struct

import platform

logging.basicConfig(filename=cfg.LOGGING_FILE, level=logging.DEBUG)
logging.getLogger("pymongo").setLevel(logging.INFO)


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
    devices = list()
    try:
        collection_name = database["devices"]
        devices = list(collection_name.find(
            {"status.provisioned": True},
            {"id": 1, "type": 1, "name": 1, "changed": 1, "_id": 0, "fields_values": 1}).limit(cfg.MAX_DEVICES))
        return devices
    except Exception as e:
        logging.exception(e)
            
def getLastRtDataDevice(id):
    try:
        collection_name = database["rtData"]
        # Sort by sampleTime in descending order (newest first)
        # and limit to 1 document
        device = collection_name.find_one(
            {"id": id},
            sort=[("sampleTime",DESCENDING)],
            projection={"field_values": 1}  # Optionally project only "fields_values"
        )
        return device.get("field_values")
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
    return calculate_checksum(data)

def calculate_checksum(data:bytearray):
    """
    -Description: this fuction calculate the checksum for a given comand
    -param text: string with the data, ex device = 03 , id = 03 cmd = 0503110000
    -return: cheksum for the given command
    """

    crc = hex(Crc16Xmodem.calc(data))
    if (len(crc) == 5):
        checksum = crc[3:5] + '0' + crc[2:3]
    elif (len(crc) == 4):
        checksum = crc[2:4].zfill(2) + crc[4:6].zfill(2)
    else:
        checksum = crc[4:6] + crc[2:4]
    return checksum

def get_query_frame_from_fields(fields_group_arr, fields_arr,device, target_field_group_name):
    """Retrieves field group data from the specified arrays."""

    frame = {}
    try:
        for field_group in fields_group_arr:
            if field_group["name"] == target_field_group_name:
                for field in fields_arr:
                    if str(field_group["_id"]) == field["group_id"]:
                        frame[field["name"]] = field.get("default_value",None)
                        device_fields_values = device["fields_values"]
                        id = str(field['_id'])
                        if id in device_fields_values:
                            device_field_values = device_fields_values[id]
                            if 'default_value' in device_field_values:
                                if  device_field_values['default_value'] != '':
                                    frame[field["name"]] =  device_field_values['default_value']
                        
                return frame  # Exit the function as soon as data is found

        logging.warning("Field group not found: %s", target_field_group_name)
        return {}  # Return an empty frame if the field group is not found

    except Exception as e:
        logging.error("Error retrieving field group data: %s", e)
        return {}

def construct_query_status_frame(device_id, frame):
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
        query = (
                frame["start_byte"] + base_frame_data + checksum + frame["end_byte"]
        )
        return query

    except Exception as e:
        logging.error(f"construct_query_status_frame: {e}")
        return {}


def construct_query_frame(device_id, frame):
    """Constructs a sniffer query status frame."""
    devices = {}
    devices['sniffer'] = 0x0A
    devices['vlad'] = 0x09

    start_byte = bytes([START_BYTE])
    end_byte = bytes([END_BYTE])
    device = frame.get("device","") # 0 is broadcast
    device = devices.get(device,0)
    command = int(frame.get("command", "0"), 16) # 0 is no command
    data_size = 1
    try:
        # Attempt to convert data to integer (hexadecimal)
        data = int(frame.get("data", "0"), 16)
    except ValueError:
        # If conversion fails, assume data is a string and create a bytearray
        data = bytearray(frame.get("data", "0"), "utf-8")
        data_size = len(data)
        data.append(0xFF)
    # **Fix:** Convert data back to bytes if it's a bytearray
    try:
        if not frame:
            logging.error("Empty frame packet")
            return {}
                
        base = [
            device,
            device_id,
            command,
            data_size,
        ]
        if isinstance(data, bytearray):
            base = [device,device_id,command,data_size ]
            base.extend(data)  # Append individual bytes
        else:
            base.append(data)
        base_bytearray = bytearray(base)
        checksum = Crc16Xmodem.calc(base_bytearray)
        checksum1_byte = bytes([(checksum >> 8) & 0xFF])  # Most significant byte
        checksum2_byte = bytes([checksum & 0xFF])  # Least significant
        query = bytearray(start_byte+base_bytearray + checksum2_byte + checksum1_byte + end_byte)

        return query

    except Exception as e:
        logging.error(f"construc_query_frame:{e}")
        return {}

def response_validate(hex_response, device_id,response_size, command):
    #response_size = MESSAGE_BASE_SIZE + response_size
    # Validations
    if not hex_response.strip():
        logging.error("Query reception failed: Response empty")
        return False

    if len(hex_response) != response_size:
        logging.error(f"Incorrect response length: {len(hex_response)} and waited {response_size}")
        return False

    if hex_response[0] != START_BYTE or hex_response[-1] != END_BYTE:
        logging.error("Query reception failed: Incorrect start or end byte")
        return False

    if hex_response[ID_INDEX] != device_id:
        logging.error("Query reception failed: Incorrect ID received: {}".format(device_id))
        return False

    if hex_response[COMMAND_INDEX] != command:
        logging.error("Query reception failed: Incorrect command received: {}".format(hex_response[COMMAND_INDEX]))
        return False
    
    return True

def decode_sniffer_io_query(data):
    """Decodes received data from a byte array and returns a dictionary.

  Args:
      data: A byte array containing the received data.

  Returns:
      A dictionary containing the decoded values.
  """
    decoded_data = {
        "Input Voltage [V]": (data[0] | data[1] << 8),  # byte 1-2
        "Output Voltage [V]": (data[2] | data[3] << 8),  # byte 3-4
        "Input Current [mA]": (data[4] | data[5] << 8),  # byte 5-6
        "Output Current [mA]": (data[6] | data[7] << 8),  # byte 7-8
        "Input Current Switch": data[8],  # byte 9 (more succinct)
        "Output Current Switch": data[9],  # byte 10 (more succinct)
        "Digital Input 1": data[10],  # byte 11 (more succinct)
        "Digital Input 2": data[11],  # byte 12 (more succinct)
        "Digital Output 1": data[12],  # byte 13 (more succinct)
        "Digital Output 2": data[13],  # byte 14 (more succinct)
        "Serial Switch": data[14]  # byte 15 (more succinct)
    }
    return decoded_data

def decode_sniffer_io_set(data):
    return "This is case 1"

def decode_sniffer_io_modbus(data,type,multiplier): 
    if type == 'float':
        if len(data) != 4:
            raise ValueError("Byte array must be 4 bytes long for a 32-bit float.")
        # Use struct.unpack to efficiently unpack the byte array
        byte_array = bytes(data)
        data_hex = byte_array.hex()
        logging.info(f"float data_hex: {data_hex}")
        value = struct.unpack('f', byte_array)[0]
        value = round(value,5)
        decoded_data = {"reply":value}
    elif type == 'integer':
        if len(data) == 1:
            data_hex = byte_array.hex()
            logging.info(f"data_hex: {data_hex}")
            value = int(data[0])
        elif len(data) == 2:
            data_hex = byte_array.hex()
            logging.info(f"data_hex: {data_hex}")
            value = int(data[0]<< 8 | data[1])
        elif len(data) == 4:
            data_hex = byte_array.hex()
            logging.info(f"data_hex: {data_hex}")
            value = int(data[0]<< 24 | data[1] << 16 | data[2] << 8 | data[4])
        value = multiplier*value
        decoded_data = {"reply":value}
    else:
        decoded_data = {"reply":0} 
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
    logging.debug(f"device_conv_values: {device_conv_values}'")
    for field in field_group:
        name = field.get("name")
        field_id = str(field.get("_id"))
        if field_id in device_conv_values:
            conv_min = device_conv_values[field_id]["min"]
            conv_max = device_conv_values[field_id]["max"]
            
            if(conv_min == '' or conv_max == ''):
                conv_min = field.get("conv_min","")
                conv_max = field.get("conv_max","")
        else:
            conv_min = field.get("conv_min","")
            conv_max = field.get("conv_max","")
            
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

def delinear_map(decoded_data: dict, field_group: list,device: dict) -> dict:
    """Performs linear mapping on decoded data from a byte array.

    Args:
        decoded_data: A dictionary containing decoded values from a byte array.
        field_group: A list of dictionaries containing field definitions.

    Returns:
        A dictionary containing the decoded values with applied linear mapping.
    """

    demapped_data = {}  # Use a dictionary for mapped values
    device_conv_values = get_conv_values(device)
    logging.debug(f"device_conv_values: {device_conv_values}'")
    for field in field_group:
        name = field.get("name")
        field_id = str(field.get("_id"))
        if field_id in device_conv_values:
            conv_min = device_conv_values[field_id]["min"]
            conv_max = device_conv_values[field_id]["max"]
            
            if(conv_min == '' or conv_max == ''):
                conv_min = field.get("conv_min","")
                conv_max = field.get("conv_max","")
        else:
            conv_min = field.get("conv_min","")
            conv_max = field.get("conv_max","")
            
        try:
            # Attempt to convert conv_min and conv_max to integers first
            if(conv_min == '' or conv_max == ''):
                conv_min = 0
                conv_max = 10
            else:
                conv_min = int(conv_min)
                conv_max = int(conv_max)
            raw = float(decoded_data.get(name, 0))
            if raw < conv_min:
                raw = conv_min
            if raw > conv_max:
                raw = conv_max
            demapped = arduino_map(raw, conv_min, conv_max, 0, 4095)  # Use 0 if missing
            demapped_data[name] = int(demapped)

        except Exception:

            if (isinstance(conv_max, str) or isinstance(conv_min, str))  and  (conv_max != '' or conv_min != '') :
                demapped_data[name] = 1 if decoded_data.get(name) == conv_max else 0
            elif isinstance(decoded_data.get(name),str):
                demapped_data[name] = 0
            else:
                logging.warning(f"Mapping variables not defined for field '{name}'")
                demapped_data[name] = decoded_data.get(name)  # Use original value if missing
    return demapped_data

def extract_relevant_data(hex_response,response_size,data_start_index) -> list:


    extracted_data = []

    for i in range(response_size):
        extracted_data.append(hex_response[i+data_start_index])

    return extracted_data

def get_field_group(
        fields_group_arr: list[dict],
        fields_arr: list[dict],
        field_group_name: str,
        state: str,
) -> list:

    if not any(group.get("name") == field_group_name for group in fields_group_arr):
        logging.error("Missing "+field_group_name+" field group")
        return []

    result = []

    for field_group in fields_group_arr:

        for field in fields_arr:
            if str(field_group["_id"]) == field["group_id"] and field_group["name"] == field_group_name:
                if state in field:
                    result.append(field)

    if not result:
        logging.error(f"No fields found in '{field_group_name}' group with state '{state}'")

    return result

def get_field_visibles(sniffer_data,field_arr,field_group_arr,device) -> dict:
    field_visibles = {}
    if device:
        fields_values = device.get("fields_values", {})
        for field_group in field_group_arr:
            if sniffer_data == field_group.get("name"):
                for field in field_arr:
                    if field.get("group_id") == str(field_group.get("_id")):
                        for field_id, field_data in fields_values.items():
                            key = "visible" 
                            if str(field.get("_id")) == field_id and key in field_data and field_data.get(key) == True:
                                field_visibles[field_id] = {
                                    key: field_data.get(key),
                                }
    return field_visibles

def get_field_names(device: dict = None) -> dict:
    field_names = {}
    if device:
        fields_values = device.get("fields_values", {})
        for field_id, field_data in fields_values.items():
            key = "field_name"
            if key in field_data:
                field_names[field_id] = {
                    key: field_data.get(key),
                    
                }
    return field_names

def get_device_values(device: dict = None) -> dict:
    default_values = {}
    if device:
        fields_values = device.get("fields_values", {})
        for field_id, field_data in fields_values.items():
            key = "value"
            if key in field_data:
                default_values[field_id] = {
                    key: field_data.get(key),
                    
                }
    return default_values

def get_conv_values(device: dict = None) -> dict:
    conv_values = {}
    if device:
        fields_values = device.get("fields_values", {})
        for field_id, field_data in fields_values.items():
            if "conv_min" in field_data and "conv_max" in field_data:                 
                conv_values[field_id] = {
                    "min": field_data.get("conv_min"),
                    "max": field_data.get("conv_max"),
                }
    return conv_values

def get_alert_thresholds(device: dict = None) -> dict:
    alert_thresholds = {}
    if device:
        fields_values = device.get("fields_values", {})
        for field_id, field_data in fields_values.items():
            if "alert_max" in field_data or "alert_min" in field_data:
                alert_thresholds[field_id] = {
                    "min": field_data.get("alert_min"),
                    "max": field_data.get("alert_max"),
                }
    return alert_thresholds

def evaluate_alert(field: dict, value: int, alert_thresholds: dict
) -> dict:

    field_id = {}
    try:
        field_id = str(field["_id"])

        if field_id in alert_thresholds:
            alert_min = alert_thresholds[field_id]["min"]
            alert_max = alert_thresholds[field_id]["max"]
        else:
            alert_min = field.get("alert_min")
            alert_max = field.get("alert_max")
        try:
            # Attempt to convert conv_min and conv_max to integers first
            alert_min = int(alert_min)
            alert_max = int(alert_max)
            alert = value < alert_min or value > alert_max

        except Exception as e:
            logging.warning(f"No alert defined for {field['name']} alert_min:'{alert_min}' alert_max: '{alert_max}'")
            alert = False
    except TypeError as e:
        logging.error(f"Failed to convert field {field['name']} value {value} to integer: {e}")
        alert = False
    except Exception as e:  # Catch other unexpected exceptions
        logging.error(f"Unexpected error processing field data: {e}")
        return {}  # Return an error dictionary for informative handling

    return {field_id: {"value": value, "alert": alert}}

def get_query_status(serTx, serRx, device, fieldsArr, fieldsGroupArr,times,):
    sniffer_data = "sniffer data"
    base_field_group = "sniffer base"
    sniffer_data_visibles = get_field_visibles(sniffer_data,fieldsArr,fieldsGroupArr,device)
    if len(sniffer_data_visibles) == 0:
        return {},0
    frame = get_query_frame_from_fields(fieldsGroupArr, fieldsArr,device, base_field_group)
    if len(frame) == 0:
        return {},0
    if times == 0:
        return {},0
    # Verificar si 'sniffer_IO' está en el diccionario
    if not any(item.get('name') == base_field_group for item in fieldsGroupArr):
        logging.error(f"no {base_field_group} group created")
        return {},0
 
    device_id = int(device['id'])

    command = SNIFFER_IO_QUERY
    data = bytearray(0x00)

    if device['changed'] == True:
        data = create_sniffer_io_set_data_frame(device, fieldsArr, fieldsGroupArr, sniffer_data)
        command = SNIFFER_IO_SET
    query = get_query_frame(data,device_id,command)
    response_size = MESSAGE_BASE_SIZE + int(frame.get("response size", "0"), 16)

    try:
        hexResponse = transmit_and_receive(serTx, serRx, times, query, response_size)
        if response_validate(hexResponse, device_id,response_size, command) == False:
            return get_query_status(serTx, serRx, device, fieldsArr, fieldsGroupArr,times-1)
        
        extracted_data = extract_relevant_data(hexResponse,int(frame.get('response size',"0"), 16),int(frame.get('data start position',"0"), 16))
        decoded_data = decode_sniffer_io_query(extracted_data)
        logging.info(f"Decoded data: {decoded_data}")
        query_field_group = get_field_group(fieldsGroupArr, fieldsArr, sniffer_data, 'query')
        final_data = map_and_evaluate_decoded_data(device, decoded_data, query_field_group)
    except Exception as e:
        logging.error(e)
        sys.exit()

    logging.debug("Query reception succesfull")
    return final_data,command

def get_modbus_status(serTx, serRx, device, fieldsArr, fieldsGroupArr,times):
    base_field_group = "sniffer modbus"
    sniffer_modbus_visibles = get_field_visibles(base_field_group,fieldsArr,fieldsGroupArr,device)
    if len(sniffer_modbus_visibles) == 0:
        return {}
    frame = get_query_frame_from_fields(fieldsGroupArr, fieldsArr,device, base_field_group)
    if len(frame) == 0:
        return {}
    if times == 0:
        return {}
    if not any(item.get('name') == base_field_group for item in fieldsGroupArr):
        logging.error(f"no {base_field_group} group created")
        return {}
    device_id = int(device['id'])
    modbus_data_frame = create_modbus_data_frame(frame)
    query = get_query_frame(modbus_data_frame,device_id,SNIFFER_MODBUS)
    query_response_size = MESSAGE_BASE_SIZE + int(frame.get("response size", "0"), 16)

    try:
        hexResponse = transmit_and_receive(serTx, serRx, times, query, query_response_size)
        if response_validate(hexResponse, device_id,query_response_size, SNIFFER_MODBUS) == False:
            return get_modbus_status(serTx, serRx, device, fieldsArr, fieldsGroupArr,times-1)
        
        extracted_data = extract_relevant_data(hexResponse,int(frame.get('response size',"0"), 16),int(frame.get('data start position',"0"), 16))
        decoded_data = decode_sniffer_io_modbus(extracted_data,frame.get('type',0),frame.get('multiplier',1))
        logging.info(f"Decoded data: {decoded_data}")
        query_field_group = get_field_group(fieldsGroupArr, fieldsArr, base_field_group, 'query')
        final_data = map_and_evaluate_decoded_data(device, decoded_data, query_field_group)
    except Exception as e:
        logging.error(e)
        sys.exit()
    logging.debug("Query reception succesfull")
    return final_data

def map_and_evaluate_decoded_data(device, decoded_data, query_field_group):
    mapped_data = linear_map(decoded_data, query_field_group, device)
    logging.info(f"Mapped data: {mapped_data}")
    alert_thresholds = get_alert_thresholds(device)
    field_names = get_field_names(device)
    final_data = {}
    for query_field in query_field_group:
        try:
            field_value = mapped_data.get(query_field["name"], 0)
        except ValueError:
            logging.warning(f"Failed to convert value for field '{query_field['name']}' to integer, using 0")
            field_value = 0
        field_data = evaluate_alert(query_field, field_value, alert_thresholds)
        id = str(query_field.get("_id"))
        if id in field_names:
            name = field_names[id]['field_name']
            field_data[id]['name'] = name
        elif 'default_value' in query_field:
            field_data[id]['name'] = query_field['default_value']
            
        final_data.update(field_data)
    return final_data

def create_sniffer_io_set_data_frame(device, fieldsArr, fieldsGroupArr, sniffer_data):
    
    device_rtData_values = getLastRtDataDevice(device.get('id'))
    device_values = get_device_values(device)
    for field_id, field_data in device_values.items():
        if "value" in field_data:
            device_rtData_values[field_id]["value"] = field_data["value"]
    set_field_group = get_field_group(fieldsGroupArr, fieldsArr, sniffer_data, 'set')
    device_set_data = get_device_set_data(device_rtData_values, set_field_group)
    logging.warning(f"device_set_data:{device_set_data}")
    demmaped_data =  delinear_map(device_set_data,set_field_group,device)
    data_bytes = encode_sniffer_io_query(demmaped_data)
    data = data_bytes
    return data

def encode_sniffer_io_query(demmaped_data):
    aOut1_0_10V = demmaped_data.get("Output Voltage [V]",0)
    aOut2_x_20mA = demmaped_data.get("Output Current [mA]",0)
    dOut1 = demmaped_data.get("Digital Output 1","Off")
    dOut2 = demmaped_data.get( "Digital Output 2","Off")
    serialSW = demmaped_data.get("Serial Switch",0)
    data_bytes = bytearray(7)
        # Set each byte in the bytearray
    data_bytes[0] = aOut1_0_10V >> 8  # Most significant byte of aOut1
    data_bytes[1] = aOut1_0_10V & 0xFF  # Leasst significant byte of aOut1
    data_bytes[2] = aOut2_x_20mA >> 8  # Most significant byte of aOut2
    data_bytes[3] = aOut2_x_20mA & 0xFF  # Least significant byte of aOut2
    data_bytes[4] = dOut1
    data_bytes[5] = dOut2
    data_bytes[6] = serialSW
    return data_bytes

def get_device_set_data(device_values, set_field_group):
    device_set_data = {}
    for query_field in set_field_group:
        id = str(query_field["_id"])
        if id in device_values:
            name = query_field["name"]
            default_value = device_values.get(id,0)
            field_value = default_value.get('value',0)
            device_set_data[name] = field_value
    return device_set_data
    
def get_query_frame(modbus_data_frame,device_id,command):
    try:   
        base = [SNIFFER,device_id,command,len(modbus_data_frame)]
        if isinstance(modbus_data_frame, bytearray):
            base.extend(modbus_data_frame)  # Append individual bytes
        else:
            base.append(modbus_data_frame)
        base_bytearray = bytearray(base)
        checksum = Crc16Xmodem.calc(base_bytearray)
        checksum1_byte = bytes([(checksum >> 8) & 0xFF])  # Most significant byte
        checksum2_byte = bytes([checksum & 0xFF])  # Least significant
        return bytearray(bytes([START_BYTE])+base_bytearray + checksum2_byte + checksum1_byte + bytes([END_BYTE]))
    except Exception as e:
        logging.error(f"get_query_frame_from_fields:{e}")
        return bytearray()

def create_modbus_data_frame(frame):
    data = bytearray(frame.get("data", "0"), "utf-8")
    data_response_size =  int(frame.get("response size", "0"), 16)
    data.append(data_response_size)
    return data

def transmit_and_receive(serTx, serRx, times, query, response_size):
    message = f"Attempt: {times} "
    message += f"SENT: {query.hex()}"
    startTime = time.time()
    serTx.write(query)
    hexResponse = serRx.read(response_size)
    serTx.flushInput()
    serTx.flushOutput()
    serRx.flushInput()
    serRx.flushOutput()
    responseTime = round(time.time() - startTime, 2)
    message += " --> GET: " + hexResponse.hex()
    message += f" / Response time:{responseTime}"
    logging.debug(message)
    return hexResponse

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
    RESPONSE_LEN = 15
    MASTER = 0
    ID = 0
    QUERY_CMD = '11'
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

        # ---- Read from serials
        hexResponse = serTx.read(RESPONSE_LEN)

        responseTime = str(time.time() - startTime)
        logging.debug("GET: " + hexResponse.hex('-'))
        logging.debug("Response time: " + responseTime)

        # ---- Validations
        if (hexResponse == None or hexResponse == "" or hexResponse == " " or len(hexResponse) == 0):
            logging.error("TxQuery reception failed: " + "Response empty")
            return {}
        if ((len(hexResponse) != RESPONSE_LEN)):
            logging.error("TxQuery reception failed: " + "Incorrect response length: " + str(
                len(hexResponse)) + ", expected " + str(RESPONSE_LEN))
            return {}
        # ------------------------
        serTx.flushInput()
        serTx.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()
        
    DATA_INDEX = 6
    DATA_LENGTH_INDEX = 4
    data_bytes = hexResponse[DATA_LENGTH_INDEX]<<8 | hexResponse[DATA_LENGTH_INDEX+1]
    extracted_data = hexResponse[DATA_INDEX:DATA_INDEX+data_bytes]
    SERVER_TYPE_INDEX = 0
    ADC_0_INDEX = 1
    ADC_1_INDEX = ADC_0_INDEX + 2
    ADC_2_INDEX = ADC_1_INDEX + 1
    server_type = extracted_data[SERVER_TYPE_INDEX]
    adc_0 = extracted_data[1]<<8 + extracted_data[0]
    adc_1 = extracted_data[3]<<8 + extracted_data[2]
    adc_2 = extracted_data[4]
    response = {
        "server_type":server_type,
        "adc_0":adc_0,
        "adc_1":adc_1,
        "adc_2":adc_2
    }

    if (server_type == 2):
        logging.debug("USB0 is TX")
    else:
        logging.debug("USB0 is RX")
    return response

def sendQuery(serTx,cmd):
    """
    Sends query to master at TX port, to determine if its RX or TX master
    Args:
        serTx: Tx serial port object
    Returns:
        2 if connected master is RX, or 3 if connected master is TX
    """
    DATALEN = 1
    RESPONSE_LEN = 13
    MASTER = 0
    ID = 0
    QUERY_CMD = cmd
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

        # ---- Read from serials
        hexResponse = serTx.read(RESPONSE_LEN)

        responseTime = str(time.time() - startTime)
        logging.debug("GET: " + hexResponse.hex('-'))
        logging.debug("Response time: " + responseTime)

        # ---- Validations
        if (hexResponse == None or hexResponse == "" or hexResponse == " " or len(hexResponse) == 0):
            logging.error("TxQuery reception failed: " + "Response empty")
            return {}
        if ((len(hexResponse) != RESPONSE_LEN)):
            logging.error("TxQuery reception failed: " + "Incorrect response length: " + str(
                len(hexResponse)) + ", expected " + str(RESPONSE_LEN))
            return {}
        if ((len(hexResponse) != RESPONSE_LEN)):
            logging.error("TxQuery reception failed: " + "Incorrect response length: " + str(
                len(hexResponse)) + ", expected " + str(RESPONSE_LEN))
            return {}
        # ------------------------
        serTx.flushInput()
        serTx.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()

    try:
        DATA_INDEX = 6
        DATA_LENGTH_INDEX = 4
        data_bytes = hexResponse[DATA_LENGTH_INDEX]<<8 | hexResponse[DATA_LENGTH_INDEX+1]
        extracted_data = hexResponse[DATA_INDEX:DATA_INDEX+data_bytes]
        freq = struct.unpack('f', extracted_data)[0]
    except Exception as e:
            logging.error(e)
            return {}
    return freq

def setMasterPorts():
    """
    Assigns TX and RX port to correponding masters. 
    """
    openSerialPort(USBPORTTX, "tx")
    response = sendTxQuery(serTx)
    # TX master at correct port
    server_type = response.get("server_type",0)
    if server_type == 2:
        openSerialPort(USBPORTRX, "rx")
        logging.debug("Ports opened")
        return
    # TX master at incorrect port
    elif server_type == 3:
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
    master_device_data = dict()

    provisionedDevicesArr = getProvisionedDevices()
    fieldsArr = getFieldsDefinitions()
    fieldsGroupArr = getFieldsGroup()

    connectedDevices = 0
    times = 3
    SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
    timeNow = datetime.datetime.strptime(SampleTime, '%Y-%m-%dT%H:%M:%SZ')
    master_device_data["id"] = 0
    master_device_data["sampleTime"] = SampleTime
    master_device_data["uplink_freq"] = uplink_freq
    master_device_data["downlink_freq"] = downlink_freq
    rtData.append(json.dumps(master_device_data, default=defaultJSONconverter))
    showBanner(provisionedDevicesArr, timeNow)
    times = 3
    if len(provisionedDevicesArr) > 0:
        for device in provisionedDevicesArr:
            device_data = dict()
            logging.debug("-----------------------------------------------------")
            logging.debug(f"Device ID:{device['id']} name:{device.get('name','no name')} START")
            SampleTime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
            response = dict()
            logging.debug(f"Query Status")
            query_status,command = get_query_status(serTx, serRx, device, fieldsArr, fieldsGroupArr, times)
            logging.debug(f"Modbus Status")
            modbus_status = get_modbus_status(serTx, serRx, device, fieldsArr, fieldsGroupArr, times)
            response.update(query_status)
            response.update(modbus_status)
            logging.debug(f"\n\nDevice Response: {response}")
            device_data["id"] = device["id"]
            device_data["name"] = device["name"]
            device_data["sampleTime"] = SampleTime
            device_data["field_values"] = response
            device_data["changed"] = False

            if response:
                connectedDevices += 1
<<<<<<< HEAD
                device_data["connected"] = True
                if command == SNIFFER_IO_SET:
                    database.devices.update_one({"id": device["id"]}, {"$set": {"changed": False}})
=======
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
                logging.debug(deviceData["type"])
>>>>>>> development
            else:
                device_data["connected"] = False
                #database.devices.update_one({"id": device["id"]}, {"$set": {"changed": False}})

            updateDeviceConnectionStatus(device["id"], device_data["connected"])
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
        if serTx is None or serRx is None:
            logging.debug("Opening ports...")
            setMasterPorts()
        else:
            global downlink_freq
            global uplink_freq
            downlink_freq = sendQuery(serTx,'20')
            uplink_freq = sendQuery(serRx,'21')
            run_monitor()
        eventlet.sleep(cfg.POLLING_SLEEP)

eventlet.spawn(listen)

if __name__ == '__main__':
    socket.run(app, host='0.0.0.0', port=4200)
