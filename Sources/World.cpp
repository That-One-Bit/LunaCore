#include "Player.hpp"

#include <CTRPluginFramework.hpp>

#include "MemAddress.hpp"

int l_World_SetCloudsHeight(lua_State *L)
{
    float vel = luaL_checknumber(L, 1);

    CTRPluginFramework::Process::WriteFloat(value_address::playerWaterVelocity, vel);
    return 0;
}

static const luaL_Reg world_functions[] =
{
    {"SetCloudsHeight", l_World_SetCloudsHeight},
    {NULL, NULL}
};

int luaopen_World(lua_State *L)
{
    lua_newtable(L);
    luaL_register(L, NULL, world_functions);
    lua_setglobal(L, "World");
    return 0;
}