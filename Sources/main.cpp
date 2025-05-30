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

#include "Game/Utils/game_functions.hpp"

#define IS_VUSA_COMP(id, version) ((id) == 0x00040000001B8700LL && (version) == 9408) // 1.9.19 USA
#define IS_VEUR_COMP(id, version) ((id) == 0x000400000017CA00LL && (version) == 9392) // 1.9.19 EUR
#define IS_VJPN_COMP(id, version) ((id) == 0x000400000017FD00LL && (version) == 9424) // 1.9.19 JPN
#define IS_TARGET_ID(id) ((id) == 0x00040000001B8700LL || (id) == 0x000400000017CA00LL || (id) == 0x000400000017FD00LL)

#define PLUGIN_VERSION_MAJOR 0
#define PLUGIN_VERSION_MINOR 7
#define PLUGIN_VERSION_PATCH 0
#define PLUGIN_FOLDER "sdmc:/mc3ds/"
#define LOG_FILE PLUGIN_FOLDER"log.txt"
#define CONFIG_FILE PLUGIN_FOLDER"config.txt"
#define BASE_OFF 0x100000

namespace CTRPF = CTRPluginFramework;

lua_State *Lua_global;
int scriptsLoadedCount = 0;
CTRPF::Clock timeoutLoadClock;
std::atomic<int> luaMemoryUsage = 0;
bool enabledPatching = true;
std::unordered_map<std::string, std::string> config;

bool ReplaceStringWithPointer(u32 offset, u32 insAddr, u32 strAddr, u32 ptrAddr, u8 reg) {
    u32 baseIns = (0xe5bf << 16)|((reg & 0xF) << 12);
    u32 addrsOffset = strAddr - insAddr - 8;
    if (addrsOffset & 0xFFF != addrsOffset)
        return false;
    if (!CTRPF::Process::Write32(insAddr + offset, baseIns|addrsOffset)) return false; // Patch instruction
    return CTRPF::Process::Write32(strAddr + offset, ptrAddr); // Write pointer to offset
}

bool ReplaceConstString(u32 offset, u32 insAddr, u32 strAddr, u8 reg, const std::string& text) {
    char *textPtr = (char*)GameCalloc(text.size() + 1); // Allocate string space in game memory
    if (textPtr == NULL) return false;
    if (!CTRPF::Process::WriteString((u32)textPtr, text.c_str(), text.size() + 1)) return false; // Copy string
    return ReplaceStringWithPointer(offset, insAddr, strAddr, (u32)textPtr, reg);
}

void TimeoutLoadHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutLoadClock.HasTimePassed(CTRPF::Milliseconds(2000)))
        luaL_error(L, "Script load exceeded execution time (2000 ms)");
}

std::unordered_map<std::string, std::string> loadConfig(const std::string &filepath) {
    std::unordered_map<std::string, std::string> config;
    if (!File::Exists(filepath)) {
        Core::Debug::LogMessage("Config file not found. Using defaults", false);
        return config;
    }
    std::string fileContent = Core::Utils::LoadFile(filepath);
    std::string line;
    size_t pos = 0, newLinePos = 0;
    const char *fileContentPtr = fileContent.c_str();

    while (pos < fileContent.size()) {
        while (*fileContentPtr != '\n' && *fileContentPtr != '\0') {
            fileContentPtr++;
            newLinePos++;
        }
        line = fileContent.substr(pos, newLinePos - pos);
        size_t delPos = line.find('=');
        if (delPos != std::string::npos) {
            std::string key = Core::Utils::strip(line.substr(0, delPos));
            std::string value = Core::Utils::strip(line.substr(delPos + 1));
            if (!key.empty() && !value.empty()) {
                config[key] = value;
                OSD::Notify(CTRPF::Utils::Format("Key: '%s', value: '%s'", key.c_str(), value.c_str()));
            }
        }
        fileContentPtr++;
        newLinePos++;
        pos = newLinePos;
    }
    return config;
}

bool saveConfig(const std::string &filePath, std::unordered_map<std::string, std::string> &config) {
    CTRPF::File configFile;
    CTRPF::File::Open(configFile, filePath, CTRPF::File::WRITE);
    if (!configFile.IsOpen()) {
        if (!File::Exists(filePath)) {
            File::Create(filePath);
            File::Open(configFile, filePath);
            if (!configFile.IsOpen())
                return false;
        } else
            return false;
    }
    configFile.Clear();
    for (auto key : config) {
        std::string writeContent = CTRPF::Utils::Format("%s = %s\n", key.first.c_str(), key.second.c_str());
        configFile.Write(writeContent.c_str(), writeContent.size());
    }
    return true;
}

