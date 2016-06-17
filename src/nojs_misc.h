#ifndef NOJS_MISC_H
#define NOJS_MISC_H
#include "v8.h"

namespace NoJS {

class ThreadContext;

namespace Misc {

void ContributeToBridge (ThreadContext*, v8::Local<v8::Object>);

}

}

#endif
