#include "Player/Camera.hpp"

#include <CTRPluginFramework.hpp>

#include "Minecraft.hpp"
#include "string_hash.hpp"

namespace CTRPF = CTRPluginFramework;

enum player_camera_offsets : u32 {
    cameraFOV = 0x100000 + 0x2CEE80,
};

// ----------------------------------------------------------------------------

/*
!include Sources/Player/Player.cpp
$Game.LocalPlayer.Camera
=Game.LocalPlayer.Camera.FOV = 0.0
=Game.LocalPlayer.Camera.Yaw = 0.0
=Game.LocalPlayer.Camera.Pitch = 0.0
*/
int l_Camera_index(lua_State *L)
{
    if (lua_type(L, 2) != LUA_TSTRING)
        return 0;

    uint32_t key = hash(lua_tostring(L, 2));
    bool valid_key = true;

    switch (key) {
        case hash("FOV"): {
            float fov;
            CTRPF::Process::ReadFloat(player_camera_offsets::cameraFOV, fov);
            lua_pushnumber(L, fov);
            break;
        }
        case hash("Yaw"):
            lua_pushnumber(L, Minecraft::GetYaw());
            break;
        case hash("Pitch"):
            lua_pushnumber(L, Minecraft::GetPitch());
            break;
        default:
            valid_key = false;
            break;
    }

    if (valid_key)
        return 1;
    else
        return 0;
}

int l_Camera_newindex(lua_State *L)
{
    if (lua_type(L, 2) != LUA_TSTRING)
        return luaL_error(L, "Attempt to set unknown member of object");

    uint32_t key = hash(lua_tostring(L, 2));
    bool valid_key = true;

    switch (key) {
        case hash("FOV"):
            CTRPF::Process::WriteFloat(player_camera_offsets::cameraFOV, luaL_checknumber(L, 3));
            break;
        case hash("Yaw"):
            Minecraft::SetYaw(luaL_checknumber(L, 3));
            break;
        case hash("Pitch"):
            Minecraft::SetPitch(luaL_checknumber(L, 3));
            break;
        default:
            valid_key = false;
            break;
    }

    if (valid_key)
        return 0;
    else
        return luaL_error(L, "'%s' is not a valid member of object or is not and editable value", key);
}

// ----------------------------------------------------------------------------

int l_register_Player_Camera_metatables(lua_State *L)
{
    luaL_newmetatable(L, "CameraMetatable");
    lua_pushcfunction(L, l_Camera_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_Camera_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
    return 0;
}

int l_register_Player_Camera(lua_State *L)
{
    // Required to be called inside table definition
    l_register_Player_Camera_metatables(L);

    lua_newtable(L); // LocalPlayer.Camera
    luaC_setmetatable(L, "PlayerCameraMetatable");
    lua_setfield(L, -2, "Camera");
    return 0;
}