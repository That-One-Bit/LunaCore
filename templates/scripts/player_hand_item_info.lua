local Gamepad = Game.Gamepad
local Debug = Game.Debug

Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isPressed(Gamepad.KeyCodes.DPADDOWN) then
        if Game.World.Loaded then
            local playerHand = Game.LocalPlayer.Inventory.Slots["hand"]
            local itemIDHex = string.format("%X", playerHand.ItemID)
            Debug.message("ItemID: "..itemIDHex)
            Debug.message("Item Name: "..playerHand.ItemName)
            Debug.message("Item Data: "..playerHand.ItemData)
        end
    end
end)