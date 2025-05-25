#include "Utils.hpp"

#include "Debug.hpp"

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
        Core::Debug::LogError("Core Utils module failed load: " + std::string(lua_tostring(L, -1)));
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