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

void CoreGraphicsPushCommand(const DrawCommandInfo &cmd);

bool CoreGraphicsDrawFrameCallback(const CTRPluginFramework::Screen &screen);

bool CoreRegisterGraphicsModule(lua_State *L);