bool getConfigBoolValue(std::unordered_map<std::string, std::string> &config, const std::string &field, bool def) {
    bool value = false;
    if (config.find(field) != config.end()) {
        if (config[field] == "true")
            value = true;
    } else { // Load default
        value = def;
        if (def)
            config[field] = "true";
    }
    if (!value)
        config[field] = "false";
    return value;
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

    void PatchMenuLayout() {
        ReplaceConstString(BASE_OFF, 0x16edd8, 0x16f170, 9, "      Play"); // menu.play
        ReplaceConstString(BASE_OFF, 0x16ef24, 0x16f1a0, 0, " "); // menu.multiplayer
        ReplaceConstString(BASE_OFF, 0x16f078, 0x16f1b8, 0, " "); // menu.options
        ReplaceConstString(BASE_OFF, 0x16f228, 0x16f600, 0, " "); // menu.skins
        ReplaceConstString(BASE_OFF, 0x16f37c, 0x16f610, 0, Utils::Format("  LunaCore %d.%d.%d", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH)); // menu.achievements
        ReplaceConstString(BASE_OFF, 0x16f4c8, 0x16f628, 0, " "); // menu.manual
        ReplaceConstString(BASE_OFF, 0x16f650, 0x16f884, 0, " "); // menu.store

        Process::Write8(0x16ef0c+BASE_OFF, 0x1d); // Menu buttons width (All but Play)
        Process::Write8(0x16edc4+BASE_OFF, 116); // Menu buttons width (Only Play)

        Process::Write8(0x16edd4+BASE_OFF, 66); // Menu buttons height (Play)
        Process::Write8(0x16ef18+BASE_OFF, 0x1c); // Menu buttons height (Multiplayer)
        Process::Write8(0x16f070+BASE_OFF, 0x1c); // Menu buttons height (Options)
        Process::Write8(0x16f220+BASE_OFF, 0x1c); // Menu buttons height (Skins)
        Process::Write8(0x16f364+BASE_OFF, 0x1c); // Menu buttons height (Achievements, Manual, Store)

        Process::Write8(0x16edc0+BASE_OFF, 120); // Menu buttons pos x (Play)
        Process::Write8(0x16ef1c+BASE_OFF, 255); // Menu buttons pos x (Multiplayer)
        Process::Write8(0x16f06c+BASE_OFF, 255); // Menu buttons pos x (Options)
        Process::Write8(0x16f214+BASE_OFF, 15); // Menu buttons pos x (Skins)
        Process::Write8(0x16f370+BASE_OFF, 15 + 0x1c + 10); // Menu buttons pos x (Achievements)
        Process::Write8(0x16f4b4+BASE_OFF, 255 - 0x1c - 10); // Menu buttons pos x (Manual)
        Process::Write8(0x16f640+BASE_OFF, 255); // Menu buttons pos x (Store)

        Process::Write8(0x16eeb0+BASE_OFF, 0); // Menu buttons submenuID (Play)
        Process::Write8(0x16ef2c+BASE_OFF, 5); // Menu buttons submenuID (Multiplayer)
        Process::Write8(0x16f060+BASE_OFF, 1); // Menu buttons submenuID (Options)
        Process::Write8(0x16f374+BASE_OFF, 3); // Menu buttons submenuID (Achievements)
        Process::Write8(0x16f4c0+BASE_OFF, 4); // Menu buttons submenuID (Manual)
        Process::Write8(0x16f5f4+BASE_OFF, 6); // Menu buttons submenuID (Store)

        Process::Write8(0x91a168, 75); // Menu buttons pox y (Play)
        Process::Write8(0x91a16c, 75); // Menu buttons pox y (Multiplayer)
        Process::Write8(0x91a170, 75 + 0x1c + 10); // Menu buttons pox y (Options)
        Process::Write8(0x91a174, 206); // Menu buttons pox y (Skins)
        Process::Write8(0x91a178, 206); // Menu buttons pox y (Achievements)
        Process::Write8(0x91a17c, 206); // Menu buttons pox y (Manual)
        Process::Write8(0x91a180, 206); // Menu buttons pox y (Store)

        Process::Write8(0x16edbc+BASE_OFF, 1); // Menu buttons icon scale (Play)
        Process::Write32(0x16ee04+BASE_OFF, 0xe3a02000);
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
        u16 gameVer = Process::GetVersion();
        if (IS_VUSA_COMP(titleID, gameVer) || IS_VEUR_COMP(titleID, gameVer) || IS_VJPN_COMP(titleID, gameVer) || System::IsCitra())
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
                std::string error_msg(lua_tostring(L, -1));
                Core::Utils::Replace(error_msg, "\t", "    ");
                Core::Debug::LogError(error_msg);
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
        Directory::Open(dir, "sdmc:/mc3ds/scripts");
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
                if (!saveConfig(CONFIG_FILE, config))
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
                if (!saveConfig(CONFIG_FILE, config))
                    Core::Debug::LogMessage("Failed to save configs", true);
            }
        }));
        menu.Append(optionsFolder);
    }

    int main()
    {
        u64 titleID = Process::GetTitleID();
        if (!IS_TARGET_ID(titleID))
            return 0;

        if (!Directory::IsExists(PLUGIN_FOLDER))
            Directory::Create(PLUGIN_FOLDER);
        if (!Core::Debug::OpenLogFile(LOG_FILE))
            OSD::Notify(Utils::Format("Failed to open log file '%s'", LOG_FILE));

        Core::Debug::LogMessage("Starting plugin", false);
        Core::Debug::LogMessage(Utils::Format("Plugin version: %d.%d.%d", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH), false);

        Core::Debug::LogMessage(Utils::Format("Loading config file '%s'", CONFIG_FILE), false);
        config = loadConfig(CONFIG_FILE);

        bool loadMenuLayout = getConfigBoolValue(config, "custom_game_menu_layout", true);
        bool loadScripts = getConfigBoolValue(config, "enable_scripts", true);

        // Update configs
        if (!saveConfig(CONFIG_FILE, config))
            Core::Debug::LogMessage("Failed to save configs", true);

        menu = new PluginMenu("LunaCore", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH,
            "Allows to execute Lua scripts.");

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
        lua_pushfstring(Lua_global, "%s;sdmc:/mc3ds/scripts/?.lua;sdmc:/mc3ds/scripts/?/init.lua", current_path);
        lua_setfield(Lua_global, -2, "path");
        lua_pop(Lua_global, 1);
        Core::Debug::LogMessage("Lua environment loaded", false);

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);
        menu->ShowWelcomeMessage(false);

        Core::Debug::LogMessage("Script core loaded", true);
        u16 gameVer = Process::GetVersion();
        if (System::IsCitra())
            Core::Debug::LogMessage("Emulator detected, unable to get game version. Enabled patching by default", true);
        else if (IS_VUSA_COMP(titleID, gameVer))
            Core::Debug::LogMessage(Utils::Format("Game USA region version '%hu' (1.9.19) detected", gameVer), false);
        else if (IS_VEUR_COMP(titleID, gameVer))
            Core::Debug::LogMessage(Utils::Format("Game EUR region version '%hu' (1.9.19) detected", gameVer), false);
        else if (IS_VJPN_COMP(titleID, gameVer))
            Core::Debug::LogMessage(Utils::Format("Game JPN region version '%hu' (1.9.19) detected", gameVer), false);
        else {
            enabledPatching = false;
            Core::Debug::LogMessage(Utils::Format("Incompatible version detected: '%hu'! Needed 1.9.19. Some features may not work"), true);
        }

        UpdateLuaStatistics();
        OSD::Run(DrawMonitors);

        // Wait until some things has been loaded otherwise instability may occur
        //Core::Debug::LogMessage("Waiting to load scripts", false);
        //Sleep(Seconds(8));
        if (Directory::IsExists("sdmc:/mc3ds/scripts")) {
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
                        menu->Callback(PreloadScripts); // Callback that iterates over all scripts under 'sdmc:/mc3ds/scripts'
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

        if (loadMenuLayout && enabledPatching)
            PatchMenuLayout();

        // Launch menu and mainloop
        Core::Debug::LogMessage("Starting plugin mainloop", false);
        menu->Run();
        Core::Debug::CloseLogFile();

        delete menu;

        // Exit plugin
        return 0;
    }
}