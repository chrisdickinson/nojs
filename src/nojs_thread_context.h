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

      inline v8::Isolate* GetIsolate();
    private:
      ThreadContext();
      void Initialize();
      DISALLOW_COPY_AND_ASSIGN(ThreadContext);

      uv_loop_t* m_loop;
      v8::Isolate* m_isolate;
  };
}

#endif
