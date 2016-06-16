#ifndef NOJS_VALIDATE_ARGUMENTS_H
#define NOJS_VALIDATE_ARGUMENTS_H
#include "nojs_thread_context_inl.h"
#include "v8.h"
#include <string>

namespace NoJS {

namespace Validation {

#define CreateError(isolate, Type, msg)   \
  v8::MaybeLocal<v8::Value>(              \
    Type(v8::String::NewFromUtf8(         \
      isolate,                            \
      msg.c_str(),                        \
      v8::NewStringType::kInternalized    \
    ).ToLocalChecked())                   \
  )

template <typename Head, typename Tail>
class And {
  public:
    inline static v8::MaybeLocal<v8::Value> Check(
      ThreadContext* tc,
      const v8::FunctionCallbackInfo<v8::Value>& val
    ) {
      v8::MaybeLocal<v8::Value> err = Head::Check(tc, val);
      if (err.IsEmpty()) {
        return Tail::Check(tc, val);
      }
      return err;
    };
};

template <typename Head, typename Tail>
class Or {
  public:
    inline static v8::MaybeLocal<v8::Value> Check(
      ThreadContext* tc,
      const v8::FunctionCallbackInfo<v8::Value>& val
    ) {
      v8::MaybeLocal<v8::Value> err = Head::Check(tc, val);
      if (err.IsEmpty()) {
        return err;
      }
      return Tail::Check(tc, val);
    };
};

template<int N>
class Length {
  public:
    inline static v8::MaybeLocal<v8::Value> Check(
      ThreadContext* tc,
      const v8::FunctionCallbackInfo<v8::Value>& args
    ) {
      if (args.Length() >= N) {
        return v8::MaybeLocal<v8::Value>();
      }

      std::string msg(
        std::string("Invalidate number of arguments. Got ") +
        std::to_string(args.Length()) +
        std::string(", expected ") +
        std::to_string(N)
      );

      return CreateError(tc->GetIsolate(), v8::Exception::RangeError, msg);
    }
};

namespace Is {
#define FORWARD_TO_VALUE(V) \
  V(Undefined)              \
  V(Null)                   \
  V(True)                   \
  V(False)                  \
  V(Name)                   \
  V(String)                 \
  V(Symbol)                 \
  V(Function)               \
  V(Array)                  \
  V(Object)                 \
  V(Boolean)                \
  V(Number)                 \
  V(External)               \
  V(Int32)                  \
  V(Uint32)                 \
  V(Date)                   \
  V(ArgumentsObject)        \
  V(BooleanObject)          \
  V(NumberObject)           \
  V(StringObject)           \
  V(SymbolObject)           \
  V(NativeError)            \
  V(RegExp)                 \
  V(GeneratorFunction)      \
  V(GeneratorObject)        \
  V(Promise)                \
  V(Map)                    \
  V(Set)                    \
  V(WeakMap)                \
  V(WeakSet)                \
  V(ArrayBuffer)            \
  V(ArrayBufferView)        \
  V(TypedArray)             \
  V(Uint8Array)

#define V(XS)                                             \
inline static bool XS(const v8::Local<v8::Value>& val) {  \
  return val->Is ## XS();                                 \
}
FORWARD_TO_VALUE(V)
#undef V
}

template<int Pos, typename Validator>
class Slot {
  public:
    inline static v8::MaybeLocal<v8::Value> Check(
      ThreadContext* tc,
      const v8::FunctionCallbackInfo<v8::Value>& args
    ) {
      return Validator::Check(tc, args[Pos], Pos);
    }
};

template<bool (*Member)(const v8::Local<v8::Value>&), typename Name>
class Type {
  public:
    inline static v8::MaybeLocal<v8::Value> Check(
      ThreadContext* tc,
      const v8::Local<v8::Value>& arg,
      const int Pos = 0
    ) {
      bool check = Member(arg);
      if (check) {
        return v8::MaybeLocal<v8::Value>();
      }

      std::string msg(
        std::string("Expected argument #") +
        std::to_string(Pos) +
        std::string(" to be of type ") +
        std::string(Name::Value)
      );

      return CreateError(tc->GetIsolate(), v8::Exception::TypeError, msg);
    }
};

#define V(XS)                                 \
class _Name_ ## XS {                          \
  public:                                     \
    static const char* Value;                 \
};                                            \
typedef Type<Is:: XS, _Name_ ## XS> Is ## XS;
FORWARD_TO_VALUE(V)
#undef V
}

}

#endif
