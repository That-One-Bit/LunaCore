local modPath = Core.getModpath("LunaCoreAPI")
---@type ItemGroupID
local itemGroupIdentifier = dofile(modPath .. "/src/itemGroupIdentifier.lua")

CoreAPI.ItemGroups = {
    BUILDING_BOCKS = 1,
    DECORATION = 2,
    REDSTONE = 3,
    FOOD_MINERALS = 4,
    TOOLS = 5,
    POTIONS = 6,
    OTHERS = 7
}

---Return a new ItemGroupIdentifier
---@param id number
---@param position (number|GroupRelativeItemPosition)|?
---@return ItemGroupID
function CoreAPI.ItemGroups.newItemGroupIdentifier(id, position)
    if id < 1 or id > 7 then
        error("Invalid item group ID")
    end
    return itemGroupIdentifier(id, position)
end

---@class GroupRelativeItemPosition
local creativeRelativePosition = CoreAPI.Utils.Classic:extend()

function creativeRelativePosition:new(id, origin)
    self.id = id
    self.origin = origin
end

---Returns the item id of the item origin
---@return integer?
function creativeRelativePosition:getId()
    if type(self.id) == "string" then
        local itemId = CoreAPI.Items.getItemId(self.id)
        return itemId
    else
        return self.id
    end
end

---Returns a table that can be used as relative position from an item id
---@param id number|string
---@return GroupRelativeItemPosition
function CoreAPI.ItemGroups.afterItem(id)
    if type(id) ~= "number" and type(id) ~= "string" then
        error("Invalid item name or item id")
    end
    return creativeRelativePosition(id, 1)
end

---Returns a table that can be used as relative position from an item id
---@param id number|string
---@return GroupRelativeItemPosition
function CoreAPI.ItemGroups.beforeItem(id)
    if type(id) ~= "number" and type(id) ~= "string" then
        error("Invalid item name or item id")
    end
    return creativeRelativePosition(id, 2)
end

CoreAPI.ItemGroups.ItemGroupIdentifier = itemGroupIdentifier
CoreAPI.ItemGroups.GroupRelativeItemPosition = creativeRelativePosition