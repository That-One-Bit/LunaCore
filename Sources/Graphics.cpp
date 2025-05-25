#include "Graphics.hpp"

#include <queue>
#include <mutex>
#include <string>

namespace CTRPF = CTRPluginFramework;

std::queue<DrawCommandInfo> drawQueueTop;
std::queue<DrawCommandInfo> drawQueueBottom;
std::mutex drawQueueMutexTop;
std::mutex drawQueueMutexBottom;

void CoreGraphicsPushCommand(const DrawCommandInfo &cmd)
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

bool CoreGraphicsDrawFrameCallback(const CTRPF::Screen &screen) {
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
                screen.DrawRect(cmd.x, cmd.y, cmd.width, cmd.height, cmd.color1, cmd.filled);
                break;
        }
        localQueue.pop();
    }
    return true;
}

bool CoreRegisterGraphicsModule(lua_State *L)
{
    return true;
}