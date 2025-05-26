#include "Modules.hpp"

#include <string>

#include "lua_json.hpp"

#include "FileLoader.hpp"

#include "Bits.hpp"
#include "System.hpp"
#include "Async.hpp"
#include "Debug.hpp"
#include "Gamepad.hpp"
#include "World.hpp"
#include "Player/Player.hpp"
#include "Event.hpp"
#include "Graphics.hpp"

#include "Utils.hpp"

namespace Core {
    bool RegisterGameModule(lua_State *L)
    {
        //Use global Game as entry point related to all game functions
        //$Game
        lua_newtable(L);
        lua_setglobal(L, "Game");
        Core::Game::RegisterDebugModule(L); // Maybe move to System or to a global idk
        Core::Game::RegisterGamepadModule(L);
        Core::Game::RegisterWorldModule(L);
        Core::Game::RegisterLocalPlayerModule(L);
        Core::Game::RegisterEventModule(L);
        Core::Game::RegisterGraphicsModule(L);

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
                __newindex = function(tbl, key, value)
                    if key == "Game" then
                        error("Cannot overwrite global 'Game'", 2)
                    else
                        rawset(tbl, key, value)
                    end
                end
            })
        )";
        if (luaL_dostring(L, lua_Code))
        {
            Core::Debug::LogError("Core error. Unable to set 'Game' global read-only: "+std::string(lua_tostring(L, -1)));
            lua_pop(L, 1);
            return false;
        }
        return true;
    }
}

void Core::LoadModules(lua_State *L)
{
    Core::RegisterCustomFileLoader(L);
    Core::PreloadJsonModule(L);

    Core::RegisterUtilsModule(L);
    
    Core::RegisterBitsModule(L);
    Core::RegisterSystemModule(L);
    Core::RegisterAsyncModule(L);
    Core::RegisterGameModule(L);

    Core::UnregisterUtilsModule(L);
}