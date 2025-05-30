#pragma once

#include <string>

#include "lua_common.h"

namespace Core {
    namespace Utils {
        std::string strip(const std::string &str);

        std::string LoadFile(const std::string &filepath);

        void Replace(std::string &str, const std::string &pattern, const std::string &repl);
    }
    
    bool RegisterUtilsModule(lua_State *L);

    void UnregisterUtilsModule(lua_State *L);
}