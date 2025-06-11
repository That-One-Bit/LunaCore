#include "Core/Utils/Bits.hpp"

#include <stdint.h>

typedef uint32_t u32;

// ----------------------------------------------------------------------------

//$Bits

// ----------------------------------------------------------------------------

/*
## a: integer
## b: integer
## return: integer
### Bits.lshift
*/
static int l_Bits_lshift(lua_State *L) {
	u32 a = luaL_checkinteger(L, 1);
	u32 b = luaL_checkinteger(L, 2);
	lua_pushinteger(L, a << b);
	return 1;
}

/*
## a: integer
## b: integer
## return: integer
### Bits.rshift
*/
static int l_Bits_rshift(lua_State *L) {
	u32 a = luaL_checkinteger(L, 1);
	u32 b = luaL_checkinteger(L, 2);
	lua_pushinteger(L, a >> b);
	return 1;
}

/*
## a: integer
## b: integer
## return: integer
### Bits.band
*/
static int l_Bits_band(lua_State *L) {
	u32 a = luaL_checkinteger(L, 1);
	u32 b = luaL_checkinteger(L, 2);
	lua_pushinteger(L, a&b);
	return 1;
}

/*
## a: integer
## b: integer
## return: integer
### Bits.bor
*/
static int l_Bits_bor(lua_State *L) {
	u32 a = luaL_checkinteger(L, 1);
	u32 b = luaL_checkinteger(L, 2);
	lua_pushinteger(L, a|b);
	return 1;
}

/*
## a: integer
## b: integer
## return: integer
### Bits.bxor
*/
static int l_Bits_bxor(lua_State *L) {
	u32 a = luaL_checkinteger(L, 1);
	u32 b = luaL_checkinteger(L, 2);
	lua_pushinteger(L, a^b);
	return 1;
}

/*
## a: integer
## return: integer
### Bits.bnot
*/
static int l_Bits_bnot(lua_State *L) {
	u32 a = luaL_checkinteger(L, 1);
	lua_pushinteger(L, ~a);
	return 1;
}

static const luaL_Reg bits_functions[] =
{
    {"lshift", l_Bits_lshift},
    {"rshift", l_Bits_rshift},
    {"band", l_Bits_band},
    {"bor", l_Bits_bor},
	{"bxor", l_Bits_bxor},
	{"bnot", l_Bits_bnot},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

bool Core::RegisterBitsModule(lua_State *L)
{
	luaC_register_global(L, bits_functions, "Bits");
    return true;
}