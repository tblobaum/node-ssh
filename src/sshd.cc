#include "sshd.h"
#include "client.h"

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
    
    sessionSymbol = NODE_PSYMBOL("session");
    
    target->Set(
        String::NewSymbol("Server"),
        constructor_template->GetFunction()
    );
    
    Client::Initialize();
    target->Set(
        String::NewSymbol("Client"),
        Client::constructor_template->GetFunction()
    );
}

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
    
    eio_custom(Accept, EIO_PRI_DEFAULT, AcceptAfter, sshd);
    ev_ref(EV_DEFAULT_UC);
    
    return args.This();
}

int SSHD::Accept (eio_req *req) {
    SSHD *server = (SSHD *) req->data;
    Client client;
    int r = ssh_bind_accept(server->sshbind, client.session);
    if (r == SSH_ERROR) return 1;
    if (ssh_handle_key_exchange(client.session)) return 1;
    
    return 0;
}

int SSHD::AcceptAfter (eio_req *req) {
    SSHD *server = (SSHD *) req->data;
    
    if (!server->closed) {
        eio_custom(Accept, EIO_PRI_DEFAULT, AcceptAfter, server);
        ev_ref(EV_DEFAULT_UC);
    }
    
    Handle<Object> clientObj = Client::constructor_template
        ->GetFunction()->NewInstance();
    
    Client *client = ObjectWrap::Unwrap<Client>(clientObj);
    
    Handle<Value> argv[1];
    argv[0] = clientObj;
    
    server->Emit(sessionSymbol, 1, argv);
    
    eio_custom(Client::Message, EIO_PRI_DEFAULT, Client::MessageAfter, client);
    ev_ref(EV_DEFAULT_UC);
}

Handle<Value> SSHD::Close(const Arguments &args) {
    HandleScope scope;
    SSHD *sshd = ObjectWrap::Unwrap<SSHD>(args.This());
    sshd->closed = true;
    return args.This();
}
