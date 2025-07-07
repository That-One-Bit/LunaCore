#pragma once

#include "types.h"

typedef void *(*GameMemallocFunc)(size_t size);
#define GameMemalloc ((GameMemallocFunc)(0x11493c))

typedef u32 (*GameStrlenFunc)(const char *str);
#define GameStrlen ((GameStrlenFunc)(0x2fe990))

inline void *GameCalloc(size_t size) {
    void *dstPtr = GameMemalloc(size);
    if (dstPtr == NULL)
        return NULL;
    for (int i = 0; i < size; i++)
        *reinterpret_cast<char*>((char*)dstPtr+i) = 0;
    return dstPtr;
}