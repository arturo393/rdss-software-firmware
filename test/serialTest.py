import serial
import logging

ser = serial.Serial(
    port='COM1',\
    baudrate=115200,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
        timeout=5)

print("Connected to: " + ser.portstr)

#this will store the line
#line = ''
line = []

while True:
    for c in ser.read():
        line.append(c)
        #print(c)
        if c == 255:
            #print("Line: " + ''.join(line))
            for cmd_int in line:
                ser.write(cmd_int.to_bytes())
            print("Message retransmitted:")
            print(line)
            print("Message length: " + str(len(line)))
            print("-----")
            line = []
            ser.flushInput()
            ser.flushOutput()
            break
ser.close()

'''
while True:
    for c in ser.read():
        line = line+('{0:02x}'.format(c))
        if(c == 255):
            #line = line[12:18]
            #print("Line: " + ''.join(line))
            cmd_bytes = bytearray.fromhex(line)
            hex_byte = ''
            for cmd_byte in cmd_bytes:
                hex_byte = ('{0:02x}'.format(cmd_byte))
                ser.write(bytes.fromhex(hex_byte))
            print("Message length: " + str(int(len(line)/2)))
            print("Message retransmitted: " + line)
            ser.flushInput()
            ser.flushOutput()
            line = ''
            break
ser.close()
'''