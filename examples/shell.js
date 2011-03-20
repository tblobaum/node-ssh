var ssh = require('ssh');
var repl = require('repl');

ssh.createServer(function (session) {
    session.on('password', function (user, pass, cb) {
        cb(user === 'foo' && pass === 'bar');
    });
    
    session.on('shell', function (sh) {
        repl.start('node-ssh $ ', sh);
    });
});
