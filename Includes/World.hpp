#pragma once

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

int luaopen_World(lua_State *L);