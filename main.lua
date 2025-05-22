local activated = false
local localPlayer = Game.LocalPlayer
local world = Game.World
local Gamepad = Game.Gamepad

Game.Event.OnKeyPressed:Connect(function ()
    -- This enables/disables fast swimming speed
    if Gamepad.isDown(Gamepad.KeyCodes.ZL) then
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

    if Gamepad.isDown(Gamepad.KeyCodes.TOUCHPAD) then
        local px, py = Gamepad.getTouch()
        Game.Debug.message("Touch at x: "..px.." y: "..py)
    end
end)

Async.create(function ()
    while not localPlayer.OnGround do
        Async.wait()
    end
    Game.Debug.message("Player is now on ground")
end)

Game.Debug.message("Loaded main.lua")