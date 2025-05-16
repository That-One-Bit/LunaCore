#include "World.hpp"

#include <CTRPluginFramework.hpp>

namespace CTRPF = CTRPluginFramework;

enum world_offsets : u32
{
    cloudsHeight = 0x100000 + 0x2C5398
};

//$Game.World

// ----------------------------------------------------------------------------

//$Game.World.CloudsHeight

/*
- Sets world clouds height
## height: number
### Game.World.CloudsHeight.set
*/
int l_World_CloudsHeight_set(lua_State *L)
{
    float height = luaL_checknumber(L, 1);

    CTRPF::Process::WriteFloat(world_offsets::cloudsHeight, height);
    return 0;
}

static const luaL_Reg world_clouds_functions[] =
{
    {"set", l_World_CloudsHeight_set},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

int l_register_World(lua_State *L)
{
    lua_getglobal(L, "Game");
    lua_newtable(L);
    luaC_register_field(L, world_clouds_functions, "CloudsHeight");
    lua_setfield(L, -2, "World");
    lua_pop(L, 1);
    return 0;
}