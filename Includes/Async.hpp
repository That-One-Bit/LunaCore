#pragma once

#include "lua_common.h"

namespace Core {
    void AsyncHandlerCallback();

    bool RegisterAsyncModule(lua_State *L);
}