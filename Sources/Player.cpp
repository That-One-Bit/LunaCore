#include "Player.hpp"

#include <CTRPluginFramework.hpp>

#include "MemAddress.hpp"

int l_Player_GetSwimSpeed(lua_State *L) 
{
    float vel;
    CTRPluginFramework::Process::ReadFloat(value_address::playerSwimSpeed, vel);
    lua_pushnumber(L, vel);
    return 1;
}

int l_Player_SetSwimSpeed(lua_State *L) 
{
    float vel = luaL_checknumber(L, 1);

    CTRPluginFramework::Process::WriteFloat(value_address::playerSwimSpeed, vel);
    return 0;
}

static const luaL_Reg player_functions[] =
{
    {"GetSwimSpeed", l_Player_GetSwimSpeed},
    {"SetSwimSpeed", l_Player_SetSwimSpeed},
    {NULL, NULL}
};

int luaopen_Player(lua_State *L)
{
    lua_newtable(L);
    luaL_register(L, NULL, player_functions);
    lua_setglobal(L, "Player");
    return 0;
}
