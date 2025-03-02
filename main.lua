local activated = false

function HoldKeyLWhileKeyTouch()
    while Gamepad.IsDown(Gamepad.KeyCode.TOUCHPAD) do
        Gamepad.PressButton(Gamepad.KeyCode.L)
        Async.wait()
    end
end

Gamepad.KeyPressed:Connect(function ()
    -- This enables/disables fast swimming speed
    if Gamepad.IsDown(Gamepad.KeyCode.ZL) then
        if activated then
            Player.SwimSpeed.set(0.02)
            Debug.Message("Normal swimming speed")
            activated = false
        else
            Player.SwimSpeed.set(0.1)
            Debug.Message("Fast swimming speed")
            activated = true
        end
    end

    -- This imits the cheat code of Cracko298 'Touch To Place'
    if Gamepad.IsDown(Gamepad.KeyCode.TOUCHPAD) then
        Async.create(HoldKeyLWhileKeyTouch)
        Gamepad.PressButton(Gamepad.KeyCode.L)
    end
end)

World.SetCloudsHeight(0)