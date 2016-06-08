#ifndef NOJS_THREAD_CONTEXT_INL_H
#define NOJS_THREAD_CONTEXT_INL_H
#include "nojs_thread_context.h"

namespace NoJS {
  v8::Isolate* ThreadContext::GetIsolate() {
    return m_isolate;
  };

  uv_loop_t* ThreadContext::GetUVLoop() {
    return m_loop;
  };

  ThreadContext* ThreadContext::From(const v8::FunctionCallbackInfo<v8::Value>& args) {
    ASSERT(args.Data()->IsExternal());
    return static_cast<ThreadContext*>(
      args.Data().As<v8::External>()->Value()
    );
  }

  v8::Local<v8::FunctionTemplate> ThreadContext::NewFunctionTemplate(
    v8::FunctionCallback callback,
    v8::Local<v8::Signature> signature
  ) {
    return v8::FunctionTemplate::New(
      GetIsolate(),
      callback,
      PersistentToLocal(GetIsolate(), m_as_external),
      signature
    );
  }

  void ThreadContext::Set(
    v8::Local<v8::Object> lhs,
    const char* name,
    v8::Local<v8::Object> rhs
  ) {
    v8::Local<v8::String> name_obj = v8::String::NewFromUtf8(
      GetIsolate(),
      name,
      v8::NewStringType::kInternalized
    ).ToLocalChecked();
    lhs->Set(name_obj, rhs);
  };

  void ThreadContext::SetMethod(
    v8::Local<v8::Object> lhs,
    const char* name,
    v8::FunctionCallback callback
  ) {
    v8::Local<v8::Function> rhs = NewFunctionTemplate(callback)->GetFunction();
    v8::Local<v8::String> name_obj = v8::String::NewFromUtf8(
      GetIsolate(),
      name,
      v8::NewStringType::kInternalized
    ).ToLocalChecked();
    lhs->Set(name_obj, rhs);
    rhs->SetName(name_obj);
  };
}

#endif
