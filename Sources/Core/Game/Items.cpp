#include "Core/Game/Items.hpp"

#include <cstring>
#include "string_hash.hpp"

#include "Game/Minecraft.hpp"

namespace CTRPF = CTRPluginFramework;
using ItemClass = Game::ItemWrapper;
using Item = ItemClass::Item;

Item *Core::Items::SearchItemByName(const std::string& name) {
    short i = 1;
    while (i <= ItemClass::MAX_ITEMS) {
        if (ItemClass::mItems[i] != nullptr) {
            if (name == ItemClass::mItems[i]->nameId)
                return ItemClass::mItems[i];
        }
        i++;
    }
    return nullptr;
}

Item *Core::Items::SearchItemByID(u16 id) {
    if (id <= ItemClass::MAX_ITEMS)
        return ItemClass::mItems[id];
    return nullptr;
}

u32 Core::Items::GetRenderIDByItemID(u16 id) {
    short i = 1;
    while (i <= ItemClass::MAX_ITEMS) {
        if (ItemClass::mItems[i] != nullptr) {
            if (id == ItemClass::mItems[i]->itemId) {
                u32 renderID = Minecraft::GetRenderID((u32)&ItemClass::mItems[i]);
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
    Item *itemData = Core::Items::SearchItemByName(name);
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
    Item *itemData = Core::Items::SearchItemByID(itemID);
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

bool Core::Module::RegisterItemsModule(lua_State *L)
{
    lua_getglobal(L, "Game");
    luaC_register_field(L, items_functions, "Items");
    lua_pop(L, 1);
    return true;
}