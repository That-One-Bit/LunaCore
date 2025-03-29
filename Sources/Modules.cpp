#include "Modules.hpp"

#include "lua_json.hpp"

#include "Bits.hpp"
#include "Debug.hpp"
#include "System.hpp"
#include "Async.hpp"
#include "Gamepad.hpp"
#include "World.hpp"
#include "Camera.hpp"
#include "Player.hpp"

void loadScriptingModules(lua_State *L)
{
    register_lua_json_module(L);

    luaopen_Bits(L);
    luaopen_Debug(L);
    luaopen_System(L);
    luaopen_Async(L);
    luaopen_Gamepad(L);
    luaopen_World(L);
    luaopen_Camera(L);
    luaopen_Player(L);
}