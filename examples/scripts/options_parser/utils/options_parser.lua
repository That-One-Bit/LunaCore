local parser = {}
local stringUtils = require("utils.string_utils")

local function bytesToUint32(b)
    local num = Bits.band(b[1], 0xFF)
    num = Bits.bor(num, Bits.lshift(Bits.band(b[2], 0xFF), 8))
    num = Bits.bor(num, Bits.lshift(Bits.band(b[3], 0xFF), 16))
    num = Bits.bor(num, Bits.lshift(Bits.band(b[4], 0xFF), 24))
    return num
end

local function Uint32ToBytes(num)
    local b = {}
    table.insert(b, Bits.band(num, 0xFF))
    table.insert(b, Bits.band(Bits.rshift(num, 8), 0xFF))
    table.insert(b, Bits.band(Bits.rshift(num, 16), 0xFF))
    table.insert(b, Bits.band(Bits.rshift(num, 24), 0xFF))
    return b
end

function parser.parse_data(data)
    local b = { string.byte(data, 1, 4) }
    local len = bytesToUint32(b)
    local content = string.sub(data, 5, 5+len)
    local splittedContent = stringUtils.split(content, "\n")
    local i = 1
    local dataTbl = {}
    while splittedContent[i] ~= nil and #splittedContent[i] > 0 do
        local lineSplit = stringUtils.split(splittedContent[i], ":")
        local key = lineSplit[1]
        local value = lineSplit[2]
        if #lineSplit >= 3 then
            for j = 3, #lineSplit do
                value = value..":"..lineSplit[j]
            end
        end
        if value ~= nil and #value > 0 and value[1] == "[" and value[#value] == "]" then
            value = stringUtils.split(string.gsub(value, "[", ""):gsub("]", ""), ",")
        end
        dataTbl[key] = value
        i = i + 1
    end
    return dataTbl
end

return parser