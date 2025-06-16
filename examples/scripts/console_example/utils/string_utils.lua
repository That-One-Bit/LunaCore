local stringUtils = {}

--- Returns a table with a string splitted
---@param s string
---@param separator string
---@return table<string>
function stringUtils.split(s, separator)
    if separator == nil then
        separator = "%s"
    end
    local escaped_delimiter = separator:gsub('([%.%*%+%?%^%$%[%]%(%)%{%}%|%\\])', '%%%1')
    local pattern = string.format('([^%s]+)', escaped_delimiter)
    local result = {}
    for str in s:gmatch(pattern) do
        table.insert(result, str)
    end
    return result
end

return stringUtils