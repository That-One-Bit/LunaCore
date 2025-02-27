#include "Modules.hpp"

#include "Bits.hpp"
#include "Debug.hpp"
#include "Gamepad.hpp"
#include "World.hpp"
#include "Player.hpp"

void loadScriptingModules(lua_State *L)
{
    luaopen_Bits(L);
    luaopen_Debug(L);
    luaopen_Gamepad(L);
    luaopen_World(L);
    luaopen_Player(L);
}