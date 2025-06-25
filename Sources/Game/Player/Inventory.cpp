#include "Game/Player/Inventory.hpp"

#include <vector>

#include <CTRPluginFramework.hpp>

#include "string_hash.hpp"
#include "Game/Minecraft.hpp"

#include "Game/Items.hpp"

namespace CTRPF = CTRPluginFramework;

// ----------------------------------------------------------------------------

//$Game.LocalPlayer.Inventory
//$Game.LocalPlayer.Inventory.Slots
//$Game.LocalPlayer.Inventory.ArmorSlots

// ----------------------------------------------------------------------------

/*
@@InventorySlot
=InventorySlot.Slot = 0
=InventorySlot.ItemID = 0
=InventorySlot.ItemCount = 0
=InventorySlot.ItemData = 0
=InventorySlot.ItemName = ""
*/

static int l_Inventory_Slots_index(lua_State *L)
{
    if (lua_type(L, 2) == LUA_TNUMBER) {
        int index = lua_tonumber(L, 2);
        if (index >= 1 && index <= 36) {
            lua_newtable(L);
            lua_pushnumber(L, index);
            lua_setfield(L, -2, "Slot");
            luaC_setmetatable(L, "InventorySlotClass"); // At the end or setfield doesn't work
            return 1;
        }
    }
    else if (lua_type(L, 2) == LUA_TSTRING) {
        uint32_t key = hash(lua_tostring(L, 2));
        bool valid = true;

        switch (key) {
            case hash("hand"): {
                int handSlot = Minecraft::GetHeldSlotNumber();
                if (handSlot >= 1) {
                    lua_newtable(L);
                    lua_pushnumber(L, handSlot);
                    lua_setfield(L, -2, "Slot");
                    luaC_setmetatable(L, "InventorySlotClass"); // Again at the END
                }
                else
                    lua_pushnil(L);
                break;
            }
            default:
                valid = false;
                break;
        }

        if (valid)
            return 1;
        else
            return 0;
    }
    return 0;
}

// ----------------------------------------------------------------------------

static int l_Inventory_Slot_class_index(lua_State *L)
{
    if (lua_type(L, 2) != LUA_TSTRING)
        return 0;

    uint32_t key = hash(lua_tostring(L, 2));
    bool valid = true;

    lua_getfield(L, 1, "Slot");
    if (lua_type(L, -1) != LUA_TNUMBER)
    {
        lua_pop(L, 1);
        return luaL_error(L, "Slot index is invalid");
    }
    int slotIndex = lua_tonumber(L, -1);
    lua_pop(L, 1);

    if (slotIndex < 1 || slotIndex > 36)
        return luaL_error(L, "Slot index out of range");
    
    Core::Game::InventorySlot *slotData = (Core::Game::InventorySlot*)Minecraft::GetSlotAddress(slotIndex);
    if (slotData == NULL)
        return 0;

    switch (key) {
        case hash("ItemID"): {
            Core::Game::Item *itemData = slotData->itemData;
            if (itemData)
                lua_pushnumber(L, itemData->itemId);
            else
                lua_pushnumber(L, 0);
            break;
        }
        case hash("ItemCount"):
            lua_pushnumber(L, slotData->itemCount);
            break;
        case hash("ItemData"):
            lua_pushnumber(L, slotData->dataValue);
            break;
        case hash("ItemName"): {
            Core::Game::Item *itemData = slotData->itemData;
            if (itemData)
                lua_pushstring(L, itemData->nameId.c_str());
            else
                lua_pushstring(L, "empty_slot");
            break;
        }
        default:
            valid = false;
            break;
    }

    if (valid)
        return 1;
    else
        return 0;
}

