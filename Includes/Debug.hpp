#pragma once

#include <string>

#include "lua_common.h"

namespace Core {
    namespace Debug {
        bool OpenLogFile(const std::string &filepath);

        void CloseLogFile();

        void LogMessage(const std::string& msg, bool showOnScreen);

        void LogError(const std::string& msg);

        void Message(const std::string& msg);

        void Error(const std::string& msg);
    }

    namespace Game {
        bool RegisterDebugModule(lua_State *L);
    }
}