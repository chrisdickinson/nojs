#ifndef NOJS_FS_H
#define NOJS_FS_H
#include "v8.h"

namespace NoJS {

class ThreadContext;

namespace FS {

void ContributeToBridge (ThreadContext*, v8::Local<v8::Object>);

}

}

#endif
