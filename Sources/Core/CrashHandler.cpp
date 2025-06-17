#include "Core/CrashHandler.hpp"

#include <cstdlib>

#include "Game/Utils/game_functions.hpp"
#include "Core/Debug.hpp"

namespace CTRPF = CTRPluginFramework;

extern "C" lua_State* Lua_global;
static void *reservedMemory = nullptr;

namespace Core {
    bool CrashHandler::abort = false;
    CrashHandler::CoreState CrashHandler::core_state = CrashHandler::CoreState::CORE_STAGE1;
    CrashHandler::GameState CrashHandler::game_state = CrashHandler::GameState::GAME_LOADING;

    void CrashHandler::ReserveMemory() {
        reservedMemory = malloc(8 * 1024);
    }

    void CrashHandler::OnAbort() {
        abort = true;
        *(u32*)nullptr = 0;
        for (;;);
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
            {
                Core::Debug::LogError("[CRITICAL] Game crashed due to an unhandled error");
                u8 rnd = CTRPF::Utils::Random(0, (sizeof(errorMsg) / sizeof(errorMsg[0])) - 1);
                Core::Debug::LogRaw("    "); Core::Debug::LogRaw("\""+std::string(errorMsg[rnd])+"\"\n");
                Core::Debug::LogRaw("    "); Core::Debug::LogRaw(CTRPF::Utils::Format("Core state: %s\n", getCoreStateString(Core::CrashHandler::core_state)));
                Core::Debug::LogRaw("    "); Core::Debug::LogRaw(CTRPF::Utils::Format("Game state: %s\n", getGameStateString(Core::CrashHandler::game_state)));
            }

            bool possibleOOM = false;
            if (Lua_global != NULL)
                possibleOOM = lua_gc(Lua_global, LUA_GCCOUNT, 0) >= 1900;
            if (possibleOOM) {
                Core::Debug::LogRaw("    ");
                Core::Debug::LogRaw(CTRPF::Utils::Format("Possible reason: Out of memory. Lua memory was %u bytes\n", lua_gc(Lua_global, LUA_GCCOUNT, 0) * 1024 + lua_gc(Lua_global, LUA_GCCOUNTB, 0)));
            }
            Core::Debug::CloseLogFile();
            if (possibleOOM)
                return CTRPF::Process::EXCB_RETURN_HOME;
            CTRPF::Screen topScreen = CTRPF::OSD::GetTopScreen();
            //topScreen.Fade(0.0f);
            topScreen.DrawRect(20, 20, 360, 200, CTRPF::Color::Black, true);
            const char *titleMsg = "Oops.. Game crashed!";
            const char *tipMsg = "Press any button to exit";
            if (abort)
                titleMsg = "Oh no... Game abort!";

            topScreen.DrawSysfont(titleMsg, 25, 25, CTRPF::Color::Red);
            topScreen.DrawSysfont(tipMsg, 25, 40, CTRPF::Color::White);
            CTRPF::OSD::SwapBuffers();
        }
        CTRPF::Controller::Update();
        if (CTRPF::Controller::GetKeysDown()) return CTRPF::Process::EXCB_RETURN_HOME;
        else return CTRPF::Process::EXCB_LOOP;
    }
}