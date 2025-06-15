local filesystem = Core.Filesystem
local debug = Core.Debug

local optionsFile = filesystem.open("extdata:/options.txt", "r")
if optionsFile ~= nil then
    local optionsData = optionsFile:read("*all")
    if optionsData ~= nil then
        local outSdFile = filesystem.open("sdmc:/Minecraft 3DS/options.txt", "w")
        if outSdFile ~= nil then
            if not outSdFile:write(optionsData) then
                debug.message("Failed to write to sdmc:/Minecraft 3DS/options.txt")
            end
        else
            debug.message("Failed to open sdmc:/Minecraft 3DS/options.txt")
        end
    else
        debug.message("Failed to read extdata:/options.txt")
    end
else
    debug.message("Failed to open extdata:/options.txt")
end