#pragma once

#include <cstdint>
#include <cstddef>

#include <CTRPluginFramework.hpp>

typedef uint32_t u32;

#define BASE_ADDR_OFFSET 0x100000

typedef void *(*GameMemallocFunc)(size_t size);
#define GameMemalloc ((GameMemallocFunc)(0x1493c+BASE_ADDR_OFFSET))

typedef u32 (*GameStrlenFunc)(const char *str);
#define GameStrlen ((GameStrlenFunc)(0x1fe990+BASE_ADDR_OFFSET))

inline void *GameCalloc(size_t size) {
    void *dstPtr = GameMemalloc(size);
    if (dstPtr == NULL)
        return NULL;
    for (int i = 0; i < size; i++)
        CTRPluginFramework::Process::Write8((u32)(dstPtr)+i, 0);
    return dstPtr;
}

/*
Steps to replace a constant string with a dynamic string:
- Find address of instruction where the string is mentioned
- Replace instruction 
    - If adr param_1(r0),sid_stringid then
    - Replace with ldr r0,[pc, #offset] with offset (sid_stringid address) - 8 - (instruction address)
- Write in (sid_stringid address) a new pointer with your string
*/