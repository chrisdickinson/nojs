#ifndef NOJS_UV_REQUEST_H
#define NOJS_UV_REQUEST_H
#include "v8.h"
#include "nojs_async.h"
#include "nojs_object_inl.h"
#include "nojs_utils.h"
#include <iostream>

namespace NoJS {

template <typename T, AsyncType S>
class Request : public AsyncObject<S> {
  public:

    template <typename Target>
    static Target* New(ThreadContext*);
    inline virtual ~Request() {
    };
  protected:
    inline Request(
      ThreadContext* tc,
      v8::Local<v8::Object> val
    ) : AsyncObject<S>(tc, val) {
      m_request.data = this;
    };
    inline void Resolve(v8::Local<v8::Value>);
    inline void Reject(v8::Local<v8::Value>);
    T m_request;
};

template <typename T, AsyncType S>
void Request<T, S>::Resolve(v8::Local<v8::Value> val) {
  ThreadContext* tc = ObjectBase::GetThreadContext();
  v8::Isolate* isolate = tc->GetIsolate();
  v8::HandleScope scope(isolate);

  ObjectBase::GetJSObject().template As<v8::Promise::Resolver>()->Resolve(val);
  ObjectBase::MakeWeak<Request<T, S>>(this);
}

template <typename T, AsyncType S>
void Request<T, S>::Reject(v8::Local<v8::Value> val) {
  ThreadContext* tc = ObjectBase::GetThreadContext();
  v8::Isolate* isolate = tc->GetIsolate();
  v8::HandleScope scope(isolate);

  ObjectBase::GetJSObject().template As<v8::Promise::Resolver>()->Reject(val);
  ObjectBase::MakeWeak<Request<T, S>>(this);
}

template <typename T, AsyncType S>
template <typename Target>
Target* Request<T, S>::New(ThreadContext* tc) {
  v8::Isolate* isolate = tc->GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(isolate);
  Target* result = new Target(tc, resolver);

  ASSERT(result != nullptr);
  return result;
}

}

#endif
