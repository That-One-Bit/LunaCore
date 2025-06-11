#pragma once

#include "lua_common.h"

typedef uint32_t u32;

namespace Core {
    namespace Game {
        bool RegisterGamepadModule(lua_State *L);
    }

    namespace Gamepad {
        void BlockKey(u32 keyCode);
    }
}