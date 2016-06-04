#ifndef NOJS_THREAD_CONTEXT_INL_H
#define NOJS_THREAD_CONTEXT_INL_H
#include "nojs_thread_context.h"

namespace NoJS {
  inline v8::Isolate* ThreadContext::GetIsolate() {
    return m_isolate;
  };
}

#endif
