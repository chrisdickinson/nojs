#include "uv.h"
#include "v8.h"

#include "nojs_validate_arguments.h"
#include "nojs_thread_context_inl.h"
#include "nojs_object_inl.h"
#include "nojs_uv_request.h"
#include "nojs_string.h"

namespace NoJS {

typedef Validation::And<
  Validation::Length<4>,
  Validation::And<
    Validation::And<
      Validation::Or<
        Validation::Slot<0, Validation::IsString>,
        Validation::Slot<0, Validation::IsArrayBufferView>
      >,
      Validation::Slot<1, Validation::IsString>
    >,
    Validation::And<
      Validation::Slot<2, Validation::IsNumber>,
      Validation::Slot<3, Validation::IsNumber>
    >
  >
> RunScriptValidator;

static v8::Local<v8::Value> CreateStringView(const v8::Local<v8::Value>& in) {
  return in;
}

// content, filename, line, column
void RunScriptInContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
  ThreadContext* tc = ThreadContext::From(args);
  v8::Isolate* isolate(tc->GetIsolate());
  v8::HandleScope scope(isolate);

  v8::MaybeLocal<v8::Value> err = RunScriptValidator::Check(tc, args);

  if (!err.IsEmpty()) {
    isolate->ThrowException(
      err.ToLocalChecked()
    );
    return;
  }

  v8::Local<v8::String> code = (
    args[0]->IsString()
    ? args[0]
    : CreateStringView(args[0])
  ).As<v8::String>();

  v8::ScriptOrigin origin(args[1], args[2].As<v8::Integer>(), args[3].As<v8::Integer>());
  v8::MaybeLocal<v8::Script> script = v8::Script::Compile(
    isolate->GetCurrentContext(),
    code,
    &origin
  );

  if (script.IsEmpty()) {
    args.GetReturnValue().Set(v8::Null(isolate));
    return;
  }

  v8::MaybeLocal<v8::Value> value = script.ToLocalChecked()->Run(
    isolate->GetCurrentContext()
  );

  if (value.IsEmpty()) {
    args.GetReturnValue().Set(v8::Null(isolate));
    return;
  }
  args.GetReturnValue().Set(value.ToLocalChecked());
}

namespace VM {

void ContributeToBridge (ThreadContext* tc, v8::Local<v8::Object> bridge) {
  v8::Isolate* isolate(tc->GetIsolate());
  v8::HandleScope scope(isolate);

  tc->SetMethod(bridge, "run", RunScriptInContext);
}

}

}
