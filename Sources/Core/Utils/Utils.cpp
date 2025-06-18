#include "Core/Utils/Utils.hpp"

#include <cstdlib>

#include <CTRPluginFramework.hpp>

#include "Core/Debug.hpp"

namespace CTRPF = CTRPluginFramework;

std::string Core::Utils::formatTime(time_t time) {
    struct tm *timeInfo = localtime(&time);
    std::string outStr;
    if (timeInfo->tm_hour < 10)
        outStr += "0";
    outStr += std::to_string(timeInfo->tm_hour)+":";
    if (timeInfo->tm_min < 10)
        outStr += "0";
    outStr += std::to_string(timeInfo->tm_min)+":";
    if (timeInfo->tm_sec < 10)
        outStr += "0";
    outStr += std::to_string(timeInfo->tm_sec);
    return outStr;
}

std::string Core::Utils::strip(const std::string &str) {
    if (str.empty())
        return "";
    size_t startPos = 0;
    size_t endPos = str.size();
    while (str[startPos] == ' ' && startPos < str.size())
        startPos++;
    while (str[endPos - 1] == ' ' && endPos > startPos)
        endPos--;
    if (startPos == endPos)
        return "";
    std::string newStr = str.substr(startPos, endPos - startPos);
    return newStr;
}

bool Core::Utils::startsWith(const std::string &str, const std::string &prefix) {
    return str.compare(0, prefix.size(), prefix) == 0;
}

std::string Core::Utils::LoadFile(const std::string &filepath)
{
    std::string content;
    CTRPF::File file_ptr;
    CTRPF::File::Open(file_ptr, filepath, CTRPF::File::READ);
    if (!file_ptr.IsOpen())
        return content;
    file_ptr.Seek(0, CTRPF::File::SeekPos::END);
    size_t fileSize = file_ptr.Tell();
    file_ptr.Seek(0, CTRPF::File::SeekPos::SET);
    char *fileContent = (char *)malloc(fileSize + 1);
    if (fileContent == NULL) 
        return content;
    file_ptr.Read(fileContent, fileSize);
    fileContent[fileSize] = '\0';
    content = std::string(fileContent);
    free(fileContent);
    return content;
}

void Core::Utils::Replace(std::string &str, const std::string &pattern, const std::string &repl)
{
    size_t pos = 0;
    while ((pos = str.find(pattern, pos)) != std::string::npos) {
        str.replace(pos, pattern.size(), repl);
        pos += repl.size();
    }
}

bool Core::RegisterUtilsModule(lua_State *L)
{
    const char *lua_Code = R"(
        function readOnlyTable(tbl, tblName)
            local mt_newindex_func = function(_, key, _)
                error(tblName.." is read-only")
            end
            return setmetatable({}, {__index = tbl, __newindex = mt_newindex_func})
        end
    )";
    if (luaL_dostring(L, lua_Code))
    {
        Core::Debug::LogError("Core::Utils::Load error: " + std::string(lua_tostring(L, -1)));
        lua_pop(L, 1);
        return false;
    }
    return true;
}

void Core::UnregisterUtilsModule(lua_State *L)
{
    lua_pushnil(L);
    lua_setglobal(L, "readOnlyTable");
    lua_gc(L, LUA_GCCOLLECT, 0);
}