#include "Event.hpp"

#include <CTRPluginFramework.hpp>

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
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    lua_setfield(L, -2, "BaseEvent");

    lua_setfield(L, -2, "Event");
    lua_pop(L, 1);

    const char* luaCode = R"(
        function Game.Event.BaseEvent:Connect(func)
            table.insert(self.listeners, func)
        end

        function Game.Event.BaseEvent:Trigger()
            for _, listener in ipairs(self.listeners) do
                listener()
            end
        end
    )";
    if (luaL_dostring(L, luaCode))
    {
        CTRPluginFramework::OSD::Notify("Lua error: " + std::string(lua_tostring(L, -1)));
        lua_pop(L, 1);
    }

    luaL_newmetatable(L, "EventClass");
    int metatableIdx = lua_gettop(L);
    lua_getglobal(L,"Game");
    lua_getfield(L, -1, "Event");
    lua_getfield(L, -1, "BaseEvent");
    lua_setfield(L, metatableIdx, "__index");
    lua_pop(L, 3);
    
    lua_getglobal(L, "Game");
    lua_getfield(L, -1, "Event");

    //$@@@Game.Event.OnNewFrame: EventClass
    lua_newtable(L);
    luaC_setmetatable(L, "EventClass");
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    lua_setfield(L, -2, "OnNewFrame");

    //$@@@Game.Event.OnKeyPressed: EventClass
    lua_newtable(L);
    luaC_setmetatable(L, "EventClass");
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    lua_setfield(L, -2, "OnKeyPressed");

    //$@@@Game.Event.OnKeyReleased: EventClass
    lua_newtable(L);
    luaC_setmetatable(L, "EventClass");
    lua_newtable(L);
    lua_setfield(L, -2, "listeners");
    lua_setfield(L, -2, "OnKeyReleased");

    lua_pop(L, 2);
    return 0;
}