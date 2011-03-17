#include "client.h"

static int accept(eio_req *);
static int acceptAfter(eio_req *);
static int message(eio_req *);
static int messageAfter(eio_req *);

void Client::Initialize() {
    HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->Inherit(EventEmitter::constructor_template);
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->SetClassName(String::NewSymbol("Client"));
    
    NODE_SET_PROTOTYPE_METHOD(constructor_template, "end", End);
}

Persistent<FunctionTemplate> Client::constructor_template;

Handle<Value> Client::New(const Arguments &args) {
    HandleScope scope;
    
    Client *client = new Client();
    client->Wrap(args.This());
    
    return args.This();
}

Client::Client() {
    session = ssh_new();
}

int Client::Message(eio_req *req) {
    Client *client = (Client *) req->data;
    
    ssh_message msg = ssh_message_get(client->session);
    client->messageQueue.push_back(msg);
    
    return 0;
}

int Client::MessageAfter(eio_req *req) {
    HandleScope scope;
    Client *client = (Client *) req->data;
    
    eio_custom(Message, EIO_PRI_DEFAULT, MessageAfter, client);
    ev_ref(EV_DEFAULT_UC);
    
    while (!client->messageQueue.empty()) {
        ssh_message msg = client->messageQueue.front();
        client->messageQueue.pop_front();
        
        Handle<Value> argv[1];
        argv[0] = Integer::New(ssh_message_type(msg));
        
        client->Emit(String::NewSymbol("message"), 1, argv);
    }
    
    return 0;
}

Handle<Value> Client::End(const Arguments &args) {
    HandleScope scope;
    
    return args.This();
}
