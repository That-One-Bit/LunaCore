local Gamepad = Game.Gamepad
local Debug = Core.Debug

-- Checks if player is holding a diamond
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isPressed(Gamepad.KeyCodes.DPADDOWN) then
        if Game.World.Loaded then
            local playerHand = Game.LocalPlayer.Inventory.Slots["hand"]
            local targetItem = "diamond"
            if playerHand.ItemName == targetItem then -- You can also put the name directly
                Debug.message("Player is holding a diamond")
            end
        end
    end
end)