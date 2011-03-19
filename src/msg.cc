#include "msg.h"

Persistent<FunctionTemplate> Msg::constructor_template;

Msg::Msg(ssh_message msg) {
    message = msg;
}

Msg::Msg() {}

void Msg::Initialize() {
    HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->SetClassName(String::NewSymbol("Message"));
}

Handle<Value> Msg::New(const Arguments &args) {
    HandleScope scope;
    
    Msg *msg = new Msg();
    msg->Wrap(args.This());
    
    return args.This();
}
