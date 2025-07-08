#pragma once

#include <string>
#include <unordered_map>
#include "types.h"

#include "lua_common.h"

extern lua_State *Lua_global;

extern std::unordered_map<std::string, std::string> modPaths;