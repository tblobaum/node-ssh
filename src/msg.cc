#include "msg.h"

Persistent<FunctionTemplate> Msg::constructor_template;

Msg::Msg(ssh_message msg) {
    Wrap(Persistent<Object>::New(
        constructor_template->GetFunction()->NewInstance()
    ));
    
    message = msg;
    
    int type = ssh_message_type(msg);
    int subtype = ssh_message_subtype(msg);
    
    handle_->Set(String::NewSymbol("type"), Integer::New(type));
    handle_->Set(String::NewSymbol("subtype"), Integer::New(subtype));
    
    if (type == SSH_REQUEST_AUTH) {
        if (subtype == SSH_AUTH_METHOD_PASSWORD) {
            handle_->Set(
                String::NewSymbol("user"),
                String::New(ssh_message_auth_user(msg))
            );
            handle_->Set(
                String::NewSymbol("password"),
                String::New(ssh_message_auth_password(msg))
            );
        }
    }
    else if (type == SSH_REQUEST_CHANNEL_OPEN) {
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
    NODE_SET_PROTOTYPE_METHOD(
        constructor_template, "authReplySuccess", AuthReplySuccess
    );
    NODE_SET_PROTOTYPE_METHOD(
        constructor_template, "openChannel", OpenChannel
    );
    NODE_SET_PROTOTYPE_METHOD(
        constructor_template, "channelReplySuccess", ChannelReplySuccess
    );
}

Handle<Value> Msg::New(const Arguments &args) {
    HandleScope scope;
    return args.This();
}

Handle<Value> Msg::ReplyDefault(const Arguments &args) {
    HandleScope scope;
    
    ssh_message msg = ObjectWrap::Unwrap<Msg>(args.This())->message;
    ssh_message_reply_default(msg);
    ssh_message_free(msg);
    
    return args.This();
}

Handle<Value> Msg::AuthSetMethods(const Arguments &args) {
    HandleScope scope;
    
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
    return args.This();
}

Handle<Value> Msg::AuthReplySuccess(const Arguments &args) {
    ssh_message msg = ObjectWrap::Unwrap<Msg>(args.This())->message;
    if (args[0]->IsUndefined()) {
        ssh_message_auth_reply_success(msg, 0);
    }
    else {
        ssh_message_auth_reply_success(
            msg,
            Local<Number>::Cast(args[0])->Int32Value()
        );
    }
    ssh_message_free(msg);
    return args.This();
}

Handle<Value> Msg::OpenChannel(const Arguments &args) {
    ssh_message msg = ObjectWrap::Unwrap<Msg>(args.This())->message;
    Handle<Value> chanObj = Chan::Create(
        ssh_message_channel_request_open_reply_accept(msg)
    );
    ssh_message_free(msg);
    
    return chanObj;
}

Handle<Value> Msg::ChannelReplySuccess(const Arguments &args) {
    ssh_message msg = ObjectWrap::Unwrap<Msg>(args.This())->message;
    ssh_message_channel_request_reply_success(msg);
    ssh_message_free(msg);
    
    return args.This();
}
