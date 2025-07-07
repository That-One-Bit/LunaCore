#pragma once

#include <string>
#include <time.h>

#include "lua_common.h"

namespace Core {
    namespace Utils {
        std::string formatTime(time_t time);

        std::string strip(const std::string &str);

        bool startsWith(const std::string &str, const std::string &prefix);

        std::string LoadFile(const std::string &filepath);

        void Replace(std::string &str, const std::string &pattern, const std::string &repl);
    }
    
    bool RegisterUtilsModule(lua_State *L);

    void UnregisterUtilsModule(lua_State *L);
}