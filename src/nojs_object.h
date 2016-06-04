#ifndef NOJS_OBJECT_H
#define NOJS_OBJECT_H

#include "v8.h"

namespace NoJS {

class ThreadContext;

// Object<T>
// ============================================================
// A tracked reference to a v8 value (type represented by `T`.)
template <typename T>
class Object {
  public:
    inline Object(ThreadContext*, v8::Local<T> handle);
    inline virtual ~Object();

    inline v8::Local<T> GetJSObject();
    inline v8::Persistent<T>& GetPersistent();
    inline ThreadContext* GetThreadContext() const;


    template <typename Type>
    inline void AssociateWeak(Type* ptr);
    inline void Disassociate();
  private:
    Object();

    template <typename Type>
    static inline void OnGCCallback(const v8::WeakCallbackInfo<Type>& data);

    v8::Persistent<T> m_js_object;
    ThreadContext* m_thread_context;
};


}

#endif
