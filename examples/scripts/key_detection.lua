local keycodes = Game.Gamepad.KeyCodes

-- This will be executed only when key A is pressed
Game.Event.OnKeyPressed:Connect(function ()
    if Game.Gamepad.isPressed(keycodes.A) then
        Game.Debug.message("Pressed key A")
    end
end)

-- This will be executed only when key A is released
Game.Event.OnKeyReleased:Connect(function ()
    if Game.Gamepad.isPressed(keycodes.A) then
        Game.Debug.message("Released key A")
    end
end)

-- This will be executed while key B is down
Game.Event.OnKeyDown:Connect(function ()
    if Game.Gamepad.isPressed(keycodes.B) then
        Game.Debug.message("Key B is down")
    end
end)