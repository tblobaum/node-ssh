var sshd = require('./build/default/sshd');
var constants = sshd.constants;

var exports = module.exports = function (keys) {
    if (keys === undefined) {
        keys = {
            rsa : process.env.HOME + '/home/substack/.ssh/id_rsa',
            dsa : process.env.HOME + '/home/substack/.ssh/id_dsa',
        };
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
    return module.exports().createServer(port, host);
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
                session.emit(m.user, m.password, function (ok) {
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
                    })
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
            var write = chan.write;
            
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
});
