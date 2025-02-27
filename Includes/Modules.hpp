#pragma once

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

void loadScriptingModules(lua_State *L);