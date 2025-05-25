#pragma once

#include "lua_common.h"

namespace Core {
    bool RegisterUtilsModule(lua_State *L);

    void UnregisterUtilsModule(lua_State *L);
}