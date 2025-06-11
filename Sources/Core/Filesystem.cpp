#include "Core/Filesystem.hpp"

#include <CTRPluginFramework.hpp>

#include <string>

#include "string_hash.hpp"

namespace CTRPF = CTRPluginFramework;

typedef struct {
    CTRPF::File *filePtr;
    int mode;
} FilesystemFile;

static int l_Filesystem_open(lua_State *L) {
    std::string filepath(luaL_checkstring(L, 1));
    std::string filemode(luaL_checkstring(L, 2));

    FilesystemFile* fileStruct = (FilesystemFile*)lua_newuserdata(L, sizeof(FilesystemFile));
    luaC_setmetatable(L, "FilesystemFile");
    fileStruct->filePtr = new CTRPF::File;
    if (filemode == "w")
        fileStruct->mode = CTRPF::File::WRITE|CTRPF::File::CREATE;
    else if (filemode == "r")
        fileStruct->mode = CTRPF::File::READ;
    else if (filemode == "a")
        fileStruct->mode = CTRPF::File::APPEND|CTRPF::File::CREATE;
    else if (filemode == "rw" || filemode == "wr" || filemode == "r+" || filemode == "w+")
        fileStruct->mode = CTRPF::File::RWC;
    else {
        lua_pop(L, 1);
        return luaL_error(L, "Invalid mode");
    }

    CTRPF::File::Open(*fileStruct->filePtr, filepath, fileStruct->mode);

    if (!fileStruct->filePtr->IsOpen()) {
        lua_pop(L, 1);
        lua_pushnil(L);
    }
    return 1;
}

static int l_Filesystem_File_read(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    size_t bytes = 0;
    if (lua_type(L, 2) == LUA_TSTRING) {
        if (std::string(lua_tostring(L, 2)) == "*all") {
            bytes = std::string::npos;
        } else
            return luaL_error(L, "Invalid number of bytes to read. Use a number or '*all' to read whole file");
    } else
        bytes = luaL_checkinteger(L, 2);

    if ((fileStruct->mode & CTRPF::File::READ) == 0)
        return luaL_error(L, "File not opened with read mode");

    if (fileStruct->filePtr->IsOpen()) {
        if (bytes == std::string::npos) {
            bytes = fileStruct->filePtr->GetSize();
            fileStruct->filePtr->Rewind();
        }
        char *fileContent = new char[bytes];
        if (fileStruct->filePtr->Read(fileContent, bytes) != CTRPF::File::SUCCESS) {
            delete fileContent;
            lua_pushnil(L);
        } else {
            lua_pushlstring(L, fileContent, bytes);
            delete fileContent;
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int l_Filesystem_File_write(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    size_t bytes = 0;
    const char* data = luaL_checklstring(L, 2, &bytes);
    if (lua_gettop(L) > 2)
        bytes = luaL_checkinteger(L, 3);

    if ((fileStruct->mode & (CTRPF::File::WRITE|CTRPF::File::APPEND)) == 0)
        return luaL_error(L, "File not opened with write mode");

    if (fileStruct->filePtr->IsOpen())
        lua_pushboolean(L, fileStruct->filePtr->Write(data, bytes) == CTRPF::File::SUCCESS);
    else
        lua_pushboolean(L, false);

    return 1;
}

static int l_Filesystem_File_tell(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    lua_pushinteger(L, fileStruct->filePtr->Tell());
    return 1;
}

static int l_Filesystem_File_flush(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    lua_pushboolean(L, fileStruct->filePtr->Flush() == CTRPF::File::SUCCESS);
    return 1;
}

static int l_Filesystem_File_seek(lua_State *L) {
    FilesystemFile* fileStruct = (FilesystemFile*)luaL_checkudata(L, 1, "FilesystemFile");
    std::string whence = "cur";
    size_t offset = 0;
    if (lua_gettop(L) > 1)
        whence = luaL_checkstring(L, 2);
    if (lua_gettop(L) > 2)
        offset = luaL_checkinteger(L, 3);
    
    CTRPF::File::SeekPos seekPos;
    if (whence == "cur")
        seekPos = CTRPF::File::CUR;
    else if (whence == "set")
        seekPos = CTRPF::File::SET;
    else if (whence == "end")
        seekPos = CTRPF::File::END;
    else
        return luaL_error(L, "Invalid seek pos");

    if (fileStruct->filePtr->Seek(offset, seekPos) != CTRPF::File::SUCCESS)
        lua_pushnil(L);
    else
        lua_pushinteger(L, fileStruct->filePtr->Tell());
    return 1;
}

static const luaL_Reg filesystem_file_methods[] = {
    {"read", l_Filesystem_File_read},
    {"write", l_Filesystem_File_write},
    {"tell", l_Filesystem_File_tell},
    {"flush", l_Filesystem_File_flush},
    {"seek", l_Filesystem_File_seek},
    {NULL, NULL}
};

static inline void RegisterFilesystemMetatables(lua_State *L) {
    luaL_newmetatable(L, "FilesystemFile");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, filesystem_file_methods);
    lua_pushcfunction(L, luaC_invalid_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
}

static const luaL_Reg filesystem_functions[] = {
    {"open", l_Filesystem_open},
    {NULL, NULL}
};

bool Core::RegisterFilesystemModule(lua_State *L) {
    lua_getglobal(L, "Core");
    luaC_register_field(L, filesystem_functions, "Filesystem");
    lua_pop(L, 1);
    return true;
}