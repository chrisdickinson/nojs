#include "uv.h"
#include "v8.h"

#include "nojs_validate_arguments.h"
#include "nojs_thread_context_inl.h"
#include "nojs_object_inl.h"
#include "nojs_uv_request.h"
#include "nojs_string.h"

namespace NoJS {

void GetCWD(const v8::FunctionCallbackInfo<v8::Value>& args) {
  ThreadContext* tc = ThreadContext::From(args);
  v8::Isolate* isolate(tc->GetIsolate());
  v8::HandleScope scope(isolate);

#ifdef _WIN32
  char buf[MAX_PATH * 4];
#else
  char buf[PATH_MAX];
#endif

  size_t len = sizeof(buf);
  int err = uv_cwd(buf, &len);
  if (err) {
    isolate->ThrowException(v8::Exception::Error(
      v8::String::NewFromUtf8(
        isolate,
        uv_strerror(err),
        v8::NewStringType::kInternalized
      ).ToLocalChecked()
    ));
    return;
  }
  args.GetReturnValue().Set(
    v8::String::NewFromUtf8(
      isolate,
      buf,
      v8::String::kNormalString,
      len
    )
  );
}

namespace Misc {

void ContributeToBridge (ThreadContext* tc, v8::Local<v8::Object> bridge) {
  v8::Isolate* isolate(tc->GetIsolate());
  v8::HandleScope scope(isolate);

  tc->SetMethod(bridge, "cwd", GetCWD);
}

}

}
