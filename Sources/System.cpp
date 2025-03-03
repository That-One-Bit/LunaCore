#include "System.hpp"

#include <time.h>

int l_System_GetTime(lua_State *L)
{
    lua_pushnumber(L, time(NULL));
    return 1;
}

static const luaL_Reg system_functions[] =
{
    {"GetTime", l_System_GetTime},
    {NULL, NULL}
};

int luaopen_System(lua_State *L)
{
    lua_newtable(L);
    luaL_register(L, NULL, system_functions);
    lua_setglobal(L, "System");
    return 0;
}