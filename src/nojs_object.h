#ifndef NOJS_OBJECT_H
#define NOJS_OBJECT_H

#include "v8.h"

namespace NoJS {

class ThreadContext;

// Object
// ============================================================
// A tracked reference to a v8 object.
class Object {
  public:
    inline Object(ThreadContext*, v8::Local<v8::Object> handle);
    inline virtual ~Object();

    inline v8::Local<v8::Object> GetJSObject();
    inline v8::Persistent<v8::Object>& GetPersistent();
    inline ThreadContext* GetThreadContext() const;


    template <typename Type>
    inline void AssociateWeak(Type* ptr);
    inline void Disassociate();
  private:
    Object();

    template <typename Type>
    static inline void OnGCCallback(const v8::WeakCallbackInfo<Type>& data);

    v8::Persistent<v8::Object> m_js_object;
    ThreadContext* m_thread_context;
};

}

#endif
