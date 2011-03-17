#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>

#include <cerrno>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>

using namespace v8;
using namespace node;

class SSHD : ObjectWrap {
private:
    static Persistent<FunctionTemplate> ct;
    
protected:
    static Handle<Value> New(const Arguments &args);
    
public:
    ssh_bind sshbind;
    bool closed;
    
    SSHD(const Arguments &);
    
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
        
        Persistent<FunctionTemplate> ct;
        Local<FunctionTemplate> t = FunctionTemplate::New(Server);
        ct = Persistent<FunctionTemplate>::New(t);
        ct->InstanceTemplate()->SetInternalFieldCount(1);
        ct->SetClassName(String::NewSymbol("Server"));
        
        NODE_SET_PROTOTYPE_METHOD(ct, "listen", Listen);
        NODE_SET_PROTOTYPE_METHOD(ct, "close", Close);
        target->Set(String::NewSymbol("Server"), ct->GetFunction());
    }
    
    static Handle<Value> Server(const Arguments &args);
    static Handle<Value> Listen(const Arguments &args);
    static Handle<Value> Close(const Arguments &args);
    
    static int Accept(eio_req *);
    static int Accept_After(eio_req *);
    static int Message(eio_req *);
    static int Message_After(eio_req *);
};

Persistent<FunctionTemplate> SSHD::ct;

struct Dispatch {
    ssh_message message;
    ssh_session session;
    SSHD *server;
    
    Dispatch(SSHD *sshd) {
        message = NULL;
        server = sshd;
        session = ssh_new();
    }
};

SSHD::SSHD(const Arguments &args) {
    closed = false;
    sshbind = ssh_bind_new();
    Local<Object> keys = Local<Object>::Cast(args[0]);
    
    Local<Object> dsaObj = Local<Object>::Cast(
        keys->Get(String::NewSymbol("dsa"))
    );
    if (!dsaObj->IsUndefined()) {
        Local<String> dsa = dsaObj->ToString();
        char *dsa_s = new char[ dsa->Length() + 1 ];
        dsa->WriteAscii(dsa_s);
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, dsa_s);
    }
    
    Local<Object> rsaObj = Local<Object>::Cast(
        keys->Get(String::NewSymbol("rsa"))
    );
    if (!rsaObj->IsUndefined()) {
        Local<String> rsa = rsaObj->ToString();
        char *rsa_s = new char[ rsa->Length() + 1 ];
        rsa->WriteAscii(rsa_s);
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, rsa_s);
    }
}

Handle<Value> SSHD::Server(const Arguments &args) {
    HandleScope scope;
    
    SSHD *sshd = new SSHD(args);
    sshd->Wrap(args.This());
    return args.This();
}

Handle<Value> SSHD::Listen(const Arguments &args) {
    HandleScope scope;
    SSHD *sshd = ObjectWrap::Unwrap<SSHD>(args.This());
    
    if (!args[0]->IsUndefined()) {
        sshd->setPort(args[0]);
    }
    
    if (!args[1]->IsUndefined()) {
        sshd->setHost(args[1]);
    }
    
    if (ssh_bind_listen(sshd->sshbind) < 0) {
        return Exception::Error(
            String::New(ssh_get_error(sshd->sshbind))
        );
    }
    
    Dispatch *d = new Dispatch(sshd);
    eio_custom(Accept, EIO_PRI_DEFAULT, Accept_After, d);
    ev_ref(EV_DEFAULT_UC);
    
    return args.This();
}

Handle<Value> SSHD::Close(const Arguments &args) {
    HandleScope scope;
    SSHD *sshd = ObjectWrap::Unwrap<SSHD>(args.This());
    sshd->closed = true;
    return args.This();
}

int SSHD::Accept(eio_req *req) {
    Dispatch *d = (Dispatch *) req->data;
    SSHD *sshd = d->server;
    
    int r = ssh_bind_accept(sshd->sshbind, d->session);
    if (r == SSH_ERROR) return 1;
    if (ssh_handle_key_exchange(d->session)) return 1;
    
    return 0;
}

int SSHD::Accept_After(eio_req *req) {
    HandleScope scope;
    SSHD *sshd = (SSHD *) req->data;
    Dispatch *d = new Dispatch(sshd);
    
    eio_custom(Message, EIO_PRI_DEFAULT, Message_After, d);
    ev_ref(EV_DEFAULT_UC);
    
    if (!sshd->closed) {
        eio_custom(Accept, EIO_PRI_DEFAULT, Accept_After, sshd);
        ev_ref(EV_DEFAULT_UC);
    }
    return 0;
}

int SSHD::Message(eio_req *req) {
    Dispatch *d = (Dispatch *) req->data;
    d->message = ssh_message_get(d->session);
    
    return 0;
}

int SSHD::Message_After(eio_req *req) {
    Dispatch *d = (Dispatch *) req->data;
    
    if (d->message) {
        eio_custom(Message, EIO_PRI_DEFAULT, Message_After, d);
        ev_ref(EV_DEFAULT_UC);
    }
}

extern "C" void init(Handle<Object> target) {
    HandleScope scope;
    SSHD::Initialize(target);
}
