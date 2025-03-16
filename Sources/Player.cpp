#include "Player.hpp"

#include <CTRPluginFramework.hpp>

#include "MemAddress.hpp"

int l_Player_ReachDistance_get(lua_State *L) 
{
    float distance;
    CTRPluginFramework::Process::ReadFloat(value_address::playerReachDistance, distance);
    lua_pushnumber(L, distance);
    return 1;
}

int l_Player_ReachDistance_set(lua_State *L) 
{
    float distance = luaL_checknumber(L, 1);

    CTRPluginFramework::Process::WriteFloat(value_address::playerReachDistance, distance);
    return 0;
}

static const luaL_Reg player_reachdistance_methods[] =
{
    {"get", l_Player_ReachDistance_get},
    {"set", l_Player_ReachDistance_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

int l_Player_SwimSpeed_get(lua_State *L) 
{
    float vel;
    CTRPluginFramework::Process::ReadFloat(value_address::playerSwimSpeed, vel);
    lua_pushnumber(L, vel);
    return 1;
}

int l_Player_SwimSpeed_set(lua_State *L) 
{
    float vel = luaL_checknumber(L, 1);

    CTRPluginFramework::Process::WriteFloat(value_address::playerSwimSpeed, vel);
    return 0;
}

static const luaL_Reg player_swimspeed_methods[] =
{
    {"get", l_Player_SwimSpeed_get},
    {"set", l_Player_SwimSpeed_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

int l_Player_Position_get(lua_State *L) 
{
    float x, y, z;
    CTRPluginFramework::Process::ReadFloat(value_address::playerPositionX, x);
    CTRPluginFramework::Process::ReadFloat(value_address::playerPositionY, y);
    CTRPluginFramework::Process::ReadFloat(value_address::playerPositionZ, z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 1;
}

static const luaL_Reg player_position_methods[] =
{
    {"get", l_Player_Position_get},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

int luaopen_Player(lua_State *L)
{
    lua_newtable(L); // Player

    lua_newtable(L); // ReachDistance
    luaL_register(L, NULL, player_reachdistance_methods);
    lua_setfield(L, -2, "ReachDistance");

    lua_newtable(L); // SwimSpeed
    luaL_register(L, NULL, player_swimspeed_methods);
    lua_setfield(L, -2, "SwimSpeed");
    
    lua_setglobal(L, "Player");
    return 0;
}
