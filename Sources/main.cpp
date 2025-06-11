#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>

#include <string>
#include <unordered_map>
#include <atomic>

#include <cstring>

#include "lua_common.h"
#include "Minecraft.hpp"
#include "Modules.hpp"
#include "Debug.hpp"
#include "Event.hpp"
#include "Async.hpp"
#include "Graphics.hpp"
#include "Utils/Utils.hpp"
#include "Config.hpp"

#include "Game/GameHooks.hpp"
#include "Game/GameState.hpp"
#include "Game/MainMenuLayoutLoad.hpp"
#include "Game/LoadingWorldScreenMessage.hpp"

#define IS_VUSA_COMP(id, version) ((id) == 0x00040000001B8700LL && (version) == 9408) // 1.9.19 USA
#define IS_VEUR_COMP(id, version) ((id) == 0x000400000017CA00LL && (version) == 9392) // 1.9.19 EUR
#define IS_VJPN_COMP(id, version) ((id) == 0x000400000017FD00LL && (version) == 9424) // 1.9.19 JPN
#define IS_TARGET_ID(id) ((id) == 0x00040000001B8700LL || (id) == 0x000400000017CA00LL || (id) == 0x000400000017FD00LL)

#define PLUGIN_VERSION_MAJOR 0
#define PLUGIN_VERSION_MINOR 9
#define PLUGIN_VERSION_PATCH 0
#define PLUGIN_FOLDER "sdmc:/Minecraft 3DS"
#define LOG_FILE PLUGIN_FOLDER"/log.txt"
#define CONFIG_FILE PLUGIN_FOLDER"/config.txt"
#define BASE_OFF 0x100000

namespace CTRPF = CTRPluginFramework;

lua_State *Lua_global;
int scriptsLoadedCount = 0;
CTRPF::Clock timeoutLoadClock;
std::atomic<int> luaMemoryUsage = 0;
bool enabledPatching = true;
std::unordered_map<std::string, std::string> config;
GameState_s GameState;
u32 currentCamFOVOffset = 0;

void TimeoutLoadHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutLoadClock.HasTimePassed(CTRPF::Milliseconds(2000)))
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
        u64 titleID = Process::GetTitleID();
        if (!IS_TARGET_ID(titleID))
            return;
        settings.UseGameHidMemory = true;
        ToggleTouchscreenForceOn();

        if (!Directory::IsExists(PLUGIN_FOLDER))
            Directory::Create(PLUGIN_FOLDER);
        if (!Core::Debug::OpenLogFile(LOG_FILE))
            OSD::Notify(Utils::Format("Failed to open log file '%s'", LOG_FILE));
        Core::Debug::LogMessage(Utils::Format("Loading config file '%s'", CONFIG_FILE), false);
        config = Core::Config::LoadConfig(CONFIG_FILE);

        bool disableZRandZL = Core::Config::GetBoolValue(config, "disable_zl_and_zr", false);

        u16 gameVer = Process::GetVersion();
        if (IS_VUSA_COMP(titleID, gameVer) || IS_VEUR_COMP(titleID, gameVer) || IS_VJPN_COMP(titleID, gameVer) || System::IsCitra()) {
            Minecraft::PatchProcess();
            SetMainMenuLayoutLoadCallback();
            SetLoadingWorldScreenMessageCallback();
            SetLeaveLevelPromptCallback();
            if (disableZRandZL) {
                Process::Write32(0x819530+BASE_OFF, 0); // Pos keycode for ZL
                Process::Write32(0x819534+BASE_OFF, 0); // Pos keycode for ZR
            }
            //hookSomeFunctions();
        } else {
            enabledPatching = false;
        }
    }

    // This function is called when the process exits
    // Useful to save settings, undo patchs or clean up things
    void    OnProcessExit(void)
    {
        ToggleTouchscreenForceOn();
    }

    bool LoadScript(lua_State *L, const std::string& fp)
    {
        bool success = true;
        std::string fileContent = Core::Utils::LoadFile(fp);
        if (fileContent.empty())
        {
            Core::Debug::LogMessage("Failed to open file"+fp, false);
            return false;
        }
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

        int status_code = luaL_loadbuffer(L, fileContent.c_str(), fileContent.size(), fp.c_str());
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

    void PreloadScripts()
    {
        static int readFiles = 0;
        Directory dir;
        Directory::Open(dir, PLUGIN_FOLDER"/scripts");
        if (!dir.IsOpen()) {
            *menu -= PreloadScripts;
            return;
        }
        std::vector<std::string> files;
        if (readFiles >= dir.ListFiles(files, ".lua")) {
            *menu -= PreloadScripts;
            lua_gc(Lua_global, LUA_GCCOLLECT, 0); // If needed collect all garbage
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

    void searchPlayerCamera(std::vector<u32> &out) {
        std::vector<u32> firstMatchAddress;
        u32 startAddr = 0x30000000;
        u32 endAddr = 0x35F80000;

        float value = 70.0f;
        if (std::memcmp(&value, (u32*)0x341F313C, 4) == 0) {
            out.push_back(0x341F313C);
            return;
        }
        u32 actualAddr = startAddr;
        while (actualAddr < endAddr) {
            if (std::memcmp(&value, (u32*)actualAddr, 4) == 0) {
                firstMatchAddress.push_back(actualAddr);
            }
            actualAddr += 4;
        }

        for (auto addr : firstMatchAddress) {
            u32 offsetAddress = addr - 0x118;
            std::vector<u32> newAddress;
            u32 value;
            int count = 0;
            actualAddr = startAddr;
            while (actualAddr < endAddr) {
                if (std::memcmp(&offsetAddress, (u32*)actualAddr, 4) == 0) {
                    newAddress.push_back(actualAddr);
                    count++;
                }
                if (count > 10)
                    break;
                actualAddr += 4;
            }
            if (newAddress.size() == 10) {
                out.push_back(addr);
                break;
            }
        }
    }

    void changeCameraFOV(MenuEntry *entry) {
        std::vector<u32> cameraFOVAddrs;
        if (currentCamFOVOffset != 0) {
            Keyboard keyboard("Enter FOV value");
            float fovval;
            keyboard.Open(fovval);
            Process::WriteFloat(currentCamFOVOffset, fovval);
            return;
        }
        if (MessageBox("First use! This can freeze the game for up to 45 seconds", DialogType::DialogOkCancel)()) {
            searchPlayerCamera(cameraFOVAddrs);
            if (cameraFOVAddrs.size() == 0) {
                MessageBox("No matches")();
            } else {
                MessageBox("Found a match")();
                Keyboard keyboard("Enter FOV value");
                float fovval;
                keyboard.Open(fovval);
                u32 fovAddr = cameraFOVAddrs[0];
                Process::WriteFloat(fovAddr, fovval);
                currentCamFOVOffset = fovAddr;
            }
        }
    }

    void InitMenu(PluginMenu &menu)
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
        auto optionsFolder = new MenuFolder("Options");
        auto devFolder = new MenuFolder("Experiments");
        optionsFolder->Append(new MenuEntry("Toggle Script Loader", nullptr, [](MenuEntry *entry)
        {
            bool changed = false;
            if (config["enable_scripts"] == "true") {
                if (MessageBox("Do you want to DISABLE script loader?", DialogType::DialogYesNo)()) {
                    config["enable_scripts"] = "false";
                    changed = true;
                }
            } else {
                if (MessageBox("Do you want to ENABLE script loader?", DialogType::DialogYesNo)()) {
                    config["enable_scripts"] = "true";
                    changed = true;
                }
            }
            if (changed) {
                MessageBox("Restart the game to apply the changes")();
                if (!Core::Config::SaveConfig(CONFIG_FILE, config))
                    Core::Debug::LogMessage("Failed to save configs", true);
            }
        }));
        optionsFolder->Append(new MenuEntry("Toggle Menu Layout", nullptr, [](MenuEntry *entry)
        {
            bool changed = false;
            if (config["custom_game_menu_layout"] == "true") {
                if (MessageBox("Do you want to DISABLE custom menu layout?", DialogType::DialogYesNo)()) {
                    config["custom_game_menu_layout"] = "false";
                    changed = true;
                }
            } else {
                if (MessageBox("Do you want to ENABLE custom menu layout?", DialogType::DialogYesNo)()) {
                    config["custom_game_menu_layout"] = "true";
                    changed = true;
                }
            }
            if (changed) {
                MessageBox("Restart the game to apply the changes")();
                if (!Core::Config::SaveConfig(CONFIG_FILE, config))
                    Core::Debug::LogMessage("Failed to save configs", true);
            }
        }));
        optionsFolder->Append(new MenuEntry("Toggle Block ZL and ZR keys", nullptr, [](MenuEntry *entry)
        {
            bool changed = false;
            if (config["disable_zl_and_zr"] == "true") {
                if (MessageBox("Do you want to ENABLE ZL and ZR keys?", DialogType::DialogYesNo)()) {
                    config["disable_zl_and_zr"] = "false";
                    Process::Write32(0x819530+BASE_OFF, KEY_ZL); // Pos keycode for ZL
                    Process::Write32(0x819534+BASE_OFF, KEY_ZR); // Pos keycode for ZR
                    changed = true;
                }
            } else {
                if (MessageBox("Do you want to DISABLE ZL and ZR keys (only scripts will be able to use them)?", DialogType::DialogYesNo)()) {
                    config["disable_zl_and_zr"] = "true";
                    Process::Write32(0x819530+BASE_OFF, 0); // Pos keycode for ZL
                    Process::Write32(0x819534+BASE_OFF, 0); // Pos keycode for ZR
                    changed = true;
                }
            }
            if (changed) {
                MessageBox("Restart the game to apply the changes")();
                if (!Core::Config::SaveConfig(CONFIG_FILE, config))
                    Core::Debug::LogMessage("Failed to save configs", true);
            }
        }));
        devFolder->Append(new MenuEntry("Change Camera FOV", nullptr, changeCameraFOV));
        menu.Append(optionsFolder);
        menu.Append(devFolder);
    }

    int main()
    {
        u64 titleID = Process::GetTitleID();
        if (!IS_TARGET_ID(titleID))
            return 0;

        if (!Directory::IsExists(PLUGIN_FOLDER"/layouts"))
            Directory::Create(PLUGIN_FOLDER"/layouts");
        if (!Directory::IsExists(PLUGIN_FOLDER"/scripts"))
            Directory::Create(PLUGIN_FOLDER"/scripts");

        Core::Debug::LogMessage("Starting plugin", false);
        Core::Debug::LogMessage(Utils::Format("Plugin version: %d.%d.%d", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH), false);

        bool loadMenuLayout = Core::Config::GetBoolValue(config, "custom_game_menu_layout", true);
        bool loadScripts = Core::Config::GetBoolValue(config, "enable_scripts", true);

        // Update configs
        if (!Core::Config::SaveConfig(CONFIG_FILE, config))
            Core::Debug::LogMessage("Failed to save configs", true);

        if (loadMenuLayout && enabledPatching) {
            if (File::Exists(PLUGIN_FOLDER"/layouts/menu_layout.json") && LoadGameMenuLayout(PLUGIN_FOLDER"/layouts/menu_layout.json"))
                PatchGameMenuLayoutFunction();
            else
                PatchMenuCustomLayoutDefault(PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH);
        }

        menu = new PluginMenu("LunaCore", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH,
            "Allows to execute Lua scripts.");
        std::string &title = menu->Title();
        title.assign("LunaCore Plugin Menu");

        if (!loadScripts) {
            Core::Debug::LogMessage("Scripts disabled", false);
        } else {
            Core::Debug::LogMessage("Loading Lua environment", false);
            Lua_global = luaL_newstate();
            luaL_openlibs(Lua_global);
            Core::Debug::LogMessage("Loading core modules", false);
            Core::LoadModules(Lua_global);

            // Set Lua path
            lua_getglobal(Lua_global, "package");
            lua_getfield(Lua_global, -1, "path");
            const char *current_path = lua_tostring(Lua_global, -1);
            lua_pop(Lua_global, 1);
            std::string newPath(current_path);
            newPath += ";" PLUGIN_FOLDER "/scripts/?.lua;" PLUGIN_FOLDER "/scripts/?/init.lua";
            lua_pushstring(Lua_global, newPath.c_str());
            lua_setfield(Lua_global, -2, "path");
            lua_pop(Lua_global, 1);
            Core::Debug::LogMessage("Lua environment loaded", false);
        }

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);
        menu->ShowWelcomeMessage(false);

        Core::Debug::LogMessage("LunaCore loaded", true);
        u16 gameVer = Process::GetVersion();
        if (System::IsCitra())
            Core::Debug::LogMessage("Emulator detected, unable to get game version. Enabled patching by default", true);
        else if (IS_VUSA_COMP(titleID, gameVer))
            Core::Debug::LogMessage(Utils::Format("Game USA region version '%hu' (1.9.19) detected", gameVer), false);
        else if (IS_VEUR_COMP(titleID, gameVer))
            Core::Debug::LogMessage(Utils::Format("Game EUR region version '%hu' (1.9.19) detected", gameVer), false);
        else if (IS_VJPN_COMP(titleID, gameVer))
            Core::Debug::LogMessage(Utils::Format("Game JPN region version '%hu' (1.9.19) detected", gameVer), false);
        else
            Core::Debug::LogMessage(Utils::Format("Incompatible version detected: '%hu'! Needed 1.9.19. Some features may not work"), true);

        // Wait until some things has been loaded otherwise instability may occur
        //Core::Debug::LogMessage("Waiting to load scripts", false);
        //Sleep(Seconds(8));
        if (loadScripts) {
            UpdateLuaStatistics();
            OSD::Run(DrawMonitors);

            Directory dir;
            Directory::Open(dir, PLUGIN_FOLDER"/scripts");
            if (dir.IsOpen()) {
                std::vector<std::string> dirFiles;
                if (dir.ListFiles(dirFiles, ".lua") > 0) { // Only add core callbacks if any script under directory
                    int luaFiles = 0;
                    for (auto file : dirFiles) {
                        if (strcmp(file.c_str() + file.size() - 4, ".lua") == 0)
                            luaFiles++;
                    }
                    if (luaFiles > 0) {
                        menu->Callback(PreloadScripts); // Callback that iterates over all scripts under 'PLUGIN_FOLDER/scripts'
                        menu->Callback(Core::EventHandlerCallback);
                        menu->Callback(Core::AsyncHandlerCallback);
                        menu->Callback(UpdateLuaStatistics);
                        OSD::Run(Core::GraphicsHandlerCallback);
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
        lua_close(Lua_global);

        // Exit plugin
        return 0;
    }
}