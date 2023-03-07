#!/usr/bin/env python3
BASE_PATH = "/opt/rdss/monitor/"
FORMAT = 'utf-8'
POLLING_SLEEP = 2
LOGGING_FILE = "monitor.log"

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
    "host": "database",
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
    "timeout": 3,
    "write_timeout": 10
}
