db.users.insert({
  name: 'test3@test3.cl',
  company: 'test3',
  password: '123',
  rolname: 'TestRol'
});

db.roles.insert({
  name: 'TestRol',
  users: [
      {
          name: 'test3@test3.cl'
      }
  ]
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
  image: ''
});