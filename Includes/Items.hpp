#pragma once

#include <CTRPluginFramework.hpp>

#include <string>

#include "lua_common.h"

namespace Core {
    namespace Game {
        typedef struct {
            void *unknown1;
            u8 maxStackSize;
            u8 unknown2[3];
            char *atlasName;
            int unknown4;
            u8 unknown5[2];
            u16 itemID;
            char *idName1;
            char *idName2;
            u8 unknown6[16];
            u16 blockID;
        } ItemData;

        bool RegisterItemsModule(lua_State *L);

        namespace Items {
            Core::Game::ItemData *SearchItemByName(const std::string& name);

            Core::Game::ItemData *SearchItemByID(u16 id);

            u32 GetRenderIDByItemID(u16 id);
        }
    }
}