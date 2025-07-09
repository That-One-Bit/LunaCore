#include "Core/Event.hpp"

#include <string>
#include <atomic>

#include <CTRPluginFramework.hpp>

#include "Core/Debug.hpp"
#include "Core/CrashHandler.hpp"
#include "Core/Game/Gamepad.hpp"
#include "Core/Utils/Utils.hpp"
#include "CoreGlobals.hpp"

namespace CTRPF = CTRPluginFramework;

CTRPF::Clock timeoutEventClock;
extern std::atomic<bool> graphicsIsTop;

void Core::EventRestartClock() {
    timeoutEventClock.Restart();
}

void TimeoutEventHook(lua_State *L, lua_Debug *ar)
{
    if (timeoutEventClock.HasTimePassed(CTRPluginFramework::Milliseconds(5000)))
        luaL_error(L, "Event listener exceeded execution time (5000 ms)");
}

void Core::EventHandlerCallback()
{
    lua_State *L = Lua_global;
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_LUA_EXEC;

    // KeyPressed Event
    u32 pressedKeys = CTRPF::Controller::GetKeysPressed();
    u32 downKeys = CTRPF::Controller::GetKeysDown();
    u32 releasedKeys = CTRPF::Controller::GetKeysReleased();
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
                Core::Debug::LogError("Core::Event::OnKeyPressed error: " + std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        else {
            Core::Debug::LogError("Core::Event::OnKeyPressed::Trigger error. Unexpected type");
            lua_pop(L, 1);
        }
        lua_pop(L, 3);
    }

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
                Core::Debug::LogError("Core::Event::OnKeyDown error: " + std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        else {
            Core::Debug::LogError("Core::Event::OnKeyDown::Trigger error. Unexpected type");
            lua_pop(L, 1);
        }
        lua_pop(L, 3);
    }

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
                Core::Debug::LogError("Core::Event::OnKeyReleased error: " + std::string(lua_tostring(L, -1)));
                lua_pop(L, 1);
            }
        }
        else {
            Core::Debug::LogError("Core::Event::OnKeyReleased::Trigger error. Unexpected type");
            lua_pop(L, 1);
        }
        lua_pop(L, 3);
    }

    static float lastSlider = 0;
    float slider = osGet3DSliderState();
    if (lastSlider != slider) {
        CTRPF::OSD::Notify(CTRPF::Utils::Format("%f", slider));
        lastSlider = slider;
    }
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_GAME;
}

