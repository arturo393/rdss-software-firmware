# Prerequisites

Update monitor and frontend installation code

```
cd /opt/frontend
npm run build
service frontendRDSS restart
```

## Create rtData collection

go to MongoDb shell

```
# mongo
> use rdss
```

and execute the following commands:

```

db.createCollection('rtData');
db.rtData.createIndex({ 'metaData.deviceId': 1 });
db.rtData.createIndex({ sampleTime: -1 }, { expireAfterSeconds: 31536000 });
db.rtData.createIndex({ 'metaData.deviceId': 1, sampleTime: -1 });
```

# Migration Data

## Export historical data

On linux console, execute the following commands:

```
cd /opt/rdss/frontend/migration
mongoexport --db rdss --collection devices --out data/devices.json

```

you should find the data in the file `data/devices.json`

## Execute migration

This procedure should be executed for each deviceId:

Open a browser and for each device visit:

http://RDSS-FRONTEND-IP/api/devices/migrateRTData?deviceId=&lt;DEVICEID&gt;

Example, if you want to migrate `deviceId = 1` data go to:

http://RDSS-FRONTEND-IP/api/devices/migrateRTData?deviceId=1

**Note:** If installation is not in your local host, replace `localhost` with the IP address (`RDSS-FRONTEND-IP`) of your installation.

# Check migration status

Login into RDSS frontend application and check RTData graphics

# Removing old data

open mongodb console an execute:

`db.devices.updateOne({id: DEVICEID},{$unset: {rtData: 1}})`

example for device ID `5`:
`db.devices.updateOne({id: 5},{$unset: {rtData: 1}})`
