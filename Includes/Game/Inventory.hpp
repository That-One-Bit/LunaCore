#pragma once

#include "types.h"

#include "Game/Items.hpp"

namespace Game {
    class Inventory {
        public:
        typedef struct {
            u8 itemCount;
            u8 unk1 = 0;
            u16 dataValue = 0;
            bool show = true;
            u8 unk2[3] = {};
            u32 enchant = 0;
            Item *itemData = NULL;
            u32 renderID = 0;
            u8 unk3[24] = {};
        } InventorySlot;
    };
}
