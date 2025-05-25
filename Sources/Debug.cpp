#include "Debug.hpp"

#include <string>
#include <time.h>
#include <CTRPluginFramework.hpp>

namespace CTRPF = CTRPluginFramework;

CTRPF::File logFile;

bool DebugOpenLogFile(const char *filepath)
{
    if (!CTRPF::File::Exists(filepath))
        CTRPF::File::Create(filepath);
    CTRPF::File::Open(logFile, filepath);
    if (logFile.IsOpen())
        logFile.Clear();
    return logFile.IsOpen();
}

void DebugCloseLogFile()
{
    if (logFile.IsOpen())
        logFile.Close();
}

void DebugWriteLog(const std::string& msg)
{
    if (logFile.IsOpen()) {
        time_t rawTime = time(NULL);
        struct tm *timeInfo = localtime(&rawTime);
        std::string out_msg("[");
        if (timeInfo->tm_hour < 10)
            out_msg += "0";
        out_msg += std::to_string(timeInfo->tm_hour)+":";
        if (timeInfo->tm_min < 10)
            out_msg += "0";
        out_msg += std::to_string(timeInfo->tm_min)+":";
        if (timeInfo->tm_sec < 10)
            out_msg += "0";
        out_msg += std::to_string(timeInfo->tm_sec)+"] "+msg+"\n";
        logFile.Write(out_msg.c_str(), out_msg.size());
        logFile.Flush();
    }
}

void DebugLogMessage(const std::string& msg, bool showOnScreen)
{
    if (showOnScreen)
        CTRPluginFramework::OSD::Notify(msg);
    DebugWriteLog(msg);
}

void DebugLogError(const std::string& msg)
{
    DebugError(msg);
    DebugWriteLog(std::string("[ERROR] ")+msg);
}

void DebugError(const std::string& msg)
{
    CTRPluginFramework::OSD::Notify(msg, CTRPluginFramework::Color::Red, CTRPluginFramework::Color::Black);
}

//!include Sources/Modules.cpp
//$Game.Debug

// ----------------------------------------------------------------------------

/*
- Displays a notification on screen
## msg: string
### Game.Debug.message
*/
int l_Debug_message(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);

    CTRPluginFramework::OSD::Notify(msg);
    return 0;
}

/*
- Appends the message to log file. Optionally shows the message on screen
## msg: string
## showOnScreen: boolean
### Game.Debug.log
*/
int l_Debug_log(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    bool showOnScreen = false;
    if (lua_type(L, 2) == LUA_TBOOLEAN)
        showOnScreen = lua_toboolean(L, 2);
    DebugLogMessage(msg, showOnScreen);
    return 0;
}

/*
- Appends the error message to log file and shows it on screen
## msg: string
### Game.Debug.logerror
*/
int l_Debug_logerror(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    DebugLogError(msg);
    return 0;
}

/*
- Show error on screen
## msg: string
### Game.Debug.error
*/
int l_Debug_error(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    DebugError(msg);
    return 0;
}

static const luaL_Reg debug_functions[] =
{
    {"message", l_Debug_message},
    {"log", l_Debug_log},
    {"logerror", l_Debug_logerror},
    {"error", l_Debug_error},
    {NULL, NULL}
};

// ----------------------------------------------------------------------------

int l_register_Debug(lua_State *L)
{
    lua_getglobal(L, "Game");
    luaC_register_field(L, debug_functions, "Debug");
    lua_pop(L, 1);
    return 0;
}