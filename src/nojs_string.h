#ifndef NOJS_STRING_H
#define NOJS_STRING_H
#include "v8.h"

namespace NoJS {

class BufferValue {
  public:
    explicit BufferValue(v8::Isolate*, v8::Local<v8::Value>);

    enum State {
      OK=0,
      FAIL=1
    };

    ~BufferValue() {
      if (m_str != m_buf) {
        free(m_str);
      }
    }

    const char* Contents() const {
      return m_state == OK ? m_str : nullptr;
    };
  private:
    char* m_str;
    char m_buf[1024];
    State m_state;
};

}

#endif
