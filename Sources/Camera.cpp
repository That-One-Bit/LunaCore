#include "Camera.hpp"

#include <CTRPluginFramework.hpp>

#include "MemAddress.hpp"

int l_Camera_FOV_get(lua_State *L) 
{
    float fov;
    CTRPluginFramework::Process::ReadFloat(value_address::cameraFOV, fov);
    lua_pushnumber(L, fov);
    return 1;
}

int l_Camera_FOV_set(lua_State *L) 
{
    float fov = luaL_checknumber(L, 1);

    CTRPluginFramework::Process::WriteFloat(value_address::cameraFOV, fov);
    return 0;
}

static const luaL_Reg camera_fov_methods[] =
{
    {"get", l_Camera_FOV_get},
    {"set", l_Camera_FOV_set},
    {NULL, NULL}
};

int luaopen_Player(lua_State *L)
{
    lua_newtable(L); // Camera

    lua_newtable(L); // FOV
    luaL_register(L, NULL, camera_fov_methods);
    lua_setfield(L, -2, "FOV");
    
    lua_setglobal(L, "Camera");
    return 0;
}
