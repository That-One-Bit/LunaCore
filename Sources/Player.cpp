#include "Player.hpp"

#include <CTRPluginFramework.hpp>
#include "Minecraft.hpp"

namespace CTRPF = CTRPluginFramework;

enum player_offsets : u32 {
    cameraFOV = 0x100000 + 0x2CEE80,
    playerReachDistance = 0x100000 + 0x54FB68,
    playerSwimSpeed = 0x100000 + 0x3EA090,
    playerPositionX = 0xAC1E48,
    playerPositionY = 0xAC1E4C,
    playerPositionZ = 0xAC1E50,
};

//$Game.LocalPlayer

// ----------------------------------------------------------------------------

//$Game.LocalPlayer.MaxHP

/*
- Gets current local player max hp
## return: number
### Game.LocalPlayer.MaxHP.get
*/
int l_Player_MaxHP_get(lua_State *L) 
{
    float maxhp = Minecraft::GetMaxHP();
    lua_pushnumber(L, maxhp);
    return 1;
}

/*
- Sets local player max hp
## maxhp: number
### Game.LocalPlayer.MaxHP.set
*/
int l_Player_MaxHP_set(lua_State *L) 
{
    float maxhp = luaL_checknumber(L, 1);
    Minecraft::SetMaxHP(maxhp);
    return 0;
}

static const luaL_Reg player_maxhp_methods[] =
{
    {"get", l_Player_MaxHP_get},
    {"set", l_Player_MaxHP_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

//$Game.LocalPlayer.HP

/*
- Gets current local player hp
## return: number
### Game.LocalPlayer.HP.get
*/
int l_Player_HP_get(lua_State *L) 
{
    float maxhp = Minecraft::GetMaxHP();
    lua_pushnumber(L, maxhp);
    return 1;
}

/*
- Sets local player hp
## maxhp: number
### Game.LocalPlayer.HP.set
*/
int l_Player_HP_set(lua_State *L) 
{
    float maxhp = luaL_checknumber(L, 1);
    Minecraft::SetMaxHP(maxhp);
    return 0;
}

static const luaL_Reg player_hp_methods[] =
{
    {"get", l_Player_HP_get},
    {"set", l_Player_HP_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

//$Game.LocalPlayer.Position

/*
- Gets local player position
## return: number
## return: number
## return: number
### Game.LocalPlayer.Position.get
*/
int l_Player_Position_get(lua_State *L) 
{
    float x, y, z;
    Minecraft::GetPlayerPosition(x, y, z);
    //CTRPF::Process::ReadFloat(player_offsets::playerPositionX, x);
    //CTRPF::Process::ReadFloat(player_offsets::playerPositionY, y);
    //CTRPF::Process::ReadFloat(player_offsets::playerPositionZ, z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 1;
}

/*
- Sets player position
## x: number
## y: number
## z: number
### Game.LocalPlayer.Position.set
*/
int l_Player_Position_set(lua_State *L) 
{
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float z = luaL_checknumber(L, 3);
    Minecraft::SetPlayerPosition(x, y, z);
    //CTRPF::Process::WriteFloat(player_offsets::playerPositionX, x);
    //CTRPF::Process::WriteFloat(player_offsets::playerPositionY, y);
    //CTRPF::Process::WriteFloat(player_offsets::playerPositionZ, z);
    return 0;
}

static const luaL_Reg player_position_methods[] =
{
    {"get", l_Player_Position_get},
    {"set", l_Player_Position_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

//$Game.LocalPlayer.SwimSpeed

/*
- Gets current local player swim speed
## return: number
### Game.LocalPlayer.SwimSpeed.get
*/
int l_Player_SwimSpeed_get(lua_State *L) 
{
    float vel;
    CTRPF::Process::ReadFloat(player_offsets::playerSwimSpeed, vel);
    lua_pushnumber(L, vel);
    return 1;
}

/*
- Sets local player swim speed
## vel: number
### Game.LocalPlayer.SwimSpeed.set
*/
int l_Player_SwimSpeed_set(lua_State *L) 
{
    float vel = luaL_checknumber(L, 1);

    CTRPF::Process::WriteFloat(player_offsets::playerSwimSpeed, vel);
    return 0;
}

static const luaL_Reg player_swimspeed_methods[] =
{
    {"get", l_Player_SwimSpeed_get},
    {"set", l_Player_SwimSpeed_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

//$Game.LocalPlayer.ReachDistance

/*
- Gets current local player block reach distance
## return: number
### Game.LocalPlayer.ReachDistance.get
*/
int l_Player_ReachDistance_get(lua_State *L) 
{
    float distance;
    CTRPF::Process::ReadFloat(player_offsets::playerReachDistance, distance);
    lua_pushnumber(L, distance);
    return 1;
}

/*
- Sets local player block reach distance
## distance: number
### Game.LocalPlayer.ReachDistance.set
*/
int l_Player_ReachDistance_set(lua_State *L) 
{
    float distance = luaL_checknumber(L, 1);

    CTRPF::Process::WriteFloat(player_offsets::playerReachDistance, distance);
    return 0;
}

static const luaL_Reg player_reachdistance_methods[] =
{
    {"get", l_Player_ReachDistance_get},
    {"set", l_Player_ReachDistance_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

//$Game.LocalPlayer.Camera
//$Game.LocalPlayer.Camera.FOV

/*
- Gets current local player camera FOV
## return: number
### Game.LocalPlayer.Camera.FOV.get
*/
int l_Player_Camera_FOV_get(lua_State *L) 
{
    float fov;
    CTRPF::Process::ReadFloat(0, fov);
    lua_pushnumber(L, fov);
    return 1;
}

/*
- Sets local player camera FOV
## fov: number
### Game.LocalPlayer.Camera.FOV.set
*/
int l_Player_Camera_FOV_set(lua_State *L) 
{
    float fov = luaL_checknumber(L, 1);

    CTRPF::Process::WriteFloat(0, fov);
    return 0;
}

static const luaL_Reg player_camera_fov_methods[] =
{
    {"get", l_Player_Camera_FOV_get},
    {"set", l_Player_Camera_FOV_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

int l_register_LocalPlayer(lua_State *L)
{
    lua_getglobal(L, "Game");
    lua_newtable(L); // LocalPlayer

    luaC_register_field(L, player_maxhp_methods, "MaxHP");
    luaC_register_field(L, player_hp_methods, "HP");
    luaC_register_field(L, player_position_methods, "Position");
    luaC_register_field(L, player_swimspeed_methods, "SwimSpeed");
    luaC_register_field(L, player_reachdistance_methods, "ReachDistance");

    lua_newtable(L); // LocalPlayer.Camera
    luaC_register_field(L, player_camera_fov_methods, "FOV");
    lua_setfield(L, -2, "Camera");
    
    lua_setfield(L, -2, "LocalPlayer");
    lua_pop(L, 1);
    return 0;
}
