#include "CoreInit.hpp"

#include <CTRPluginFramework.hpp>
#include <vector>
#include <algorithm>
#include "json.hpp"
#include "string_hash.hpp"

#include "Core/Debug.hpp"
#include "Core/Utils/Utils.hpp"
#include "Modules.hpp"
#include "CoreConstants.hpp"
#include "CoreGlobals.hpp"

using json = nlohmann::json;
namespace CTRPF = CTRPluginFramework;

extern int scriptsLoadedCount;
CTRPF::Clock timeoutLoadClock;

void TimeoutLoadHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutLoadClock.HasTimePassed(CTRPF::Milliseconds(2000)))
        luaL_error(L, "Script load exceeded execution time (2000 ms)");
}

void Core::LoadLuaEnv() {
    lua_State* L = Lua_global;
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

bool Core::LoadBuffer(const char *buffer, size_t size, const char* name) {
    lua_State* L = Lua_global;
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

bool Core::LoadScript(const std::string& fp)
{
    lua_State* L = Lua_global;
    std::string fileContent = Core::Utils::LoadFile(fp);
    if (fileContent.empty())
    {
        Core::Debug::LogMessage("Failed to open file"+fp, false);
        return false;
    }
    return LoadBuffer(fileContent.c_str(), fileContent.size(), fp.c_str());
}

void Core::PreloadScripts()
{
    CTRPF::Directory dir;
    CTRPF::Directory::Open(dir, PLUGIN_FOLDER"/scripts");
    if (!dir.IsOpen())
        return;

    std::vector<std::string> files;
    dir.ListFiles(files, ".lua");
    if (files.size() > 0) {
        Core::Debug::LogMessage("Loading scripts", false);
        for (auto &file : files) {
            if (strcmp(file.c_str() + file.size() - 4, ".lua") != 0) // Double check to skip not .lua files
                continue;

            std::string fullPath("sdmc:" + dir.GetFullName() + "/" + file);
            Core::Debug::LogMessage("Loading script '"+fullPath+"'", false);
            if (LoadScript(fullPath.c_str()))
                scriptsLoadedCount++;
        }
        lua_gc(Lua_global, LUA_GCCOLLECT, 0); // If needed collect all garbage
    }
}

bool LoadMod(const std::string& modName, std::unordered_map<std::string, std::string> &modsAvailable, std::vector<u32> &modsLoading, std::vector<u32> &modsLoaded, std::vector<u32> &modsDiscarded)
{
    if (std::find(modsDiscarded.begin(), modsDiscarded.end(), hash(modName.c_str())) != modsDiscarded.end() || 
    std::find(modsLoading.begin(), modsLoading.end(), hash(modName.c_str())) != modsLoading.end())
        return false;
    if (std::find(modsLoaded.begin(), modsLoaded.end(), hash(modName.c_str())) != modsLoaded.end())
        return true;

    modsLoading.emplace_back(hash(modName.c_str()));
    std::string fileContent = Core::Utils::LoadFile(PLUGIN_FOLDER "/mods/" + modsAvailable[modName] + "/mod.json");
    if (fileContent.empty()) {
        Core::Debug::LogError(CTRPF::Utils::Format("Failed to load '%s'. Failed to open 'mod.json'", modsAvailable[modName].c_str()));
        modsDiscarded.emplace_back(hash(modName.c_str()));
        return false;
    }
    
    json j = json::parse(fileContent, nullptr, false);
    fileContent.clear();
    if (j.is_discarded()) {
        Core::Debug::LogError(CTRPF::Utils::Format("Failed to load '%s'. Failed to parse 'mod.json'", modsAvailable[modName].c_str()));
        modsDiscarded.emplace_back(hash(modName.c_str()));
        return false;
    }

    if (j.contains("dependencies") && j["dependencies"].is_array()) {
        for (const auto& modDependency : j["dependencies"]) {
            bool sucess = false;
            if (modDependency.is_string() && modsAvailable.contains(modDependency))
                sucess = LoadMod(modsAvailable[modDependency], modsAvailable, modsLoading, modsLoaded, modsDiscarded);

            if (!sucess) {
                Core::Debug::LogError(CTRPF::Utils::Format("Failed to load '%s'. Failed to load dependency '%s'", modsAvailable[modName].c_str(), std::string(modDependency).c_str()));
                modsDiscarded.emplace_back(hash(modName.c_str()));
                return false;
            }
        }
    }

    if (j.contains("optional_dependencies") && j["optional_dependencies"].is_array()) {
        for (const auto& modDependency : j["optional_dependencies"]) {
            if (modDependency.is_string() && modsAvailable.contains(modDependency))
                LoadMod(modsAvailable[modDependency], modsAvailable, modsLoading, modsLoaded, modsDiscarded);
        }
    }

    if (!CTRPF::File::Exists(PLUGIN_FOLDER "/mods/" + modsAvailable[modName] + "/init.lua")) {
        Core::Debug::LogError(CTRPF::Utils::Format("Failed to load '%s'. Failed to open 'init.lua'", modsAvailable[modName].c_str()));
        modsDiscarded.emplace_back(hash(modName.c_str()));
        return false;
    }
    modPaths[modName] = PLUGIN_FOLDER "/mods/" + modsAvailable[modName];
    if (!Core::LoadScript(PLUGIN_FOLDER "/mods/" + modsAvailable[modName] + "/init.lua")) {
        Core::Debug::LogError(CTRPF::Utils::Format("Failed to load '%s'. Failed to load 'init.lua'", modsAvailable[modName].c_str()));
        modsDiscarded.emplace_back(hash(modName.c_str()));
        modPaths.erase(modName);
        return false;
    }
    auto it = std::find(modsLoading.begin(), modsLoading.end(), hash(modName.c_str()));
    if (it != modsLoading.end())
        modsLoading.erase(it);
    modsLoaded.emplace_back(hash(modName.c_str()));
    return true;
}

void Core::LoadMods() 
{
    CTRPF::Directory modsDir;
    CTRPF::Directory::Open(modsDir, PLUGIN_FOLDER "/mods");
    if (!modsDir.IsOpen()) {
        Core::Debug::LogMessage("No mods loaded", false);
        return;
    }

    std::unordered_map<std::string, std::string> modsAvailable;
    std::vector<u32> modsLoading;
    std::vector<u32> modsLoaded;
    std::vector<u32> modsDiscarded;

    std::vector<std::string> mods;
    modsDir.ListDirectories(mods);
    for (auto& dir : mods) {
        std::string fileContent = Core::Utils::LoadFile(PLUGIN_FOLDER "/mods/" + dir + "/mod.json");
        if (fileContent.empty()) {
            Core::Debug::LogError(CTRPF::Utils::Format("Failed to load '%s'. Failed to open 'mod.json'", dir.c_str()));
            continue;
        }
        
        json j = json::parse(fileContent, nullptr, false);
        if (j.is_discarded()) {
            Core::Debug::LogError(CTRPF::Utils::Format("Failed to load '%s'. Failed to parse 'mod.json'", dir.c_str()));
            continue;
        }

        if (j.contains("name") && j["name"].is_string()) {
            modsAvailable[j["name"]] = dir;
        } else {
            Core::Debug::LogError(CTRPF::Utils::Format("Failed to load '%s'. Missing name in 'mod.json'", dir.c_str()));
            continue;
        }
    }

    for (auto it = modsAvailable.begin(); it != modsAvailable.end(); it++) {
        if (std::find(modsDiscarded.begin(), modsDiscarded.end(), hash(it->first.c_str())) != modsDiscarded.end() || 
        std::find(modsLoaded.begin(), modsLoaded.end(), hash(it->first.c_str())) != modsLoaded.end())
            continue;
        LoadMod(it->first, modsAvailable, modsLoading, modsLoaded, modsDiscarded);
    }
    modsLoaded.clear();
    modsDiscarded.clear();
    modsLoading.clear();
    modsAvailable.clear();
}