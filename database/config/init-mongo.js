db.getSiblingDB('rdss');

db.createUser({
  user: 'admin',
  pwd: 'Admin123',
  roles: [{ role: 'readWrite', db: 'rdss' }],
});

db.users.insert({
  name: 'test3@test3.cl',
  company: 'test3',
  password: '123',
  rolname: 'TestRol',
});

db.roles.insert({
  name: 'TestRol',
  users: [
    {
      name: 'test3@test3.cl',
    },
  ],
});

db.devices.insert({
  id: 1,
  name: 'vlad1',
  status: { provisioned: false, connected: false, x: 100, y: '100' },
  type: 'vlad',
  data: [{ fecha: { $timestamp: {} }, voltaje: 12, otro: 0.123 }],
});
db.devices.insert({
  id: 2,
  name: 'vlad2',
  status: { provisioned: false, connected: false, x: 100, y: '100' },
  type: 'vlad',
  data: [{ fecha: { $timestamp: {} }, voltaje: 12, otro: 0.123 }],
});

db.config.insert({
  minVoltage: 20,
  maxVoltage: 50,
  minCurrent: 0.01,
  maxCurrent: 1.5,
  minUplink: 0,
  maxUplink: 30,
  minDownlink: 0,
  maxDownlink: 30,
  minDownlinkOut: -100,
  maxDownlinkOut: 100,
  image: '',
});
