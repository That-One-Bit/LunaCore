#pragma once

extern "C" 
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

int luaopen_System(lua_State *L);