void Core::Event::TriggerOnPlayerJoinWorld(lua_State *L) {
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_LUA_EXEC;
    lua_getglobal(L, "Game");
    lua_getfield(L, -1, "Event");
    lua_getfield(L, -1, "OnPlayerJoinWorld");
    lua_getfield(L, -1, "Trigger");

    if (lua_isfunction(L, -1))
    {
        lua_pushvalue(L, -2);
        if (lua_pcall(L, 1, 0, 0))
        {
            Core::Debug::LogError("Core::Event::OnPlayerJoinWorld error: " + std::string(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
    }
    else {
        Core::Debug::LogError("Core::Event::OnPlayerJoinWorld::Trigger error. Unexpected type");
        lua_pop(L, 1);
    }
    lua_pop(L, 3);
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_GAME;
}

void Core::Event::TriggerOnPlayerLeaveWorld(lua_State *L) {
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_LUA_EXEC;
    lua_getglobal(L, "Game");
    lua_getfield(L, -1, "Event");
    lua_getfield(L, -1, "OnPlayerLeaveWorld");
    lua_getfield(L, -1, "Trigger");

    if (lua_isfunction(L, -1))
    {
        lua_pushvalue(L, -2);
        if (lua_pcall(L, 1, 0, 0))
        {
            Core::Debug::LogError("Core::Event::OnPlayerLeaveWorld error: " + std::string(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
    }
    else {
        Core::Debug::LogError("Core::Event::OnPlayerLeaveWorld::Trigger error. Unexpected type");
        lua_pop(L, 1);
    }
    lua_pop(L, 3);
    Core::CrashHandler::core_state = Core::CrashHandler::CORE_GAME;
}

// ----------------------------------------------------------------------------

//$Game.Event

// ----------------------------------------------------------------------------

//@@EventClass

/*
- Adds a function to call when this events fires
## func: function
### EventClass:Connect
*/
static int l_Event_BaseEvent_Connect(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    lua_getfield(L, 1, "listeners");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, 1, "listeners");
    }

    int index = lua_objlen(L, -1) + 1;
    lua_pushvalue(L, 2);
    lua_rawseti(L, -2, index);
    
    lua_pop(L, 1);
    return 0;
}

/*
- Fire this event
### EventClass:Trigger
*/
static int l_Event_BaseEvent_Trigger(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    int argc = lua_gettop(L) - 1;

    lua_getfield(L, 1, "listeners");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "'listeners' is not a table");
    }
    int listenersIdx = lua_gettop(L);
    int len = lua_objlen(L, listenersIdx);

    for (int i = len; i >= 1; --i) {
        lua_rawgeti(L, listenersIdx, i);
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            continue;
        }

        for (int j = 0; j < argc; ++j) 
            lua_pushvalue(L, 2 + j);

        lua_getglobal(L, "debug");
        lua_getfield(L, -1, "traceback");
        lua_remove(L, -2);

        int errfunc = lua_gettop(L);

        lua_insert(L, errfunc - (argc + 1));

        lua_sethook(L, TimeoutEventHook, LUA_MASKCOUNT, 100); // Timeout hook
        timeoutEventClock.Restart();
        
        if (lua_pcall(L, argc, 0, errfunc - (argc + 1))) {
            std::string errMsg(lua_tostring(L, -1));
            Core::Utils::Replace(errMsg, "\t", "    ");
            Core::Debug::LogError("Event error: "+errMsg);
            lua_pop(L, 1);
            lua_remove(L, errfunc - (argc + 1));

            // Remove listener with table.remove
            lua_getglobal(L, "table");
            lua_getfield(L, -1, "remove");
            lua_remove(L, -2);
            lua_pushvalue(L, listenersIdx);
            lua_pushinteger(L, i);
            if (lua_pcall(L, 2, 1, 0))
                Core::Debug::LogError(CTRPF::Utils::Format("Core::Event::BaseEvent::Trigger error: %s", lua_tostring(L, -1)));
            lua_pop(L, 1); // Remove either error string or returned value
            lua_gc(L, LUA_GCCOLLECT, 0);
        } else {
            lua_remove(L, errfunc - (argc + 1));
        }

        lua_sethook(L, nullptr, 0, 0); // Disable after use
    }
    lua_pop(L, 1); // Pop listeners
    return 0;
}

// ----------------------------------------------------------------------------

bool Core::Module::RegisterEventModule(lua_State *L)
{
    lua_getglobal(L, "Game");
    lua_newtable(L); // Game.Event

    //$@@@Game.Event.BaseEvent: EventClass
    lua_newtable(L);
    lua_pushcfunction(L, l_Event_BaseEvent_Connect);
    lua_setfield(L, -2, "Connect");
    lua_pushcfunction(L, l_Event_BaseEvent_Trigger);
    lua_setfield(L, -2, "Trigger");
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    lua_setfield(L, -2, "BaseEvent");

    lua_setfield(L, -2, "Event");
    lua_pop(L, 1);

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

    //$@@@Game.Event.OnPlayerJoinWorld: EventClass
    lua_newtable(L);
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    luaC_setmetatable(L, "EventClass");
    lua_setfield(L, -2, "OnPlayerJoinWorld");

    //$@@@Game.Event.OnPlayerLeaveWorld: EventClass
    lua_newtable(L);
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    luaC_setmetatable(L, "EventClass");
    lua_setfield(L, -2, "OnPlayerLeaveWorld");

    lua_pop(L, 2);

    const char *lua_Code = R"(
        Game.Event.BaseEvent = readOnlyTable(Game.Event.BaseEvent, "BaseEvent")
    )";
    if (luaL_dostring(L, lua_Code))
    {
        Core::Debug::LogError("Core::Event::Load error: "+std::string(lua_tostring(L, -1)));
        lua_pop(L, 1);
        return false;
    }
    return true;
}