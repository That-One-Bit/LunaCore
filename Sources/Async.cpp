#include "Async.hpp"

#include <CTRPluginFramework.hpp>

/*
$Async

- Adds the function to the queue that will run apart from the game until the functions ends
## func: function
## ...: any
### Async.create

- Yeilds the current task until time has passed. Always returns true
## seconds: number?
## return: boolean
### Async.wait
*/
int luaopen_Async(lua_State *L)
{
    const char *luaCode = R"(
        Async = {}
        Async.scripts = {}

        function Async.create(func, ...)
            if type(func) ~= "function" then
                error("Expected function in 'func'", 2)
            else
                local args = { ... }
                table.insert(Async.scripts, coroutine.create(function ()
                    func(unpack(args))
                end))
            end
        end

        function Async.wait(seconds)
            if seconds == nil then
                coroutine.yield()
                return true
            end
            local start = System.getTime()
            while System.getTime() - start < seconds do
                coroutine.yield()
            end
            return true
        end

        function Async.tick()
            for i = #Async.scripts, 1, -1 do
                if coroutine.status(Async.scripts[i]) ~= "dead" then
                    local success, message = coroutine.resume(Async.scripts[i])
                    if not success then
                        Game.Debug.message("Error in script: "..message)
                        table.remove(Async.scripts, i)
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