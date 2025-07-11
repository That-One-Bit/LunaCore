#pragma once

#include <string>

#include "lua_common.h"

namespace Core {
    void EventRestartClock();

    void EventHandlerCallback();

    namespace Event {
        void TriggerEvent(lua_State* L, const std::string& eventName);
    }

    namespace Module {
        bool RegisterEventModule(lua_State *L);
    }
}