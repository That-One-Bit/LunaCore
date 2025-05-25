#include "System.hpp"

#include <time.h>

// ----------------------------------------------------------------------------

//$System

// ----------------------------------------------------------------------------

/*
- Returns UNIX time
## return: number
### System.getTime
*/
static int l_System_getTime(lua_State *L)
{
    lua_pushnumber(L, time(NULL));
    return 1;
}

static const luaL_Reg system_functions[] =
{
    {"getTime", l_System_getTime},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

bool Core::RegisterSystemModule(lua_State *L)
{
    luaC_register_global(L, system_functions, "System");
    return true;
}