local Gamepad = Game.Gamepad

-- This will make the game detect R key pressed when the touch screen is pressed
Game.Event.OnKeyPressed:Connect(function ()
    if Gamepad.isPressed(Gamepad.KeyCodes.TOUCHPAD) then
        Gamepad.pressButton(Gamepad.KeyCodes.R)
    end
end)