#include "Player/Inventory.hpp"

#include <vector>

#include <CTRPluginFramework.hpp>

#include "string_hash.hpp"
#include "Minecraft.hpp"

namespace CTRPF = CTRPluginFramework;

// ----------------------------------------------------------------------------

/*
@@InventorySlot
=InventorySlot.Slot = 0
=InventorySlot.ItemID = 0
=InventorySlot.ItemCount = 0
=InventorySlot.ItemData = 0
=InventorySlot.ItemName = ""

$Game.LocalPlayer.Inventory
$Game.LocalPlayer.Inventory.Slots
*/

int l_Inventory_Slots_index(lua_State *L)
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

int l_Inventory_Slot_class_index(lua_State *L)
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
        case hash("ItemID"):
            lua_pushinteger(L, Minecraft::GetItemID(slotIndex));
            break;
        case hash("ItemCount"):
            lua_pushinteger(L, Minecraft::GetItemCount(slotIndex));
            break;
        case hash("ItemData"):
            lua_pushinteger(L, Minecraft::GetItemData(slotIndex));
            break;
        case hash("ItemName"): {
            auto itemsList = Minecraft::GetItemList();
            u32 itemID = Minecraft::GetItemID(slotIndex);
            u32 currItemID;
            bool found = false;
            for (auto item : itemsList) {
                CTRPF::Process::Read32(item.idAddress, currItemID);
                if (currItemID == itemID) {
                    lua_pushstring(L, item.name.c_str());
                    found = true;
                    break;
                }
            }
            if (!found)
                lua_pushstring(L, "Unknown Item");
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

int l_Inventory_Slot_class_newindex(lua_State *L)
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
        case hash("ItemID"):
            Minecraft::SetItemID(slotIndex, lua_tointeger(L, 3));
            break;
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

int l_register_Player_Inventory_metatables(lua_State *L)
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
    return 0;
}

int l_register_Player_Inventory(lua_State *L)
{
    // Required to be called inside table definition
    l_register_Player_Inventory_metatables(L);

    lua_newtable(L); // LocalPlayer.Inventory
    lua_newtable(L); // LocalPlayer.Inventory.Slots
    luaC_setmetatable(L, "InventorySlotsMetatable");
    lua_setfield(L, -2, "Slots");
    lua_setfield(L, -2, "Inventory");
    return 0;
}