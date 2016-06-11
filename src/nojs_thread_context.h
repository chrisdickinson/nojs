#ifndef NOJS_THREAD_H
#define NOJS_THREAD_H
#include "nojs_utils.h"
#include "v8.h"

typedef struct uv_loop_s uv_loop_t;
#ifndef NOJS_CONTEXT_EMBEDDER_DATA_INDEX
#define NOJS_CONTEXT_EMBEDDER_DATA_INDEX 32
#endif

namespace NoJS {
  class ThreadContext {
    public:
      static ThreadContext* New();
      void Dispose();
      void Run();

      inline v8::Isolate* GetIsolate();
      inline uv_loop_t* GetUVLoop();

      inline v8::Local<v8::FunctionTemplate> NewFunctionTemplate(
        v8::FunctionCallback,
        v8::Local<v8::Signature> = v8::Local<v8::Signature>()
      );
      inline void SetMethod(v8::Local<v8::Object>, const char*, v8::FunctionCallback);
      inline void Set(v8::Local<v8::Object>, const char*, v8::Local<v8::Value>);

      inline static ThreadContext* From(const v8::FunctionCallbackInfo<v8::Value>&);
      static const int kContextEmbedderDataIndex = NOJS_CONTEXT_EMBEDDER_DATA_INDEX;
    private:
      ThreadContext(v8::Isolate*, v8::Local<v8::Context>, uv_loop_t*);
      ~ThreadContext();
      void Initialize();
      DISALLOW_COPY_AND_ASSIGN(ThreadContext);

      uv_loop_t* m_loop;
      v8::Isolate* m_isolate;
      v8::Persistent<v8::Context> m_context;
      v8::Persistent<v8::External> m_as_external;
  };
}

#endif
