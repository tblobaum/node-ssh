#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <v8.h>

#include <libssh/libssh.h>
#include <libssh/server.h>

using namespace v8;
using namespace node;

class SSHD : ObjectWrap {
private:
    static Persistent<FunctionTemplate> ct;
    
protected:
    static Handle<Value> New(const Arguments &args);

public:
    ssh_bind sshbind;
    Handle<Function> handler;
    
    SSHD(const Arguments &args) {
        sshbind = ssh_bind_new();
        
        if (args[0]->IsFunction()) {
            handler = Handle<Function>::Cast(args[0]);
        }
        else {
            ThrowException(Exception::TypeError(
                String::New("first argument must be a function")
            ));
        }
    }
    
    void setPort(Local<Value> port) {
        if (port->IsNumber()) {
            int32_t *port_i = new int32_t;
            *port_i = Local<Number>::Cast(port)->Int32Value();
            
            ssh_bind_options_set(
                sshbind,
                SSH_BIND_OPTIONS_BINDPORT,
                port_i
            );
        }
        else if (port->IsString()) {
            Local<String> s = Local<String>::Cast(port);
            char *port_s = new char[ s->Length() + 1 ];
            s->WriteAscii(port_s); // null terminates
            
            ssh_bind_options_set(
                sshbind,
                SSH_BIND_OPTIONS_BINDPORT_STR,
                port_s
            );
        }
        else {
            ThrowException(Exception::TypeError(
                String::New("port must be a string or integer")
            ));
        }
    }
    
    void setHost(Local<Value> hostObj) {
        if (!hostObj->IsString()) {
            ThrowException(Exception::TypeError(
                String::New("host must be a string")
            ));
            return;
        }
        
        Local<String> host = Local<String>::Cast(hostObj);
        char *host_s = new char[ host->Length() + 1 ];
        host->WriteAscii(host_s);
        
        ssh_bind_options_set(
            sshbind,
            SSH_BIND_OPTIONS_BINDADDR,
            host_s
        );
    }
    
    static void Initialize(Handle<Object> & target) {
        HandleScope scope;
        
        Local<FunctionTemplate> t = FunctionTemplate::New(Server);
        
        ct = Persistent<FunctionTemplate>::New(t);
        ct->InstanceTemplate()->SetInternalFieldCount(1);
        ct->SetClassName(String::NewSymbol("Server"));
        NODE_SET_PROTOTYPE_METHOD(ct, "listen", listen);
        
        target->Set(String::NewSymbol("Server"), ct->GetFunction());
    }
    
    static Handle<Value> Server(const Arguments &args);
    static Handle<Value> listen(const Arguments &args);
};

Persistent<FunctionTemplate> SSHD::ct;

Handle<Value> SSHD::Server(const Arguments &args) {
    HandleScope scope;
    
    SSHD *sshd = new SSHD(args);
    sshd->Wrap(args.This());
    return args.This();
}

Handle<Value> SSHD::listen(const Arguments &args) {
    HandleScope scope;
    
    SSHD *sshd = ObjectWrap::Unwrap<SSHD>(args.This());
    
    if (!args[0]->IsUndefined()) {
        sshd->setPort(args[0]);
    }
    
    if (!args[1]->IsUndefined()) {
        sshd->setHost(args[1]);
    }
    
    ssh_session session = ssh_new();
    
    if (ssh_bind_listen(sshd->sshbind) < 0) {
        return ThrowException(Exception::Error(
            String::New(ssh_get_error(sshd->sshbind))
        ));
    }
    
    int r = ssh_bind_accept(sshd->sshbind, session);
    if (r == SSH_ERROR) {
        return ThrowException(Exception::Error(
            String::New(ssh_get_error(sshd->sshbind))
        ));
    }
    
    return Undefined();
}

extern "C" void init(Handle<Object> target) {
    HandleScope scope;
    SSHD::Initialize(target);
}
