#!/usr/bin/env python3

FORMAT = 'utf-8'
POLLING_SLEEP = 2
LOGGING_FILE = "apiRDSS.log"
MAX_VOLTAGE = 100

mssql = {
    "host": "localhost",
    "user": "sa",
    "passwd": "Admin.123",
    "db": "RDSSdb",
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
