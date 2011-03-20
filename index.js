var sshd = require('./build/default/sshd');
var constants = sshd.constants;
var path = require('path');

var exports = module.exports = function (keys) {
    if (keys === undefined) {
        keys = {};
        var dsaFile = process.env.HOME + '/.ssh/id_dsa';
        var rsaFile = process.env.HOME + '/.ssh/id_rsa';
        
        if (path.existsSync(dsaFile)) {
            keys.dsa = dsaFile;
        }
        else if (path.existsSync(rsaFile)) {
            keys.rsa = rsaFile;
        }
    }
    
    if (!keys.dsa && !keys.rsa) {
        throw new Error('no keys specified');
    }
    
    return {
        createServer : function (cb) {
            var server = new sshd.Server(keys);
            server.on('session', function (s) {
                cb(wrapSession(s));
            });
            return server;
        }
    }
};

exports.sshd = sshd;

exports.createServer = function (cb) {
    return module.exports().createServer(cb);
};

var authNames = {};
authNames[constants.SSH_AUTH_METHOD_PASSWORD] = 'password';

function wrapSession (session) {
    var authed = false;
    var chan = null;
    
    session.on('message', function (m) {
        if (m.type === constants.SSH_REQUEST_AUTH) {
            var name = authNames[m.subtype];
            if (session._events[name]) {
                session.emit('password', m.user, m.password, function (ok) {
                    if (ok) {
                        m.authReplySuccess();
                        authed = true;
                    }
                    else {
                        m.replyDefault();
                    }
                });
            }
            else {
                var methods = Object.keys(authNames)
                    .filter(function (c) {
                        return session._events[authNames[c]];
                    })
                    .reduce(function (a, b) {
                        return a | b;
                    }, 0)
                ;
                m.authSetMethods(methods);
                m.replyDefault();
            }
        }
        else if (!authed) {
            m.replyDefault()
        }
        else if (m.type === constants.SSH_REQUEST_CHANNEL_OPEN
        && m.subtype === constants.SSH_CHANNEL_SESSION) {
            chan = m.openChannel();
            var write = chan.write.bind(chan);
            
            chan.pause = function () {
                // TODO
            };
            
            chan.resume = function () {
                // TODO
            };
            
            chan.write = function (buf) {
                if (Buffer.isBuffer(buf)) {
                    write(buf);
                }
                else if (typeof buf === 'string') {
                    write(new Buffer(buf));
                }
                else {
                    throw new TypeError('can only write strings and Buffers');
                }
            };
            
            chan.pipe = function (stream, opts) {
                if (!opts) opts = {};
                
                chan.on('data', function (buf) {
                    stream.write(buf);
                });
                
                if (opts.end !== false) {
                    chan.on('end', function () {
                        stream.end();
                    });
                }
            };
        }
        else if (chan
        && m.type === constants.SSH_REQUEST_CHANNEL
        && m.subtype === constants.SSH_CHANNEL_REQUEST_SHELL) {
            m.channelReplySuccess();
            session.emit('shell', chan);
        }
        else {
            m.replyDefault()
        }
    });
    
    return session;
}
