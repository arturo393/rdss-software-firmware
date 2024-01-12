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

#this will store the line
#line = ''
line = []

while True:
    logging.debug("Starting read...")
    try:
        logging.debug("Waiting...")
        for c in ser.read():
            line.append(c)
            logging.debug(c)
            if c == 255:
                #logging.debug("Line: " + ''.join(line))
                for cmd_int in line:
                    ser.write(cmd_int.to_bytes())
                logging.debug("Message retransmitted:")
                logging.debug(line)
                logging.debug("Message length: " + str(len(line)))
                logging.debug("-----")
                line = []
                ser.flushInput()
                ser.flushOutput()
                break
    except Exception as e:
        logging.error(e)
        sys.exit()
ser.close()