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
    Game::ItemInstance* actualPos = *Item::creativeItems;
    while (actualPos < *Item::creativeItemsEnd) {
        if (actualPos->unknown1 == groupId && (*reinterpret_cast<Item**>((u32)(actualPos) + 0xc))->itemId == itemId)
            return actualPos->unknown2;
        actualPos = actualPos + 1;
    }
    return 0x7FFF;
}

// ----------------------------------------------------------------------------

//$Game.Items
//@@GameItem

// ----------------------------------------------------------------------------

/*
- Find an item using its ID
## name: string
## return: GameItem?
### Game.Items.findItemByName
*/
static int l_Items_findItemByName(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    Item *itemData = Core::Items::SearchItemByName(name);
    if (itemData == NULL) {
        lua_pushnil(L);
    } else {
        Item** item_ptr = (Item**)lua_newuserdata(L, sizeof(void*));
        *item_ptr = itemData;
        luaC_setmetatable(L, "GameItem");
    }
    return 1;
}

/*
- Find and item using its name
## itemID: integer
## return: GameItem?
### Game.Items.findItemByID
*/
static int l_Items_findItemByID(lua_State *L) {
    u16 itemID = luaL_checknumber(L, 1);
    Item *itemData = Core::Items::SearchItemByID(itemID);
    if (itemData == NULL) {
        lua_pushnil(L);
    } else {
        Item** item_ptr = (Item**)lua_newuserdata(L, sizeof(void*));
        *item_ptr = itemData;
        luaC_setmetatable(L, "GameItem");
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
## return: GameItem?
### Game.Items.registerItem
*/
static int l_Items_registerItem(lua_State *L) {
    const char* itemName = luaL_checkstring(L, 1);
    u16 itemId = luaL_checknumber(L, 2);
    Item* regItem = Game::registerItem(itemName, itemId);
    if (regItem == nullptr)
        lua_pushnil(L);
    else {
        Item** item_ptr = (Item**)lua_newuserdata(L, sizeof(void*));
        *item_ptr = regItem;
        luaC_setmetatable(L, "GameItem");
    }
    return 1;
}

/*
- Takes a registered item with Game.Items.registerItem, and sets its texture
## item: GameItem
## textureName: string
## textureIndex: integer
### GameItem:setTexture
*/
static int l_Items_registerItemTexture(lua_State *L) {
    Item* item_ptr = *(Item**)luaC_funccheckudata(L, 1, "GameItem");
    const char* textureName = luaL_checkstring(L, 2);
    u16 textureIndex = luaL_checkinteger(L, 3);
    item_ptr->setTexture(hash(textureName), textureIndex);
    return 0;
}

/*
- Takes a registered item with Game.Items.registerItem, and registers it in creative menu
## item: GameItem
## groupId: integer
## position: integer
### Game.Items.registerCreativeItem
*/
static int l_Items_registerCreativeItem(lua_State *L) {
    Item* item_ptr = *(Item**)luaC_funccheckudata(L, 1, "GameItem");
    u16 groupId = luaL_checkinteger(L, 2);
    s16 position = luaL_checkinteger(L, 3);
    Item::addCreativeItem(item_ptr, groupId, position);
    return 0;
}

static const luaL_Reg items_functions[] = {
    {"findItemByName", l_Items_findItemByName},
    {"findItemByID", l_Items_findItemByID},
    {"getCreativePosition", l_Items_getCreativePosition},
    {"registerItem", l_Items_registerItem},
    {"registerCreativeItem", l_Items_registerCreativeItem},
    {NULL, NULL}
};

/*
=GameItem.StackSize = 64
=GameItem.ID = 1
=GameItem.NameID = ""
=GameItem.DescriptionID = ""
*/
static int l_GameItem_index(lua_State *L) {
    Item* item_ptr = *(Item**)lua_touserdata(L, 1);
    if (lua_type(L, 2) != LUA_TSTRING)
        return 0;
    u32 key = hash(lua_tostring(L, 2));

    switch (key) {
        case hash("setTexture"):
            lua_pushcfunction(L, l_Items_registerItemTexture);
            break;
        case hash("StackSize"):
            lua_pushnumber(L, item_ptr->maxStackSize);
            break;
        case hash("ID"):
            lua_pushnumber(L, item_ptr->itemId);
            break;
        case hash("NameID"):
            lua_pushstring(L, item_ptr->nameId.c_str());
            break;
        case hash("DescriptionID"):
            lua_pushstring(L, item_ptr->descriptionId.c_str());
            break;
        default:
            lua_pushnil(L);
            break;
    }
    return 1;
}

static int l_GameItem_newindex(lua_State *L) {
    Item* item_ptr = *(Item**)lua_touserdata(L, 1);
    if (lua_type(L, 2) != LUA_TSTRING)
        return luaL_error(L, "Unable to set field '?' of 'GameItem' instance");
    u32 key = hash(lua_tostring(L, 2));

    switch (key) {
        case hash("StackSize"):
            item_ptr->maxStackSize = luaC_indexchecknumber(L, 3);
            break;
        default:
            luaL_error(L, "Unable to set field '%s' of 'GameItem' instance", lua_tostring(L, 2));
            break;
    }
    return 0;
}

// ----------------------------------------------------------------------------

static inline void RegisterItemsMetatables(lua_State *L) {
    luaL_newmetatable(L, "GameItem");
    lua_pushcfunction(L, l_GameItem_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_GameItem_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushstring(L, "GameItem");
    lua_setfield(L, -2, "__name");
    lua_pop(L, 1);
}

bool Core::Module::RegisterItemsModule(lua_State *L)
{
    RegisterItemsMetatables(L);
    lua_getglobal(L, "Game");
    luaC_register_field(L, items_functions, "Items");
    lua_pop(L, 1);
    return true;
}