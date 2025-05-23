#include "Async.hpp"

#include <CTRPluginFramework.hpp>

extern CTRPluginFramework::Clock timeoutClock;
extern bool timeoutClockStarted;

void AsyncTimeoutHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutClock.HasTimePassed(CTRPluginFramework::Milliseconds(2000)) && timeoutClockStarted)
        luaL_error(L, "Exceeded execution time (2000 ms)");
}

int l_registerCoroutineTimeoutHook(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTHREAD);

    lua_State *co = lua_tothread(L, 1);

    lua_sethook(co, AsyncTimeoutHook, LUA_MASKCOUNT, 100);
    return 0;
}

int l_startInternalClock(lua_State *L)
{
    timeoutClock.Restart();
    timeoutClockStarted = true;
    return 0;
}

int l_stopInternalClock(lua_State *L)
{
    timeoutClockStarted = false;
    return 0;
}

int l_restartInternalClock(lua_State *L)
{
    timeoutClock.Restart();
    return 0;
}

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
                local co = coroutine.create(function ()
                    Async.startInternalClock()
                    func(...)
                    Async.stopInternalClock()
                end)
                Async.registerTimeoutHook(co)
                table.insert(Async.scripts, co)
            end
        end

        function Async.wait(seconds)
            Async.restartInternalClock()
            if seconds == nil then
                coroutine.yield()
                return true
            end
            local start = System.getTime()
            while System.getTime() - start < seconds do
                Async.restartInternalClock()
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
    lua_getglobal(L, "Async");
    lua_pushcfunction(L, l_registerCoroutineTimeoutHook);
    lua_setfield(L, -2, "registerTimeoutHook");
    lua_pushcfunction(L, l_restartInternalClock);
    lua_setfield(L, -2, "restartInternalClock");
    lua_pushcfunction(L, l_startInternalClock);
    lua_setfield(L, -2, "startInternalClock");
    lua_pushcfunction(L, l_stopInternalClock);
    lua_setfield(L, -2, "stopInternalClock");
    lua_pop(L, 1);
    return 0;
}