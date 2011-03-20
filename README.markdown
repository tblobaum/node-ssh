ssh
===

Create ssh servers in node.js!

example
=======

    var ssh = require('ssh');
    var repl = require('repl');
    
    ssh.createServer(function (session) {
        session.on('password', function (user, pass, cb) {
            cb(user === 'foo' && pass === 'bar');
        });
        
        session.on('shell', function (sh) {
            repl.start('node-ssh $ ', sh);
        });
    }).listen(2222);

installation
============

You'll need a version of [libssh](http://libssh.org/) with my patches:

    git clone git://github.com/substack/libssh.git master
    cd libssh
    ./configure --prefix=$PREFIX && make && make install

That should install libssh.pc, which is used by pkg-config during the
wscript installation. Make sure libssh.pc got installed into your
$PKG_CONFIG_PATH someplace.

You can then install with [npm](http://npmjs.org):

    npm install ssh
