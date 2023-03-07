#!/bin/sh

echo "Creating fake serial port..."
socat -d -d pty,link=/dev/ttyUSB0,raw,echo=0 pty,raw,echo=0 &
echo "Starting Monitor..."
python3 /opt/rdss/monitor/monitor.py &
cat /dev/null > /opt/rdss/monitor/monitor.log
tail -f /opt/rdss/monitor/monitor.log