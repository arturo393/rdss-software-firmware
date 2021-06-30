/* eslint-disable comma-dangle */
var express = require('express');
const cors = require('cors');
var app = express();
app.use(cors());

var server = require('http').createServer(app);
var io = require('socket.io')(server, { transports: ['websocket', 'polling'] });

io.on('connection', (client) => {
  console.log('Client connected...');
});

app.get('/', (req, res) => {
  io.sockets.emit('set_name_event', {
    event: 'set_name_event',
    leave: false,
    handle: 'SET_NAME_EVENT',
    name: req.query.name,
  });
  res.end();
});

server.listen(4200);
