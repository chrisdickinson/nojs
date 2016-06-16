#include "nojs_validate_arguments.h"

namespace NoJS {

namespace Validation {

#define V(XS)                                 \
const char* _Name_ ## XS ::Value = #XS;       \
typedef Type<Is:: XS, _Name_ ## XS> Is ## XS;
FORWARD_TO_VALUE(V)
#undef V
#undef FORWARD_TO_VALUE

}

}
