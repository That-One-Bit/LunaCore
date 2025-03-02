#include "Gamepad.hpp"

#include <CTRPluginFramework.hpp>

int l_Gamepad_IsPressed(lua_State *L)
{
    CTRPluginFramework::Key keycode = (CTRPluginFramework::Key)lua_tointeger(L, 1);

    if (CTRPluginFramework::Controller::IsKeyPressed(keycode))
        lua_pushboolean(L, true);
    else
        lua_pushboolean(L, false);
    return 1;
}

int l_Gamepad_IsDown(lua_State *L)
{
    CTRPluginFramework::Key keycode = (CTRPluginFramework::Key)lua_tointeger(L, 1);

    if (CTRPluginFramework::Controller::IsKeyDown(keycode))
        lua_pushboolean(L, true);
    else
        lua_pushboolean(L, false);
    return 1;
}

int l_Gamepad_PressButton(lua_State *L)
{
    CTRPluginFramework::Key keycode = (CTRPluginFramework::Key)lua_tointeger(L, 1);

    /*CTRPluginFramework::Process::Write32(0x00B329E8, keycode);
    CTRPluginFramework::Process::Write32(0x00B32948, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A00, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A18, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A30, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A48, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A60, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A78, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A90, keycode);
    CTRPluginFramework::Process::Write32(0x00B32AB0, keycode);*/
    CTRPluginFramework::Controller::InjectKey(keycode);
    return 0;
}

static const luaL_Reg gamepad_functions[] =
{
    {"IsPressed", l_Gamepad_IsPressed},
    {"IsDown", l_Gamepad_IsDown},
    {"PressButton", l_Gamepad_PressButton},
    {NULL, NULL}
};

int luaopen_Gamepad(lua_State *L)
{
    lua_newtable(L);
    luaL_register(L, NULL, gamepad_functions);
    lua_setglobal(L, "Gamepad");

    const char *luaCode = R"(
        Gamepad.KeyPressed = {}
        Gamepad.KeyPressed.listeners = {}

        Gamepad.KeyCode = {}
        Gamepad.KeyCode.A = 1
        Gamepad.KeyCode.B = Bits.lshift(1, 1)
        Gamepad.KeyCode.SELECT = Bits.lshift(1, 2)
        Gamepad.KeyCode.START = Bits.lshift(1, 3)
        Gamepad.KeyCode.DPADRIGHT = Bits.lshift(1, 4)
        Gamepad.KeyCode.DPADLEFT = Bits.lshift(1, 5)
        Gamepad.KeyCode.DPADUP = Bits.lshift(1, 6)
        Gamepad.KeyCode.DPADDOWN = Bits.lshift(1, 7)
        Gamepad.KeyCode.R = Bits.lshift(1, 8)
        Gamepad.KeyCode.L = Bits.lshift(1, 9)
        Gamepad.KeyCode.X = Bits.lshift(1, 10)
        Gamepad.KeyCode.Y = Bits.lshift(1, 11)
        Gamepad.KeyCode.ZL = Bits.lshift(1, 14)
        Gamepad.KeyCode.ZR = Bits.lshift(1, 15)
        Gamepad.KeyCode.TOUCHPAD = Bits.lshift(1, 20)
        Gamepad.KeyCode.CSTICKRIGHT = Bits.lshift(1, 24)
        Gamepad.KeyCode.CSTICKLEFT = Bits.lshift(1, 25)
        Gamepad.KeyCode.CSTICKUP = Bits.lshift(1, 26)
        Gamepad.KeyCode.CSTICKDOWN = Bits.lshift(1, 27)
        Gamepad.KeyCode.CPADRIGHT = Bits.lshift(1, 28)
        Gamepad.KeyCode.CPADLEFT = Bits.lshift(1, 29)
        Gamepad.KeyCode.CPADUP = Bits.lshift(1, 30)
        Gamepad.KeyCode.CPADDOWN = Bits.lshift(1, 31)
        Gamepad.KeyCode.UP = Bits.bor(Gamepad.KeyCode.DPADUP, Gamepad.KeyCode.CPADUP)
        Gamepad.KeyCode.DOWN = Bits.bor(Gamepad.KeyCode.DPADDOWN, Gamepad.KeyCode.CPADDOWN)
        Gamepad.KeyCode.LEFT = Bits.bor(Gamepad.KeyCode.DPADLEFT, Gamepad.KeyCode.CPADLEFT)
        Gamepad.KeyCode.RIGHT = Bits.bor(Gamepad.KeyCode.DPADRIGHT, Gamepad.KeyCode.CPADRIGHT)
        Gamepad.KeyCode.CPAD = Bits.bor(Bits.bor(Gamepad.KeyCode.CPADLEFT, Gamepad.KeyCode.CPADRIGHT), Bits.bor(Gamepad.KeyCode.CPADUP, Gamepad.KeyCode.CPADDOWN))
        Gamepad.KeyCode.CSTICK = Bits.bor(Bits.bor(Gamepad.KeyCode.CSTICKLEFT, Gamepad.KeyCode.CSTICKRIGHT), Bits.bor(Gamepad.KeyCode.CSTICKUP, Gamepad.KeyCode.CSTICKDOWN))

        function Gamepad.KeyPressed:Connect(func)
            table.insert(self.listeners, func)
        end

        function Gamepad.KeyPressed:Trigger()
            for _, listener in ipairs(self.listeners) do
                listener()
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