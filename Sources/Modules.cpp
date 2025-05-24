#include "Modules.hpp"

#include <string>

#include "lua_json.hpp"

#include "Bits.hpp"
#include "System.hpp"
#include "Async.hpp"
#include "Debug.hpp"
#include "Gamepad.hpp"
#include "World.hpp"
#include "Player/Player.hpp"
#include "Event.hpp"

#include "Utils.hpp"

void loadScriptingModules(lua_State *L)
{
    luaopen_Bits(L);
    luaopen_System(L);
    luaopen_Async(L);

    register_lua_json_module(L);

    l_register_Utils(L);
    //Use global Game as entry point related to all game functions
    //$Game
    lua_newtable(L);
    lua_setglobal(L, "Game");
    l_register_Debug(L); // Maybe move to System or to a global idk
    l_register_Gamepad(L);
    l_register_World(L);
    l_register_LocalPlayer(L);
    l_register_Event(L);

    const char *lua_Code = R"(
        local realGame = readOnlyTable(Game, "Game")
        Game = nil

        setmetatable(_G, {
            __index = function(_, key)
                if key == "Game" then
                    return realGame
                else
                    return rawget(_G, key)
                end
            end,
            __newindex = function(_, key, value)
                if key == "Game" then
                    error("Cannot overwrite global 'Game'", 2)
                else
                    rawset(_G, key, value)
                end
            end
        })
    )";

    if (luaL_dostring(L, lua_Code))
    {
        DebugLogError("Engine error. Unable to set 'Game' global read-only: "+std::string(lua_tostring(L, -1)));
        lua_pop(L, 1);
    }

    l_unregister_Utils(L);
}