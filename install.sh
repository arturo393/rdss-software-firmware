#!/bin/bash

echo "Installing MongoDB..."
# Ref: https://docs.mongodb.com/manual/tutorial/install-mongodb-on-ubuntu/
sudo apt-get -y install gnupg
wget -qO - https://www.mongodb.org/static/pgp/server-5.0.asc | sudo apt-key add -
touch /etc/apt/sources.list.d/mongodb-org-5.0.list
echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu focal/mongodb-org/5.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-5.0.list
sudo apt-get update
sudo apt-get install -y mongodb-org
ps --no-headers -o comm 1


