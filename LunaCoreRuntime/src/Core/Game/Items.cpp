#include "Core/Game/Items.hpp"

#include <cstring>
#include "string_hash.hpp"

#include "Game/Minecraft.hpp"

namespace CTRPF = CTRPluginFramework;
using Item = Game::Item;

Item *Core::Items::SearchItemByName(const std::string& name) {
    short i = 1;
    while (i <= Item::MAX_ITEMS) {
        if (Item::mItems[i] != nullptr) {
            if (Item::mItems[i]->nameId == name)
                return Item::mItems[i];
        }
        i++;
    }
    return nullptr;
}

Item *Core::Items::SearchItemByID(u16 id) {
    if (id <= Item::MAX_ITEMS)
        return Item::mItems[id];
    return nullptr;
}

void* Core::Items::GetRenderIDByItemID(u16 id) {
    if (id <= Item::MAX_ITEMS) {
        if (Item::renderTable[id] != nullptr) {
            return Item::renderTable[id];
        }
    }
    return nullptr;
}

u16 Core::Items::GetCreativeItemPositionOfGroup(u16 itemId, u16 groupId) {
    Game::ItemInstance* actualPos = Item::creativeItems;
    while (actualPos < Item::creativeItemsEnd) {
        if (actualPos->unknown1 == groupId && (*reinterpret_cast<Item**>((u8*)(actualPos) + 0xc))->itemId == itemId)
            return actualPos->unknown2;
        actualPos = actualPos + 1;
    }
    return 0x7FFF;
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

/*
- Get the item position in creative using the id
## itemID: integer
## groupID: integer
## return: number
### Game.Items.getCreativePosition
*/
static int l_Items_getCreativePosition(lua_State *L) {
    u16 itemID = luaL_checknumber(L, 1);
    u16 groupID = luaL_checknumber(L, 2);
    u16 position = Core::Items::GetCreativeItemPositionOfGroup(itemID, groupID);
    lua_pushnumber(L, position);
    return 1;
}

/*
- Creates a new item and stores it in the game's items table. Returns the address to the item
## itemName: string
## itemId: integer
## return: lightuserdata?
### Game.Items.registerItem
*/
static int l_Items_registerItem(lua_State *L) {
    const char* itemName = luaL_checkstring(L, 1);
    u16 itemId = luaL_checknumber(L, 2);
    Item* regItem = Game::registerItem(itemName, itemId);
    if (regItem == nullptr)
        lua_pushnil(L);
    else
        lua_pushlightuserdata(L, regItem);
    return 1;
}

/*
- Takes a registered item with Game.Items.registerItem, and sets its texture
## item: lightuserdata
## textureName: string
## textureIndex: integer
### Game.Items.registerItemTexture
*/
static int l_Items_registerItemTexture(lua_State *L) {
    Item* item = nullptr;
    if (lua_type(L, 1) == LUA_TLIGHTUSERDATA)
        item = (Item*)lua_touserdata(L, 1);
    else
        return luaL_typerror(L, 1, "lightuserdata");
    const char* textureName = luaL_checkstring(L, 2);
    u16 textureIndex = luaL_checkinteger(L, 3);
    item->setTexture(hash(textureName), textureIndex);
    return 0;
}

/*
- Takes a registered item with Game.Items.registerItem, and registers it in creative menu
## item: lightuserdata
## groupId: integer
## position: integer
### Game.Items.registerCreativeItem
*/
static int l_Items_registerCreativeItem(lua_State *L) {
    Item* item = nullptr;
    if (lua_type(L, 1) == LUA_TLIGHTUSERDATA)
        item = (Item*)lua_touserdata(L, 1);
    else
        return luaL_typerror(L, 1, "lightuserdata");
    u16 groupId = luaL_checkinteger(L, 2);
    s16 position = luaL_checkinteger(L, 3);
    Item::addCreativeItem(item, groupId, position);
    return 0;
}

static const luaL_Reg items_functions[] = {
    {"findItemIDByName", l_Items_findItemIDByName},
    {"findItemNameByID", l_Items_findItemNameByID},
    {"getCreativePosition", l_Items_getCreativePosition},
    {"registerItem", l_Items_registerItem},
    {"registerItemTexture", l_Items_registerItemTexture},
    {"registerCreativeItem", l_Items_registerCreativeItem},
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