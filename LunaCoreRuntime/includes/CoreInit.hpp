#pragma once

#include <string>
#include "lua_common.h"

namespace Core {
    void LoadLuaEnv(lua_State *L);

    bool LoadBuffer(lua_State *L, const char *buffer, size_t size, const char* name);

    bool LoadScript(lua_State *L, const std::string& fp);

    void PreloadScripts();
}