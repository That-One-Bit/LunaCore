#include "Game/Items.hpp"

#include <cstring>
#include "string_hash.hpp"

#include "Game/Minecraft.hpp"

namespace CTRPF = CTRPluginFramework;

Core::Game::Item *Core::Game::Items::SearchItemByName(const std::string& name) {
    short i = 1;
    while (i <= MAX_ITEMS) {
        if (mItems[i] != nullptr) {
            if (name == mItems[i]->nameId)
                return mItems[i];
        }
        i++;
    }
    return nullptr;
}

Core::Game::Item *Core::Game::Items::SearchItemByID(u16 id) {
    if (id <= MAX_ITEMS)
        return mItems[id];
    return nullptr;
}

u32 Core::Game::Items::GetRenderIDByItemID(u16 id) {
    short i = 1;
    while (i <= MAX_ITEMS) {
        if (mItems[i] != nullptr) {
            if (id == mItems[i]->itemId) {
                u32 renderID = Minecraft::GetRenderID((u32)mItems[i]);
                return renderID;
            }
        }
        i++;
    }
    return 0;
}

// ----------------------------------------------------------------------------

//$Game.Items

// ----------------------------------------------------------------------------

/*
- Find the item ID using its name
## name: string
## return: integer?
### Game.Items.findItemIDByName
*/
static int l_Items_findItemIDByName(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    Core::Game::Item *itemData = Core::Game::Items::SearchItemByName(name);
    if (itemData == NULL) {
        lua_pushnil(L);
    } else {
        lua_pushnumber(L, itemData->itemId);
    }

    return 1;
}

/*
- Find the item ID using its name
## itemID: integer
## return: string?
### Game.Items.findItemNameByID
*/
static int l_Items_findItemNameByID(lua_State *L) {
    u16 itemID = luaL_checknumber(L, 1);
    Core::Game::Item *itemData = Core::Game::Items::SearchItemByID(itemID);
    if (itemData == NULL) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, itemData->nameId.c_str());
    }

    return 1;
}

static const luaL_Reg items_functions[] = {
    {"findItemIDByName", l_Items_findItemIDByName},
    {"findItemNameByID", l_Items_findItemNameByID},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

bool Core::Game::RegisterItemsModule(lua_State *L)
{
    lua_getglobal(L, "Game");
    luaC_register_field(L, items_functions, "Items");
    lua_pop(L, 1);
    return true;
}