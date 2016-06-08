#ifndef NOJS_UV_HANDLE_H
#define NOJS_UV_HANDLE_H
#include "nojs_async.h"

namespace NoJS {

template <AsyncSource S>
class Handle : public AsyncObject<S> {
  public:
  private:
    uv_handle_t* m_handle;
};

}

#endif
