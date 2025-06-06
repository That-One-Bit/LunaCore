local keycodes = Game.Gamepad.KeyCodes
local Debug = Core.Debug

-- This will be executed only when key A is pressed
Game.Event.OnKeyPressed:Connect(function ()
    if Game.Gamepad.isPressed(keycodes.A) then
        Debug.message("Pressed key A")
    end
end)

-- This will be executed only when key A is released
Game.Event.OnKeyReleased:Connect(function ()
    if Game.Gamepad.isPressed(keycodes.A) then
        Debug.message("Released key A")
    end
end)

-- This will be executed while key B is down
Game.Event.OnKeyDown:Connect(function ()
    if Game.Gamepad.isPressed(keycodes.B) then
        Debug.message("Key B is down")
    end
end)