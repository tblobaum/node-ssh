#ifndef NODE_SSH_CONSTANTS_H
#define NODE_SSH_CONSTANTS_H

#include <node.h>
#include <v8.h>
#include <libssh/libssh.h>

using namespace v8;
using namespace node;

Persistent<Object> Constants();

#endif
