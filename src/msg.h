#ifndef NODE_SSH_MESSAGE_H
#define NODE_SSH_MESSAGE_H

#define protected public

#include <node.h>
#include <node_object_wrap.h>
#include <node_events.h>
#include <v8.h>

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>

using namespace v8;
using namespace node;

class Msg : public ObjectWrap {
public:
    ssh_message message;
    Msg();
    void prepare(ssh_message, Persistent<Object>);
    
    static Persistent<FunctionTemplate> constructor_template;
    static Handle<Value> New(const Arguments &);
    static Handle<Value> ReplyDefault(const Arguments &);
    static Handle<Value> AuthSetMethods(const Arguments &);
    static void Initialize();
};
    
#endif
