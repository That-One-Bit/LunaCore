#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <atomic>

#include <cstdlib>
#include <cstring>

#include "lua_common.h"
#include "Minecraft.hpp"
#include "Modules.hpp"
#include "Debug.hpp"
#include "Event.hpp"
#include "Async.hpp"
#include "Graphics.hpp"

#define IS_VERSION_COMPATIBLE(version) ((version) == 9408) // 1.9.19
#define EMULATOR_VERSION(version) ((version) == 9216)
#define IS_TARGET_ID(id) ((id) == 0x00040000001B8700LL)

#define PLUGIN_VERSION_MAJOR 0
#define PLUGIN_VERSION_MINOR 1
#define PLUGIN_VERSION_PATCH 0
#define LOG_FILE "sdmc:/mc3ds/log.txt"

lua_State *Lua_global;
int scriptsLoadedCount = 0;
CTRPluginFramework::Clock timeoutLoadClock;
std::atomic<int> luaMemoryUsage = 0;

void TimeoutLoadHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutLoadClock.HasTimePassed(CTRPluginFramework::Milliseconds(2000)))
        luaL_error(L, "Script load exceeded execution time (2000 ms)");
}

namespace CTRPluginFramework
{
    PluginMenu *menu;
    // This patch the NFC disabling the touchscreen when scanning an amiibo, which prevents ctrpf to be used
    static void    ToggleTouchscreenForceOn(void)
    {
        static u32 original = 0;
        static u32 *patchAddress = nullptr;

        if (patchAddress && original)
        {
            *patchAddress = original;
            return;
        }

        static const std::vector<u32> pattern =
        {
            0xE59F10C0, 0xE5840004, 0xE5841000, 0xE5DD0000,
            0xE5C40008, 0xE28DD03C, 0xE8BD80F0, 0xE5D51001,
            0xE1D400D4, 0xE3510003, 0x159F0034, 0x1A000003
        };

        Result  res;
        Handle  processHandle;
        s64     textTotalSize = 0;
        s64     startAddress = 0;
        u32 *   found;

        if (R_FAILED(svcOpenProcess(&processHandle, 16)))
            return;

        svcGetProcessInfo(&textTotalSize, processHandle, 0x10002);
        svcGetProcessInfo(&startAddress, processHandle, 0x10005);
        if(R_FAILED(svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, processHandle, (u32)startAddress, textTotalSize)))
            goto exit;

        found = (u32 *)Utils::Search<u32>(0x14000000, (u32)textTotalSize, pattern);

