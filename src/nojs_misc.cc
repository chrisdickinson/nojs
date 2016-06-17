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

class SingleByteStringView : public v8::String::ExternalOneByteStringResource, public ObjectBase {
  public:
    SingleByteStringView(ThreadContext* tc, v8::Local<v8::ArrayBufferView> hnd)
      : v8::String::ExternalOneByteStringResource(),
        ObjectBase(tc, hnd) {

      v8::Local<v8::ArrayBuffer> ab(hnd->Buffer());
      m_length = hnd->ByteLength();
      m_data = static_cast<char*>(ab->GetContents().Data()) + hnd->ByteOffset();
    }

    ~SingleByteStringView() override {
    }

    void Dispose() override {
      MakeWeak<SingleByteStringView>(this);
    }

    const char* data() const override {
      return m_data;
    }

    size_t length() const override {
      return m_length;
    }
  private:
    char* m_data;
    size_t m_length;
}; 

void CreateStringView(const v8::FunctionCallbackInfo<v8::Value>& args) {
  ThreadContext* tc = ThreadContext::From(args);
  v8::Isolate* isolate(tc->GetIsolate());
  v8::HandleScope scope(isolate);

  v8::MaybeLocal<v8::Value> err = Validation::And<
    Validation::Length<1>,
    Validation::Slot<0, Validation::IsArrayBufferView>
  >::Check(tc, args);

  if (!err.IsEmpty()) {
    isolate->ThrowException(err.ToLocalChecked());
    return;
  }

  SingleByteStringView* resource = new SingleByteStringView(
    tc,
    args[0].As<v8::ArrayBufferView>()
  );

  v8::MaybeLocal<v8::String> str = v8::String::NewExternal(
    isolate,
    resource
  );

  if (str.IsEmpty()) {
    isolate->ThrowException(
      v8::Exception::Error(v8::String::NewFromUtf8(
        isolate,
        "Could not create string view.",
        v8::NewStringType::kInternalized
      ).ToLocalChecked())
    );
    return;
  }

  args.GetReturnValue().Set(str.ToLocalChecked());
}

namespace Misc {

void ContributeToBridge (ThreadContext* tc, v8::Local<v8::Object> bridge) {
  v8::Isolate* isolate(tc->GetIsolate());
  v8::HandleScope scope(isolate);

  tc->SetMethod(bridge, "cwd", GetCWD);
  tc->SetMethod(bridge, "createStringView", CreateStringView);
}

}

}
