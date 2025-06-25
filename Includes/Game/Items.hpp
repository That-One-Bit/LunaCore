#pragma once

#include <string>

#include "Game/Utils/game_functions.hpp"
#include "Game/Utils/custom_string.hpp"
#include "types.h"

namespace Game {
    class ItemWrapper {
        public:
        typedef struct {
            void *vtable;
            u8 maxStackSize;
            u8 unknown2[3];
            CustomString atlasName;
            int unknown4;
            u16 unknown5;
            u16 itemId;
            CustomString descriptionId;
            CustomString nameId;
            u8 unknown6[6];
            u16 blockID;
            u8 padding[0xac - 0x24];
        } Item;
        Item* itemWrap = NULL;

        typedef struct {
            int miningLevel;
            int durability;
            int unknown;
            int damageBonus;
            int enchantability;
        } Tier;
        
        inline static Tier* WOOD = reinterpret_cast<Tier*>(0x00b0e124);
        inline static Tier* STONE = reinterpret_cast<Tier*>(0x00b0e138);
        inline static Tier* IRON = reinterpret_cast<Tier*>(0x00b0e14c);
        inline static Tier* DIAMOND = reinterpret_cast<Tier*>(0x00b0e160);
        inline static Tier* GOLD = reinterpret_cast<Tier*>(0x00b0e174);

        constexpr static short MAX_ITEMS = 512;
        inline static Item** mItems = reinterpret_cast<Item**>(0x00b0cef0);

        ItemWrapper(const std::string& nameId, short itemId) {
            itemWrap = (Item*)GameMemalloc(sizeof(Item));
            reinterpret_cast<void*(*)(Item*, const std::string&, short)>(0x005790a4)(itemWrap, nameId, itemId);
        }

    };
}