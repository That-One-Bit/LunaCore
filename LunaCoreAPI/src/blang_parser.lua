local struct = CoreAPI.Utils.Struct

---@class BlangParser
local blang_parser = CoreAPI.Utils.Classic:extend()

---@class blang_parser_funcs
local blang_parser_funcs = {}

---New blang parser
---@param file FilesystemFile
---@return BlangParser
function blang_parser_funcs.newParser(file)
    return blang_parser(file)
end

function blang_parser:new(file)
    self.parsed = false
    if not file:isOpen() then
        return
    end
    local file_data = file:read("*all")
    if not file_data then
        return
    end
    local length = struct.unpack("<I", string.sub(file_data, 1, 4))
    local idx = 5
    local stringDataPos = idx + length * 4 * 2 + 4
    local indexData = {}
    for i = 0, length - 1 do
        local hashName = struct.unpack("<I", file_data:sub(idx + 4 * 2 * i, idx + 4 * 2 * i + 3))
        local texPos = struct.unpack("<I", file_data:sub(idx + 4 * 2 * i + 4, idx + 4 * 2 * i + 4 + 3))
        table.insert(indexData, {hashName, texPos})
    end
    local data = {}
    for index, value in ipairs(indexData) do
        local finalPos = 0
        if index < #indexData then
            finalPos = stringDataPos + indexData[index + 1][2] - 2
        else
            finalPos = #file_data - 1
        end
        data[value[1]] = string.sub(file_data, stringDataPos + value[2], finalPos)
    end
    self.data = data
    self.parsed = true
end

--- Adds a text
---@param textId string
---@return boolean
function blang_parser:containsText(textId)
    return self.data[CoreAPI.Utils.String.hash(textId:lower())] ~= nil
end

--- Adds a text
---@param textId string
---@param text string
function blang_parser:addText(textId, text)
    self.data[CoreAPI.Utils.String.hash(textId:lower())] = text
end

--- Dumps to file
---@param file FilesystemFile
---@return boolean
function blang_parser:dumpFile(file)
    if not file:isOpen() then
        return false
    end
    local keys = {}
    for key, _ in pairs(self.data) do
        table.insert(keys, key)
    end
    table.sort(keys)

    local indexData = {}
    local currStringPos = 0
    file:seek(0, "set")
    file:write(struct.pack("<I", #keys))
    for _, key in ipairs(keys) do
        table.insert(indexData, struct.pack("<I", key) .. struct.pack("<I", currStringPos))
        currStringPos = currStringPos + #self.data[key] + 1
    end
    file:write(table.concat(indexData))
    file:write(struct.pack("<I", currStringPos))
    indexData = nil
    collectgarbage("collect")

    local stringData = {}
    for _, key in ipairs(keys) do
        table.insert(stringData, self.data[key])
    end
    table.insert(stringData, "")
    file:write(table.concat(stringData, string.char(0)))
    file:flush()
    stringData = nil
    collectgarbage("collect")
    return true
end

return blang_parser_funcs