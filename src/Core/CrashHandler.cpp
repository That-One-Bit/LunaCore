#include "Core/CrashHandler.hpp"

#include <cstdlib>

#include <FsLib/fslib.hpp>
#include "Core/Debug.hpp"

namespace CTRPF = CTRPluginFramework;

extern "C" lua_State* Lua_global;
static void *reservedMemory = nullptr;

namespace Core {
    bool CrashHandler::abort = false;
    CrashHandler::PluginState CrashHandler::plg_state = CrashHandler::PLUGIN_PATCHPROCESS;
    CrashHandler::CoreState CrashHandler::core_state = CrashHandler::CORE_STAGE1;
    CrashHandler::GameState CrashHandler::game_state = CrashHandler::GAME_LOADING;

    void CrashHandler::ReserveMemory() {
        reservedMemory = malloc(4 * 1024);
    }

    void CrashHandler::OnAbort() {
        abort = true;
        *(u32*)nullptr = 0;
        for (;;);
    }

    static const char *getPluginStateString(CrashHandler::PluginState state) {
        const char *str = "unknown";
        switch (state) {
            case CrashHandler::PLUGIN_PATCHPROCESS:
                str = "Patch process";
                break;
            case CrashHandler::PLUGIN_MAIN:
                str = "Main";
                break;
            case CrashHandler::PLUGIN_MAINLOOP:
                str = "Mainloop";
                break;
            case CrashHandler::PLUGIN_EXIT:
                str = "Exit";
                break;
        }
        return str;
    }

    static const char *getCoreStateString(CrashHandler::CoreState state) {
        const char *str = "unknown";
        switch (state) {
            case CrashHandler::CORE_STAGE1:
                str = "Loading stage 1";
                break;
            case CrashHandler::CORE_STAGE2:
                str = "Loading stage 2";
                break;
            case CrashHandler::CORE_STAGE3:
                str = "Loading stage 3";
                break;
            case CrashHandler::CORE_LUA_EXEC:
                str = "Executing Lua code";
                break;
            case CrashHandler::CORE_GAME:
                str = "Executing game";
                break;
            case CrashHandler::CORE_HOOK:
                str = "Executing hook";
                break;
            case CrashHandler::CORE_EXIT:
                str = "Exiting";
                break;
        }
        return str;
    }

    static const char *getGameStateString(CrashHandler::GameState state) {
        const char *str = "unknown";
        switch (state) {
            case CrashHandler::GAME_LOADING:
                str = "Loading";
                break;
            case CrashHandler::GAME_MENU:
                str = "Menu";
                break;
            case CrashHandler::GAME_WORLD:
                str = "World";
                break;
        }
        return str;
    }

    static const char *errorMsg[] = {
        "The game had no desire",
        "It was inevitable",
        "Got distracted for a second and...",
        "The bug became a plague!",
        "Stay calm and call a trusted adult",
        "Don't panic!",
        "Always expect the unexpected",
        "Who tried sqrt(-1)?"
    };

    CTRPF::Process::ExceptionCallbackState CrashHandler::ExceptionCallback(ERRF_ExceptionInfo *excep, CpuRegisters *regs) {
        static bool first = true;
        if (first) {
            first = false;
            if (reservedMemory) {
                free(reservedMemory);
                reservedMemory = nullptr;
            }
            bool possibleOOM = false;
            if (Lua_global != NULL) {
                possibleOOM = lua_gc(Lua_global, LUA_GCCOUNT, 0) >= 2000;
                lua_close(Lua_global);
            }
            {
                Core::Debug::LogError("[CRITICAL] Game crashed due to an unhandled error");
                u8 rnd = CTRPF::Utils::Random(0, (sizeof(errorMsg) / sizeof(errorMsg[0])) - 1);
                Core::Debug::LogRaw(CTRPF::Utils::Format("\t\"%s\"\n", errorMsg[rnd]));
                Core::Debug::LogRaw(CTRPF::Utils::Format("\t\tPlugin state: %s\n", getPluginStateString(Core::CrashHandler::plg_state)));
                Core::Debug::LogRaw(CTRPF::Utils::Format("\t\tCore state: %s\n", getCoreStateString(Core::CrashHandler::core_state)));
                Core::Debug::LogRaw(CTRPF::Utils::Format("\t\tGame state: %s\n", getGameStateString(Core::CrashHandler::game_state)));
                Core::Debug::LogRaw(CTRPF::Utils::Format("\n\tException type: %X\n", excep->type));
                Core::Debug::LogRaw(CTRPF::Utils::Format("\tReturn: %X\n", regs->lr));
                Core::Debug::LogRaw(CTRPF::Utils::Format("\tAddress: %X\n", regs->pc));
            }
            const char* reasonMsg = "Unknown";
            if (possibleOOM) {
                reasonMsg = "Out of memory. Lua memory was bigger than 2048000 bytes";
            }
            Core::Debug::LogRaw(CTRPF::Utils::Format("%sPossible reason: %s\n", Core::Debug::tab, reasonMsg));
            Core::Debug::CloseLogFile();
            if (plg_state == PluginState::PLUGIN_PATCHPROCESS)
                return CTRPF::Process::EXCB_REBOOT;
            else {
                fslib::closeDevice(u"extdata");
                fslib::exit();
            }

            CTRPF::Screen topScreen = CTRPF::OSD::GetTopScreen();
            topScreen.DrawRect(20, 20, 360, 200, CTRPF::Color::Black, true);
            const char *titleMsg = "Oops.. Game crashed!";
            const char *tipMsg = "Press A button to exit or B to continue";
            if (abort)
                titleMsg = "Oh no... Game abort!";
            if (possibleOOM)
                titleMsg = "Looks like we ran out of memory!";
            topScreen.DrawSysfont(titleMsg, 25, 25, CTRPF::Color::Red);
            topScreen.DrawSysfont(tipMsg, 25, 40, CTRPF::Color::White);
            CTRPF::OSD::SwapBuffers();
        }
        CTRPF::Controller::Update();
        if (CTRPF::Controller::IsKeyDown(CTRPF::Key::A)) return CTRPF::Process::EXCB_RETURN_HOME;
        if (CTRPF::Controller::IsKeyDown(CTRPF::Key::B)) return CTRPF::Process::EXCB_DEFAULT_HANDLER;
        else return CTRPF::Process::EXCB_LOOP;
    }
}