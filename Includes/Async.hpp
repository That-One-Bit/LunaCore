#pragma once

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

int luaopen_Async(lua_State *L);