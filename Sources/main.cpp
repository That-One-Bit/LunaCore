#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cstdlib>

#include <cstdlib>

#include "lua_common.h"
#include "Minecraft.hpp"
#include "Modules.hpp"

#define IS_VERSION_COMPATIBLE(version) ((version) == 9408) // 1.9.19
#define EMULATOR_VERSION(version) ((version) == 9216)
#define IS_TARGET_ID(id) ((id) == 0x00040000001B8700LL)

lua_State *Lua_global;
int scriptsLoadedCount = 0;

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

    bool LoadScript(lua_State *L, const char *fp)
    {
        bool success = true;
        File scriptFO;
        File::Open(scriptFO, fp, File::Mode::READ);
        if (!scriptFO.IsOpen())
        {
            std::string message("Failed to open script: ");
            message += fp;
            OSD::Notify(message);
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
            lua_setmetatable(L, -2);

            int status_code = luaL_loadbuffer(L, fileContent, fileSize, fp);
            if (status_code)
            {
                const char *error = lua_tostring(L, -1);
                OSD::Notify(error);
                lua_pop(L, 2);
                success = false;
            }
            else
            {
                // Execute script on a private env
                lua_pushvalue(L, -2);
                lua_setfenv(L, -2);

                if (lua_pcall(L, 0, 0, 0))
                {
                    OSD::Notify("Script error: " + std::string(lua_tostring(L, -1)));
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
            }
        }
        scriptFO.Close();
        free(fileContent);
        return success;
    }

    bool DrawMonitors(const Screen &screen)
    {
        if (screen.IsTop)
        {
            int memusgkb = lua_gc(Lua_global, LUA_GCCOUNT, 0);
            int memusgb = lua_gc(Lua_global, LUA_GCCOUNTB, 0);
            screen.Draw("Lua memory: "+std::to_string(memusgkb * 1024 + memusgb), 10, 10, Color::Black, Color(0, 0, 0, 0));
        }
        return false;
    }

    bool ScriptingNewFrameEventCallback(const Screen &screen)
    {
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
                OSD::Notify("Script error: " + std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        else
            lua_pop(L, 1);
        lua_pop(L, 3);
        return true;
    }

    void ScriptingEventHandlerCallback()
    {
        lua_State *L = Lua_global;

        // KeyPressed Event
        u32 pressedKeys = Controller::GetKeysPressed();
        if (pressedKeys > 0)
        {
            lua_getglobal(L, "Game");
            lua_getfield(L, -1, "Event");
            lua_getfield(L, -1, "OnKeyPressed");
            lua_getfield(L, -1, "Trigger");

            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -2);
                if (lua_pcall(L, 1, 0, 0))
                {
                    OSD::Notify("Script error: " + std::string(lua_tostring(L, -1)));
                    lua_pop(L, 1);
                }
            }
            else
                lua_pop(L, 1);
            lua_pop(L, 3);
        }

        u32 downKeys = Controller::GetKeysDown();
        if (downKeys > 0)
        {
            lua_getglobal(L, "Game");
            lua_getfield(L, -1, "Event");
            lua_getfield(L, -1, "OnKeyDown");
            lua_getfield(L, -1, "Trigger");

            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -2);
                if (lua_pcall(L, 1, 0, 0))
                {
                    OSD::Notify("Script error: " + std::string(lua_tostring(L, -1)));
                    lua_pop(L, 1);
                }
            }
            else
                lua_pop(L, 1);
            lua_pop(L, 3);
        }

        u32 releasedKeys = Controller::GetKeysReleased();
        if (releasedKeys > 0)
        {
            lua_getglobal(L, "Game");
            lua_getfield(L, -1, "Event");
            lua_getfield(L, -1, "OnKeyReleased");
            lua_getfield(L, -1, "Trigger");

            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -2);
                if (lua_pcall(L, 1, 0, 0))
                {
                    OSD::Notify("Script error: " + std::string(lua_tostring(L, -1)));
                    lua_pop(L, 1);
                }
            }
            else
                lua_pop(L, 1);
            lua_pop(L, 3);
        }

        // Coroutines
        lua_getglobal(L, "Async");
        lua_getfield(L, -1, "tick");
        
        if (lua_isfunction(L, -1))
        {
            if (lua_pcall(L, 0, 0, 0))
            {
                OSD::Notify("Script error: " + std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        else
            lua_pop(L, 1);
        lua_pop(L, 1);
    }

    void PreloadScripts()
    {
        Directory dir;
        Directory::Open(dir, "sdmc:/mc3ds/scripts");
        std::vector<std::string> files;
        if (scriptsLoadedCount >= dir.ListFiles(files))
        {
            *menu -= PreloadScripts;
            return;
        }
        LoadScript(Lua_global, ("sdmc:" + dir.GetFullName() + "/" + files[scriptsLoadedCount]).c_str());
        scriptsLoadedCount++;
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

        menu = new PluginMenu("Action Replay", 0, 1, 0,
            "A blank template plugin.\nGives you access to the ActionReplay and others tools.");

        Lua_global = luaL_newstate();
        luaL_openlibs(Lua_global);
        loadScriptingModules(Lua_global);

        // Set Lua path
        lua_getglobal(Lua_global, "package");
        lua_getfield(Lua_global, -1, "path");
        const char *current_path = lua_tostring(Lua_global, -1);
        lua_pop(Lua_global, 1);
        lua_pushfstring(Lua_global, "%s;sdmc:/mc3ds/scripts/?.lua;sdmc:/mc3ds/scripts/?/init.lua", current_path);
        lua_setfield(Lua_global, -2, "path");
        lua_pop(Lua_global, 1);

        if (Directory::IsExists("sdmc:/mc3ds/scripts"))
            menu->Callback(PreloadScripts);

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);
        menu->ShowWelcomeMessage(false);

        OSD::Notify("Script engine initialized");
        u16 gameVersion = Process::GetVersion();
        if (EMULATOR_VERSION(gameVersion) || IS_VERSION_COMPATIBLE(gameVersion))
            OSD::Notify("Detected version '"+std::to_string(gameVersion)+"' (1.9.19). Enabled all features");
        else
            OSD::Notify("Incompatible version '"+std::to_string(gameVersion)+"' required 9408 (1.9.19)! Some features may be disabled");

        OSD::Run(DrawMonitors);
        menu->Callback(ScriptingEventHandlerCallback);
        OSD::Run(ScriptingNewFrameEventCallback);

        // Init our menu entries & folders
        InitMenu(*menu);

        // Launch menu and mainloop
        menu->Run();

        delete menu;

        // Exit plugin
        return 0;
    }
}