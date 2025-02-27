#include "Bits.hpp"

typedef uint32_t u32;

int l_Bits_lshift(lua_State *L) 
{
    u32 a = luaL_checkinteger(L, 1);
    u32 n = luaL_checkinteger(L, 2);

    u32 res = a << n;
    lua_pushinteger(L, res);
    return 1;
}

int l_Bits_rshift(lua_State *L) 
{
    u32 a = luaL_checkinteger(L, 1);
    u32 n = luaL_checkinteger(L, 2);

    u32 res = a >> n;
    lua_pushinteger(L, res);
    return 1;
}

int l_Bits_band(lua_State *L) 
{
    u32 a = luaL_checkinteger(L, 1);
    u32 b = luaL_checkinteger(L, 2);

    u32 res = a & b;
    lua_pushinteger(L, res);
    return 1;
}

int l_Bits_bor(lua_State *L) 
{
    u32 a = luaL_checkinteger(L, 1);
    u32 b = luaL_checkinteger(L, 2);

    u32 res = a | b;
    lua_pushinteger(L, res);
    return 1;
}

static const luaL_Reg bits_functions[] =
{
    {"lshift", l_Bits_lshift},
    {"rshift", l_Bits_rshift},
    {"band", l_Bits_band},
    {"bor", l_Bits_bor},
    {NULL, NULL}
};

int luaopen_Bits(lua_State *L)
{
    lua_newtable(L);
    luaL_register(L, NULL, bits_functions);
    lua_setglobal(L, "Bits");
    return 0;
}