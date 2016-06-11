#include "nojs_constants.h"
#include "nojs_thread_context.h"
#include "nojs_thread_context_inl.h"
#include "nojs_utils.h"
#include "nojs_utils_inl.h"

#include <errno.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace NoJS {

namespace Constants {

#define DEFINE_CONSTANT(tc, target, constant) \
    do {                                      \
    tc->Set(target, #constant, v8::Number::New(tc->GetIsolate(), constant));     \
    } while (0)


void ContributeToBridge (ThreadContext* tc, v8::Local<v8::Object> bridge) {
  v8::Isolate* isolate(tc->GetIsolate());
  v8::HandleScope scope(isolate);
  v8::Local<v8::Object> constants = v8::Object::New(isolate);
  tc->Set(bridge, "constants", constants);

  DEFINE_CONSTANT(tc, constants, O_RDONLY);
  DEFINE_CONSTANT(tc, constants, O_WRONLY);
  DEFINE_CONSTANT(tc, constants, O_RDWR);

  DEFINE_CONSTANT(tc, constants, S_IFMT);
  DEFINE_CONSTANT(tc, constants, S_IFREG);
  DEFINE_CONSTANT(tc, constants, S_IFDIR);
  DEFINE_CONSTANT(tc, constants, S_IFCHR);
#ifdef S_IFBLK
  DEFINE_CONSTANT(tc, constants, S_IFBLK);
#endif

#ifdef S_IFIFO
  DEFINE_CONSTANT(tc, constants, S_IFIFO);
#endif

#ifdef S_IFLNK
  DEFINE_CONSTANT(tc, constants, S_IFLNK);
#endif

#ifdef S_IFSOCK
  DEFINE_CONSTANT(tc, constants, S_IFSOCK);
#endif

#ifdef O_CREAT
  DEFINE_CONSTANT(tc, constants, O_CREAT);
#endif

#ifdef O_EXCL
  DEFINE_CONSTANT(tc, constants, O_EXCL);
#endif

#ifdef O_NOCTTY
  DEFINE_CONSTANT(tc, constants, O_NOCTTY);
#endif

#ifdef O_TRUNC
  DEFINE_CONSTANT(tc, constants, O_TRUNC);
#endif

#ifdef O_APPEND
  DEFINE_CONSTANT(tc, constants, O_APPEND);
#endif

#ifdef O_DIRECTORY
  DEFINE_CONSTANT(tc, constants, O_DIRECTORY);
#endif

#ifdef O_EXCL
  DEFINE_CONSTANT(tc, constants, O_EXCL);
#endif

#ifdef O_NOFOLLOW
  DEFINE_CONSTANT(tc, constants, O_NOFOLLOW);
#endif

#ifdef O_SYNC
  DEFINE_CONSTANT(tc, constants, O_SYNC);
#endif

#ifdef O_SYMLINK
  DEFINE_CONSTANT(tc, constants, O_SYMLINK);
#endif

#ifdef O_DIRECT
  DEFINE_CONSTANT(tc, constants, O_DIRECT);
#endif

#ifdef O_NONBLOCK
  DEFINE_CONSTANT(tc, constants, O_NONBLOCK);
#endif

#ifdef S_IRWXU
  DEFINE_CONSTANT(tc, constants, S_IRWXU);
#endif

#ifdef S_IRUSR
  DEFINE_CONSTANT(tc, constants, S_IRUSR);
#endif

#ifdef S_IWUSR
  DEFINE_CONSTANT(tc, constants, S_IWUSR);
#endif

#ifdef S_IXUSR
  DEFINE_CONSTANT(tc, constants, S_IXUSR);
#endif

#ifdef S_IRWXG
  DEFINE_CONSTANT(tc, constants, S_IRWXG);
#endif

#ifdef S_IRGRP
  DEFINE_CONSTANT(tc, constants, S_IRGRP);
#endif

#ifdef S_IWGRP
  DEFINE_CONSTANT(tc, constants, S_IWGRP);
#endif

#ifdef S_IXGRP
  DEFINE_CONSTANT(tc, constants, S_IXGRP);
#endif

#ifdef S_IRWXO
  DEFINE_CONSTANT(tc, constants, S_IRWXO);
#endif

#ifdef S_IROTH
  DEFINE_CONSTANT(tc, constants, S_IROTH);
#endif

#ifdef S_IWOTH
  DEFINE_CONSTANT(tc, constants, S_IWOTH);
#endif

#ifdef S_IXOTH
  DEFINE_CONSTANT(tc, constants, S_IXOTH);
#endif

#ifdef F_OK
  DEFINE_CONSTANT(tc, constants, F_OK);
#endif

#ifdef R_OK
  DEFINE_CONSTANT(tc, constants, R_OK);
#endif

#ifdef W_OK
  DEFINE_CONSTANT(tc, constants, W_OK);
#endif

#ifdef X_OK
  DEFINE_CONSTANT(tc, constants, X_OK);
#endif
}

}

}
