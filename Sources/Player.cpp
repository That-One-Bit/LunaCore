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

int luaopen_Player(lua_State *L)
{
    lua_newtable(L); // Player

    lua_newtable(L); // SwimSpeed

    lua_pushcfunction(L, l_Player_GetSwimSpeed);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, l_Player_SetSwimSpeed);
    lua_setfield(L, -2, "set");
    
    lua_setfield(L, -2, "SwimSpeed");

    lua_setglobal(L, "Player");
    return 0;
}
