var sshd = require('./build/default/sshd');
var server = new sshd.Server({ rsa : '/home/substack/.ssh/id_rsa' });
server.listen(6060);

server.on('session', function (s) {
    console.log('got session!');
    
    var authed = false;
    s.on('message', function (m) {
        console.dir(m);
        if (m.type === sshd.constants.SSH_REQUEST_AUTH) {
            if (m.subtype == sshd.constants.SSH_AUTH_METHOD_PASSWORD) {
                if (m.user === 'foo' && m.password === 'bar') {
                    m.authReplySuccess();
                    authed = true;
                    console.log('auth success!');
                }
                else {
                    m.replyDefault();
                    console.log('auth failure!');
                }
            }
            else {
                m.authSetMethods(sshd.constants.SSH_AUTH_METHOD_PASSWORD);
                m.replyDefault();
            }
        }
        else if (!authed) {
            m.replyDefault()
        }
        else if (m.type === sshd.constants.SSH_REQUEST_CHANNEL_OPEN
        && m.subtype === sshd.constants.SSH_CHANNEL_SESSION) {
            var ch = m.openChannel();
            console.dir({ ch : ch });
            ch.on('data', function (buf) {
                console.log(buf);
            });
            ch.on('end', function () {
                console.log('channel ended');
            });
        }
        else if (m.type === sshd.constants.SSH_REQUEST_CHANNEL
        && m.subtype === sshd.constants.SSH_CHANNEL_REQUEST_SHELL) {
            m.channelReplySuccess();
        }
        else {
            m.replyDefault()
        }
    })
});
