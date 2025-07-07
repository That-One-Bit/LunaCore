#pragma once

#include "types.h"

namespace Game {
    class Item;

    class ItemInstance {
        public:
        u8 padding[0x14];
        short unknown1;
        short unknown2;
        u8 padding2[24];

        ItemInstance(Item* item) {
            reinterpret_cast<ItemInstance*(*)(ItemInstance*, Item*)>(0x001d2444)(this, item);
        }

        ItemInstance(short itemId, short count = 1, short unkn = 0) {
            reinterpret_cast<ItemInstance*(*)(ItemInstance*, short, short, short)>(0x001d2894)(this, itemId, count, unkn);
        }
    };
}