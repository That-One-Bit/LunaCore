#include "Gamepad.hpp"

#include <CTRPluginFramework.hpp>

int l_Debug_Message(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);

    CTRPluginFramework::OSD::Notify(msg);
    return 0;
}

static const luaL_Reg debug_functions[] =
{
    {"Message", l_Debug_Message},
    {NULL, NULL}
};

int luaopen_Debug(lua_State *L)
{
    lua_newtable(L);
    luaL_register(L, NULL, debug_functions);
    lua_setglobal(L, "Debug");
    return 0;
}