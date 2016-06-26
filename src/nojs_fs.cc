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
    Resolution(const Kind k, const Local<Value>& val)
      : value(val), kind(k) {
    }
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
          return Resolution(
            Resolution::REJECT,
            CreateError(
              tc->GetIsolate(),
              v8::Exception::Error,
              std::string(uv_strerror(raw_req->result))
            ).ToLocalChecked()
          );
        }

        Resolution res(Resolution::RESOLVE, Op::Resolver::Resolve(tc, raw_req));
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

  class JSBridge {
    public:

      static v8::Local<v8::Function> GetConstructor() {
        return StrongPersistentToLocal(stats_constructor);
      };
      static void SetConstructor(v8::Isolate* isolate, v8::Local<v8::Function> fn) {
        stats_constructor.Reset(isolate, fn);
      };

    private:
      JSBridge();
      static v8::Persistent<v8::Function> stats_constructor;
  };

  v8::Persistent<v8::Function> JSBridge::stats_constructor;

  template <typename StatImpl>
  class _StatImplementation : public JSBridge {
    public:
      class Resolver {
        public:
          inline static v8::Local<v8::Value> Resolve(
            ThreadContext* tc,
            uv_fs_t* req
          ) {
            v8::Isolate* isolate(tc->GetIsolate());
            v8::EscapableHandleScope scope(isolate);
            uv_stat_t* stat = reinterpret_cast<uv_stat_t*>(req->ptr);

            v8::Local<v8::Value> argv[] = {
              /*dev =*/     v8::Integer::New(isolate, stat->st_dev),
              /*mode =*/    v8::Integer::New(isolate, stat->st_mode),
              /*nlink =*/   v8::Integer::New(isolate, stat->st_nlink),
              /*uid =*/     v8::Integer::New(isolate, stat->st_uid),
              /*gid =*/     v8::Integer::New(isolate, stat->st_gid),
              /*rdev =*/    v8::Integer::New(isolate, stat->st_rdev),

              /*ino =*/     v8::Number::New(isolate, static_cast<double>(stat->st_ino)),
              /*size =*/    v8::Number::New(isolate, static_cast<double>(stat->st_size)),
  #ifdef __POSIX__
              /*blksize =*/ v8::Integer::New(isolate, stat->st_blksize),
              /*blocks =*/  v8::Number::New(isolate, static_cast<double>(stat->st_blocks)),
  #else
              /*blksize =*/ v8::Undefined(isolate),
              /*blocks =*/  v8::Undefined(isolate),
  #endif
              /*atime_msec =*/
                            v8::Number::New(isolate, (
                              (static_cast<double>(stat->st_atim.tv_sec) * 1000) +
                              (static_cast<double>(stat->st_atim.tv_nsec) / 1000000)
                            )),

              /*ctime_msec =*/
                            v8::Number::New(isolate, (
                              (static_cast<double>(stat->st_ctim.tv_sec) * 1000) +
                              (static_cast<double>(stat->st_ctim.tv_nsec) / 1000000)
                            )),

              /*mtime_msec =*/
                            v8::Number::New(isolate, (
                              (static_cast<double>(stat->st_mtim.tv_sec) * 1000) +
                              (static_cast<double>(stat->st_mtim.tv_nsec) / 1000000)
                            )),

              /*birthtime_msec =*/
                            v8::Number::New(isolate, (
                              (static_cast<double>(stat->st_birthtim.tv_sec) * 1000) +
                              (static_cast<double>(stat->st_birthtim.tv_nsec) / 1000000)
                            ))
            };

            if (argv[sizeof(argv) - 1].IsEmpty()) {
              return v8::Local<v8::Value>();
            }

            v8::Local<v8::Value> out = JSBridge::GetConstructor()->NewInstance(
              isolate->GetCurrentContext(),
              arraysize(argv),
              argv
            ).FromMaybe(v8::Local<v8::Value>());

            return scope.Escape(out);
          }
      };

      typedef Validation::And<
        Validation::Length<1>,
        typename StatImpl::Validator
      > Validator;

      inline static int Execute(
        ThreadContext* tc,
        const FunctionCallbackInfo<Value>& args,
        uv_fs_t* raw_request,
        OnCompleteCallback cb
      ) {
        typename StatImpl::Path path(tc->GetIsolate(), args[0]);
        return StatImpl::Execute(
          tc->GetUVLoop(),
          raw_request,
          path.Contents(),
          cb
        );
      };
  };

  class _Stat {
    public:
      typedef Validation::Or<
        Validation::Slot<0, Validation::IsString>,
        Validation::Slot<0, Validation::IsArrayBufferView>
      > Validator;
      typedef BufferValue Path;

      inline static int Execute(
        uv_loop_t* loop,
        uv_fs_t* req,
        const char* path,
        OnCompleteCallback cb) {
        return uv_fs_stat(loop, req, path, cb);
      }
  };

  class _LStat {
    public:
      typedef Validation::Or<
        Validation::Slot<0, Validation::IsString>,
        Validation::Slot<0, Validation::IsArrayBufferView>
      > Validator;
      typedef BufferValue Path;

      inline static int Execute(
        uv_loop_t* loop,
        uv_fs_t* req,
        const char* path,
        OnCompleteCallback cb) {
        return uv_fs_lstat(loop, req, path, cb);
      }
  };

  class _FStat {
    private:
      class FD {
        public:
          FD(v8::Isolate* _unused, v8::Local<v8::Value> val)
            : m_value(val->Int32Value()) {
          }
          inline int32_t Contents() {
            return m_value;
          }
        private:
          int32_t m_value;
      };
    public:
      typedef Validation::Or<
        Validation::Slot<0, Validation::IsString>,
        Validation::Slot<0, Validation::IsArrayBufferView>
      > Validator;
      typedef FD Path;

      inline static int Execute(
        uv_loop_t* loop,
        uv_fs_t* req,
        int32_t path,
        OnCompleteCallback cb) {
        return uv_fs_fstat(loop, req, path, cb);
      }
  };

  typedef BasicOperation<_OpenImplementation> Open;
  typedef BasicOperation<_ReadImplementation> Read;
  typedef BasicOperation<_CloseImplementation> Close;

  typedef BasicOperation<_StatImplementation<_Stat>> Stat;
  typedef BasicOperation<_StatImplementation<_LStat>> LStat;
  typedef BasicOperation<_StatImplementation<_FStat>> FStat;
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

