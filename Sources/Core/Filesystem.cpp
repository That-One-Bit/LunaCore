#include "Core/Filesystem.hpp"

#include <CTRPluginFramework.hpp>
#include <FsLib/fslib.hpp>

#include <string>
#include <cstring>
#include <codecvt>
#include <locale>

#include "string_hash.hpp"

namespace CTRPF = CTRPluginFramework;

typedef struct {
    fslib::File *filePtr;
    int mode;
    size_t size;
} FilesystemFile;

bool startsWith(const std::string &str, const std::string &prefix) {
    return str.compare(0, prefix.size(), prefix) == 0;
}

// ----------------------------------------------------------------------------

//$Core.Filesystem

// ----------------------------------------------------------------------------

//@@FilesystemFile

/*
- Opens a file. Returns nil if the file wasn't opened with an error message. Use sdmc:/ for sd card or extdata:/ for game extdata
## fp: string
## mode: string
## return: FilesystemFile?
## return: string?
### Core.Filesystem.open
*/
static int l_Filesystem_open(lua_State *L) {
    std::string filepath(luaL_checkstring(L, 1));
    std::string filemode(luaL_checkstring(L, 2));

    bool success = false;

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string utf16_path = converter.from_bytes(filepath);
    FilesystemFile* fileStruct = (FilesystemFile*)lua_newuserdata(L, sizeof(FilesystemFile));
    luaC_setmetatable(L, "FilesystemFile");
    if (filemode == "w")
        fileStruct->mode = FS_OPEN_WRITE|FS_OPEN_CREATE;
    else if (filemode == "r")
        fileStruct->mode = FS_OPEN_READ;
    else if (filemode == "a")
        fileStruct->mode = FS_OPEN_APPEND|FS_OPEN_CREATE;
    else if (filemode == "rw" || filemode == "wr" || filemode == "r+" || filemode == "w+")
        fileStruct->mode = FS_OPEN_WRITE|FS_OPEN_READ|FS_OPEN_CREATE;
    else {
        lua_pop(L, 1);
        return luaL_error(L, "Invalid mode");
    }
    fileStruct->filePtr = new fslib::File(utf16_path.c_str(), fileStruct->mode);
    
    if (!fileStruct->filePtr->isOpen()) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushstring(L, fslib::getErrorString());
    } else {
        fileStruct->size = fileStruct->filePtr->getSize();
        success = true;
    }

    if (success)
        return 1;
    else
        return 2;
}

// ----------------------------------------------------------------------------

/*
- Reads the specified amount of bytes to read, or use "*all" to read all file and returns the data in a string or nil if error
## bytes: any
## return: string?
### FilesystemFile:read
*/
static int l_Filesystem_File_read(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    size_t bytes = 0;
    if (lua_type(L, 2) == LUA_TSTRING) {
        std::string readAmount(lua_tostring(L, 2));
        if (readAmount == "*all" || readAmount == "*a") {
            bytes = std::string::npos;
        } else
            return luaL_error(L, "Invalid number of bytes to read. Use a number or '*all' to read whole file");
    } else
        bytes = luaL_checkinteger(L, 2);

    if ((fileStruct->mode & FS_OPEN_READ) == 0)
        return luaL_error(L, "File not opened with read mode");

    if (!fileStruct->filePtr->isOpen())
        return luaL_error(L, "File closed");

    if (bytes == std::string::npos) {
        bytes = fileStruct->size;
        fileStruct->filePtr->seek(0, SEEK_SET);
    }
    char *fileContent = new char[bytes];
    size_t bytesRead = fileStruct->filePtr->read(fileContent, bytes);
    if (bytesRead == -1)    
        lua_pushnil(L);
    else
        lua_pushlstring(L, fileContent, bytesRead);
        
    delete fileContent;
    return 1;
}

/*
- Writes all data to file of the specified amount of bytes if provided. Returns true is success, false otherwise
## data: string
## bytes: integer?
## return: boolean
### FilesystemFile:write
*/
static int l_Filesystem_File_write(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    size_t bytes = 0;
    const char* data = luaL_checklstring(L, 2, &bytes);
    if (lua_gettop(L) > 2)
        bytes = luaL_checkinteger(L, 3);

    if ((fileStruct->mode & (FS_OPEN_WRITE|FS_OPEN_APPEND)) == 0)
        return luaL_error(L, "File not opened with write mode");

    if (!fileStruct->filePtr->isOpen())
        return luaL_error(L, "File closed");
    
    lua_pushboolean(L, fileStruct->filePtr->write(data, bytes) != -1);

    return 1;
}

/*
- Returns the actual position in the file
## return: integer
### FilesystemFile:tell
*/
static int l_Filesystem_File_tell(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    lua_pushinteger(L, fileStruct->filePtr->tell());
    return 1;
}

/*
- Flushes all file data in write buffer
## return: boolean
### FilesystemFile:flush
*/
static int l_Filesystem_File_flush(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    lua_pushboolean(L, fileStruct->filePtr->flush());
    return 1;
}

/*
- Sets the position in file and returns the new position or nil if error
## offset: integer
## whence: string?
## return: integer
### FilesystemFile:seek
*/
static int l_Filesystem_File_seek(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    std::string whence = "cur";
    size_t offset = 0;
    if (lua_gettop(L) > 1)
        whence = luaL_checkstring(L, 2);
    if (lua_gettop(L) > 2)
        offset = luaL_checkinteger(L, 3);
    
    int seekPos;
    if (whence == "cur")
        seekPos = SEEK_CUR;
    else if (whence == "set")
        seekPos = SEEK_END;
    else if (whence == "end")
        seekPos = SEEK_END;
    else
        return luaL_error(L, "Invalid seek pos");

    fileStruct->filePtr->seek(offset, seekPos);
    lua_pushinteger(L, fileStruct->filePtr->tell());
    return 1;
}

/*
- Closes the file
### FilesystemFile:close
*/
static int l_Filesystem_File_close(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    if (fileStruct->filePtr->isOpen())
        fileStruct->filePtr->close();
    return 0;
}

static int l_Filesystem_File_gc(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    if (fileStruct->filePtr->isOpen())
        fileStruct->filePtr->close();
    return 0;
}

static const luaL_Reg filesystem_file_methods[] = {
    {"read", l_Filesystem_File_read},
    {"write", l_Filesystem_File_write},
    {"tell", l_Filesystem_File_tell},
    {"flush", l_Filesystem_File_flush},
    {"seek", l_Filesystem_File_seek},
    {"close", l_Filesystem_File_close},
    {NULL, NULL}
};

static inline void RegisterFilesystemMetatables(lua_State *L) {
    luaL_newmetatable(L, "FilesystemFile");
    lua_newtable(L);
    luaL_register(L, NULL, filesystem_file_methods);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, luaC_invalid_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, l_Filesystem_File_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static const luaL_Reg filesystem_functions[] = {
    {"open", l_Filesystem_open},
    {NULL, NULL}
};

bool Core::Module::RegisterFilesystemModule(lua_State *L) {
    RegisterFilesystemMetatables(L);

    lua_getglobal(L, "Core");
    luaC_register_field(L, filesystem_functions, "Filesystem");
    lua_pop(L, 1);
    return true;
}