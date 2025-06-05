local player = Game.LocalPlayer
local debug = Game.Debug
local gamepad = Game.Gamepad

Game.Event.OnKeyPressed:Connect(function ()
    if Game.World.Loaded then
        if gamepad.isPressed(gamepad.KeyCodes.DPADLEFT) then
            debug.message("Player current hp: "..player.CurrentHP)
            debug.message("Player current hunger: "..player.CurrentHunger)
            debug.message("Player current level: "..player.CurrentLevel)
        end
    end
end)