local optionsparser = require("options_parser")
local filesystem = Core.Filesystem
local debug = Core.Debug

local function openAndReadOptionsFile()
    local optionsFile = filesystem.open("extdata:/options.txt", "r")
    if optionsFile ~= nil then
        local optionsData = optionsFile:read("*all")
        if optionsData ~= nil then
            local options = optionsparser.parse_data(optionsData)
            debug.message("Success: "..options["game_language"])
        else
            debug.message("Failed to read extdata:/options.txt")
        end
    else
        debug.message("Failed to open extdata:/options.txt")
    end
end

openAndReadOptionsFile()