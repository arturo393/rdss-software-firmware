db.createUser({
  user: 'admin',
  pwd: 'Admin123',
  roles: [{ role: 'readWrite', db: 'rdss' }],
});

db.getSiblingDB('rdss');
db.devices.insert({ id: 1, name: 'vlad1' });
