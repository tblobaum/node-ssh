#include "sshd.h"
#include <libssh/callbacks.h>

static int minit_cb (void **) {
    printf("minit\n"); fflush(stdout);
    return 0;
}

static int mdestroy_cb (void **) {
    printf("mdestroy\n"); fflush(stdout);
    return 0;
}

static int mlock_cb (void **) {
    printf("mlock\n"); fflush(stdout);
    return 0;
}

static int munlock_cb (void **) {
    printf("munlock\n"); fflush(stdout);
    return 0;
}

static unsigned long threads_id_cb (void) {
    return 1;
}

extern "C" void init(Handle<Object> target) {
    ssh_threads_callbacks_struct *cbs = new ssh_threads_callbacks_struct;
    
    cbs->mutex_init = minit_cb;
    cbs->mutex_destroy = mdestroy_cb;
    cbs->mutex_lock = mlock_cb;
    cbs->mutex_unlock = munlock_cb;
    cbs->thread_id = threads_id_cb;
    
    ssh_threads_set_callbacks(cbs);
    ssh_init();
    
    HandleScope scope;
    SSHD::Initialize(target);
}
