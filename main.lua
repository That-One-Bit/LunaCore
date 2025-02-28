local activated = false

function HoldKeyLWhileKeyTouch()
    while Gamepad.isPressed(Gamepad.KeyCodes.TOUCHPAD) do
        Gamepad.virtualPress(Gamepad.KeyCodes.L)
        Async.wait()
    end
end

Gamepad.KeyPressed:Connect(function ()
    -- This enables/disables fast swimming speed
    if Gamepad.isPressed(Gamepad.KeyCodes.ZL) then
        if activated then
            Player.SetSwimVelocity(0.02)
            Debug.showMessage("Normal swimming speed")
            activated = false
        else
            Player.SetSwimVelocity(0.1)
            Debug.showMessage("Fast swimming speed")
            activated = true
        end
    end

    -- This imits the cheat code of Cracko298 'Touch To Place'
    if Gamepad.isPressed(Gamepad.KeyCodes.TOUCHPAD) then
        Async.create(HoldKeyLWhileKeyTouch)
        Gamepad.virtualPress(Gamepad.KeyCodes.L)
    end
end)