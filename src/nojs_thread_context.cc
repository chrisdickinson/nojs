#include <list>
#include <iostream>
#include <assert.h>

#include "v8-debug.h"
#include "v8-profiler.h"
#include "uv.h"

#include "nojs_thread_context.h"
#include "nojs_thread_context_inl.h"
#include "nojs_natives.h"
#include "nojs_utils_inl.h"
#include "nojs_constants.h"
#include "nojs_fs.h"

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
  ThreadContext* tc = ThreadContext::From(info);
  Isolate* isolate(tc->GetIsolate());
  HandleScope scope(isolate);

  v8::String::Utf8Value output(info[0]);
  std::cout << *output << std::endl;
}

void InitializeBridgeObject (ThreadContext* tc, Local<Context> context, Local<Object> bridge) {
  Isolate* isolate = tc->GetIsolate();
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

  // bridge.print = fn
  tc->SetMethod(bridge, "print", Print);
  FS::ContributeToBridge(tc, bridge);
  Constants::ContributeToBridge(tc, bridge);
}

}

static std::list<ThreadContext*> threads;
static ArrayBufferAllocator allocator;

ThreadContext::ThreadContext(
  v8::Isolate* isolate,
  v8::Local<v8::Context> context,
  uv_loop_t* loop) :
  m_loop(loop),
  m_isolate(isolate),
  m_context(isolate, context) {
  HandleScope handle_scope(m_isolate);
  Context::Scope context_scope(context);
  context->SetAlignedPointerInEmbedderData(kContextEmbedderDataIndex, this);
  m_as_external.Reset(m_isolate, v8::External::New(m_isolate, this));
}

ThreadContext::~ThreadContext() {
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
  Local<Context> context = PersistentToLocal(m_isolate, m_context);
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

  i::InitializeBridgeObject(this, context, bridge);
  bootstrap_function->Call(Null(m_isolate), 1, args);

  {
    v8::SealHandleScope seal(m_isolate);
    bool more = false;
    // initialize -> run uv -> run microtasks -> run uv (check for more?)

    uv_run(m_loop, UV_RUN_ONCE);

    do {
      m_isolate->RunMicrotasks();
      more = uv_run(m_loop, UV_RUN_ONCE);
    } while (more == true);
  }
  uv_run(m_loop, UV_RUN_DEFAULT);
}

void ThreadContext::Dispose() {
  uv_run(m_loop, UV_RUN_DEFAULT);
  free(m_loop);
  m_isolate->Dispose();

  m_loop = nullptr;
  m_isolate = nullptr;
}

ThreadContext* ThreadContext::New() {
  uv_loop_t* loop = reinterpret_cast<uv_loop_t*>(malloc(sizeof(uv_loop_t)));
  uv_loop_init(loop);
  uv_disable_stdio_inheritance();

  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = &allocator;
  Isolate* isolate = Isolate::New(create_params);
  Isolate::Scope isolate_scope(isolate);
  HandleScope handle_scope(isolate);

  ThreadContext* const thread(new ThreadContext(
    isolate,
    Context::New(isolate),
    loop
  ));

  threads.push_back(thread);
  return thread;
}

}
