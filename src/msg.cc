#include "msg.h"

Persistent<FunctionTemplate> Msg::constructor_template;

Msg::Msg() {}

void Msg::prepare(ssh_message msg, Persistent<Object> target) {
    message = msg;
    
    int type = ssh_message_type(msg);
    int subtype = ssh_message_subtype(msg);
    
    target->Set(String::NewSymbol("type"), Integer::New(type));
    target->Set(String::NewSymbol("subtype"), Integer::New(subtype));
    
    if (type == SSH_REQUEST_AUTH) {
        if (subtype == SSH_AUTH_METHOD_PASSWORD) {
            target->Set(
                String::NewSymbol("user"),
                String::New(ssh_message_auth_user(msg))
            );
            target->Set(
                String::NewSymbol("password"),
                String::New(ssh_message_auth_password(msg))
            );
        }
    }
}

void Msg::Initialize() {
    HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->SetClassName(String::NewSymbol("Message"));
    
    NODE_SET_PROTOTYPE_METHOD(
        constructor_template, "replyDefault", ReplyDefault
    );
    NODE_SET_PROTOTYPE_METHOD(
        constructor_template, "authSetMethods", AuthSetMethods
    );
}

Handle<Value> Msg::New(const Arguments &args) {
    HandleScope scope;
    
    Msg *msg = new Msg();
    msg->Wrap(args.This());
    
    return args.This();
}

Handle<Value> Msg::ReplyDefault(const Arguments &args) {
    ssh_message msg = ObjectWrap::Unwrap<Msg>(args.This())->message;
    ssh_message_reply_default(msg);
    ssh_message_free(msg);
}

Handle<Value> Msg::AuthSetMethods(const Arguments &args) {
    ssh_message msg = ObjectWrap::Unwrap<Msg>(args.This())->message;
    if (args[0]->IsNumber()) {
        ssh_message_auth_set_methods(
            msg,
            Local<Number>::Cast(args[0])->Int32Value()
        );
    }
    else {
        ThrowException(Exception::TypeError(
            String::New("methods must be an integer")
        ));
    }
}
