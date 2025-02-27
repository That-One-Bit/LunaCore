#include "Gamepad.hpp"

#include <string>

#include <CTRPluginFramework.hpp>

int l_Gamepad_isPressed(lua_State *L)
{
    CTRPluginFramework::Key keycode = (CTRPluginFramework::Key)lua_tointeger(L, 1);

    if (CTRPluginFramework::Controller::IsKeyDown(keycode) | CTRPluginFramework::Controller::IsKeyPressed(keycode))
        lua_pushboolean(L, true);
    else
        lua_pushboolean(L, false);
    return 1;
}

int l_Gamepad_virtualPress(lua_State *L)
{
    CTRPluginFramework::Key keycode = (CTRPluginFramework::Key)lua_tointeger(L, 1);

    CTRPluginFramework::Process::Write32(0x00B329E8, keycode);
    CTRPluginFramework::Process::Write32(0x00B32948, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A00, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A18, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A30, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A48, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A60, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A78, keycode);
    CTRPluginFramework::Process::Write32(0x00B32A90, keycode);
    CTRPluginFramework::Process::Write32(0x00B32AB0, keycode);
    return 0;
}

static const luaL_Reg gamepad_functions[] =
{
    {"isPressed", l_Gamepad_isPressed},
    {"virtualPress", l_Gamepad_virtualPress},
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

        Gamepad.KeyCodes = {}
        Gamepad.KeyCodes.A = 1
        Gamepad.KeyCodes.B = Bits.lshift(1, 1)
        Gamepad.KeyCodes.SELECT = Bits.lshift(1, 2)
        Gamepad.KeyCodes.START = Bits.lshift(1, 3)
        Gamepad.KeyCodes.DPADRIGHT = Bits.lshift(1, 4)
        Gamepad.KeyCodes.DPADLEFT = Bits.lshift(1, 5)
        Gamepad.KeyCodes.DPADUP = Bits.lshift(1, 6)
        Gamepad.KeyCodes.DPADDOWN = Bits.lshift(1, 7)
        Gamepad.KeyCodes.R = Bits.lshift(1, 8)
        Gamepad.KeyCodes.L = Bits.lshift(1, 9)
        Gamepad.KeyCodes.X = Bits.lshift(1, 10)
        Gamepad.KeyCodes.Y = Bits.lshift(1, 11)
        Gamepad.KeyCodes.ZL = Bits.lshift(1, 14)
        Gamepad.KeyCodes.ZR = Bits.lshift(1, 15)
        Gamepad.KeyCodes.TOUCHPAD = Bits.lshift(1, 20)
        Gamepad.KeyCodes.CSTICKRIGHT = Bits.lshift(1, 24)
        Gamepad.KeyCodes.CSTICKLEFT = Bits.lshift(1, 25)
        Gamepad.KeyCodes.CSTICKUP = Bits.lshift(1, 26)
        Gamepad.KeyCodes.CSTICKDOWN = Bits.lshift(1, 27)
        Gamepad.KeyCodes.CPADRIGHT = Bits.lshift(1, 28)
        Gamepad.KeyCodes.CPADLEFT = Bits.lshift(1, 29)
        Gamepad.KeyCodes.CPADUP = Bits.lshift(1, 30)
        Gamepad.KeyCodes.CPADDOWN = Bits.lshift(1, 31)
        Gamepad.KeyCodes.UP = Bits.bor(Gamepad.KeyCodes.DPADUP, Gamepad.KeyCodes.CPADUP)
        Gamepad.KeyCodes.DOWN = Bits.bor(Gamepad.KeyCodes.DPADDOWN, Gamepad.KeyCodes.CPADDOWN)
        Gamepad.KeyCodes.LEFT = Bits.bor(Gamepad.KeyCodes.DPADLEFT, Gamepad.KeyCodes.CPADLEFT)
        Gamepad.KeyCodes.RIGHT = Bits.bor(Gamepad.KeyCodes.DPADRIGHT, Gamepad.KeyCodes.CPADRIGHT)
        Gamepad.KeyCodes.CPAD = Bits.bor(Bits.bor(Gamepad.KeyCodes.CPADLEFT, Gamepad.KeyCodes.CPADRIGHT), Bits.bor(Gamepad.KeyCodes.CPADUP, Gamepad.KeyCodes.CPADDOWN))
        Gamepad.KeyCodes.CSTICK = Bits.bor(Bits.bor(Gamepad.KeyCodes.CSTICKLEFT, Gamepad.KeyCodes.CSTICKRIGHT), Bits.bor(Gamepad.KeyCodes.CSTICKUP, Gamepad.KeyCodes.CSTICKDOWN))

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