static int l_Inventory_Slot_class_newindex(lua_State *L)
{
    if (lua_type(L, 2) != LUA_TSTRING)
        return luaL_error(L, "Attempt to set unknown member of slot");

    uint32_t key = hash(lua_tostring(L, 2));
    bool valid = true;

    lua_getfield(L, 1, "Slot");
    if (lua_type(L, -1) != LUA_TNUMBER)
    {
        lua_pop(L, 1);
        return luaL_error(L, "Slot index is invalid");
    }
    int slotIndex = lua_tonumber(L, -1);
    lua_pop(L, 1);

    if (slotIndex < 1 || slotIndex > 36)
        return luaL_error(L, "Slot index out of range");

    Core::Game::InventorySlot* slotData = (Core::Game::InventorySlot*)Minecraft::GetSlotAddress(slotIndex);
    if (slotData == NULL)
        return 0;

    switch (key) {
        case hash("ItemID"): {
            u16 itemID = luaL_checknumber(L, 3);
            Core::Game::Item *itemData = Core::Game::Items::SearchItemByID(itemID);
            if (itemData) {
                u32 renderID = Core::Game::Items::GetRenderIDByItemID(itemID);
                slotData->itemData = itemData;
                //slotData->renderID = renderID;
            }
            else
                return luaL_error(L, "Unknown ID '%u'", itemID);
            break;
        }
        case hash("ItemCount"):
            slotData->itemCount = lua_tonumber(L, 3);
            break;
        case hash("ItemData"):
            slotData->dataValue= lua_tonumber(L, 3);
            break;
        default:
            valid = false;
            break;
    }

    if (valid)
        return 0;
    else
        return luaL_error(L, "'%s' is not a valid member of object or is read-only value", lua_tostring(L, 2));
}

// ----------------------------------------------------------------------------

/*
@@InventoryArmorSlot
=InventoryArmorSlot.Slot = 0
=InventoryArmorSlot.ItemID = 0
=InventoryArmorSlot.ItemData = 0
=InventoryArmorSlot.ItemName = ""
*/

static int l_Inventory_ArmorSlots_index(lua_State *L)
{
    if (lua_type(L, 2) == LUA_TNUMBER) {
        int index = lua_tonumber(L, 2);
        if (index >= 1 && index <= 4) {
            lua_newtable(L);
            lua_pushnumber(L, index);
            lua_setfield(L, -2, "Slot");
            luaC_setmetatable(L, "InventoryArmorSlotClass"); // At the end or setfield doesn't work
            return 1;
        }
    }
    else if (lua_type(L, 2) == LUA_TSTRING) {
        uint32_t key = hash(lua_tostring(L, 2));
        bool valid = true;

        switch (key) {
            case hash("helmet"): {
                u32 slot = Minecraft::GetArmorSlotAddress(1);
                if (slot != 0) {
                    lua_newtable(L);
                    lua_pushnumber(L, 1);
                    lua_setfield(L, -2, "Slot");
                    luaC_setmetatable(L, "InventoryArmorSlotClass"); // Again at the END
                } else
                    lua_pushnil(L);
                break;
            }
            case hash("chestplate"): {
                u32 slot = Minecraft::GetArmorSlotAddress(2);
                if (slot != 0) {
                    lua_newtable(L);
                    lua_pushnumber(L, 2);
                    lua_setfield(L, -2, "Slot");
                    luaC_setmetatable(L, "InventoryArmorSlotClass"); // Again at the END
                } else
                    lua_pushnil(L);
                break;
            }
            case hash("leggings"): {
                u32 slot = Minecraft::GetArmorSlotAddress(3);
                if (slot != 0) {
                    lua_newtable(L);
                    lua_pushnumber(L, 3);
                    lua_setfield(L, -2, "Slot");
                    luaC_setmetatable(L, "InventoryArmorSlotClass"); // Again at the END
                } else
                    lua_pushnil(L);
                break;
            }
            case hash("boots"): {
                u32 slot = Minecraft::GetArmorSlotAddress(4);
                if (slot != 0) {
                    lua_newtable(L);
                    lua_pushnumber(L, 4);
                    lua_setfield(L, -2, "Slot");
                    luaC_setmetatable(L, "InventoryArmorSlotClass"); // Again at the END
                } else
                    lua_pushnil(L);
                break;
            }
            default:
                valid = false;
                break;
        }

        if (valid)
            return 1;
        else
            return 0;
    }
    return 0;
}

// ----------------------------------------------------------------------------

