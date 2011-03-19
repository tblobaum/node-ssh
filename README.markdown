ssh
===

Create ssh servers (and later, clients too) in node.js!

Coming soon.

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

Note: the above does not work yet, coming soon
