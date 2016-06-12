#include "nojs_string.h"
#include "nojs_utils.h"

namespace NoJS {

BufferValue::BufferValue(v8::Isolate* isolate, v8::Local<v8::Value> value)
  : m_str(m_buf),
    m_state(FAIL) {
  if (value.IsEmpty()) {
    return;
  }

  // assume utf8 for strings
  if (value->IsString()) {
    v8::String::Utf8Value string(value);
    size_t len = string.length();
    if (len > sizeof(m_buf)) {
      m_str = static_cast<char*>(malloc(len + 1));
      ASSERT(m_str != nullptr);
    }

    memcpy(m_str, *string, len);
    m_str[len] = '\0';
    m_state = OK;
  }

  if (value->IsArrayBufferView()) {
    v8::HandleScope scope(isolate);
    v8::Local<v8::ArrayBufferView> ab = value.As<v8::ArrayBufferView>();
    size_t offs = ab->ByteOffset();
    size_t len = ab->ByteLength();
    if (len > sizeof(m_buf)) {
      m_str = static_cast<char*>(malloc(len + 1));
      ASSERT(m_str != nullptr);
    }

    memcpy(
      m_str,
      static_cast<char*>(ab->Buffer()->GetContents().Data()) + offs,
      len
    );
    m_str[len - 1] = '\0';
    m_state = OK;
  }
}

}
