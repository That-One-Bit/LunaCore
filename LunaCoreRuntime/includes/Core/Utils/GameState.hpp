#pragma once

#include <atomic>

typedef struct {
    std::atomic<bool> MainMenuLoaded = false;
    std::atomic<bool> WorldLoaded = false;
} GameState_s;