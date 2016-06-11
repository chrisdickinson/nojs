#ifndef NOJS_CONSTANTS_H
#define NOJS_CONSTANTS_H
#include "v8.h"

namespace NoJS {

class ThreadContext;

namespace Constants {

void ContributeToBridge (ThreadContext*, v8::Local<v8::Object>);

}

}

#endif

