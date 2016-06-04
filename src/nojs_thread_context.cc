#include <list>
#include <iostream>
#include <assert.h>

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
using v8::String;
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

namespace i {

void Print (const v8::FunctionCallbackInfo<Value>& info) {
  Isolate* isolate(info.GetIsolate());
  HandleScope scope(isolate);

  v8::String::Utf8Value output(info[0]);

  std::cout << *output << std::endl;
}

void InitializeBridgeObject (Isolate* isolate, Local<Context> context, Local<Object> bridge) {
  HandleScope handle_scope(isolate);

  Local<Object> sources = Object::New(isolate);
  int natives_count = NativesCollection::GetBuiltinsCount();

  for (int idx = 0; idx < natives_count; ++idx) {
    std::string name(NativesCollection::GetScriptName(idx));
    std::string src(NativesCollection::GetScriptSource(idx));
    v8::Maybe<bool> success = sources->Set(context, String::NewFromUtf8(
      isolate,
      name.data(),
      NewStringType::kNormal,
      name.size()
    ).ToLocalChecked(), String::NewFromUtf8(
      isolate,
      src.data(),
      NewStringType::kNormal,
      src.size()
    ).ToLocalChecked());

    if (!success.FromMaybe(false)) {
      abort();
    }
  }

  // bridge.sources = {}
  {
    v8::Maybe<bool> success = bridge->Set(context, String::NewFromUtf8(
      isolate,
      "sources",
      NewStringType::kNormal
    ).ToLocalChecked(), sources);

    if (!success.FromMaybe(false)) {
      abort();
    }
  }

  v8::Local<v8::Function> function = v8::FunctionTemplate::New(
    isolate,
    Print,
    Null(isolate),
    v8::Local<v8::Signature>()
  )->GetFunction();

  // bridge.print = fn
  {
    v8::Maybe<bool> success = bridge->Set(context, String::NewFromUtf8(
      isolate,
      "print",
      NewStringType::kNormal
    ).ToLocalChecked(), function);

    if (!success.FromMaybe(false)) {
      abort();
    }
  }
}

}

static std::list<ThreadContext*> threads;
static ArrayBufferAllocator allocator;

ThreadContext::ThreadContext() :
  m_loop(nullptr),
  m_isolate(nullptr) {
}


void ThreadContext::Initialize() {
  m_loop = reinterpret_cast<uv_loop_t*>(malloc(sizeof(uv_loop_t)));
  uv_loop_init(m_loop);
  uv_disable_stdio_inheritance();

  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = &allocator;
  m_isolate = Isolate::New(create_params);
}

void ThreadContext::Run() {
  Isolate::Scope isolate_scope(m_isolate);
  HandleScope handle_scope(m_isolate);
  Local<Context> context = Context::New(m_isolate);
  Context::Scope context_scope(context);

  const int main_idx {NativesCollection::GetIndex("main")};
  const std::string main_source {NativesCollection::GetScriptSource(main_idx)};

  const std::string main_name {NativesCollection::GetScriptName(main_idx)};

  Local<String> code = String::NewFromUtf8(
    m_isolate,
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
  Local<Object> bridge = Object::New(m_isolate);
  Local<Value> args[] = {bridge};
  assert(result->IsFunction());
  Local<Function> bootstrap_function = Local<Function>::Cast(result);

  i::InitializeBridgeObject(m_isolate, context, bridge);
  bootstrap_function->Call(Null(m_isolate), 1, args);

  uv_run(m_loop, UV_RUN_DEFAULT);
}

void ThreadContext::Dispose() {
  uv_run(m_loop, UV_RUN_DEFAULT);
  free(m_loop);
  m_isolate->Dispose();

  m_loop = nullptr;
  m_isolate = nullptr;
}

Isolate* ThreadContext::GetIsolate() {
  return m_isolate;
}

ThreadContext* ThreadContext::New() {
  ThreadContext* const thread{new ThreadContext()};
  thread->Initialize();
  threads.push_back(thread);
  return thread; 
}

}
