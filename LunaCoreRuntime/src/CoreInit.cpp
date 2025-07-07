#include "CoreInit.hpp"

#include <CTRPluginFramework.hpp>

#include "Core/Debug.hpp"
#include "Core/Utils/Utils.hpp"
#include "Modules.hpp"
#include "CoreConstants.hpp"

namespace CTRPF = CTRPluginFramework;

extern lua_State *Lua_global;
extern int scriptsLoadedCount;
CTRPF::Clock timeoutLoadClock;

void TimeoutLoadHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutLoadClock.HasTimePassed(CTRPF::Milliseconds(2000)))
        luaL_error(L, "Script load exceeded execution time (2000 ms)");
}

void Core::LoadLuaEnv(lua_State *L) {
    Core::Debug::LogMessage("Loading standard libs", false);
    luaL_openlibs(L);
    Core::Debug::LogMessage("Loading core modules", false);
    Core::LoadModules(L);

    // Set Lua path
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    const char *current_path = lua_tostring(L, -1);
    lua_pop(L, 1);
    std::string newPath(current_path);
    newPath += ";" PLUGIN_FOLDER "/scripts/?.lua;" PLUGIN_FOLDER "/scripts/?/init.lua";
    lua_pushstring(L, newPath.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
    Core::Debug::LogMessage("Lua environment loaded", false);
}

bool Core::LoadBuffer(lua_State *L, const char *buffer, size_t size, const char* name) {
    bool success = true;
    lua_newtable(L);
    lua_newtable(L);
    lua_getglobal(L, "_G");
    lua_setfield(L, -2, "__index");
    lua_getglobal(L, "_G");
    lua_getmetatable(L, -1);
    lua_getfield(L, -1, "__newindex");
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 2);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    luaL_unref(L, LUA_REGISTRYINDEX, ref);

    int status_code = luaL_loadbuffer(L, buffer, size, name);
    if (status_code)
    {
        Core::Debug::LogError("Script load error: "+std::string(lua_tostring(L, -1)));
        lua_pop(L, 2);
        success = false;
    }
    else
    {
        // Execute script on a private env
        lua_pushvalue(L, -2);
        lua_setfenv(L, -2);

        lua_sethook(L, TimeoutLoadHook, LUA_MASKCOUNT, 100);
        timeoutLoadClock.Restart();
        if (lua_pcall(L, 0, 0, 0))
        {
            std::string error_msg(lua_tostring(L, -1));
            Core::Utils::Replace(error_msg, "\t", "    ");
            Core::Debug::LogError("Script load error: "+error_msg);
            lua_pop(L, 2);
            success = false;
        }
        else
        {
            // If success copy state to global env
            lua_getglobal(L, "_G");
            lua_pushnil(L);
            while (lua_next(L, -3))
            {
                lua_pushvalue(L, -2);
                lua_insert(L, -2);
                lua_settable(L, -4);
            }
            lua_pop(L, 2);
        }
        lua_sethook(L, nullptr, 0, 0);
    }
    return success;
}

bool Core::LoadScript(lua_State *L, const std::string& fp)
{
    std::string fileContent = Core::Utils::LoadFile(fp);
    if (fileContent.empty())
    {
        Core::Debug::LogMessage("Failed to open file"+fp, false);
        return false;
    }
    return LoadBuffer(L, fileContent.c_str(), fileContent.size(), fp.c_str());
}

void Core::PreloadScripts()
{
    Core::Debug::LogMessage("Loading scripts", false);
    CTRPF::Directory dir;
    CTRPF::Directory::Open(dir, PLUGIN_FOLDER"/scripts");
    if (!dir.IsOpen()) {
        CTRPF::MessageBox("Failed to reload scripts")();
        return;
    }
    std::vector<std::string> files;
    dir.ListFiles(files, ".lua");
    for (auto &file : files) {
        if (strcmp(file.c_str() + file.size() - 4, ".lua") != 0) // Double check to skip not .lua files
            continue;

        std::string fullPath("sdmc:" + dir.GetFullName() + "/" + file);
        Core::Debug::LogMessage("Loading script '"+fullPath+"'", false);
        if (LoadScript(Lua_global, fullPath.c_str()))
            scriptsLoadedCount++;
    }
    lua_gc(Lua_global, LUA_GCCOLLECT, 0); // If needed collect all garbage
}