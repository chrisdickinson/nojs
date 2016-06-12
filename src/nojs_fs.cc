#include "uv.h"
#include "v8.h"

#include "nojs_thread_context_inl.h"
#include "nojs_object_inl.h"
#include "nojs_uv_request.h"
#include "nojs_string.h"

namespace NoJS {

using v8::Local;
using v8::Value;
using v8::HandleScope;
using v8::FunctionCallbackInfo;
using v8::Promise;

namespace fsops {
  typedef void (*OnCompleteCallback)(uv_fs_t*);

  struct Resolution {
    Local<Value> value;
    enum Kind {
      REJECT,
      RESOLVE
    } kind;
  };


  class Open {
    public:
      inline static int Execute(
        ThreadContext* tc,
        const FunctionCallbackInfo<Value>& args,
        uv_fs_t* raw_request,
        OnCompleteCallback cb
      ) {

        int len = args.Length();
        // open(path : ArrayBuffer, flags : int, mode : int)
        switch (len) {
          case 0:
          case 1:
            return -1;
          default:
          case 2:
            if (!args[1]->IsInt32() || !args[2]->IsInt32()) {
              return -1;
            }
            break;
        }
        int flags = args[1]->Int32Value();
        int mode = args[2]->Int32Value();
        const char* path = BufferValue(tc->GetIsolate(), args[0]).Contents();
        int err = uv_fs_open(tc->GetUVLoop(), raw_request, path, flags, mode, cb);
        return err;
      };
      inline static Resolution OnComplete(ThreadContext* tc, uv_fs_t* raw_req) {
        if (raw_req->result < 0) {
          return Resolution{
            .kind=Resolution::REJECT,
            .value=v8::Integer::New(tc->GetIsolate(), raw_req->result)
          };
        }

        return Resolution{
          .kind=Resolution::RESOLVE,
          .value=v8::Integer::New(tc->GetIsolate(), raw_req->result)
        };
      };
  };

}

template <typename FSOperation>
class FSRequest : public Request<uv_fs_t, AsyncType::UVFSRequest> {
  public:
    typedef Request<uv_fs_t, AsyncType::UVFSRequest> Parent;

    Local<v8::Value> Execute(const FunctionCallbackInfo<Value>& args) {
      Local<v8::Promise::Resolver> resolver = GetJSObject().template As<v8::Promise::Resolver>();
      Local<v8::Promise> promise = resolver->GetPromise();
      int err = FSOperation::Execute(GetThreadContext(), args, &m_request, After);

      if (err < 0) {
        uv_fs_t* uv_req = &m_request;
        uv_req->result = err;
        uv_req->path = nullptr;
        After(uv_req);
      }

      return promise;
    }

    static FSRequest<FSOperation>* New(ThreadContext* tc) {
      return Parent::New<FSRequest<FSOperation> >(tc);
    };

  private:
    friend Parent;
    FSRequest(ThreadContext* tc, Local<v8::Object> val)
      : Request<uv_fs_t, AsyncType::UVFSRequest>(tc, val) {
    };
    static void After(uv_fs_t* raw_req) {
      FSRequest<FSOperation>* request = static_cast<FSRequest<FSOperation>*>(
        raw_req->data);
      ThreadContext* tc = request->GetThreadContext();
      HandleScope handle_scope(tc->GetIsolate());
      fsops::Resolution res(FSOperation::OnComplete(tc, raw_req));

      switch (res.kind) {
        case fsops::Resolution::RESOLVE:
          request->Resolve(res.value);
        break;
        case fsops::Resolution::REJECT:
          request->Reject(res.value);
        break;
      }
    }
};

template <typename FSOperation>
class FSRequestSync {
  public:
    static FSRequestSync<FSOperation>* New(ThreadContext* tc) {
      return new FSRequestSync<FSOperation>(tc);
    };

    Local<v8::Value> Execute(const FunctionCallbackInfo<Value>& args) {
      int err = FSOperation::Execute(m_context, args, &m_request, nullptr);

      if (err < 0) {
        m_request.result = err;
        m_request.path = nullptr;
      }
      return OnReady();
    }

  private:
    FSRequestSync<FSOperation>(ThreadContext* context)
      : m_context(context) {

    }

    Local<v8::Value> OnReady() {
      fsops::Resolution res(FSOperation::OnComplete(m_context, &m_request));

      if (res.kind == fsops::Resolution::RESOLVE) {
        return res.value;
      }

      m_context->GetIsolate()->ThrowException(
        v8::Exception::Error(v8::String::NewFromUtf8(
          m_context->GetIsolate(),
          "Things are okay.",
          v8::NewStringType::kInternalized
        ).ToLocalChecked())
      );
      return v8::Null(m_context->GetIsolate());
    }

    uv_fs_t m_request;
    ThreadContext* m_context;
};

namespace FS {

template <typename T>
void BindRequest (const v8::FunctionCallbackInfo<Value>& args) {
  ThreadContext* tc = ThreadContext::From(args);
  v8::Isolate* isolate(tc->GetIsolate());
  HandleScope scope(isolate);
  T* open = T::New(tc);
  Local<v8::Value> obj = open->Execute(args);
  return args.GetReturnValue().Set(obj);
}

void ContributeToBridge (ThreadContext* tc, v8::Local<v8::Object> bridge) {
  v8::Isolate* isolate(tc->GetIsolate());
  HandleScope scope(isolate);

  tc->SetMethod(bridge, "fsOpen", BindRequest<FSRequest<fsops::Open>>);
  tc->SetMethod(bridge, "fsOpenSync", BindRequest<FSRequestSync<fsops::Open>>);
}

}

}
