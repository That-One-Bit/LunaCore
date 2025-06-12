#pragma once

#include "lua_common.h"

namespace Core {
    void EventRestartClock();

    void EventHandlerCallback();

    namespace Event {
        void TriggerOnPlayerJoinWorld(lua_State *L);

        void TriggerOnPlayerLeaveWorld(lua_State *L);
    }

    namespace Game {
        bool RegisterEventModule(lua_State *L);
    }
}