#include "Utils.hpp"

#include "Debug.hpp"

int l_register_Utils(lua_State *L)
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
        DebugLogMessage("Engine Async module failed load: " + std::string(lua_tostring(L, -1)), true);
        lua_pop(L, 1);
    }
    return 0;
}

int l_unregister_Utils(lua_State *L)
{
    lua_pushnil(L);
    lua_setglobal(L, "readOnlyTable");
    lua_gc(L, LUA_GCCOLLECT, 0);
    return 0;
}