local player = Game.LocalPlayer
local gamepad = Game.Gamepad
local coalId = nil
local diamondId = nil

Game.Event.OnKeyPressed:Connect(function ()
    if Game.World.Loaded then
        if gamepad.isDown(gamepad.KeyCodes.DPADDOWN) then
            if coalId == nil then -- You cannot use findItem when the game isn't loaded, wait until World is loaded to prevent nil results
                coalId = Game.Items.findItemIDByName("coal") -- Make sure to not call this function many times as it can lag the game, use one time and store the value
                diamondId = Game.Items.findItemIDByName("diamond")
            end
            local playerHand = player.Inventory.Slots["hand"]
            if playerHand.ItemID == coalId and playerHand.ItemData == 0 then
                playerHand.ItemID = diamondId
            end
        end
    end
end)