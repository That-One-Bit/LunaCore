#pragma once

#include "lua_common.h"

namespace Core {
    void EventHandlerCallback();

    namespace Game {
        bool RegisterEventModule(lua_State *L);
    }
}