#pragma once

#include <atomic>

typedef struct {
    std::atomic<bool> MainMenuLoaded = false;
    std::atomic<bool> WorldLoaded = false;
    std::atomic<bool> LoadingItems = false;
    std::atomic<bool> SettingItemsTextures = false;
    std::atomic<bool> LoadingCreativeItems = false;
    std::atomic<bool> CoreLoaded = false;
    std::atomic<bool> GameExecutingLua = false;
} GameState_s;