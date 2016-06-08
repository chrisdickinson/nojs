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
    enum {
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
        HandleScope handle_scope(tc->GetIsolate());

        Local<Value> val = v8::Null(tc->GetIsolate());
        return Resolution{.kind=Resolution::REJECT, .value=val};
      };
  };

}

template <typename FSOperation>
class FSRequest : public Request<uv_fs_t, AsyncType::UVFSRequest> {
  public:
    typedef Request<uv_fs_t, AsyncType::UVFSRequest> Parent;

    void Execute(const FunctionCallbackInfo<Value>& args) {
      int err = FSOperation::Execute(GetThreadContext(), args, &m_request, After);

      if (err < 0) {
        uv_fs_t* uv_req = &m_request;
        uv_req->result = err;
        uv_req->path = nullptr;
        After(uv_req);
      }
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
      delete request;
    }
};

class SyncFSRequest {
  public:
  private:
    static void After(uv_fs_t* raw_req) {
    }
};

namespace FS {

void JSOpen (const v8::FunctionCallbackInfo<Value>& args) {
  ThreadContext* tc = ThreadContext::From(args);
  v8::Isolate* isolate(tc->GetIsolate());
  HandleScope scope(isolate);
  FSRequest<fsops::Open>* open = FSRequest<fsops::Open>::New(tc);
  open->Execute(args);
  return args.GetReturnValue().Set(open->GetJSObject());
}

void ContributeToBridge (ThreadContext* tc, v8::Local<v8::Object> bridge) {
  v8::Isolate* isolate(tc->GetIsolate());
  HandleScope scope(isolate);

  tc->SetMethod(bridge, "fsOpen", JSOpen);

}

}

}
