#pragma once

#include <CTRPluginFramework.hpp>

namespace Core {
    class CrashHandler {
        public:
            static bool abort;

            enum PluginState {
                PLUGIN_PATCHPROCESS = 0,
                PLUGIN_MAIN,
                PLUGIN_MAINLOOP,
                PLUGIN_EXIT
            };
            enum CoreState {
                CORE_STAGE1 = 0,
                CORE_STAGE2,
                CORE_STAGE3,
                CORE_LOADING_MODS,
                CORE_LUA_EXEC,
                CORE_HOOK,
                CORE_GAME,
                CORE_EXIT,
                CORE_HOOKING,
            };
            enum GameState {
                GAME_LOADING = 0,
                GAME_MENU,
                GAME_WORLD,
            };

            static PluginState plg_state;
            static CoreState core_state;
            static GameState game_state;

            static void ReserveMemory();

            __attribute__((noreturn)) static void OnAbort();

            static CTRPluginFramework::Process::ExceptionCallbackState ExceptionCallback(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
    };
}