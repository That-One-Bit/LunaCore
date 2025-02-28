#include "Async.hpp"

#include <CTRPluginFramework.hpp>

#include <time.h>

// TODO: Move to another module
int l_Async_getTime(lua_State *L)
{
    lua_pushnumber(L, time(NULL));
    return 1;
}

static const luaL_Reg async_functions[] =
{
    {"getTime", l_Async_getTime},
    {NULL, NULL}
};

int luaopen_Async(lua_State *L)
{
    lua_newtable(L);
    luaL_register(L, NULL, async_functions);
    lua_setglobal(L, "Async");

    const char *luaCode = R"(
        Async.scripts = {}
        Async.dt = 0

        function Async.create(func)
            if type(func) ~= "function" then
                error("Expected function in 'func'", 2)
            else
                table.insert(Async.scripts, coroutine.create(func))
            end
        end

        function Async.wait(seconds)
            if seconds == nil then
                coroutine.yield()
                return
            end
            -- TODO: Lua get time
            local start = Async.getTime()
            while Async.getTime() - start < seconds do
                coroutine.yield()
            end
        end

        function Async.tick()
            for i = #Async.scripts, 1, -1 do
                if coroutine.status(Async.scripts[i]) ~= "dead" then
                    local success, message = coroutine.resume(Async.scripts[i])
                    if not success then
                        Debug.showMessage("Error in script: "..message)
                    end
                else
                    table.remove(Async.scripts, i)
                end
            end
        end
    )";
    if (luaL_dostring(L, luaCode))
    {
        const char *error = lua_tostring(L, -1);
        CTRPluginFramework::OSD::Notify("Lua error: " + std::string(lua_tostring(L, -1)));
    }
    return 0;
}