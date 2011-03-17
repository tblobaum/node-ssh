#include "sshd.h"

extern "C" void init(Handle<Object> target) {
    HandleScope scope;
    SSHD::Initialize(target);
}

void SSHD::setPort(Local<Value> port) {
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

void SSHD::setHost(Local<Value> hostObj) {
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

void SSHD::Initialize(Handle<Object> & target) {
    HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->Inherit(EventEmitter::constructor_template);
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->SetClassName(String::NewSymbol("Server"));
    
    NODE_SET_PROTOTYPE_METHOD(constructor_template, "listen", Listen);
    NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", Close);
    target->Set(
        String::NewSymbol("Server"),
        constructor_template->GetFunction()
    );
}

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

Handle<Value> SSHD::New(const Arguments &args) {
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
