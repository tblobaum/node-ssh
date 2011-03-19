#include "chan.h"

Persistent<FunctionTemplate> Chan::constructor_template;

Chan::Chan(ssh_channel chan) {
    channel = chan;
    done = false;
}

void Chan::Initialize() {
    HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->Inherit(EventEmitter::constructor_template);
    constructor_template->SetClassName(String::NewSymbol("Channel"));
}

Handle<Value> Chan::New(const Arguments &args) {
    HandleScope scope;
    
    return args.This();
}

Handle<Value> Chan::New(ssh_channel chan) {
    HandleScope scope;
    
    Chan *c = new Chan(chan);
    Local<Object> obj = Chan::constructor_template
        ->GetFunction()->NewInstance();
    c->Wrap(obj);
    
    eio_custom(ReadChannel, EIO_PRI_DEFAULT, ReadChannelAfter, c);
    
    //obj->Ref();
    return obj;
}

int Chan::ReadChannel(eio_req *req) {
    Chan *chan = (Chan *) req->data;
    char *buf = new char[2048];
    int i = ssh_channel_read(chan->channel, buf, 2048, 0);
    if (i == 0) chan->done = true;
    
    std::pair<int, char *> pair(i, buf);
    chan->buffers.push_back(pair);
    
    return 0;
}

int Chan::ReadChannelAfter(eio_req *req) {
    HandleScope scope;
    
    Chan *chan = (Chan *) req->data;
    if (!chan->done) {
        eio_custom(ReadChannel, EIO_PRI_DEFAULT, ReadChannelAfter, chan);
    }
    
    while (!chan->buffers.empty()) {
        std::pair<int, char *> p = chan->buffers.front();
        int i = p.first;
        if (i == 0) {
            chan->Emit(String::NewSymbol("end"), 0, NULL);
        }
        else {
            char *buf = p.second;
            Handle<Object> bufObj;
            Buffer::New(p.second, i)->Wrap(bufObj);;
            
            Handle<Value> argv[1];
            argv[0] = bufObj;
            chan->Emit(String::NewSymbol("data"), 1, argv);
            delete p.second;
        }
    }
    
    return 0;
}
