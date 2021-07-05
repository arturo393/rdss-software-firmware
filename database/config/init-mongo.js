db.createUser({
  user: 'admin',
  pwd: 'Admin123',
  roles: [{ role: 'readWrite', db: 'rdss' }],
});

db.getSiblingDB('rdss');
db.devices.insert({ id: 1, name: 'vlad1' });
db.devices.insert({
  id: 2,
  name: 'vlad2',
  status: { provisioned: false, connected: false, x: 100, y: '100' },
  type: 'vlad',
  data: [{ fecha: { $timestamp: {} }, voltaje: 12, otro: 0.123 }],
});
