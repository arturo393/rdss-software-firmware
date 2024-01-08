#!/usr/bin/env python3
BASE_PATH = "C:\\Users\\sigmadev\\Desktop\\sniffer\\sniffer-software\\monitor\\src\\"
#BASE_PATH = "/opt/sniffer/monitor/"
FORMAT = 'utf-8'
POLLING_SLEEP = 2
LOGGING_FILE = "monitor.log"
AGC_DATA = BASE_PATH+"agcdata.csv"
POWER_DATA = BASE_PATH+"power.csv"
VOLTAGE_DATA = BASE_PATH+"voltage.csv"
CURRENT_DATA = BASE_PATH+"current.csv"
# Variables parches para gráficos
MAX_DEVICES = 255
MAX_VOLTAGE = 80
MIN_PTX = -50
MAX_PTX = 10
RANGE_MIN_PTX = 30
RANGE_MAX_PTX = 255

log = {
    "format": "%(asctime)s [RDSS/%(processName)s] %(levelname)s %(message)s",
    "filename": "monitor.out"
}

db = {
    "host": "192.168.60.76",
    "user": "admin",
    "passwd": "Admin123",
    "dbname": "rdss",
    "port": "27017"

}

serial = {
    "port": "/dev/ttyUSB0",
    "baudrate": 19200,
    "parity": "N",
    "stopbits": 1,
    "bytesize": 8,
    "timeout": 1.8,
    "write_timeout": 12,
    "inter_byte_timeout": 0.1
}
