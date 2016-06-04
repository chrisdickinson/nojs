#ifndef NOJS_OBJECT_INL_H
#define NOJS_OBJECT_INL_H
#include "nojs_thread_context.h"
#include "nojs_thread_context_inl.h"
#include "nojs_object.h"
#include "nojs_utils.h"
#include "v8.h"

namespace NoJS {

inline Object::Object(ThreadContext* ctxt, v8::Local<v8::Object> handle)
  : m_js_object(ctxt->GetIsolate(), handle),
    m_thread_context(ctxt) {
  ASSERT(m_js_object.IsEmpty() == false);
}

inline Object::~Object() {
  ASSERT(m_js_object.IsEmpty());
}

inline v8::Persistent<v8::Object>& Object::GetPersistent() {
  return m_js_object;
}

inline ThreadContext* Object::GetThreadContext() const {
  return m_thread_context;
}

template <typename Type>
inline void Object::OnGCCallback(const v8::WeakCallbackInfo<Type>& data) {
  Type* self = data.GetParameter();
  self->GetPersistent().Reset();
  delete self;
}

template <typename Type>
inline void Object::AssociateWeak(Type* ptr) {
  v8::HandleScope scope(m_thread_context->GetIsolate());
  v8::Local<v8::Object> handle = GetJSObject();
  ASSERT(handle->InternalFieldCount() > 0);
  handle->SetAlignedPointerInInternalField(0, ptr);
  m_js_object.MarkIndependent();
  m_js_object.SetWeak<Type>(
    ptr,
    OnGCCallback<Type>,
    v8::WeakCallbackType::kParameter
  );
}

inline void Object::Disassociate() {
  m_js_object.ClearWeak();
}

}

#endif
