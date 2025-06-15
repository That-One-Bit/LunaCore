#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include <FsLib/fslib.hpp>

#include <string>
#include <unordered_map>
#include <atomic>
#include <sys/socket.h>
#include <malloc.h>

#include <cstring>

#include "lua_common.h"
#include "Game/Minecraft.hpp"
#include "Modules.hpp"
#include "Core/Debug.hpp"
#include "Core/Event.hpp"
#include "Core/Async.hpp"
#include "Core/Graphics.hpp"
#include "Core/Utils/Utils.hpp"
#include "Core/Config.hpp"
#include "Core/TCPConnection.hpp"

#include "Game/Hooks/GameHooks.hpp"
#include "Core/Utils/GameState.hpp"
#include "Game/Hooks/MainMenuLayoutLoad.hpp"
#include "Game/Hooks/LoadingWorldScreenMessage.hpp"

#define IS_VUSA_COMP(id, version) ((id) == 0x00040000001B8700LL && (version) == 9408) // 1.9.19 USA
#define IS_VEUR_COMP(id, version) ((id) == 0x000400000017CA00LL && (version) == 9392) // 1.9.19 EUR
#define IS_VJPN_COMP(id, version) ((id) == 0x000400000017FD00LL && (version) == 9424) // 1.9.19 JPN
#define IS_TARGET_ID(id) ((id) == 0x00040000001B8700LL || (id) == 0x000400000017CA00LL || (id) == 0x000400000017FD00LL)

#define PLUGIN_FOLDER "sdmc:/Minecraft 3DS"
#define LOG_FILE PLUGIN_FOLDER"/log.txt"
#define CONFIG_FILE PLUGIN_FOLDER"/config.txt"
#define BASE_OFF 0x100000

namespace CTRPF = CTRPluginFramework;

lua_State *Lua_global = NULL;
int scriptsLoadedCount = 0;
CTRPF::Clock timeoutLoadClock;
std::atomic<int> luaMemoryUsage = 0;
bool enabledPatching = true;
std::unordered_map<std::string, std::string> config;
GameState_s GameState;
u32 currentCamFOVOffset = 0;
CTRPF::PluginMenu *gmenu;

u32 *socBuffer = NULL;

void initSockets()
{
	socBuffer = (u32*)memalign(0x1000, 0x100000);
	if (!socBuffer)
		return;

	Result res = socInit(socBuffer, 0x100000);
	if (R_FAILED(res))
		return;
}

void exitSockets()
{
	socExit();
	free(socBuffer);
}

void TimeoutLoadHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutLoadClock.HasTimePassed(CTRPF::Milliseconds(2000)))
        luaL_error(L, "Script load exceeded execution time (2000 ms)");
}

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

        bool disableZLandZR = Core::Config::GetBoolValue(config, "disable_zl_and_zr", false);
        bool disableDLandDR = Core::Config::GetBoolValue(config, "disable_dleft_and_dright", false);

        u16 gameVer = Process::GetVersion();
        if (IS_VUSA_COMP(titleID, gameVer) || IS_VEUR_COMP(titleID, gameVer) || IS_VJPN_COMP(titleID, gameVer) || System::IsCitra()) {
            Minecraft::PatchProcess();
            SetMainMenuLayoutLoadCallback();
            SetLoadingWorldScreenMessageCallback();
            SetLeaveLevelPromptCallback();
            if (disableZLandZR) {
                Process::Write32(0x819530+BASE_OFF, 0); // Pos keycode for ZL
                Process::Write32(0x819534+BASE_OFF, 0); // Pos keycode for ZR
            }
            if (disableDLandDR) {
                Process::Write32(0x819530-(4*8)+BASE_OFF, 0); // Pos keycode for DPADRIGHT
                Process::Write32(0x819530-(4*7)+BASE_OFF, 0); // Pos keycode for DPADLEFT
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

    bool LoadBuffer(lua_State *L, const char *buffer, size_t size, const char* name) {
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

    bool LoadScript(lua_State *L, const std::string& fp)
    {
        std::string fileContent = Core::Utils::LoadFile(fp);
        if (fileContent.empty())
        {
            Core::Debug::LogMessage("Failed to open file"+fp, false);
            return false;
        }
        return LoadBuffer(L, fileContent.c_str(), fileContent.size(), fp.c_str());
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
        readFiles++;
    }

    void LoadLuaEnv(lua_State *L) {
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

    bool CancelOperationCallback() {
        Controller::Update();
        if (Controller::IsKeyDown(Key::B))
            return false;
        return true;
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
        auto devFolder = new MenuFolder("Developer");
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
        optionsFolder->Append(new MenuEntry("Toggle Block DPADLEFT and DPADRIGHT keys", nullptr, [](MenuEntry *entry)
        {
            bool changed = false;
            if (config["disable_zl_and_zr"] == "true") {
                if (MessageBox("Do you want to ENABLE DPADLEFT and DPADRIGHT keys?", DialogType::DialogYesNo)()) {
                    config["disable_dleft_and_dright"] = "false";
                    changed = true;
                }
            } else {
                if (MessageBox("Do you want to DISABLE DPADLEFT and DPADRIGHT keys (only scripts will be able to use them)?", DialogType::DialogYesNo)()) {
                    config["disable_dleft_and_dright"] = "true";
                    changed = true;
                }
            }
            if (changed) {
                MessageBox("Restart the game to apply the changes")();
                if (!Core::Config::SaveConfig(CONFIG_FILE, config))
                    Core::Debug::LogMessage("Failed to save configs", true);
            }
        }));
        devFolder->Append(new MenuEntry("Load script from network", nullptr, [](MenuEntry *entry) {
            initSockets();
            Core::Network::TCPServer tcp(5432);
            std::string host = tcp.getHostName();
            const Screen& topScreen = OSD::GetTopScreen();
            topScreen.DrawSysfont("Connect to host: "+host+":5432", 40, 185, Color::White);
            topScreen.DrawSysfont("Waiting connection... Press B to cancel", 40, 200, Color::White);
            OSD::SwapBuffers();
            topScreen.DrawSysfont("Connect to host: "+host+":5432", 40, 185, Color::White);
            topScreen.DrawSysfont("Waiting connection... Press B to cancel", 40, 200, Color::White);
            if (!tcp.waitConnection(CancelOperationCallback)) {
                exitSockets();
                if (!tcp.aborted)
                    MessageBox("Connection error")();
                return;
            }
            size_t fnameSize = 0;
            if (!tcp.recv(&fnameSize, sizeof(size_t))) {
                exitSockets();
                MessageBox("Failed to get filename size")();
                return;
            }
            char *namebuf = new char[fnameSize];
            if (!namebuf) {
                exitSockets();
                MessageBox("Memory error")();
                return;
            }
            if (namebuf == NULL || !tcp.recv(namebuf, fnameSize)) {
                exitSockets();
                delete namebuf;
                MessageBox("Failed to get filename")();
                return;
            }

            size_t size = 0;
            if (!tcp.recv(&size, sizeof(size_t))) {
                exitSockets();
                delete namebuf;
                MessageBox("Failed to get file size")();
                return;
            }
            char *buffer = new char[size];
            if (!buffer) {
                exitSockets();
                delete namebuf;
                MessageBox("Memory error")();
                return;
            }
            if (!tcp.recv(buffer, size)) {
                exitSockets();
                delete namebuf;
                delete buffer;
                MessageBox("Failed to get file")();
                return;
            }

            if (LoadBuffer(Lua_global, buffer, size, ("net:/"+std::string(namebuf)).c_str())) {
                MessageBox("Script loaded")();
                if (MessageBox("Do you want to save this script to the sd card?", DialogType::DialogYesNo)()) {
                    File scriptOut;
                    File::Open(scriptOut, PLUGIN_FOLDER "/scripts/"+std::string(namebuf), File::WRITE|File::CREATE);
                    if (!scriptOut.IsOpen()) {
                        MessageBox("Failed to write to sd card")();
                    } else {
                        scriptOut.Clear();
                        scriptOut.Write(buffer, size);
                        scriptOut.Flush();
                    }
                }
            } else {
                MessageBox("Error executing the script")();
            }
            exitSockets();
            delete namebuf;
            delete buffer;
        }));
        devFolder->Append(new MenuEntry("Clean Lua environment", nullptr, [](MenuEntry *entry) {
            if (!MessageBox("This will unload all loaded scripts. Continue?", DialogType::DialogYesNo)())
                return;
            Core::Debug::LogMessage("Reloading Lua environment", false);
            lua_close(Lua_global);
            Lua_global = luaL_newstate();
            LoadLuaEnv(Lua_global);
            scriptsLoadedCount = 0;
            MessageBox("Lua environment reloaded")();
        }));
        devFolder->Append(new MenuEntry("Reload scripts", nullptr, [](MenuEntry *entry) {
            if (!MessageBox("This will reload saved scripts, not including loaded by network (if not saved to sd card). Continue?", DialogType::DialogYesNo)())
                return;
            Core::Debug::LogMessage("Reloading Lua environment", false);
            lua_close(Lua_global);
            Lua_global = luaL_newstate();
            LoadLuaEnv(Lua_global);
            scriptsLoadedCount = 0;
            Core::Debug::LogMessage("Loading scripts", false);
            Directory dir;
            Directory::Open(dir, PLUGIN_FOLDER"/scripts");
            if (!dir.IsOpen()) {
                MessageBox("Failed to reload scripts")();
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
            MessageBox("Lua environment reloaded")();
        }));
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
        Core::Debug::LogMessage(Utils::Format("Plugin version: %d.%d.%d", PLG_VER_MAJ, PLG_VER_MIN, PLG_VER_PAT), false);

        bool loadMenuLayout = Core::Config::GetBoolValue(config, "custom_game_menu_layout", true);
        bool loadScripts = Core::Config::GetBoolValue(config, "enable_scripts", true);

        // Update configs
        if (!Core::Config::SaveConfig(CONFIG_FILE, config))
            Core::Debug::LogMessage("Failed to save configs", true);

        bool fslibInit = fslib::initialize();
        if (!fslibInit) {
            Core::Debug::LogMessage("Failed to initialize fslib", true);
            Core::Debug::LogMessage(fslib::getErrorString(), true);
        }

        bool openextData = true;
        uint32_t extDataID = static_cast<uint32_t>(titleID >> 8 & 0x00FFFFFF);
        FS_Archive archive;
        uint32_t binaryData[] = {MEDIATYPE_SD, extDataID, 0x00000000};
        FS_Path path = {.type = PATH_BINARY, .size = 0x0C, .data = binaryData};

        Result fsError = FSUSER_OpenArchive(&archive, ARCHIVE_EXTDATA, path);
        if (R_FAILED(fsError)) {
            openextData = false;
            Core::Debug::LogMessage("Failed to open extdata", true);
            Core::Debug::LogMessage(Utils::Format("Error opening extdata archive: 0x%08X.", fsError), true);
        } else {
            if (!fslib::mapArchiveToDevice(u"extdata", archive))
            {
                FSUSER_CloseArchive(archive);
                openextData = false;
                Core::Debug::LogMessage("Failed to open extdata", true);
                Core::Debug::LogMessage(fslib::getErrorString(), true);
            }
        }

        if (loadMenuLayout && enabledPatching) {
            if (File::Exists(PLUGIN_FOLDER"/layouts/menu_layout.json") && LoadGameMenuLayout(PLUGIN_FOLDER"/layouts/menu_layout.json"))
                PatchGameMenuLayoutFunction();
            else
                PatchMenuCustomLayoutDefault();
        }

        gmenu = new PluginMenu("LunaCore", PLG_VER_MAJ, PLG_VER_MIN, PLG_VER_PAT,
            "Allows to execute Lua scripts and other features", 2);
        std::string &title = gmenu->Title();
        title.assign("LunaCore Plugin Menu");

        Core::Debug::LogMessage("Loading Lua environment", false);
        Lua_global = luaL_newstate();
        LoadLuaEnv(Lua_global);

        // Synnchronize the menu with frame event
        gmenu->SynchronizeWithFrame(true);
        gmenu->ShowWelcomeMessage(false);

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
        
        OSD::Run(DrawMonitors);
        gmenu->Callback(UpdateLuaStatistics);
        gmenu->Callback(Core::EventHandlerCallback);
        gmenu->Callback(Core::AsyncHandlerCallback);

        if (loadScripts) {
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
                        gmenu->Callback(PreloadScripts); // Callback that iterates over all scripts under 'PLUGIN_FOLDER/scripts'
                    }
                }
            }
        }

        // Init our menu entries & folders
        InitMenu(*gmenu);

        // Launch menu and mainloop
        Core::Debug::LogMessage("Starting plugin mainloop", false);
        gmenu->Run();
        Core::Debug::LogMessage("Exiting LunaCore", false);
        Core::Debug::CloseLogFile();
        if (openextData)
            fslib::closeDevice(u"extdata");
        if (fslibInit)
            fslib::exit();

        delete gmenu;
        lua_close(Lua_global);

        // Exit plugin
        return 0;
    }
}