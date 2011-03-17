#include "sshd.h"

extern "C" void init(Handle<Object> target) {
    HandleScope scope;
    SSHD::Initialize(target);
}
