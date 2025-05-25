#pragma once

#include "lua_common.h"

#include <CTRPluginFramework.hpp>

typedef enum DrawCommandID : u8 {
    DRAWCMD_RECT,
    DRAWCMD_PRINT,
} DrawCommandID;

typedef struct {
    DrawCommandID id;
    std::string text;
    int x, y, width, height;
    u32 color1;
    u32 color2;
    bool filled;
    bool topScreen;
} DrawCommandInfo;

namespace Core {
    namespace Graphics {
        void PushCommand(const DrawCommandInfo &cmd);
    }

    bool GraphicsHandlerCallback(const CTRPluginFramework::Screen &screen);

    namespace Game {
        bool RegisterGraphicsModule(lua_State *L);
    }
}