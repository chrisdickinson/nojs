#include "uv.h"
#include "v8.h"

#include "nojs_validate_arguments.h"
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

  template <typename Op>
  class BasicOperation {
    public:
      inline static v8::MaybeLocal<v8::Value> Execute(
        ThreadContext* tc,
        const FunctionCallbackInfo<Value>& args,
        uv_fs_t* raw_request,
        OnCompleteCallback cb
      ) {

        v8::MaybeLocal<Value> err = Op::Validator::Check(tc, args);
        if (!err.IsEmpty()) {
          return err;
        }

        int cerr = Op::Execute(tc, args, raw_request, cb);

        if (cerr < 0) {
          return CreateError(
            tc->GetIsolate(),
            v8::Exception::Error,
            std::string(uv_strerror(cerr))
          );
        }

        return v8::MaybeLocal<v8::Value>();
      };

      inline static Resolution OnComplete(ThreadContext* tc, uv_fs_t* raw_req) {
        if (raw_req->result < 0) {
          uv_fs_req_cleanup(raw_req);
          return Resolution{
            .kind=Resolution::REJECT,
            .value=CreateError(
              tc->GetIsolate(),
              v8::Exception::Error,
              std::string(uv_strerror(raw_req->result))
            ).ToLocalChecked()
          };
        }

        Resolution res{
          .kind=Resolution::RESOLVE,
          .value=Op::Resolver::Resolve(tc, raw_req)
        };
        uv_fs_req_cleanup(raw_req);
        return res;
      };
  };

  class PassResult {
    public:
      inline static v8::Local<v8::Value> Resolve(ThreadContext* tc, uv_fs_t* raw_req) {
        return v8::Integer::New(tc->GetIsolate(), raw_req->result);
      }
  };

  class _OpenImplementation {
    public:
      typedef Validation::And<
          Validation::Length<3>,
          Validation::And<
            Validation::Or<
              Validation::Slot<0, Validation::IsString>,
              Validation::Slot<0, Validation::IsArrayBufferView>
            >,
            Validation::And<
              Validation::Slot<1, Validation::IsInt32>,
              Validation::Slot<2, Validation::IsInt32>
            >
          >
      > Validator;
      typedef PassResult Resolver;

      inline static int Execute(
        ThreadContext* tc,
        const FunctionCallbackInfo<Value>& args,
        uv_fs_t* raw_request,
        OnCompleteCallback cb
      ) {
        int32_t flags = args[1]->Int32Value();
        int32_t mode = args[2]->Int32Value();
        const char* path = BufferValue(tc->GetIsolate(), args[0]).Contents();
        return uv_fs_open(tc->GetUVLoop(), raw_request, path, flags, mode, cb);
      };
  };

  class _CloseImplementation {
    public:
      typedef Validation::And<
        Validation::Length<1>,
        Validation::Slot<0, Validation::IsInt32>
      > Validator;
      typedef PassResult Resolver;

      inline static int Execute(
        ThreadContext* tc,
        const FunctionCallbackInfo<Value>& args,
        uv_fs_t* raw_request,
        OnCompleteCallback cb
      ) {
        return uv_fs_close(tc->GetUVLoop(), raw_request, args[0]->Int32Value(), cb);
      };
  };

  class _ReadImplementation {
    public:
      typedef Validation::And<
        Validation::Length<4>,
        Validation::And<
          Validation::Slot<0, Validation::IsInt32>,
          Validation::And<
            Validation::Slot<1, Validation::IsArrayBuffer>,
            Validation::And<
              Validation::Slot<2, Validation::IsInt32>,
              Validation::Slot<3, Validation::IsInt32>
            >
          >
        >
      > Validator;
      typedef PassResult Resolver;

      inline static int Execute(
        ThreadContext* tc,
        const FunctionCallbackInfo<Value>& args,
        uv_fs_t* raw_request,
        OnCompleteCallback cb
      ) {
        int32_t fd = args[0]->Int32Value();
        v8::Local<v8::ArrayBuffer> buf = args[1].As<v8::ArrayBuffer>();
        size_t len = args[2]->Int32Value();
        int32_t offset = args[3]->Int32Value();
        v8::ArrayBuffer::Contents contents{buf->GetContents()};
        void* rawbuf = contents.Data();
        size_t rawbuf_len = contents.ByteLength();

        uv_buf_t uv_buf = uv_buf_init(
          reinterpret_cast<char*>(rawbuf),
          len > rawbuf_len 
          ? rawbuf_len
          : len
        );

        return uv_fs_read(
          tc->GetUVLoop(),
          raw_request,
          fd,
          &uv_buf,
          1,
          offset,
          cb
        );
      };
  };

  typedef BasicOperation<_OpenImplementation> Open;
  typedef BasicOperation<_ReadImplementation> Read;
  typedef BasicOperation<_CloseImplementation> Close;
}

template <typename FSOperation>
class FSRequest : public Request<uv_fs_t, AsyncType::UVFSRequest> {
  public:
    typedef Request<uv_fs_t, AsyncType::UVFSRequest> Parent;

    Local<v8::Value> Execute(const FunctionCallbackInfo<Value>& args) {
      Local<v8::Promise::Resolver> resolver = GetJSObject().template As<v8::Promise::Resolver>();
      Local<v8::Promise> promise = resolver->GetPromise();
      v8::MaybeLocal<Value> err = FSOperation::Execute(GetThreadContext(), args, &m_request, After);

      if (!err.IsEmpty()) {
        uv_fs_req_cleanup(&m_request);
        Reject(err.ToLocalChecked());
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
      v8::MaybeLocal<Value> err = FSOperation::Execute(m_context, args, &m_request, nullptr);

      if (!err.IsEmpty()) {
        m_context->GetIsolate()->ThrowException(
          err.ToLocalChecked()
        );
        return v8::Null(m_context->GetIsolate());
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

      m_context->GetIsolate()->ThrowException(res.value);
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

  tc->SetMethod(bridge, "fsClose", BindRequest<FSRequest<fsops::Close>>);
  tc->SetMethod(bridge, "fsCloseSync", BindRequest<FSRequestSync<fsops::Close>>);
  tc->SetMethod(bridge, "fsRead", BindRequest<FSRequest<fsops::Read>>);
  tc->SetMethod(bridge, "fsReadSync", BindRequest<FSRequestSync<fsops::Read>>);
}

}

}
