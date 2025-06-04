#include "Keyboard.hpp"

#include <CTRPluginFramework.hpp>

namespace CTRPF = CTRPluginFramework;

// ----------------------------------------------------------------------------

//$Keyboard

// ----------------------------------------------------------------------------

/*
- Opens the keyboard and returns the user input as string
## message: string?
## return: string
### Keyboard.getString
*/
static int l_Keyboard_getString(lua_State *L) {
    CTRPF::Keyboard keyboard;
    if (lua_gettop(L) >= 1) {
        const char *messageText = luaL_checkstring(L, 1);
        std::string &keyboardMsg = keyboard.GetMessage();
        keyboardMsg.assign(messageText);
        keyboard.DisplayTopScreen = true;
    }
    std::string inputText;
    if (keyboard.Open(inputText) == 0)
        lua_pushstring(L, inputText.c_str());
    else
        lua_pushstring(L, "");
    return 1;
}

/*
- Opens the keyboard and returns the user input as number
## message: string?
## return: number
### Keyboard.getNumber
*/
static int l_Keyboard_getNumber(lua_State *L) {
    CTRPF::Keyboard keyboard;
    if (lua_gettop(L) >= 1) {
        const char *messageText = luaL_checkstring(L, 1);
        std::string &keyboardMsg = keyboard.GetMessage();
        keyboardMsg.assign(messageText);
        keyboard.DisplayTopScreen = true;
    }
    float inputNumber;
    if (keyboard.Open(inputNumber) == 0)
        lua_pushnumber(L, inputNumber);
    else
        lua_pushnumber(L, -1);
    return 1;
}

/*
- Opens the keyboard and returns the user input as unsigned integer
## message: string?
## return: integer
### Keyboard.getInteger
*/
static int l_Keyboard_getInteger(lua_State *L) {
    CTRPF::Keyboard keyboard;
    if (lua_gettop(L) >= 1) {
        const char *messageText = luaL_checkstring(L, 1);
        std::string &keyboardMsg = keyboard.GetMessage();
        keyboardMsg.assign(messageText);
        keyboard.DisplayTopScreen = true;
    }
    u32 inputNumber;
    if (keyboard.Open(inputNumber) == 0)
        lua_pushinteger(L, inputNumber);
    else
        lua_pushinteger(L, -1);
    return 1;
}

/*
- Opens the keyboard and returns the user input as hexadecimal
## message: string?
## return: integer
### Keyboard.getHex
*/
static int l_Keyboard_getHex(lua_State *L) {
    CTRPF::Keyboard keyboard;
    if (lua_gettop(L) >= 1) {
        const char *messageText = luaL_checkstring(L, 1);
        std::string &keyboardMsg = keyboard.GetMessage();
        keyboardMsg.assign(messageText);
        keyboard.DisplayTopScreen = true;
    }
    u32 inputNumber;
    keyboard.IsHexadecimal(true);
    if (keyboard.Open(inputNumber) == 0)
        lua_pushinteger(L, inputNumber);
    else
        lua_pushinteger(L, -1);
    return 1;
}

static const luaL_Reg keyboard_functions[] =
{
    {"getString", l_Keyboard_getString},
    {"getNumber", l_Keyboard_getNumber},
    {"getInteger", l_Keyboard_getInteger},
    {"getHex", l_Keyboard_getHex},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

bool Core::RegisterKeyboardModule(lua_State *L) {
    luaC_register_global(L, keyboard_functions, "Keyboard");
    return true;
}