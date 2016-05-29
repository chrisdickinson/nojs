#ifndef NOJS_LIBRARIES_H
#define NOJS_LIBRARIES_H
#include <vector>

namespace NoJS {

class NativesCollection {
  public:
    static int GetBuiltinsCount();
    static int GetDebuggerCount();

    static int GetIndex(const char* name);
    static std::vector<const char> GetScriptSource(int index);
    static std::vector<const char> GetScriptName(int index);
    static std::vector<const char> GetScriptsSource();
};

}

#endif
