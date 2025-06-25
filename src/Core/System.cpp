#include "Core/System.hpp"

time_t Core::System::getTime() {
    return time(NULL);
}

// ----------------------------------------------------------------------------

//$Core.System

// ----------------------------------------------------------------------------

/*
- Returns UNIX time
## return: number
### Core.System.getTime
*/
static int l_System_getTime(lua_State *L)
{
    lua_pushnumber(L, Core::System::getTime());
    return 1;
}

static const luaL_Reg system_functions[] =
{
    {"getTime", l_System_getTime},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

bool Core::Module::RegisterSystemModule(lua_State *L)
{
    lua_getglobal(L, "Core");
    luaC_register_field(L, system_functions, "System");
    lua_pop(L, 1);
    return true;
}