local fs = Core.Filesystem
local debug = Core.Debug

local function iterateDir(path)
    local elements = fs.getDirectoryElements(path)
    for index, value in ipairs(elements) do
        debug.message(value)
    end
end

local function iterateRDir(path)
    local elements = fs.getDirectoryElements(path)
    for index, value in ipairs(elements) do
        debug.message(path..value)
        if fs.directoryExists(path..value) then -- directoryExists can also be used to detect if a path is a directory
            iterateRDir(path..value.."/")
        end
    end
end

iterateRDir("extdata:/")