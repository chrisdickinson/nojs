#ifndef NOJS_ASYNC_H
#define NOJS_ASYNC_H
#include "nojs_object.h"

namespace NoJS {

enum class AsyncType {
  Unknown = 0,
  UVTimer = 1,
  UVFSRequest = 2
};

template <AsyncType Type>
class AsyncObject : public ObjectBase {
  public:
    inline AsyncObject(ThreadContext* tc, v8::Local<v8::Object> val)
      : ObjectBase(tc, val) {
    };
    inline virtual ~AsyncObject() {
    };

  private:
    static const AsyncType kType = Type;
};

}

#endif
