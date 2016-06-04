#ifndef NOJS_OBJECT_INL_H
#define NOJS_OBJECT_INL_H
#include "nojs_object.h"
#include "v8.h"

namespace NoJS {

template <typename T>
inline Object<T>::Object(ThreadContext* ctxt, v8::Local<T> handle)
  : m_thread_context(ctxt),
  m_js_object(handle) {
  ASSERT(m_js_object.IsEmpty() == false);
}

template <typename T>
inline Object<T>::~Object() {
  ASSERT(m_js_object.IsEmpty());
}

template <typename T>
inline v8::Persistent<T>& Object<T>::GetPersistent() {
  return m_js_object;
}

template <typename T>
inline ThreadContext* Object<T>::GetThreadContext() const {
  return m_thread_context;
}

template <typename T>
template <typename Type>
inline void Object<T>::OnGCCallback(const v8::WeakCallbackInfo<Type>& data) {
  Type* self = data.GetParameter();
  self->GetPersistent().Reset();
  delete self;
}

template <typename T>
template <typename Type>
inline void Object<T>::AssociateWeak(Type* ptr) {
  v8::HandleScope scope(m_thread_context->GetIsolate());
  v8::Local<T> handle = GetJSObject();
  ASSERT(handle->InternalFieldCount() > 0);
  handle->SetAlignedPointerInInternalField(0, ptr);
  m_js_object.MarkIndependent();
  m_js_object.template SetWeak<Type>(
    ptr,
    OnGCCallback<Type>,
    v8::WeakCallbackType::kParameter
  );
}

template <typename T>
inline void Object<T>::Disassociate() {
  m_js_object.ClearWeak();
}

}

#endif
