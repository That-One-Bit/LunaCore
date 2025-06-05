local player = Game.LocalPlayer
local gamepad = Game.Gamepad
local coalId = Game.Items.findItemIDByName("coal")
local diamondId = Game.Items.findItemIDByName("diamond")

Game.Event.OnKeyPressed:Connect(function ()
    if Game.World.Loaded then
        if gamepad.isDown(gamepad.KeyCodes.DPADDOWN) then
            local playerHand = player.Inventory.Slots["hand"]
            if playerHand.ItemID == coalId and playerHand.ItemData == 0 then
                playerHand.ItemID = diamondId
            end
        end
    end
end)