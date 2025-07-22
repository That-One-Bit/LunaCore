#include "CoreGlobals.hpp"

lua_State *Lua_global = NULL;

CustomMutex Lua_Global_Mut;

bool patchEnabled = false;

std::unordered_map<std::string, std::string> modPaths;