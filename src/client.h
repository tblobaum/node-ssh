#ifndef NODE_SSH_CLIENT_H
#define NODE_SSH_CLIENT_H

#include <node.h>
#include <node_object_wrap.h>
#include <node_events.h>
#include <v8.h>

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>

#include <deque>

using namespace v8;
using namespace node;

struct Client : EventEmitter {
    ssh_session session;
    std::deque<ssh_message> messageQueue;
    Client();
    
    static Persistent<FunctionTemplate> constructor_template;
    static Handle<Value> New(const Arguments &);
    
    static void Initialize();
    static Handle<Value> End(const Arguments &);
    
    static int Message(eio_req *);
    static int MessageAfter(eio_req *);
};
    
#endif
