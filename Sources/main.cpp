#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>

#include <vector>
#include <string>
#include <unordered_map>

#include <cstdlib>
#include <cstdio>

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include "MemAddress.hpp"
#include "Modules.hpp"

void getDefaults();
void resetDefaults();

struct def_vals_s 
{
    float playerWaterVelocity;
    float cloudsHeight;
};

lua_State *Lua_global;
struct def_vals_s default_values;

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
                        OSD::Notify("Lua error: " + std::string(lua_tostring(L, -1)));
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
                    OSD::Notify("Lua error: " + std::string(lua_tostring(L, -1)));
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
        
        getDefaults();

        Lua_global = luaL_newstate();
        luaL_openlibs(Lua_global);
        loadScriptingModules(Lua_global);

        File scriptFO;
        File::Open(scriptFO, "sdmc:/mc3ds/scripts/main.lua", File::Mode::READ);
        scriptFO.Seek(0, File::SeekPos::END);
        long fileSize = scriptFO.Tell();
        scriptFO.Seek(0, File::SeekPos::SET);
        char *fileContent = (char *)malloc(fileSize + 1);
        if (fileContent != NULL)
        {
            scriptFO.Read(fileContent, fileSize);
            fileContent[fileSize] = '\0';
            int status_code = luaL_loadbuffer(Lua_global, fileContent, fileSize, "main.lua");
            if (status_code)
            {
                const char *error = lua_tostring(Lua_global, -1);
                OSD::Notify("Failed to load Lua script");
                OSD::Notify(error);
            }
            else
            {
                OSD::Notify("Lua script loaded successfully");
                // Set Lua path
                lua_getglobal(Lua_global, "package");
                lua_getfield(Lua_global, -1, "path");
                const char *current_path = lua_tostring(Lua_global, -1);
                lua_pop(Lua_global, 1);
                lua_pushfstring(Lua_global, "%s;sdmc:/mc3ds/scripts/?.lua;sdmc:/mc3ds/scripts/?/?.lua", current_path);
                lua_setfield(Lua_global, -2, "path");
                lua_pop(Lua_global, 1);

                // Execute script
                if (lua_pcall(Lua_global, 0, 0, 0))
                {
                    OSD::Notify("Lua error: " + std::string(lua_tostring(Lua_global, -1)));
                    lua_pop(Lua_global, 1);
                }
            }
        }
        scriptFO.Close();
        free(fileContent);

        // Start watcher threads
        Thread threads[1];
        s32 prio = 0;
        svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);

        threads[0] = threadCreate(ScriptEventHandler, (void*)(Lua_global), 2048, prio-1, -2, false);

        romfsInit();
        FILE *file = fopen("romfs:/atlas/atlas.items.meta_79954554.uvs", "rb");
        if (file) {
            OSD::Notify("File opened");
            fclose(file);
        }
        else
            OSD::Notify("File NOT opened");

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);

        // Init our menu entries & folders
        InitMenu(*menu);

        // Launch menu and mainloop
        menu->Run();

        delete menu;

        threadJoin(threads[0], U64_MAX);
        threadFree(threads[0]);

        // Exit plugin
        return 0;
    }
}

void getDefaults()
{
    CTRPluginFramework::Process::ReadFloat(value_address::playerWaterVelocity, default_values.playerWaterVelocity);
}

void resetDefaults()
{
    CTRPluginFramework::Process::WriteFloat(value_address::playerWaterVelocity, default_values.playerWaterVelocity);
}