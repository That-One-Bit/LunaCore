#pragma once

#include <CTRPluginFramework.hpp>

#include <string>

#include "lua_common.h"
#include "Game/Utils/custom_string.hpp"

namespace Core {
    namespace Game {
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

        bool RegisterItemsModule(lua_State *L);

        namespace Items {
            static Item** mItems = reinterpret_cast<Item**>(0x00b0cef0);
            constexpr static short MAX_ITEMS = 512;

            Item *SearchItemByName(const std::string& name);

            Item *SearchItemByID(u16 id);

            u32 GetRenderIDByItemID(u16 id);
        }
    }
}