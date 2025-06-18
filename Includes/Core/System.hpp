#pragma once

#include <time.h>
#include <string>

#include "lua_common.h"
#include "types.h"

namespace Core {
    namespace System {
        time_t getTime();
    }

    namespace Module {
        bool RegisterSystemModule(lua_State *L);
    }
}