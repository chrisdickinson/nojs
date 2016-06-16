#ifndef NOJS_VM_H
#define NOJS_VM_H
#include "v8.h"

namespace NoJS {

class ThreadContext;

namespace VM {

void ContributeToBridge (ThreadContext*, v8::Local<v8::Object>);

}

}

#endif
