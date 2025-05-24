#pragma once

#include "lua_common.h"

void ScriptingAsyncHandlerCallback();

int luaopen_Async(lua_State *L);