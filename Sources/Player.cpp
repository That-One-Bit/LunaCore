#include "Player.hpp"

#include <cstring>

#include <CTRPluginFramework.hpp>
#include "Minecraft.hpp"
#include "string_hash.hpp"

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
    return 0;
}

static const luaL_Reg player_position_methods[] =
{
    {"get", l_Player_Position_get},
    {"set", l_Player_Position_set},
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

/*
=Game.LocalPlayer.OnGround = false
=Game.LocalPlayer.Sneaking = false
=Game.LocalPlayer.Jumping = false
=Game.LocalPlayer.Sprinting = false
=Game.LocalPlayer.Flying = false
=Game.LocalPlayer.UnderWater = false
=Game.LocalPlayer.TouchingWall = false
=Game.LocalPlayer.MoveSpeed = 0.0
=Game.LocalPlayer.SwimSpeed = 0.02
=Game.LocalPlayer.CurrentHP = 0.0
=Game.LocalPlayer.MaxHP = 0.0
=Game.LocalPlayer.CurrentHunger = 0.0
=Game.LocalPlayer.MaxHunger = 0.0
=Game.LocalPlayer.ReachDistance = 0.0
*/
int l_LocalPlayer_index(lua_State *L)
{
    uint32_t key = hash(lua_tostring(L, 2));
    bool valid_key = true;

    switch (key) {
        case hash("OnGround"):
            lua_pushboolean(L, Minecraft::IsPlayerOnGround());
            break;
        case hash("Sneaking"):
            lua_pushboolean(L, Minecraft::IsPlayerSneaking());
            break;
        case hash("Jumping"):
            lua_pushboolean(L, Minecraft::IsPlayerJumping());
            break;
        case hash("Sprinting"):
            lua_pushboolean(L, Minecraft::IsPlayerSprinting());
            break;
        case hash("Flying"):
            lua_pushboolean(L, Minecraft::IsPlayerFlying());
            break;
        case hash("UnderWater"):
            lua_pushboolean(L, Minecraft::IsPlayerUnderWater());
            break;
        case hash("TouchingWall"):
            lua_pushboolean(L, Minecraft::IsPlayerTouchingWall());
            break;
        case hash("MoveSpeed"):
            lua_pushnumber(L, Minecraft::GetPlayerMoveSpeed());
            break;
        case hash("SwimSpeed"): {
            float vel;
            CTRPF::Process::ReadFloat(player_offsets::playerSwimSpeed, vel);
            lua_pushnumber(L, vel);
            break;
        }
        case hash("CurrentHP"): 
            lua_pushnumber(L, Minecraft::GetCurrentHP());
            break;
        case hash("MaxHP"):
            lua_pushnumber(L, Minecraft::GetMaxHP());
            break;
        case hash("CurrentHunger"):
            lua_pushnumber(L, Minecraft::GetCurrentHunger());
            break;
        case hash("MaxHunger"):
            lua_pushnumber(L, Minecraft::GetMaxHunger());
            break;
        case hash("ReachDistance"): {
            float distance;
            CTRPF::Process::ReadFloat(player_offsets::playerReachDistance, distance);
            lua_pushnumber(L, distance);
            break;
        }
        default:
            valid_key = false;
            break;
    }
    
    if (valid_key)
        return 1;
    else
        return 0;
}

int l_LocalPlayer_newindex(lua_State *L)
{
    uint32_t key = hash(lua_tostring(L, 2));
    bool valid_key = true;

    switch (key) {
        case hash("OnGround"):
            Minecraft::SetPlayerOnGround(lua_toboolean(L, 3));
            break;
        case hash("Sneaking"):
            Minecraft::SetPlayerSneaking(lua_toboolean(L, 3));
            break;
        case hash("Jumping"):
            Minecraft::SetPlayerJump(lua_toboolean(L, 3));
            break;
        case hash("Sprinting"):
            Minecraft::SetPlayerSprinting(lua_toboolean(L, 3));
            break;
        case hash("Flying"):
            Minecraft::SetPlayerFlying(lua_toboolean(L, 3));
            break;
        case hash("UnderWater"):
            Minecraft::SetPlayerUnderWater(lua_toboolean(L, 3));
            break;
        case hash("TouchingWall"):
            Minecraft::SetPlayerTouchingWall(lua_toboolean(L, 3));
            break;
        case hash("MoveSpeed"):
            Minecraft::SetPlayerMoveSpeed(luaL_checknumber(L, 3));
            break;
        case hash("SwimSpeed"):
            CTRPF::Process::WriteFloat(player_offsets::playerSwimSpeed, luaL_checknumber(L, 3));
            break;
        case hash("CurrentHP"):
            Minecraft::SetCurrentHP(luaL_checknumber(L, 3));
            break;
        case hash("MaxHP"):
            Minecraft::SetMaxHP(luaL_checknumber(L, 3));
            break;
        case hash("CurrentHunger"):
            Minecraft::SetCurrentHunger(luaL_checknumber(L, 3));
            break;
        case hash("MaxHunger"):
            Minecraft::SetMaxHunger(luaL_checknumber(L, 3));
            break;
        case hash("ReachDistance"):
            CTRPF::Process::WriteFloat(player_offsets::playerReachDistance, luaL_checknumber(L, 3));
            break;
    }

    if (valid_key)
        return 0;
    else
        return luaL_error(L, "'%s' is not a valid member of object or is not and editable value", key);
}

// ----------------------------------------------------------------------------

int l_register_LocalPlayer(lua_State *L)
{
    luaL_newmetatable(L, "LocalPlayerMetatable");
    lua_pushcfunction(L, l_LocalPlayer_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_LocalPlayer_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);

    lua_getglobal(L, "Game");
    lua_newtable(L); // LocalPlayer
    luaC_setmetatable(L, "LocalPlayerMetatable");

    luaC_register_field(L, player_position_methods, "Position");

    lua_newtable(L); // LocalPlayer.Camera
    luaC_register_field(L, player_camera_fov_methods, "FOV");
    lua_setfield(L, -2, "Camera");
    
    lua_setfield(L, -2, "LocalPlayer");
    lua_pop(L, 1);
    return 0;
}
