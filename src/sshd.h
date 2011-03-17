#ifndef NODE_SSH_SSHD_H
#define NODE_SSH_SSHD_H

#include <node.h>
#include <node_object_wrap.h>
#include <node_events.h>
#include <v8.h>

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>

#include <cerrno>

using namespace v8;
using namespace node;

class SSHD : EventEmitter {
private:
    static Persistent<FunctionTemplate> constructor_template;
    
protected:
    static Handle<Value> New(const Arguments &args);

public:
    ssh_bind sshbind;
    bool closed;
    SSHD(const Arguments &);
    
    void setPort(Local<Value>);
    void setHost(Local<Value>);
    
    static void Initialize(Handle<Object> &);
    static Handle<Value> Listen(const Arguments &args);
    static Handle<Value> Close(const Arguments &args);
    
    static int Accept(eio_req *);
    static int AcceptAfter(eio_req *);
    static Persistent<String> sessionSymbol;
};

Persistent<FunctionTemplate> SSHD::constructor_template;

#endif
