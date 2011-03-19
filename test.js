var sshd = require('./build/default/sshd');
var server = new sshd.Server({ rsa : '/home/substack/.ssh/id_rsa' });
server.listen(6060);

server.on('session', function (s) {
    console.log('got session!');
    
    s.on('message', function (m) {
        console.log('message!');
        console.dir(m);
        if (m.type === sshd.constants.SSH_REQUEST_AUTH) {
console.log('SSH_REQUEST_AUTH!!!');
console.dir([ m.subtype, sshd.constants.SSH_AUTH_METHOD_PASSWORD ]);
            if (m.subtype == sshd.constants.SSH_AUTH_METHOD_PASSWORD) {
                console.log('woooo!');
            }
            else {
                m.authSetMethods(sshd.constants.SSH_AUTH_METHOD_PASSWORD);
                m.replyDefault();
            }
        }
        else {
            m.replyDefault()
        }
    })
});
