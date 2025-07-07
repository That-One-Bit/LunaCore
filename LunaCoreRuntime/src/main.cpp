#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include <FsLib/fslib.hpp>

#include <string>
#include <unordered_map>
#include <atomic>

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
#include "Core/CrashHandler.hpp"

#include "Game/Hooks/GameHooks.hpp"
#include "Core/Utils/GameState.hpp"
#include "Game/Hooks/MainMenuLayoutLoad.hpp"
#include "Game/Hooks/LoadingWorldScreenMessage.hpp"

#include "CoreInit.hpp"
#include "PluginInit.hpp"
#include "CoreConstants.hpp"

#define IS_VUSA_COMP(id, version) ((id) == 0x00040000001B8700LL && (version) == 9408) // 1.9.19 USA
#define IS_VEUR_COMP(id, version) ((id) == 0x000400000017CA00LL && (version) == 9392) // 1.9.19 EUR
#define IS_VJPN_COMP(id, version) ((id) == 0x000400000017FD00LL && (version) == 9424) // 1.9.19 JPN
#define IS_TARGET_ID(id) ((id) == 0x00040000001B8700LL || (id) == 0x000400000017CA00LL || (id) == 0x000400000017FD00LL)

#define BASE_OFF 0x100000

namespace CTRPF = CTRPluginFramework;

lua_State *Lua_global = NULL;
int scriptsLoadedCount = 0;
std::atomic<int> luaMemoryUsage = 0;
bool enabledPatching = true;
std::unordered_map<std::string, std::string> config;
GameState_s GameState;
u32 currentCamFOVOffset = 0;
CTRPF::PluginMenu *gmenu;

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
        Core::CrashHandler::plg_state = Core::CrashHandler::PLUGIN_PATCHPROCESS;
        u64 titleID = Process::GetTitleID();
        if (!IS_TARGET_ID(titleID))
            return;
        settings.UseGameHidMemory = true;
        ToggleTouchscreenForceOn();
        System::OnAbort = Core::CrashHandler::OnAbort;
        Process::exceptionCallback = Core::CrashHandler::ExceptionCallback;
        Process::ThrowOldExceptionOnCallbackException = true;
        Core::CrashHandler::ReserveMemory();

        Core::CrashHandler::core_state = Core::CrashHandler::CORE_STAGE1;
        if (!Directory::IsExists(PLUGIN_FOLDER))
            Directory::Create(PLUGIN_FOLDER);
        if (!Core::Debug::OpenLogFile(LOG_FILE))
            OSD::Notify(Utils::Format("Failed to open log file '%s'", LOG_FILE));
        Core::Debug::LogMessage(Utils::Format("LunaCore version: %d.%d.%d", PLG_VER_MAJ, PLG_VER_MIN, PLG_VER_PAT), false);
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
            hookSomeFunctions();
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
            screen.Draw("Lua memory: "+std::to_string(luaMemoryUsage.load()), 10, 10, Color::Black, Color(0, 0, 0, 0));
        return false;
    }

    int main()
    {
        Core::CrashHandler::plg_state = Core::CrashHandler::PLUGIN_MAIN;
        u64 titleID = Process::GetTitleID();
        if (!IS_TARGET_ID(titleID))
            return 0;
        
        Core::CrashHandler::core_state = Core::CrashHandler::CORE_STAGE2;

        if (!fslib::initialize()) {
            Core::Debug::LogError("Failed to initialize fslib");
            Core::Debug::LogRaw(std::string(fslib::getErrorString())+"\n");
        }

        if (!fslib::openExtData(u"extdata", static_cast<uint32_t>(titleID >> 8 & 0x00FFFFFF))) {
            Core::Debug::LogError("Failed to open extdata");
            Core::Debug::LogRaw(std::string(fslib::getErrorString())+"\n");
        }

        if (!Directory::IsExists(PLUGIN_FOLDER"/layouts"))
            Directory::Create(PLUGIN_FOLDER"/layouts");
        if (!Directory::IsExists(PLUGIN_FOLDER"/scripts"))
            Directory::Create(PLUGIN_FOLDER"/scripts");

        bool loadMenuLayout = Core::Config::GetBoolValue(config, "custom_game_menu_layout", true);
        bool loadScripts = Core::Config::GetBoolValue(config, "enable_scripts", true);

        // Update configs
        if (!Core::Config::SaveConfig(CONFIG_FILE, config))
            Core::Debug::LogMessage("Failed to save configs", true);

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
        Core::CrashHandler::core_state = Core::CrashHandler::CORE_LUA_EXEC;
        Lua_global = luaL_newstate();
        Core::LoadLuaEnv(Lua_global);
        Core::CrashHandler::core_state = Core::CrashHandler::CORE_STAGE3;

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

        if (loadScripts)
            Core::PreloadScripts(); // Compiles, loads and execute the scripts under the scripts folder

        // Init our menu entries & folders
        InitMenu(*gmenu);

        // Launch menu and mainloop
        Core::Debug::LogMessage("Starting plugin mainloop", false);
        Core::CrashHandler::core_state = Core::CrashHandler::CORE_GAME;
        Core::CrashHandler::plg_state = Core::CrashHandler::PLUGIN_MAINLOOP;
        gmenu->Run();
        Core::CrashHandler::plg_state = Core::CrashHandler::PLUGIN_EXIT;
        Core::CrashHandler::core_state = Core::CrashHandler::CORE_EXIT;

        // Cleanup
        Core::Debug::LogMessage("Exiting LunaCore", false);
        Core::Debug::CloseLogFile();
        fslib::closeDevice(u"extdata");
        fslib::exit();

        delete gmenu;
        lua_close(Lua_global);

        // Exit plugin
        return 0;
    }
}