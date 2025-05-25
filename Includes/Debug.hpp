#pragma once

#include <string>

#include "lua_common.h"

bool DebugOpenLogFile(const char *filepath);

void DebugCloseLogFile();

void DebugLogMessage(const std::string& msg, bool showOnScreen);

void DebugLogError(const std::string& msg);

void DebugError(const std::string& msg);

int l_register_Debug(lua_State *L);