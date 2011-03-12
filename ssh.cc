#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>

using namespace v8;
using namespace node;

class SSH {
private:
    static Persistent<FunctionTemplate> ctS;
    static Persistent<FunctionTemplate> ctC;
public:
    static void Initialize(Handle<Object> target) {
        HandleScope scope;
        
        Local<FunctionTemplate> s = FunctionTemplate::New(Server);
        Local<FunctionTemplate> c = FunctionTemplate::New(Client);
        
        ctS = Persistent<FunctionTemplate>::New(s);
        ctC = Persistent<FunctionTemplate>::New(c);
        
        ctS->InstanceTemplate()->SetInternalFieldCount(1);
        ctC->InstanceTemplate()->SetInternalFieldCount(1);
        
        ctS->SetClassName(String::NewSymbol("Server"));
        ctC->SetClassName(String::NewSymbol("Client"));
        
        target->Set(String::NewSymbol("Server"), ctS->GetFunction());
        target->Set(String::NewSymbol("Client"), ctC->GetFunction());
    }
    
    static Handle<Value> Server(const Arguments &args) {
        HandleScope scope;
        
        return args.This();
    }
    
    static Handle<Value> Client(const Arguments &args) {
        HandleScope scope;
        
        return args.This();
    }
    
protected:
    static Handle<Value> New(const Arguments &args);
};

Persistent<FunctionTemplate> SSH::ctS;
Persistent<FunctionTemplate> SSH::ctC;

extern "C" void init(Handle<Object> target) {
    HandleScope scope;
    SSH::Initialize(target);
}
