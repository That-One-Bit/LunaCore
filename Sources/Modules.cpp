#include "Modules.hpp"

#include "lua_json.hpp"

#include "Bits.hpp"
#include "System.hpp"
#include "Async.hpp"
#include "Debug.hpp"
#include "Gamepad.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "Event.hpp"

void loadScriptingModules(lua_State *L)
{
    luaopen_Bits(L);
    luaopen_System(L);
    luaopen_Async(L);

    register_lua_json_module(L);

    //Use global Game as entry point related to all game functions
    //$Game
    lua_newtable(L);
    lua_setglobal(L, "Game");
    l_register_Debug(L); // Maybe move to System or to a global idk
    l_register_Gamepad(L);
    l_register_World(L);
    l_register_LocalPlayer(L);
    l_register_Event(L);
}