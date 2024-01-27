db.dropUser('admin');
db.users.remove({});
db.devices.remove({});
db.config.remove({});
db.roles.remove({});

db.createUser({
  user: 'admin',
  pwd: 'Admin123',
  roles: [{ role: 'readWrite', db: 'rdss' }],
});

db.users.insertOne({
  email: 'admin@uqomm.com',
  name: 'admin',
  password: 'Admin.123',
  rol: 'sadmin',
});

//loop
for (i = 1; i <= 255; i++) {
  const randomGroupId = Math.floor(Math.random() * 3) + 1;
  db.devices.insert({
    id: i,
    status: { provisioned: false, connected: false, x: 100, y: 100 },
    type: 'sniffer',
    group_id: randomGroupId
  });
}

for (i = 1; i <= 3; i++) {
  db.groups.insert({
    id: i,
    name: "Grupo "+i,
  });
}


db.createCollection('rtData');
db.rtData.createIndex({ 'metaData.deviceId': 1 });
db.rtData.createIndex({ sampleTime: -1 }, { expireAfterSeconds: 31536000 });
db.rtData.createIndex({ 'metaData.deviceId': 1, sampleTime: -1 });

db.config.insert({
  // minVoltage: 20,
  // maxVoltage: 50,
  // minCurrent: 0.01,
  // maxCurrent: 1.5,
  // minUplink: 0,
  // maxUplink: 30,
  // minDownlink: 0,
  // maxDownlink: 30,
  // minDownlinkOut: -100,
  // maxDownlinkOut: 100,
  image: '',
});