static int l_Inventory_ArmorSlot_class_index(lua_State *L)
{
    if (lua_type(L, 2) != LUA_TSTRING)
        return 0;

    uint32_t key = hash(lua_tostring(L, 2));
    bool valid = true;

    lua_getfield(L, 1, "Slot");
    if (lua_type(L, -1) != LUA_TNUMBER)
    {
        lua_pop(L, 1);
        return luaL_error(L, "Slot index is invalid");
    }
    int slotIndex = lua_tonumber(L, -1);
    lua_pop(L, 1);

    if (slotIndex < 1 || slotIndex > 4)
        return luaL_error(L, "Slot index out of range");
    
    Core::Game::InventorySlot *slotData = (Core::Game::InventorySlot*)Minecraft::GetArmorSlotAddress(slotIndex);
    if (slotData == NULL)
        return 0;

    switch (key) {
        case hash("ItemID"): {
            Core::Game::Item *itemData = slotData->itemData;
            if (itemData)
                lua_pushnumber(L, itemData->itemId);
            else
                lua_pushnumber(L, 0);
            break;
        }
        case hash("ItemData"):
            lua_pushnumber(L, slotData->dataValue);
            break;
        case hash("ItemName"): {
            Core::Game::Item *itemData = slotData->itemData;
            if (itemData)
                lua_pushstring(L, itemData->nameId.c_str());
            else
                lua_pushstring(L, "empty_slot");
            break;
        }
        default:
            valid = false;
            break;
    }

    if (valid)
        return 1;
    else
        return 0;
}

static int l_Inventory_ArmorSlot_class_newindex(lua_State *L)
{
    if (lua_type(L, 2) != LUA_TSTRING)
        return luaL_error(L, "Attempt to set unknown member of slot");

    uint32_t key = hash(lua_tostring(L, 2));
    bool valid = true;

    lua_getfield(L, 1, "Slot");
    if (lua_type(L, -1) != LUA_TNUMBER)
    {
        lua_pop(L, 1);
        return luaL_error(L, "Slot index is invalid");
    }
    int slotIndex = lua_tonumber(L, -1);
    lua_pop(L, 1);

    if (slotIndex < 1 || slotIndex > 36)
        return luaL_error(L, "Slot index out of range");

    Core::Game::InventorySlot* slotData = (Core::Game::InventorySlot*)Minecraft::GetSlotAddress(slotIndex);
    if (slotData == NULL)
        return 0;

    switch (key) {
        case hash("ItemID"): {
            u16 itemID = luaL_checknumber(L, 3);
            Core::Game::Item *itemData = Core::Game::Items::SearchItemByID(itemID);
            if (itemData != NULL) {
                u32 renderID = Core::Game::Items::GetRenderIDByItemID(itemID);
                slotData->itemData = itemData;
                //slotData->renderID = renderID;
            }
            else
                return luaL_error(L, "Unknown ID '%u'", itemID);
            break;
        }
        case hash("ItemData"):
            slotData->dataValue= lua_tonumber(L, 3);
            break;
        default:
            valid = false;
            break;
    }

    if (valid)
        return 0;
    else
        return luaL_error(L, "'%s' is not a valid member of object or is read-only value", lua_tostring(L, 2));
}

// ----------------------------------------------------------------------------

static inline void RegisterInventoryMetatables(lua_State *L)
{
    luaL_newmetatable(L, "InventorySlotsMetatable");
    lua_pushcfunction(L, l_Inventory_Slots_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, luaC_invalid_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);

    luaL_newmetatable(L, "InventorySlotClass");
    lua_pushcfunction(L, l_Inventory_Slot_class_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_Inventory_Slot_class_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);

    luaL_newmetatable(L, "InventoryArmorSlotsMetatable");
    lua_pushcfunction(L, l_Inventory_ArmorSlots_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, luaC_invalid_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);

    luaL_newmetatable(L, "InventoryArmorSlotClass");
    lua_pushcfunction(L, l_Inventory_ArmorSlot_class_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_Inventory_ArmorSlot_class_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
}

// Required to be called inside LocalPlayer definition
bool Core::Game::LocalPlayer::RegisterInventoryModule(lua_State *L)
{
    RegisterInventoryMetatables(L);

    lua_newtable(L); // LocalPlayer.Inventory
    lua_newtable(L); // LocalPlayer.Inventory.Slots
    luaC_setmetatable(L, "InventorySlotsMetatable");
    lua_setfield(L, -2, "Slots");
    lua_newtable(L); // LocalPlayer.Inventory.ArmorSlots
    luaC_setmetatable(L, "InventoryArmorSlotsMetatable");
    lua_setfield(L, -2, "ArmorSlots");
    lua_setfield(L, -2, "Inventory");
    return true;
}