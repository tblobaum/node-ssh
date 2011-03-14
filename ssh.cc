#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <v8.h>

#include <libssh/libssh.h>
#include <libssh/server.h>

using namespace v8;
using namespace node;

class SSH {
private:
    static Persistent<FunctionTemplate> cts;
    static Persistent<FunctionTemplate> ctc;
public:
    static void Initialize(Handle<Object> target) {
        HandleScope scope;
        
        Local<FunctionTemplate> s = FunctionTemplate::New(Server);
        Local<FunctionTemplate> c = FunctionTemplate::New(Client);
        
        cts = Persistent<FunctionTemplate>::New(s);
        cts->InstanceTemplate()->SetInternalFieldCount(1);
        cts->SetClassName(String::NewSymbol("Server"));
        NODE_SET_PROTOTYPE_METHOD(cts, "listen", Server_listen);
        
        ctc = Persistent<FunctionTemplate>::New(c);
        ctc->InstanceTemplate()->SetInternalFieldCount(1);
        ctc->SetClassName(String::NewSymbol("Client"));
        
        target->Set(String::NewSymbol("Server"), cts->GetFunction());
        target->Set(String::NewSymbol("Client"), ctc->GetFunction());
    }
    
    static Handle<Value> Server(const Arguments &args);
    static Handle<Value> Client(const Arguments &args);
    
    Handle<Value> listen(const Arguments &args);
    
protected:
    static Handle<Value> New(const Arguments &args);
    ssh_bind sshbind;
};

Persistent<FunctionTemplate> SSH::cts;
Persistent<FunctionTemplate> SSH::ctc;

Handle<Value> SSH::Server(const Arguments &args) {
    HandleScope scope;
    
    Local<Number> port = Local<Number>::Cast(args[0]);
    Local<String> host = Local<String>::Cast(args[1]);
    
    sshbind = ssh_bind_new();
    
    if (!host->IsUndefined()) {
        ssh_bind_options_set(
            sshbind,
            SSH_BIND_OPTIONS_BINDADDR,
            Buffer::Data(Local<Object>::Cast(host))
        );
    }
    
    if (!port->IsUndefined()) {
        ssh_bind_options_set(
            sshbind,
            SSH_BIND_OPTIONS_BINDPORT,
            Buffer::Data(Local<Object>::Cast(host))
        );
    }
    
    return args.This();
}

Handle<Value> SSH::listen(const Arguments &args) {
    HandleScope scope;
    
    Local<Number> port = Local<Number>::Cast(args[0]);
    Local<String> host = Local<String>::Cast(args[1]);
    
    if (!host->IsUndefined()) {
        ssh_bind_options_set(
            sshbind,
            SSH_BIND_OPTIONS_BINDADDR,
            Buffer::Data(Local<Object>::Cast(host))
        );
    }
    
    if (!port->IsUndefined()) {
        ssh_bind_options_set(
            sshbind,
            SSH_BIND_OPTIONS_BINDPORT,
            Buffer::Data(Local<Object>::Cast(host))
        );
    }
    
    sshbind = ssh_bind_new();
    
    return Undefined();
}

Handle<Value> SSH::Client(const Arguments &args) {
    HandleScope scope;
    
    return args.This();
}

extern "C" void init(Handle<Object> target) {
    HandleScope scope;
    SSH::Initialize(target);
}
