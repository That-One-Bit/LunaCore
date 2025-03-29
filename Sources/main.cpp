#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

#include <cstdlib>

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include "MemAddress.hpp"
#include "Modules.hpp"

lua_State *Lua_global;

static bool keepRunning = true;

namespace CTRPluginFramework
{
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
        ToggleTouchscreenForceOn();
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

    void ScriptEventHandler(void *arg)
    {
        lua_State *L = (lua_State *)arg;
        u32 lastKeys = 0;

        while (keepRunning)
        {
            // KeyPressed Event
            u32 currentKeys = Controller::GetKeysPressed();
            if (currentKeys != lastKeys && currentKeys > 0)
            {
                lua_getglobal(L, "Gamepad");
                lua_getfield(L, -1, "KeyPressed");
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
                lua_pop(L, 2);
            }
            lastKeys = currentKeys;

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

            Sleep(Milliseconds(16));
        }
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
    }

    int main()
    {
        PluginMenu *menu = new PluginMenu("Action Replay", 0, 1, 0,
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
        {
            Directory dir;
            Directory::Open(dir, "sdmc:/mc3ds/scripts");
            std::vector<std::string> files;
            dir.ListFiles(files);
            for (auto &file : files)
            {
                LoadScript(Lua_global, ("sdmc:" + dir.GetFullName() + "/" + file).c_str());
            }
        }

        // Start watcher threads
        Thread threads[1];
        s32 prio = 0;
        svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);

        threads[0] = threadCreate(ScriptEventHandler, (void*)(Lua_global), 2048, prio-1, -2, false);

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);
        menu->ShowWelcomeMessage(false);

        // Init our menu entries & folders
        OSD::Notify("Script engine is active");
        InitMenu(*menu);

        // Launch menu and mainloop
        menu->Run();

        delete menu;

        keepRunning = false;
        threadJoin(threads[0], U64_MAX);
        threadFree(threads[0]);

        // Exit plugin
        return 0;
    }
}