        if (found != nullptr)
        {
            original = found[13];
            patchAddress = (u32 *)PA_FROM_VA((found + 13));
            found[13] = 0xE1A00000;
        }

        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, textTotalSize);
        exit:
        svcCloseHandle(processHandle);
    }

    // This function is called before main and before the game starts
    // Useful to do code edits safely
    void    PatchProcess(FwkSettings &settings)
    {
        if (!IS_TARGET_ID(Process::GetTitleID()))
            return;
        ToggleTouchscreenForceOn();
        u16 gameVersion = Process::GetVersion();
        if (IS_VERSION_COMPATIBLE(gameVersion) || EMULATOR_VERSION(gameVersion))
            Minecraft::PatchProcess();
    }

    // This function is called when the process exits
    // Useful to save settings, undo patchs or clean up things
    void    OnProcessExit(void)
    {
        lua_close(Lua_global);
        ToggleTouchscreenForceOn();
    }

    bool LoadScript(lua_State *L, const std::string& fp)
    {
        bool success = true;
        File scriptFO;
        File::Open(scriptFO, fp, File::Mode::READ);
        if (!scriptFO.IsOpen())
        {
            Core::Debug::LogMessage("Failed to open file"+fp, false);
            return false;
        }
        scriptFO.Seek(0, File::SeekPos::END);
        long fileSize = scriptFO.Tell();
        scriptFO.Seek(0, File::SeekPos::SET);
        char *fileContent = (char *)malloc(fileSize + 1);
        if (fileContent != NULL)
        {
            scriptFO.Read(fileContent, fileSize);
            fileContent[fileSize] = '\0';

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

            int status_code = luaL_loadbuffer(L, fileContent, fileSize, fp.c_str());
            if (status_code)
            {
                Core::Debug::LogError(std::string(lua_tostring(L, -1)));
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
                    Core::Debug::LogError("\""+fp+"\""+": " + std::string(lua_tostring(L, -1)));
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
        }
        scriptFO.Close();
        free(fileContent);
        return success;
    }

    void UpdateLuaStatistics()
    {
        lua_State *L = Lua_global;
        int memusgkb = lua_gc(L, LUA_GCCOUNT, 0);
        int memusgb = lua_gc(L, LUA_GCCOUNTB, 0);
        luaMemoryUsage.store(memusgkb * 1024 + memusgb);
    }

    bool DrawMonitors(const Screen &screen)
    {
        if (screen.IsTop)
        {
            screen.Draw("Lua memory: "+std::to_string(luaMemoryUsage.load()), 10, 10, Color::Black, Color(0, 0, 0, 0));
        }
        return false;
    }

    bool ScriptingNewFrameEventCallback(const Screen &screen)
    {
        // TODO: Cannot access to lua state
        lua_State *L = Lua_global;

        lua_getglobal(L, "Game");
        lua_getfield(L, -1, "Event");
        lua_getfield(L, -1, "OnNewFrame");
        lua_getfield(L, -1, "Trigger");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, -2);
            lua_pushboolean(L, screen.IsTop);
            if (lua_pcall(L, 2, 0, 0))
            {
                Core::Debug::LogMessage("Script error: " + std::string(lua_tostring(L, -1)), true);
                lua_pop(L, 1);
            }
        }
        else
            lua_pop(L, 1);
        lua_pop(L, 3);
        return true;
    }

    void PreloadScripts()
    {
        static int readFiles = 0;
        Directory dir;
        Directory::Open(dir, "sdmc:/mc3ds/scripts");
        if (!dir.IsOpen()) {
            *menu -= PreloadScripts;
            return;
        }
        std::vector<std::string> files;
        if (readFiles >= dir.ListFiles(files, ".lua")) {
            *menu -= PreloadScripts;
            return;
        }
        if (strcmp(files[readFiles].c_str() + files[readFiles].size() - 4, ".lua") != 0) { // Double check to skip not .lua files
            readFiles++;
            return;
        }
        std::string fullPath("sdmc:" + dir.GetFullName() + "/" + files[readFiles]);
        Core::Debug::LogMessage("Loading script '"+fullPath+"'", false);
        if (LoadScript(Lua_global, fullPath.c_str()))
            scriptsLoadedCount++;
        readFiles++;
    }

    void    InitMenu(PluginMenu &menu)
    {
        // Create your entries here, or elsewhere
        // You can create your entries whenever/wherever you feel like it
        
        // Example entry
        /*menu += new MenuEntry("Test", nullptr, [](MenuEntry *entry)
        {
            std::string body("What's the answer ?\n");

            body += std::to_string(42);

            MessageBox("UA", body)();
        });*/
        auto developerFolder = new MenuFolder("Developer Tools");
        menu.Append(developerFolder);
    }

    int main()
    {
        if (!IS_TARGET_ID(Process::GetTitleID()))
            return 0;

        if (!Directory::IsExists("sdmc:/mc3ds"))
            Directory::Create("sdmc:/mc3ds");
        if (!Core::Debug::OpenLogFile(LOG_FILE))
            OSD::Notify("Failed to open log file");

        Core::Debug::LogMessage("Starting plugin", false);
        Core::Debug::LogMessage("Plugin version: "+std::to_string(PLUGIN_VERSION_MAJOR)+"."+std::to_string(PLUGIN_VERSION_MINOR)+"."+std::to_string(PLUGIN_VERSION_PATCH), false);

        menu = new PluginMenu("Script Engine", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH,
            "Allows to execute Lua scripts.");

        Core::Debug::LogMessage("Loading Lua environment", false);
        Lua_global = luaL_newstate();
        luaL_openlibs(Lua_global);
        Core::Debug::LogMessage("Loading scripting modules", false);
        Core::LoadModules(Lua_global);

        // Set Lua path
        lua_getglobal(Lua_global, "package");
        lua_getfield(Lua_global, -1, "path");
        const char *current_path = lua_tostring(Lua_global, -1);
        lua_pop(Lua_global, 1);
        lua_pushfstring(Lua_global, "%s;sdmc:/mc3ds/scripts/?.lua;sdmc:/mc3ds/scripts/?/init.lua", current_path);
        lua_setfield(Lua_global, -2, "path");
        lua_pop(Lua_global, 1);
        Core::Debug::LogMessage("Lua environment loaded", false);

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);
        menu->ShowWelcomeMessage(false);

        Core::Debug::LogMessage("Script core loaded", true);
        u16 gameVersion = Process::GetVersion();
        if (EMULATOR_VERSION(gameVersion) || IS_VERSION_COMPATIBLE(gameVersion))
            Core::Debug::LogMessage("Detected version '"+std::to_string(gameVersion)+"' (1.9.19). Enabled all features", false);
        else
            Core::Debug::LogMessage("Incompatible version '"+std::to_string(gameVersion)+"' required 9408 (1.9.19)! Some features may be disabled", true);

        OSD::Run(DrawMonitors);

        // Wait until some things has been loaded otherwise instability may occur
        Core::Debug::LogMessage("Waiting to load scripts", false);
        Sleep(Seconds(15));
        if (Directory::IsExists("sdmc:/mc3ds/scripts")) {
            menu->Callback(PreloadScripts); // Callback that iterates over all scripts under 'sdmc:/mc3ds/scripts'

            Directory dir;
            Directory::Open(dir, "sdmc:/mc3ds/scripts");
            if (dir.IsOpen()) {
                std::vector<std::string> dirFiles;
                if (dir.ListFiles(dirFiles, ".lua") > 0) { // Only add core callbacks if any script under directory
                    int luaFiles = 0;
                    for (auto file : dirFiles) {
                        if (strcmp(file.c_str() + file.size() - 4, ".lua") == 0)
                            luaFiles++;
                    }
                    if (luaFiles > 0) {
                        menu->Callback(Core::EventHandlerCallback);
                        OSD::Run(Core::GraphicsHandlerCallback);
                        menu->Callback(Core::AsyncHandlerCallback);
                        menu->Callback(UpdateLuaStatistics);
                    }
                }
            }
        }

        // Init our menu entries & folders
        InitMenu(*menu);

        // Launch menu and mainloop
        Core::Debug::LogMessage("Starting plugin mainloop", false);
        menu->Run();
        Core::Debug::CloseLogFile();

        delete menu;

        // Exit plugin
        return 0;
    }
}