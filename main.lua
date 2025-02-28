local activated = false

function HoldKeyLWhileKeyTouch()
    while Gamepad.IsDown(Gamepad.KeyCodes.TOUCHPAD) do
        Gamepad.PressButton(Gamepad.KeyCodes.L)
        Async.wait()
    end
end

Gamepad.KeyPressed:Connect(function ()
    -- This enables/disables fast swimming speed
    if Gamepad.IsDown(Gamepad.KeyCodes.ZL) then
        if activated then
            Player.SwimSpeed.set(0.02)
            Debug.showMessage("Normal swimming speed")
            activated = false
        else
            Player.SwimSpeed.set(0.1)
            Debug.showMessage("Fast swimming speed")
            activated = true
        end
    end

    -- This imits the cheat code of Cracko298 'Touch To Place'
    if Gamepad.IsDown(Gamepad.KeyCodes.TOUCHPAD) then
        Async.create(HoldKeyLWhileKeyTouch)
        Gamepad.PressButton(Gamepad.KeyCodes.L)
    end
end)
