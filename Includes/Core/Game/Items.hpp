#pragma once

#include <CTRPluginFramework.hpp>

#include <string>

#include "lua_common.h"
#include "Game/Items.hpp"

namespace Core {
    namespace Module {
        bool RegisterItemsModule(lua_State *L);
    }

    namespace Items {
        using Item = Game::ItemWrapper::Item;
        
        Item *SearchItemByName(const std::string& name);

        Item *SearchItemByID(u16 id);

        u32 GetRenderIDByItemID(u16 id);
    }
}