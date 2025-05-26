#include "Async.hpp"

#include <CTRPluginFramework.hpp>

#include "Debug.hpp"
#include "Utils/Utils.hpp"

extern lua_State *Lua_global;
CTRPluginFramework::Clock timeoutAsynClock;

static void TimeoutAsyncHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutAsynClock.HasTimePassed(CTRPluginFramework::Milliseconds(5000)))
        luaL_error(L, "Async coroutine exceeded execution time (5000 ms)");
}

void Core::AsyncHandlerCallback()
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
            Core::Debug::LogError("Core error: " + std::string(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
    }
    else
        lua_pop(L, 1);
    lua_pop(L, 1);
}

// ----------------------------------------------------------------------------

//$Async

// ----------------------------------------------------------------------------

/*
- Adds the function to the queue that will run apart from the game until the functions ends
## func: function
### Async.create
*/
static int l_Async_create(lua_State *L)
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

static int l_Async_tick(lua_State *L)
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
            
            lua_getglobal(L, "debug");
            lua_getfield(L, -1, "traceback");
            lua_remove(L, -2);
            lua_pushvalue(L, coIdx);
            lua_pushstring(L, errMsg);

            if (lua_pcall(L, 2, 1, 0)) {
                Core::Debug::LogError("Core error: "+std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            } else {
                std::string traceback(lua_tostring(L, -1));
                Core::Utils::Replace(traceback, "\t", "    ");
                Core::Debug::LogError(traceback);
                lua_pop(L, 1);
            }
            lua_pop(L, 1); // Remove co

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

// ----------------------------------------------------------------------------

/*
- Yeilds the current task until time has passed. Always returns true
## seconds: number?
## return: boolean
### Async.wait
*/

bool Core::RegisterAsyncModule(lua_State *L)
{
    lua_newtable(L);
    lua_newtable(L);
    lua_setfield(L, -2, "scripts");
    lua_pushcfunction(L, l_Async_create);
    lua_setfield(L, -2, "create");
    lua_pushcfunction(L, l_Async_tick);
    lua_setfield(L, -2, "tick");
    lua_setglobal(L, "Async");

    const char *luaCode = R"(
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

        Async = readOnlyTable(Async, "Async")
    )";
    if (luaL_dostring(L, luaCode))
    {
        Core::Debug::LogError("Core Async module failed load: " + std::string(lua_tostring(L, -1)));
        lua_pop(L, 1);
        return false;
    }
    return true;
}