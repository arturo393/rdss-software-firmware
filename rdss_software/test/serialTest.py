#!/usr/bin/env python3
import serial
import logging
import sys

import sys
import psutil

import serial
import binascii
import struct


logging.basicConfig(filename="serialTest.log", level=logging.DEBUG)


def convert_hex_to_text(hex_response):
    """Converts a hexadecimal string to text, handling potential errors."""

    try:
        # Validate input length for efficiency
        if len(hex_response) % 2 != 0:
            raise ValueError("Hexadecimal string must have even length.")

        # Convert to bytes for efficient decoding
        byte_array = binascii.a2b_hex(hex_response)
        # Decode using UTF-8 for common text encoding
        text = byte_array.decode('utf-8')
        return f"{text}"

    except (ValueError, UnicodeDecodeError) as e:
        print(f"Error converting hex response: {e}")
        return "Error: Invalid hex response"  # Return meaningful error message



def float_to_bytes(f, order='little'):
  """Converts a float to a byte array in the specified byte order.

  Args:
      f: The float value to convert.
      order: The byte order (optional, defaults to 'little'). Can be 'little' or 'big'.

  Returns:
      A byte array representing the float in the specified byte order.

  Raises:
      ValueError: If the byte order is not 'little' or 'big'.
  """

  if order not in ('little', 'big'):
      raise ValueError("Invalid byte order. Must be 'little' or 'big'.")

  # Use struct.pack for efficient conversion
  return struct.pack('f', f)  # '<f' specifies a single-precision (32-bit) float


# Function to check load using psutil
def get_load_value():
    load1, load5, load15 = psutil.getloadavg()  # Get 1-minute, 5-minute, and 15-minute load averages
    return load1  # Or choose the load value you want to return

ser = serial.Serial(
    #port='COM1',\
    port="/dev/ttyS0",\
    baudrate=115200,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
    timeout=0.1)

logging.debug("Connected to port")
#this will store the line
line = []
received = []
read_bytes = bytearray()

while True:
   # logging.debug("Starting read...")
    try:
    #    logging.debug("Starting read...")
    #    logging.debug("Waiting...")
        hexResponse = ser.read(20)
        if len(hexResponse) > 0:
            text = convert_hex_to_text(hexResponse.hex())
            message = "GET: " + hexResponse.hex()
            message += f" --> text: {text}"
            logging.debug(message)
            if text == "load":
                load_value = get_load_value()  # Call function to check load
                logging.debug(f"Load value: {load_value}")  # Log the load value

                # Do something with the load value here, e.g., send it back over serial
                data_retransmited = float_to_bytes(load_value)
                ser.write(data_retransmited)  # Example: send it back over serial
                logging.debug(f"Message retransmitted: {data_retransmited.hex()}")
        ser.flushInput()
        ser.flushOutput()

    except Exception as e:
        logging.error(e)
        sys.exit()