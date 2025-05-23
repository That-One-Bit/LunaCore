#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <cmath>

#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)

#define luaC_register_global(L, reg, globalName) ({\
        lua_newtable(L);\
        luaL_register(L, NULL, reg);\
        lua_setglobal(L, (globalName));\
    })

#define luaC_register_field(L, reg, fieldName) ({\
        lua_newtable(L);\
        luaL_register(L, NULL, reg);\
        lua_setfield(L, -2, (fieldName));\
    })

#define luaC_setfield_integer(L, num, field) ({\
        lua_pushinteger(L, (num));\
        lua_setfield(L, -2, (field));\
    })

#define luaC_setmetatable(L, name) ({\
        luaL_getmetatable(L, (name));\
        lua_setmetatable(L, -2);\
    })

static inline int luaC_invalid_newindex(lua_State *L) {
    return luaL_error(L, "Attempt to modify read-only table");
}

#ifdef __cplusplus
}
#endif