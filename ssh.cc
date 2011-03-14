#include <node.h>
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
        
        ctc = Persistent<FunctionTemplate>::New(c);
        ctc->InstanceTemplate()->SetInternalFieldCount(1);
        ctc->SetClassName(String::NewSymbol("Client"));
        
        target->Set(String::NewSymbol("Server"), cts->GetFunction());
        target->Set(String::NewSymbol("Client"), ctc->GetFunction());
    }
    
    static Handle<Value> Server(const Arguments &args);
    static Handle<Value> Client(const Arguments &args);
    
protected:
    static Handle<Value> New(const Arguments &args);
};

Persistent<FunctionTemplate> SSH::cts;
Persistent<FunctionTemplate> SSH::ctc;

Handle<Value> SSH::Server(const Arguments &args) {
    HandleScope scope;
    
    Local<Object> self = args.This();
    Local<Number> port = Local<Number>::Cast(args[0]);
    Local<Number> host = Local<Number>::Cast(args[1]);
    
    self->Set(String::NewSymbol("port"), port);
    self->Set(String::NewSymbol("host"), host);
    
    return args.This();
}

Handle<Value> SSH::Client(const Arguments &args) {
    HandleScope scope;
    
    return args.This();
}

extern "C" void init(Handle<Object> target) {
    HandleScope scope;
    SSH::Initialize(target);
}
