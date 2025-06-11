#include "Player/Inventory.hpp"

#include <vector>

#include <CTRPluginFramework.hpp>

#include "string_hash.hpp"
#include "Minecraft.hpp"

#include "Items.hpp"

namespace CTRPF = CTRPluginFramework;

// ----------------------------------------------------------------------------

//$Game.LocalPlayer.Inventory
//$Game.LocalPlayer.Inventory.Slots

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
        double num = lua_tonumber(L, 2);
        if (floor(num) != num) // Check only integers
            return 0;
        int index = lua_tointeger(L, 2);
        if (index >= 1 && index <= 36) {
            lua_newtable(L);
            lua_pushinteger(L, index);
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
                    lua_pushinteger(L, handSlot);
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
    int slotIndex = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if (slotIndex < 1 || slotIndex > 36)
        return luaL_error(L, "Slot index out of range");

    switch (key) {
        case hash("ItemID"): {
            u32 itemAddr = Minecraft::GetItemID(slotIndex);
            if (itemAddr == 0)
                lua_pushinteger(L, 0);
            else {
                Core::Game::ItemData *itemData = (Core::Game::ItemData*)itemAddr;
                lua_pushinteger(L, itemData->itemID);
            }
            break;
        }
        case hash("ItemCount"):
            lua_pushinteger(L, Minecraft::GetItemCount(slotIndex));
            break;
        case hash("ItemData"):
            lua_pushinteger(L, Minecraft::GetItemData(slotIndex));
            break;
        case hash("ItemName"): {
            u32 itemAddr = Minecraft::GetItemID(slotIndex);
            if (itemAddr == 0) {
                lua_pushstring(L, "empty_slot");
            } else {
                Core::Game::ItemData *itemData = (Core::Game::ItemData*)itemAddr;
                lua_pushstring(L, itemData->idName2);
            }
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
    int slotIndex = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if (slotIndex < 1 || slotIndex > 36)
        return luaL_error(L, "Slot index out of range");

    switch (key) {
        case hash("ItemID"): {
            u16 itemID = luaL_checkinteger(L, 3);
            Core::Game::ItemData *itemData = Core::Game::Items::SearchItemByID(itemID);
            if (itemData != NULL) {
                Core::Game::InventorySlot* slotAddr = (Core::Game::InventorySlot*)Minecraft::GetSlotAddress(slotIndex);
                u32 renderID = Core::Game::Items::GetRenderIDByItemID(itemID);
                if (slotAddr) {
                    slotAddr->itemData = itemData;
                    //slotAddr->renderID = renderID;
                }
            }
            else
                return luaL_error(L, "Unknown ID '%u'", itemID);
            break;
        }
        case hash("ItemCount"):
            Minecraft::SetItemCount(slotIndex, lua_tointeger(L, 3));
            break;
        case hash("ItemData"):
            Minecraft::SetItemData(slotIndex, (u16)lua_tointeger(L, 3));
            break;
        default:
            valid = false;
            break;
    }

    if (valid)
        return 0;
    else
        return luaL_error(L, "'%s' is not a valid member of object or is read-only value", key);
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
}

// Required to be called inside LocalPlayer definition
bool Core::Game::LocalPlayer::RegisterInventoryModule(lua_State *L)
{
    RegisterInventoryMetatables(L);

    lua_newtable(L); // LocalPlayer.Inventory
    lua_newtable(L); // LocalPlayer.Inventory.Slots
    luaC_setmetatable(L, "InventorySlotsMetatable");
    lua_setfield(L, -2, "Slots");
    lua_setfield(L, -2, "Inventory");
    return true;
}