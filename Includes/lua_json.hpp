#pragma once

extern "C" 
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

void register_lua_json_module(lua_State *L);