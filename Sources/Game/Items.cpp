#include "Game/Items.hpp"

#include <cstring>
#include "string_hash.hpp"

#include "Game/Minecraft.hpp"

namespace CTRPF = CTRPluginFramework;

Core::Game::ItemData *GetItemData(u32 addr) {
    Core::Game::ItemData **itemData = (Core::Game::ItemData **)addr;
    return *itemData;
}

Core::Game::ItemData *Core::Game::Items::SearchItemByName(const std::string& name) {
    u32 *startAddr = (u32 *)0xB0CEF4;
    u32 *endAddr = (u32 *)0xB0D638;
    u32 *actualPtr = startAddr;

    while (actualPtr <= endAddr) {
        Core::Game::ItemData *itemData = GetItemData((u32)actualPtr);
        if (itemData != NULL) {
            if (std::strcmp(name.c_str(), itemData->idName2) == 0)
                return itemData;
        }
        actualPtr++;
    }
    return NULL;
}

Core::Game::ItemData *Core::Game::Items::SearchItemByID(u16 id) {
    u32 *startAddr = (u32 *)0xB0CEF4;
    u32 *endAddr = (u32 *)0xB0D638;
    u32 *actualPtr = startAddr;

    while (actualPtr <= endAddr) {
        Core::Game::ItemData *itemData = GetItemData((u32)actualPtr);
        if (itemData != NULL) {
            if (id == itemData->itemID)
                return itemData;
        }
        actualPtr++;
    }
    return NULL;
}

u32 Core::Game::Items::GetRenderIDByItemID(u16 id) {
    u32 *startAddr = (u32 *)0xB0CEF4;
    u32 *endAddr = (u32 *)0xB0D638;
    u32 *actualPtr = startAddr;

    while (actualPtr <= endAddr) {
        Core::Game::ItemData *itemData = GetItemData((u32)actualPtr);
        if (itemData != NULL) {
            if (id == itemData->itemID) {
                u32 renderID = Minecraft::GetRenderID((u32)actualPtr);
                return renderID;
            }
        }
        actualPtr++;
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
    Core::Game::ItemData *itemData = Core::Game::Items::SearchItemByName(name);
    if (itemData == NULL) {
        lua_pushnil(L);
    } else {
        lua_pushnumber(L, itemData->itemID);
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
    Core::Game::ItemData *itemData = Core::Game::Items::SearchItemByID(itemID);
    if (itemData == NULL) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, itemData->idName2);
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