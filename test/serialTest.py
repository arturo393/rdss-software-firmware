import serial
import logging
import sys

logging.basicConfig(filename="serialTest.log", level=logging.DEBUG)

ser = serial.Serial(
    #port='COM1',\
    port="/dev/ttyS0",\
    baudrate=115200,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
    timeout=5)

logging.debug("Connected to port")
#this will store the line
line = []
received = []
read_bytes = bytearray()

while True:
    #logging.debug("Starting read...")
    try:
        #logging.debug("Waiting...")
        for c in ser.read():
            line.append(c)
            read_bytes.append(c)
            if c == 255:
                #logging.debug("Line: " + ''.join(line))
                ser.write(read_bytes)
                ser.flush()
                logging.debug("Message retransmitted:")
                logging.debug(f"{read_bytes}")
                logging.debug("Message length: " + str(len(line)))
                logging.debug("-----")
                line = []
                read_bytes = bytearray()
                ser.flushInput()
                ser.flushOutput()
                break
    except Exception as e:
        logging.error(e)
        sys.exit()
ser.close()