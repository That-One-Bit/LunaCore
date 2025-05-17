local activated = false
local localPlayer = Game.LocalPlayer
local world = Game.World
local Gamepad = Game.Gamepad

function HoldKeyLWhileKeyTouch()
    while Gamepad.isDown(Gamepad.KeyCodes.TOUCHPAD) do
        Gamepad.pressButton(Gamepad.KeyCodes.L)
        Async.wait()
    end
end

Game.Event.OnKeyPressed:Connect(function ()
    -- This enables/disables fast swimming speed
    if Gamepad.isDown(Gamepad.KeyCodes.ZL) then
        if activated then
            localPlayer.SwimSpeed.set(0.02)
            Game.Debug.message("Normal swimming speed")
            activated = false
        else
            localPlayer.SwimSpeed.set(0.1)
            Game.Debug.message("Fast swimming speed")
            activated = true
        end
    end

    -- This imits the cheat code of Cracko298 'Touch To Place'
    if Gamepad.isDown(Gamepad.KeyCodes.TOUCHPAD) then
        Async.create(HoldKeyLWhileKeyTouch)
        Gamepad.pressButton(Gamepad.KeyCodes.L)
    end
end)

Game.Debug.message("Loaded main.lua")