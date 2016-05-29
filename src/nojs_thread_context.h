#ifndef NOJS_THREAD_H
#define NOJS_THREAD_H
#include "nojs_utils.h"

namespace v8 {
  class Isolate;
}
typedef struct uv_loop_s uv_loop_t;

namespace NoJS {
  class ThreadContext {
    public:
      static ThreadContext* New();
      void Dispose();
      void Run();
    private:
      ThreadContext();
      void Initialize();
      DISALLOW_COPY_AND_ASSIGN(ThreadContext);

      uv_loop_t* uv_loop;
      v8::Isolate* v8_isolate;
  };
}

#endif
