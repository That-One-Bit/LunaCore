local Gamepad = Game.Gamepad
local localPlayer = Game.LocalPlayer

-- Tells what item is holding the player in hand
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isDown(Gamepad.KeyCodes.DPADDOWN) then
        local handSlot = localPlayer.Inventory.Slots["hand"]
        if handSlot ~= nil then -- handSlot will be nil when player isn't loaded
            local itemName = handSlot.ItemName
            Game.Debug.message("Player is holding item: "..itemName)
        end
    end
end)

-- Tells what item is in slot 1 of the inventory
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isDown(Gamepad.KeyCodes.DPADDOWN) then
        local slot = localPlayer.Inventory.Slots[1] -- Slots 1 to 36
        if slot ~= nil then -- slot will be nil when player isn't loaded
            local itemName = slot.ItemName
            Game.Debug.message("Player has item in slot 1: "..itemName)
        end
    end
end)

-- Tells how many items are in slot 1
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isDown(Gamepad.KeyCodes.DPADDOWN) then
        local slot = localPlayer.Inventory.Slots[1] -- Slots 1 to 36
        if slot ~= nil then -- slot will be nil when player isn't loaded
            local itemCount = slot.ItemCount
            Game.Debug.message("Player has in slot 1 "..itemCount.." items")
        end
    end
end)

-- Tells the variation of the item in slot 1
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isDown(Gamepad.KeyCodes.DPADDOWN) then
        local slot = localPlayer.Inventory.Slots[1] -- Slots 1 to 36
        if slot ~= nil then -- slot will be nil when player isn't loaded
            local itemData = slot.ItemData
            Game.Debug.message("Player has in slot 1 "..itemData.." item variation")
        end
    end
end)