# Prerequisites

Update monitor and frontend installation code

Run frontend as development environment:

```
cd /opt/rdss/frontend
npm run dev

```

## Create new time series collection

go to MongoDb shell and execute the following commands:

```
db.adminCommand({ setFeatureCompatibilityVersion: '5.0' });

db.createCollection('rtData', {
  timeseries: {
    metaField: 'metaData',
    timeField: 'sampleTime',
    granularity: 'seconds',
  },
  expireAfterSeconds: 86400 * 365, // 1 year
});

db.rtData.createIndex({ 'metaData.deviceId': 1 });
```

# Migration Data

## Export historical data

On linux console, execute the following commands:

```

cd /opt/rdss/frontend/migration

mongo_dump --host=localhost --port=27017 --db=rdss --collection=devices --out=data/devices.json

```

you should find the data in the file `data/devices.json`

## Execute migration

This procedure should be executed for each deviceId:

Open a browser and for each device visit:

http://localhost:3000/api/devices/migrateRTData?deviceId=&lt;DEVICEID&gt;

Example, if you want to migrate `deviceId = 1` data go to:

http://localhost:3000/api/devices/migrateRTData?deviceId=1

**Note:** If installation is not in your local host, reeplace `localhost` with the IP address of your installation.

# Check migration status

Check migration on `npm run dev` console output
Loggin into RDSS frontend application and check RTData graphics
