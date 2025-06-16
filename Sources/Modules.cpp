#include "Modules.hpp"

#include <string>

#include <CTRPluginFramework.hpp>

#include "Core/Utils/lua_json.hpp"
#include "Core/Utils/FileLoader.hpp"

#include "Core/Utils/Bits.hpp"
#include "Core/System.hpp"
#include "Core/Filesystem.hpp"
#include "Core/Keyboard.hpp"
#include "Core/Async.hpp"
#include "Core/Debug.hpp"
#include "Core/Memory.hpp"
#include "Game/Gamepad.hpp"
#include "Game/World.hpp"
#include "Game/Player/Player.hpp"
#include "Game/Items.hpp"
#include "Core/Event.hpp"
#include "Core/Graphics.hpp"

#include "Core/Utils/Utils.hpp"

namespace Core {
    bool RegisterGameModule(lua_State *L)
    {
        //Use global Game as entry point related to all game functions
        //$Game
        lua_newtable(L);
        lua_setglobal(L, "Game");
        Core::Game::RegisterGamepadModule(L);
        Core::Game::RegisterWorldModule(L);
        Core::Game::RegisterLocalPlayerModule(L);
        Core::Game::RegisterItemsModule(L);
        Core::Game::RegisterEventModule(L);

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
            Core::Debug::LogError("Core::Load error: "+std::string(lua_tostring(L, -1)));
            lua_pop(L, 1);
            return false;
        }
        return true;
    }

    bool RegisterCoreModule(lua_State *L) {
        //Use global Core as entry point related to functions that are external to the game
        //$Core
        lua_newtable(L);
        lua_setglobal(L, "Core");
        Core::Module::RegisterDebugModule(L);
        Core::Module::RegisterSystemModule(L);
        Core::Module::RegisterKeyboardModule(L);
        Core::Module::RegisterFilesystemModule(L);
        Core::Module::RegisterGraphicsModule(L);
        Core::Module::RegisterMemoryModule(L);
        return true;
    }
}

void Core::LoadModules(lua_State *L)
{
    Core::RegisterCustomFileLoader(L);
    Core::PreloadJsonModule(L);

    Core::RegisterUtilsModule(L);
    
    Core::RegisterBitsModule(L);
    Core::RegisterAsyncModule(L);
    Core::RegisterCoreModule(L);
    Core::RegisterGameModule(L);

    Core::UnregisterUtilsModule(L);
}