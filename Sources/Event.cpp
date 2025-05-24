#include "Event.hpp"

#include <CTRPluginFramework.hpp>

#include "Debug.hpp"

namespace CTRPF = CTRPluginFramework;

extern lua_State *Lua_global;
CTRPF::Clock timeoutEventClock;
bool timeoutEventClockStarted = false;

void TimeoutEventHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutEventClock.HasTimePassed(CTRPluginFramework::Milliseconds(5000)) && timeoutEventClockStarted)
        luaL_error(L, "Event listener exceeded execution time (5000 ms)");
}

void ScriptingEventHandlerCallback()
{
    lua_State *L = Lua_global;

    // KeyPressed Event
    u32 pressedKeys = CTRPF::Controller::GetKeysPressed();
    if (pressedKeys > 0)
    {
        lua_getglobal(L, "Game");
        lua_getfield(L, -1, "Event");
        lua_getfield(L, -1, "OnKeyPressed");
        lua_getfield(L, -1, "Trigger");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, -2);
            if (lua_pcall(L, 1, 0, 0))
            {
                DebugLogError("Engine Event module error in OnKeyPressed: " + std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        else {
            DebugLogError("Unexpected type for Event.OnKeyPressed:Trigger");
            lua_pop(L, 1);
        }
        lua_pop(L, 3);
    }

    u32 downKeys = CTRPF::Controller::GetKeysDown();
    if (downKeys > 0)
    {
        lua_getglobal(L, "Game");
        lua_getfield(L, -1, "Event");
        lua_getfield(L, -1, "OnKeyDown");
        lua_getfield(L, -1, "Trigger");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, -2);
            if (lua_pcall(L, 1, 0, 0))
            {
                DebugLogError("Engine Event module error in OnKeyDown: " + std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        else {
            DebugLogError("Unexpected type for Event.OnKeyDown:Trigger");
            lua_pop(L, 1);
        }
        lua_pop(L, 3);
    }

    u32 releasedKeys = CTRPF::Controller::GetKeysReleased();
    if (releasedKeys > 0)
    {
        lua_getglobal(L, "Game");
        lua_getfield(L, -1, "Event");
        lua_getfield(L, -1, "OnKeyReleased");
        lua_getfield(L, -1, "Trigger");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, -2);
            if (lua_pcall(L, 1, 0, 0))
            {
                DebugLogError("Engine Event module error in OnKeyReleased: " + std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        else {
            DebugLogError("Unexpected type for Event.OnKeyReleased:Trigger");
            lua_pop(L, 1);
        }
        lua_pop(L, 3);
    }
}

int l_Event_BaseEvent_Connect(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    lua_pushvalue(L, 2);
    lua_pushcclosure(L, [](lua_State* L) -> int {
        lua_sethook(L, TimeoutEventHook, LUA_MASKCOUNT, 100); // Timeout hook
        timeoutEventClockStarted = true;
        timeoutEventClock.Restart();

        int nargs = lua_gettop(L);
        lua_pushvalue(L, lua_upvalueindex(1));

        for (int i = 1; i <= nargs; ++i)
            lua_pushvalue(L, i);

        int status = lua_pcall(L, nargs, LUA_MULTRET, 0);

        timeoutEventClockStarted = false;
        lua_sethook(L, nullptr, 0, 0); // Disable after use

        if (status != 0)
            return lua_error(L);
        return lua_gettop(L);
    }, 1);

    lua_getfield(L, 1, "listeners");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, 1, "listeners");
    }

    int index = lua_objlen(L, -1) + 1;
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, index);
    
    lua_pop(L, 1);
    return 0;
}

//@@EventClass
/*
- Adds a function to call when this events fires
## func: function
### EventClass:Connect

- Fire this event
### EventClass:Trigger
*/

int l_register_Event(lua_State *L)
{
    lua_getglobal(L, "Game");
    //$Game.Event
    lua_newtable(L);

    //$@@@Game.Event.BaseEvent: EventClass
    lua_newtable(L);
    lua_pushcfunction(L, l_Event_BaseEvent_Connect);
    lua_setfield(L, -2, "Connect");
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    lua_setfield(L, -2, "BaseEvent");

    lua_setfield(L, -2, "Event");
    lua_pop(L, 1);

    const char* luaCode = R"(
        local function errHandler(err)
            return {
                message = tostring(err),
                traceback = debug.traceback(err, 2)
            }
        end

        function Game.Event.BaseEvent:Trigger(...)
            for i = #self.listeners, 1, -1 do
                local args = { ... }
                local listener = self.listeners[i]
                local success, result = xpcall(function()
                    return listener(unpack(args))
                end, errHandler)
                if not success then
                    Game.Debug.error(result.message)
                    Game.Debug.logerror(result.traceback)
                    table.remove(self.listeners, i)
                end
            end
        end
    )";
    if (luaL_dostring(L, luaCode))
    {
        DebugLogMessage("Engine module 'Event.BaseEvent' failed load: " + std::string(lua_tostring(L, -1)), true);
        lua_pop(L, 1);
    }

    luaL_newmetatable(L, "EventClass");
    int metatableIdx = lua_gettop(L);
    lua_pushcfunction(L, luaC_invalid_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_getglobal(L,"Game");
    lua_getfield(L, -1, "Event");
    lua_getfield(L, -1, "BaseEvent");
    lua_setfield(L, metatableIdx, "__index");
    lua_pop(L, 3);
    
    lua_getglobal(L, "Game");
    lua_getfield(L, -1, "Event");

    //$@@@Game.Event.OnNewFrame: EventClass
    lua_newtable(L);
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    luaC_setmetatable(L, "EventClass");
    lua_setfield(L, -2, "OnNewFrame");

    //$@@@Game.Event.OnKeyPressed: EventClass
    lua_newtable(L);
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    luaC_setmetatable(L, "EventClass");
    lua_setfield(L, -2, "OnKeyPressed");

    //$@@@Game.Event.OnKeyDown: EventClass
    lua_newtable(L);
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    luaC_setmetatable(L, "EventClass");
    lua_setfield(L, -2, "OnKeyDown");

    //$@@@Game.Event.OnKeyReleased: EventClass
    lua_newtable(L);
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    luaC_setmetatable(L, "EventClass");
    lua_setfield(L, -2, "OnKeyReleased");

    lua_pop(L, 2);
    return 0;
}