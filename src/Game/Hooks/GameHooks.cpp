#include "Game/Hooks/GameHooks.hpp"

#include <vector>
#include <memory>
#include <cstring>

#include <CTRPluginFramework.hpp>

#include "lua_common.h"
#include "Core/Debug.hpp"
#include "Core/CrashHandler.hpp"

namespace CTRPF = CTRPluginFramework;

#define BASE_OFF 0x100000

static std::vector<std::unique_ptr<CoreHookContext>> hooks;

extern "C" __attribute((naked)) void hookBody() {
    asm volatile ( // r4 contains hookCtxPtr
        "mov r5, sp\n" // Save stack pointer
        "str r0, [r4, #0x0c]\n" // Store r0-r3 in hookCtxPtr->r0-r3
        "str r1, [r4, #0x10]\n"
        "str r2, [r4, #0x14]\n"
        "str r3, [r4, #0x18]\n"
        "str lr, [r4, #0x1c]\n"
        "add r0, sp, #0x38\n"
        "str r0, [r4, #0x20]\n"
        "ldr r6, [r4, #0x4]\n" // Load callback function address
        "mov r0, r4\n" // Copy hookCtxPtr to r0 (arg 1)
        "blx r6\n"
        "mov sp, r5\n" // Restore stack pointer
        "add r6, r4, #0x24\n"
        "bx r6" // Jump to restoreIns and return normal flow
    );
}

void hookReturnOverwrite(CoreHookContext *ctx, void *data) {
    asm volatile ( // r0 contains hookCtxPtr
        "ldr sp, [r0, #0x20]\n"
        "add sp, sp, #0x10\n"
        "ldmia sp!, {r4-r12, lr}\n"
        "mov r3, r0\n"
        "mov r0, r1\n"
        "ldr pc, [r3, #0x1c]"
    );
}

// Hooks an ARMv7 function, length must be at least 5 instructions to hook
// and instructions cannot be relative dependent
void hookFunction(u32 targetAddr, u32 callbackAddr) {
    const u32 asmData[] = {
        0xE92D5FFF, // stmdb sp!, {r0-r12, lr}
        0xE59F4004, // ldr r4, [pc, #0x4] pc is already 2 bytes ahead
        0xE5945000, // ldr r5, [r4, #0x0]
        0xE12FFF15, // bx r5
        // hookCtxPtr
    };
    auto hookCtx = std::make_unique<CoreHookContext>();
    hookCtx->wrapCallbackAddress = (u32)hookBody;
    hookCtx->targetAddress = targetAddr;
    hookCtx->callbackAddress = callbackAddr;

    hookCtx->restoreIns = 0xE8BD5FFF; // ldmia sp!, {r0-r12, lr}
    hookCtx->jmpIns = 0xE51FF004; // ldr pc, [pc, #-0x4] pc is already 2 bytes ahead
    hookCtx->returnAddress = targetAddr + 4 * 5;

    hookCtx->overwrittenIns[0] = *(u32*)targetAddr;
    hookCtx->overwrittenIns[1] = *((u32*)targetAddr + 1);
    hookCtx->overwrittenIns[2] = *((u32*)targetAddr + 2);
    hookCtx->overwrittenIns[3] = *((u32*)targetAddr + 3);
    hookCtx->overwrittenIns[4] = *((u32*)targetAddr + 4);

    CoreHookContext *hookCtxPtr = hookCtx.get();
    hooks.push_back(std::move(hookCtx));

    *(u32*)targetAddr = asmData[0];
    *((u32*)targetAddr + 1) = asmData[1];
    *((u32*)targetAddr + 2) = asmData[2];
    *((u32*)targetAddr + 3) = asmData[3];
    *((u32*)targetAddr + 4) = (u32)hookCtxPtr;
}

void hookFunctionCallback(CoreHookContext *ctx) {
    CTRPF::OSD::Notify(CTRPF::Utils::Format("Called %X from %X", ctx->targetAddress, ctx->lr));
}

void hookFunctionCallbackTell(CoreHookContext *ctx) {
    CTRPF::OSD::Notify(CTRPF::Utils::Format("%s", ctx->r1));
}

void AssertionErrorCallback(CoreHookContext *ctx) {
    Core::CrashHandler::CoreState lastcState = Core::CrashHandler::core_state;
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_HOOK;
    Core::Debug::LogMessage(CTRPF::Utils::Format("[Game] Warning: at %X. %s. Condition is false '%s'", ctx->lr, ctx->r0, ctx->r1), false);
    Core::CrashHandler::core_state = lastcState;
}

void DebugErrorFormatCallback(CoreHookContext *ctx) {
    Core::CrashHandler::CoreState lastcState = Core::CrashHandler::core_state;
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_HOOK;
    Core::Debug::LogMessage(CTRPF::Utils::Format("[Game] Warning: at %X. %s", ctx->lr, *(char**)(ctx->sp)), false);
    Core::CrashHandler::core_state = lastcState;
}

void AnimFunctionCallback(CoreHookContext *ctx) {
    hookReturnOverwrite(ctx, NULL);
}

void hookSomeFunctions() {
    Core::CrashHandler::CoreState lastcState = Core::CrashHandler::core_state;
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_HOOKING;
    //hookFunction(0x303e98+BASE_OFF, (u32)hookFunctionCallback);
    //hookFunction(0x30f7dc+BASE_OFF, (u32)hookFunctionCallback);
    //hookFunction(0x48c840+BASE_OFF, (u32)hookFunctionCallback);
    //hookFunction(0x4FA688+BASE_OFF, (u32)hookFunctionCallback);
    //hookFunction(0x503754+BASE_OFF, (u32)hookFunctionCallback);
    //hookFunction(0x505820+BASE_OFF, (u32)hookFunctionCallback);
    //hookFunction(0x50ca40+BASE_OFF, (u32)hookFunctionCallback);

    //hookFunction(0x144474, (u32)AssertionErrorCallback);
    //hookFunction(0x114f50, (u32)DebugErrorFormatCallback);

    //hookFunction(0x4e608c, (u32)AnimFunctionCallback);
    //CTRPF::Process::Write8(0x1fecbc, 'a');
    Core::CrashHandler::core_state = lastcState;
}