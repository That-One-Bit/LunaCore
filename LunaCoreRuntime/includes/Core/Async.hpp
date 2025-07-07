#pragma once

#include "lua_common.h"

namespace Core {
    void AsyncRestartClock();
    
    void AsyncHandlerCallback();

    bool RegisterAsyncModule(lua_State *L);
}