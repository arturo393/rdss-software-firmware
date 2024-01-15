import serial
import logging
import sys

#logging.basicConfig(filename="serialTest.log", level=logging.debug)

ser = serial.Serial(
    port='COM1',\
    #port="/dev/ttyS0",\
    baudrate=115200,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
    timeout=5)

#this will store the line
line = []

while True:
    #print("Starting read...")
    try:
        #print("Waiting...")
        for c in ser.read():
            line.append(c)
            #print(c)
            if c == 255:
                #print("Line: " + ''.join(line))
                for cmd_int in line:
                    ser.write(cmd_int.to_bytes(1, 'big'))
                print("Message retransmitted:")
                print(line)
                print("Message length: " + str(len(line)))
                print("-----")
                line = []
                ser.flushInput()
                ser.flushOutput()
                break
    except Exception as e:
        logging.error(e)
        sys.exit()
ser.close()