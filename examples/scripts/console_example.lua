local gamepad = Game.Gamepad
local keys = gamepad.KeyCodes
local console = {
    maxlines = 10,
    lines = {},
    bg = Core.Graphics.colorRGB(46, 46, 46),
    fg = Core.Graphics.colorRGB(255, 255, 255),
    frames = 0,
    firstframe = true
}

local function printToConsole(text)
    table.insert(console.lines, 1, text)
    if #console.lines > console.maxlines then
        table.remove(console.lines, #console.lines)
    end
end

---@param command string
local function processCommand(command)
    local args = command:split(" ")
    if args[1] == "execute" then
        local code = command:sub(#args[1]+2)
        local func = loadstring(string.format("return %s", code))
        if func then
            local success, result = pcall(func)
            if success then
                printToConsole(string.format("execute: %s", tostring(result)))
            else
                printToConsole(string.format("execute: %s", result))
            end
        else
            printToConsole("execute: Failed to execute Lua code")
        end
    elseif args[1] == "exit" then
        Core.Graphics.close()
    end
end

local function drawGraphics(screen)
    if screen == "top" then
        Core.Graphics.drawRectFill(20, 20, 340, 200, console.bg)
        Core.Graphics.drawText("Command console. Press B to close", 25, 25, console.fg);
        local y = 200
        for index, value in ipairs(console.lines) do
            Core.Graphics.drawText(value, 25, y, console.fg);
            y = y - 15
        end
    else
        console.frames = console.frames + 1
        if console.frames % 2 == 0 then -- Execute only when both framebuffers were drawn
            if console.firstframe then -- Avoid execute when first frame
                console.firstframe = false
                return
            end
            if console.frames > 0xFF then
                console.frames = 0
            end
            local command = Core.Keyboard.getString()
            if command == nil then
                printToConsole("> exit")
                Core.Graphics.close()
            else
                printToConsole(string.format("> %s", command))
                processCommand(command)
            end
        end
    end
end

-- You need to call open function to be able to draw on screen
-- The game will frezee and that allows to avoid artifacts on screen
-- Every other events and async task will continue execution
Game.Event.OnKeyReleased:Connect(function ()
    if gamepad.isReleased(keys.START) then
        if not Core.Graphics.isOpen() then
            console.firstframe = true
            Core.Graphics.open(drawGraphics)
        end
    end
end)