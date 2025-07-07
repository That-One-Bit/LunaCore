#pragma once

#include "lua_common.h"

#include <CTRPluginFramework.hpp>

namespace Core {
    using GraphicsFrameCallback = void(*)(void);
    using GraphicsExitCallback = void(*)(void);
    
    void GraphicsOpen(GraphicsFrameCallback frameCallback, GraphicsExitCallback exitCallback);

    void GraphicsHandlerMainloop();

    namespace Module {
        bool RegisterGraphicsModule(lua_State *L);
    }
}