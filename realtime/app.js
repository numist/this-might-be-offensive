var util = require("util"),
  io = require("socket.io").listen(1337),
  path = require("path"),
  iniparser = require("iniparser"),
  mysql = require("mysql"),
  redis = require("redis").createClient();

var db_config = iniparser.parseSync(path.normalize(path.join(__dirname, '../admin/.config'))).tmbo;

// iniparser returns everything as a literal, so we need to eval strings if they are literal strings
function checkIniString(data) {
  if (/^("|').*("|')$/.test(data))
    return eval(data);
  else
    return data;
}

var db = mysql.createClient({
  user: checkIniString(db_config.database_user),
  password: checkIniString(db_config.database_pass),
  host: checkIniString(db_config.database_host),
  database: checkIniString(db_config.database_name)
});

// Authorize by the token
function checkToken(token, callback) {
  db.query('SELECT * FROM tokens WHERE tokenid = ?;', [token], function(err, results, fields) {
    callback(null, (results.length > 0));
  });
}

io.configure(function() {
  io.set('authorization', function(handshakeData, callback) {
    if(handshakeData.query && handshakeData.query.token) {
      checkToken(handshakeData.query.token, callback);
    } else {
      callback(null, false);
    }
  });

  io.set('transports', ['htmlfile', 'xhr-polling', 'jsonp-polling']);
});

io.configure("production", function() {
  io.enable('browser client minification');  // send minified client
  io.enable('browser client etag');          // apply etag caching logic based on version number
  io.enable('browser client gzip');          // gzip the file
  io.set('log level', 1);                    // reduce logging
});

/************************************************
When a client sends "subscribe" we subscribe
them to the channel that matches the wildcard
string they sent. If the channel doesn't already
exist, then we also send a PSUBSCRIBE to Redis
with the same pattern. When we get a PMESSAGE
from Redis, we send it to the channel named after
the pattern used.

When a client unsubscribes (or switches channels)
we decrement the count of users subscribed to the
Redis channel. Once all users are unsubbed, we
PUNSUBSCRIBE to save resources.
************************************************/
var subs = {};

io.sockets.on('connection', function(socket) {
  var _channel;

  socket.on('subscribe', function(channel) {
    if(_channel && --subs[_channel] == 0)
      redis.punsubscribe(_channel);
    socket.join(channel);
    _channel = channel;
    subs[channel] = subs[channel] || 0;
    subs[channel]++;
    redis.psubscribe(channel);
  });

  socket.on('disconnect', function() {
    if(_channel && --subs[_channel] == 0)
      redis.punsubscribe(_channel);
  });
});

redis.on('pmessage', function(pattern, channel, message) {
  message = JSON.parse(message);
  io.sockets.in(pattern).emit(message.type, message.data);
});
