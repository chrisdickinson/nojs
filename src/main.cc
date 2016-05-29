#include "v8-debug.h"
#include "v8-profiler.h"
#include "v8-platform.h"
#include "v8_libplatform.h"
#include "uv.h"

#include "nojs_thread_context.h"

v8::Platform* StartV8(int argc, const char** argv) {
  v8::V8::InitializeICU();
  v8::V8::InitializeExternalStartupData(argv[0]);
  v8::Platform* platform = v8::platform::CreateDefaultPlatform(0);
  v8::V8::SetFlagsFromCommandLine(&argc, const_cast<char**>(argv), true);
  v8::V8::InitializePlatform(platform);
  v8::V8::Initialize();
  return platform;
}

int main (int argc, const char** argv) {
  v8::Platform* platform = StartV8(argc, argv);
  uv_disable_stdio_inheritance();
  NoJS::ThreadContext* tcontext = NoJS::ThreadContext::New();

  tcontext->Run();
  tcontext->Dispose();

  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete platform;
  return 0;
}
