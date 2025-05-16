#include "Gamepad.hpp"

#include <CTRPluginFramework.hpp>

namespace CTRPF = CTRPluginFramework;

//!include Sources/Modules.cpp
//$Game.Debug

// ----------------------------------------------------------------------------

/*
- Displays a notification on screen
## msg: string
### Game.Debug.message
*/
int l_Debug_message(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);

    CTRPluginFramework::OSD::Notify(msg);
    return 0;
}

static const luaL_Reg debug_functions[] =
{
    {"message", l_Debug_message},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

int l_register_Debug(lua_State *L)
{
    lua_getglobal(L, "Game");
    luaC_register_field(L, debug_functions, "Debug");
    lua_pop(L, 1);
    return 0;
}