#include "Game/Hooks/LoadingWorldScreenMessage.hpp"

#include <CTRPluginFramework.hpp>

#include "Core/Utils/GameState.hpp"
#include "lua_common.h"

#include "Core/Event.hpp"

namespace CTRPF = CTRPluginFramework;

#define BASE_OFF 0x100000

extern GameState_s GameState;
extern lua_State *Lua_global;
bool eventJoinTriggered = false;

typedef int (*MBdetectStateFunc)(int *);

void LoadingWorldScreenMessageCallback(int *ptr1, int *ptr2) {
    GameState.MainMenuLoaded.store(false);

    void (*LoadingWorldScreenMessageOriginal)(int*, int*) = (void(*)(int*,int*))(0x612ef0+BASE_OFF);
    MBdetectStateFunc *detectState = (MBdetectStateFunc*)(*ptr2 + 0x18);
    int state = detectState[0](ptr2);
    if (state == 0x10) {
        GameState.WorldLoaded.store(true);
        if (!eventJoinTriggered) {
            Core::Event::TriggerOnPlayerJoinWorld(Lua_global);
            eventJoinTriggered = true;
        }
    }

    LoadingWorldScreenMessageOriginal(ptr1, ptr2);
    return;
}

void SetLoadingWorldScreenMessageCallback() {
    CTRPF::Process::Write32(0x8c5df4+BASE_OFF, (u32)LoadingWorldScreenMessageCallback);
}

void LeaveLevelPromptCallback(int *ptr1, int param2, int param3, u32 param4) {
    void (*LeaveLevelPromptOriginal)(int*,int,int,u32) = (void(*)(int*,int,int,u32))(0x221d78+BASE_OFF);

    GameState.WorldLoaded.store(false);
    if (eventJoinTriggered) {
        Core::Event::TriggerOnPlayerLeaveWorld(Lua_global);
        eventJoinTriggered = false;
    }

    LeaveLevelPromptOriginal(ptr1, param2, param3, param4);
    return;
}

void SetLeaveLevelPromptCallback() {
    CTRPF::Process::Write32(0x8b3a70+BASE_OFF, (u32)LeaveLevelPromptCallback);
}