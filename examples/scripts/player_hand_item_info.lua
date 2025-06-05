local Gamepad = Game.Gamepad
local Debug = Game.Debug

-- You need to call Slots["hand"] everytime you want to check hand slot as it wont 
-- update any reference you saved previously
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isPressed(Gamepad.KeyCodes.DPADDOWN) then
        if Game.World.Loaded then
            local playerHand = Game.LocalPlayer.Inventory.Slots["hand"]
            Debug.message("ItemID: "..playerHand.ItemID)
            Debug.message("Item Name: "..playerHand.ItemName)
            Debug.message("Item Data: "..playerHand.ItemData)
        end
    end
end)