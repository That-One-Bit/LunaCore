#pragma once

#include "lua_common.h"

#include "Game/Items.hpp"

namespace Core {
    namespace Game {
        typedef struct {
            u8 itemCount;
            u8 unk1 = 0;
            u16 dataValue = 0;
            bool show = true;
            u8 unk2[3] = {};
            u32 enchant = 0;
            Core::Game::Item *itemData = NULL;
            u32 renderID = 0;
            u8 unk3[24] = {};
        } InventorySlot;

        namespace LocalPlayer {
            bool RegisterInventoryModule(lua_State *L);
        }
    }
}