local activated = false

Gamepad.KeyPressed:Connect(function ()
    if Gamepad.isPressed(Gamepad.KeyCodes.ZL) then
        if activated then
            Player.SetSwimVelocity(0.02)
            Debug.showMessage("Normal swim speed")
            activated = false
        else
            Player.SetSwimVelocity(0.1)
            Debug.showMessage("Fast swim speed")
            activated = true
        end
    end

    if Gamepad.isPressed(Gamepad.KeyCodes.TOUCHPAD) then
        Gamepad.virtualPress(Gamepad.KeyCodes.L)
    end
end)