void ContributeToBridge (ThreadContext* tc, v8::Local<v8::Object> bridge, v8::Local<v8::Object> bootstrap) {
  v8::Isolate* isolate(tc->GetIsolate());
  HandleScope scope(isolate);

  tc->SetMethod(bridge, "fsOpen", BindRequest<FSRequest<fsops::Open>>);
  tc->SetMethod(bridge, "fsOpenSync", BindRequest<FSRequestSync<fsops::Open>>);

  tc->SetMethod(bridge, "fsClose", BindRequest<FSRequest<fsops::Close>>);
  tc->SetMethod(bridge, "fsCloseSync", BindRequest<FSRequestSync<fsops::Close>>);

  tc->SetMethod(bridge, "fsRead", BindRequest<FSRequest<fsops::Read>>);
  tc->SetMethod(bridge, "fsReadSync", BindRequest<FSRequestSync<fsops::Read>>);

  tc->SetMethod(bridge, "fsStat", BindRequest<FSRequest<fsops::Stat>>);
  tc->SetMethod(bridge, "fsStatSync", BindRequest<FSRequestSync<fsops::Stat>>);

  tc->SetMethod(bridge, "fsLStat", BindRequest<FSRequest<fsops::LStat>>);
  tc->SetMethod(bridge, "fsLStatSync", BindRequest<FSRequestSync<fsops::LStat>>);

  tc->SetMethod(bridge, "fsFStat", BindRequest<FSRequest<fsops::FStat>>);
  tc->SetMethod(bridge, "fsFStatSync", BindRequest<FSRequestSync<fsops::FStat>>);

  assert(!tc->Get(bootstrap, "Stat").ToLocalChecked()->IsUndefined());
  fsops::JSBridge::SetConstructor(
    isolate,
    tc->Get(bootstrap, "Stat").ToLocalChecked().As<v8::Function>()
  );

}

}

}
