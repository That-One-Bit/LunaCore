#include "Graphics.hpp"

#include <queue>
#include <mutex>
#include <string>
#include <atomic>

namespace CTRPF = CTRPluginFramework;

std::queue<DrawCommandInfo> drawQueueTop;
std::queue<DrawCommandInfo> drawQueueBottom;
std::mutex drawQueueMutexTop;
std::mutex drawQueueMutexBottom;
std::atomic<bool> graphicsIsTop = false;

void Core::Graphics::PushCommand(const DrawCommandInfo &cmd)
{
    if (cmd.topScreen) {
        std::lock_guard<std::mutex> lock(drawQueueMutexTop);
        drawQueueTop.push(cmd);
    }
    else {
        std::lock_guard<std::mutex> lock(drawQueueMutexBottom);
        drawQueueBottom.push(cmd);
    }
}

bool Core::GraphicsHandlerCallback(const CTRPF::Screen &screen) {
    std::queue<DrawCommandInfo> localQueue;

    if (screen.IsTop) {
        std::lock_guard<std::mutex> lock(drawQueueMutexTop);
        if (drawQueueTop.empty())
            return true;
        std::swap(localQueue, drawQueueTop);
    }
    else {
        std::lock_guard<std::mutex> lock(drawQueueMutexBottom);
        if (drawQueueBottom.empty())
            return true;
        std::swap(localQueue, drawQueueBottom);
    }

    while (!localQueue.empty())
    {
        const DrawCommandInfo &cmd = localQueue.front();
        switch (cmd.id) {
            case DRAWCMD_PRINT:
                screen.Draw(cmd.text, cmd.x, cmd.y, cmd.color1, cmd.color2);
                break;
            case DRAWCMD_RECT:
                screen.DrawRect(cmd.x, cmd.y, cmd.width, cmd.height, cmd.color1, false);
                break;
            case DRAWCMD_RECTFILL:
                screen.DrawRect(cmd.x, cmd.y, cmd.width, cmd.height, cmd.color1, true);
                break;
        }
        localQueue.pop();
    }
    return true;
}

// ----------------------------------------------------------------------------

//--$Game.Graphics

// ----------------------------------------------------------------------------

/*
- Draws a rect on screen
--## x: integer
--## y: integer
--## width: integer
--## height: integer
--## color: integer
--### Game.Graphics.drawRect
*/
static int l_Graphics_drawRect(lua_State *L)
{
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    u32 color = luaL_checkinteger(L, 5);

    if (x < 0 || y < 0)
        return luaL_error(L, "position must be greater than 0");
    if ((width < 0 || height < 0))
        return luaL_error(L, "size must be greater than position");

    DrawCommandInfo cmd;
    cmd.id = DrawCommandID::DRAWCMD_RECT;
    cmd.x = x;
    cmd.y = y;
    cmd.width = width;
    cmd.height = height;
    cmd.color1 = color;
    cmd.topScreen = graphicsIsTop.load();
    Core::Graphics::PushCommand(cmd);
    return 0;
}

/*
- Draws a solid rect on screen
--## x: integer
--## y: integer
--## width: integer
--## height: integer
--## color: integer
--### Game.Graphics.drawRectFill
*/
static int l_Graphics_drawRectFill(lua_State *L)
{
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    u32 color = luaL_checkinteger(L, 5);

    if (x < 0 || y < 0)
        return luaL_error(L, "x and y must be greater than 0");
    if ((width < 0 || height < 0))
        return luaL_error(L, "width and height must be greater than 0");

    DrawCommandInfo cmd;
    cmd.id = DrawCommandID::DRAWCMD_RECTFILL;
    cmd.x = x;
    cmd.y = y;
    cmd.width = width;
    cmd.height = height;
    cmd.color1 = color;
    cmd.topScreen = graphicsIsTop.load();
    Core::Graphics::PushCommand(cmd);
    return 0;
}

static const luaL_Reg graphics_functions[] =
{
    {"drawRect", l_Graphics_drawRect},
    {"drawRectFill", l_Graphics_drawRectFill},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

bool Core::Game::RegisterGraphicsModule(lua_State *L)
{
    lua_getglobal(L, "Game");
    luaC_register_field(L, graphics_functions, "Graphics");
    lua_pop(L, 1);
    return true;
}