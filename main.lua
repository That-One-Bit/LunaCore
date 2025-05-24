local activated = false
local localPlayer = Game.LocalPlayer
local Gamepad = Game.Gamepad

-- Test input and LocalPlayer Inventory
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isDown(Gamepad.KeyCodes.DPADDOWN) then
        local handSlot = localPlayer.Inventory.Slots["hand"]
        if handSlot then
            local itemName = handSlot.ItemName
            Game.Debug.message("Player is holding item: "..itemName)
        end
    end
end)

-- Test Game read-only
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isDown(Gamepad.KeyCodes.DPADUP) then
        Game.LocalPlayer = nil -- >:)
    end
end)

-- Test LocalPlayer set values
Game.Event.OnKeyPressed:Connect(function ()
    -- This enables/disables fast swimming speed
    if Gamepad.isDown(Gamepad.KeyCodes.ZL) and Gamepad.isDown(Gamepad.KeyCodes.ZR) then
        if activated then
            localPlayer.SwimSpeed = 0.02
            Game.Debug.message("Normal swimming speed")
            activated = false
        else
            localPlayer.SwimSpeed = 0.1
            Game.Debug.message("Fast swimming speed")
            activated = true
        end
    end
end)

-- Test getTouch
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isDown(Gamepad.KeyCodes.TOUCHPAD) then
        local px, py = Gamepad.getTouch()
        Game.Debug.message("Touch at x: "..px.." y: "..py)
    end
end)

-- Test events timeout
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isDown(Gamepad.KeyCodes.B) then
        while true do end
    end
end)

-- Test Async function and LocalPlayer usage
Async.create(function ()
    local lastOnGround = localPlayer.OnGround
    while Async.wait() do
        if localPlayer.OnGround ~= lastOnGround then
            lastOnGround = localPlayer.OnGround
            if lastOnGround then
                Game.Debug.message("Player is on ground")
            else
                Game.Debug.message("Player is not on ground")
            end
        end
    end
end)

-- Test Async timeout
Async.create(function ()
    while not Gamepad.isDown(Gamepad.KeyCodes.A) do
        Async.wait()
    end
    local idx = 0
    while true do
        idx = idx + 1
    end
end)

Game.Debug.message("Loaded main.lua")