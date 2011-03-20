var ssh = require('ssh');

ssh.createServer(function (session) {
    session.on('password', function (user, pass, cb) {
        cb(user === 'foo' && pass === 'bar');
    });
    
    session.on('shell', function (sh) {
        sh.pipe(sh); // echo on
    });
}).listen(2222);
