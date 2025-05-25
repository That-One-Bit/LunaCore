#include "Async.hpp"

#include <CTRPluginFramework.hpp>

#include "Debug.hpp"

extern lua_State *Lua_global;
CTRPluginFramework::Clock timeoutAsynClock;

void TimeoutAsyncHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutAsynClock.HasTimePassed(CTRPluginFramework::Milliseconds(5000)))
        luaL_error(L, "Async coroutine exceeded execution time (5000 ms)");
}

void CoreAsyncHandlerCallback()
{
    lua_State *L = Lua_global;

    // Async handles timeout hooks individually so no need to configure
    // Async coroutines
    lua_getglobal(L, "Async");
    lua_getfield(L, -1, "tick");
    
    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0))
        {
            DebugLogError("Core error: " + std::string(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
    }
    else
        lua_pop(L, 1);
    lua_pop(L, 1);
}

int l_Async_create(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);

    lua_State *co = lua_newthread(L);
    lua_pushvalue(L, 1);
    lua_xmove(L, co, 1);

    lua_sethook(co, TimeoutAsyncHook, LUA_MASKCOUNT, 100);

    lua_getglobal(L, "Async");
    lua_getfield(L, -1, "scripts");
    lua_remove(L, -2);

    int len = lua_objlen(L, -1);
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, len + 1);

    lua_pop(L, 2);
    return 0;
}

int l_Async_tick(lua_State *L)
{
    lua_getglobal(L, "Async");
    lua_getfield(L, -1, "scripts");
    lua_remove(L, -2);
    int scriptsTableIdx = lua_gettop(L);

    int len = lua_objlen(L, -1);

    for (int i = len; i >= 1; --i) {
        lua_rawgeti(L, -1, i);
        lua_State *co = lua_tothread(L, -1);
        int coIdx = lua_gettop(L);

        int status = lua_status(co);
        if (status != 0 && status != LUA_YIELD) {
            lua_pop(L, 1); // Remove co
            lua_pushnil(L);
            lua_rawseti(L, scriptsTableIdx, i);
            lua_gc(L, LUA_GCCOLLECT, 0);
            continue;
        }

        timeoutAsynClock.Restart();
        int call_result = lua_resume(co, 0);

        if (call_result != 0 && call_result != LUA_YIELD) {
            const char *errMsg = lua_tostring(co, -1);
            
            // TODO: Improve this code to not use temporal globals
            lua_pushvalue(L, coIdx);
            lua_setglobal(L, "__THREAD_TEMP");
            lua_pop(L, 1); // Remove co

            lua_pushstring(L, errMsg);
            lua_setglobal(L, "__ERRMSG_TEMP");

            if (luaL_dostring(L, "Game.Debug.logerror(debug.traceback(__THREAD_TEMP, __ERRMSG_TEMP):gsub('\\t', '    '))")) {
                DebugLogError("Core error: "+std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }

            lua_pushnil(L); lua_setglobal(L, "__THREAD_TEMP");
            lua_pushnil(L); lua_setglobal(L, "__ERRMSG_TEMP");

            lua_pushnil(L);
            lua_rawseti(L, scriptsTableIdx, i);
            lua_gc(L, LUA_GCCOLLECT, 0);
            continue;
        }
        lua_pop(L, 1); // Remove co
    }
    lua_pop(L, 1); // Remove scripts
    return 0;
}

/*
$Async

- Adds the function to the queue that will run apart from the game until the functions ends
## func: function
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
    )";
    if (luaL_dostring(L, luaCode))
    {
        DebugLogMessage("Engine Async module failed load: " + std::string(lua_tostring(L, -1)), true);
        lua_pop(L, 1);
    }
    lua_getglobal(L, "Async");
    lua_pushcfunction(L, l_Async_create);
    lua_setfield(L, -2, "create");
    lua_pushcfunction(L, l_Async_tick);
    lua_setfield(L, -2, "tick");
    lua_pop(L, 1);
    return 0;
}