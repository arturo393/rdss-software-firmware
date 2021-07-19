#!/bin/bash

echo "Installing MongoDB..."
curl -fsSL https://www.mongodb.org/static/pgp/server-4.4.asc | sudo apt-key add -
echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu focal/mongodb-org/4.4 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-4.4.list
sudo apt-get update
apt -y install mongodb-org
systemctl start mongod.service
systemctl status mongod
systemctl enable mongod
mongo --eval 'db.runCommand({ connectionStatus: 1 })'
mongo localhost:27017/rdss --eval "load('/tmp/rdss2/database/config/init-mongo.js')"

# Clonar Repositorio
echo "Cloning Repo..."
cd /tmp
git clone https://gitlab.com/itaum/rdss2 rdss
cd rdss
mkdir -p /opt/rdss

# Monitor
echo "Installing Monitor..."
ln -s /usr/bin/python3 /usr/bin/python
mv monitor/src /opt/rdss/monitor
cp monitor/monitorRDSS.service /etc/systemd/system/
chmod 644 /etc/systemd/system/monitorRDSS.service
systemctl enable monitorRDSS
cd /opt/rdss/monitor
apt -y install python3-pip
pip install python-socketio pymongo crccheck pyserial sympy Flask flask_socketio eventlet
service monitorRDSS start

# Frontend 
echo "Installing Frontend..."
cd /tmp/rdss/frontend
mv src /opt/rdss/frontend
cd /opt/rdss/frontend
apt -y install nodejs
apt -y install npm
npm install
npx next build
npx next start -p 80


