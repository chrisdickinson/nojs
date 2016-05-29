#include <list>
#include <iostream>

#include "v8-debug.h"
#include "v8-profiler.h"
#include "uv.h"

#include "nojs_thread_context.h"
#include "nojs_natives.h"

using v8::Local;
using v8::HandleScope;
using v8::Context;
using v8::Isolate;
using v8::String;
using v8::Value;
using v8::Function;
using v8::Object;
using v8::NewStringType;
using v8::Null;

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
  public:
    void* Allocate(size_t len) override {
      return AllocateUninitialized(len);
    }
    void* AllocateUninitialized(size_t len) override {
      return malloc(len);
    }
    void Free(void* data, size_t) override {
      free(data);
    }
};

namespace NoJS {

static std::list<ThreadContext*> threads;
static ArrayBufferAllocator allocator;

ThreadContext::ThreadContext() :
  uv_loop(nullptr),
  v8_isolate(nullptr) {
}

void ThreadContext::Initialize() {
  uv_loop = reinterpret_cast<uv_loop_t*>(malloc(sizeof(uv_loop_t)));
  uv_loop_init(uv_loop);
  uv_disable_stdio_inheritance();

  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = &allocator;
  v8_isolate = Isolate::New(create_params);
}

void ThreadContext::Run() {
  Isolate::Scope isolate_scope(v8_isolate);
  HandleScope handle_scope(v8_isolate);
  Local<Context> context = Context::New(v8_isolate);
  Context::Scope context_scope(context);

  const int main_idx {NativesCollection::GetIndex("main")};
  const std::vector<const char> main_source {NativesCollection::GetScriptSource(main_idx)};

  const std::vector<const char> main_name {NativesCollection::GetScriptName(main_idx)};

  Local<String> code = String::NewFromUtf8(
    v8_isolate,
    main_source.data(),
    NewStringType::kNormal,
    main_source.size()
  ).ToLocalChecked();
  v8::ScriptCompiler::Source source(code);

  Local<v8::Script> script = v8::ScriptCompiler::Compile(
    context,
    &source
  ).ToLocalChecked();

  Local<Value> result = script->Run(context).ToLocalChecked();
  Local<Object> bridge = Object::New(v8_isolate);
  Local<Value> args[] = {bridge};
  assert(result->IsFunction());
  Local<Function> bootstrap_function = Local<Function>::Cast(result);
  bootstrap_function->Call(Null(v8_isolate), 1, args);

  uv_run(uv_loop, UV_RUN_DEFAULT);
}

void ThreadContext::Dispose() {
  uv_run(uv_loop, UV_RUN_DEFAULT);
  free(uv_loop);
  v8_isolate->Dispose();

  uv_loop = nullptr;
  v8_isolate = nullptr;
}

ThreadContext* ThreadContext::New() {
  ThreadContext* const thread{new ThreadContext()};
  thread->Initialize();
  threads.push_back(thread);
  return thread; 
}

}
