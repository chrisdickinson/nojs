#ifndef NOJS_LIBRARIES_H
#define NOJS_LIBRARIES_H
#include <string>

namespace NoJS {

class NativesCollection {
  public:
    static int GetBuiltinsCount();
    static int GetDebuggerCount();

    static int GetIndex(const char* name);
    static std::string GetScriptSource(int index);
    static std::string GetScriptName(int index);
    static std::string GetScriptsSource();
};

}

#endif
