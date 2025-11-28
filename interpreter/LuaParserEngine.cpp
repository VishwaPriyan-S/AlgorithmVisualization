#include "LuaParserEngine.h"

LuaParserEngine::LuaParserEngine() {
    L = luaL_newstate();
    luaL_openlibs(L);

    loadLuaParser();
    loadJsonLibrary();
}

LuaParserEngine::~LuaParserEngine() {
    lua_close(L);
}

void LuaParserEngine::loadLuaParser() {
    // Load lua-parser/*.lua files
    if (luaL_dofile(L, "lua-parser/ast.lua") != LUA_OK)
        fprintf(stderr, "AST load error: %s\n", lua_tostring(L, -1));

    if (luaL_dofile(L, "lua-parser/lparser.lua") != LUA_OK)
        fprintf(stderr, "LPARSER load error: %s\n", lua_tostring(L, -1));

    if (luaL_dofile(L, "lua-parser/parser.lua") != LUA_OK)
        fprintf(stderr, "Parser load error: %s\n", lua_tostring(L, -1));
}

void LuaParserEngine::loadJsonLibrary() {
    // Load dkjson.lua
    if (luaL_dofile(L, "json/dkjson.lua") != LUA_OK)
        fprintf(stderr, "JSON load error: %s\n", lua_tostring(L, -1));
}

std::string LuaParserEngine::parse(const std::string &code) {
    lua_getglobal(L, "parser");     // parser table
    lua_getfield(L, -1, "parse");   // parser.parse

    lua_pushlstring(L, code.c_str(), code.size());

    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        std::string err = lua_tostring(L, -1);
        lua_pop(L, 1);
        return "{\"error\":\"" + err + "\"}";
    }

    // Now the AST is at the top of the stack.
    // Convert AST → JSON via dkjson

    lua_getglobal(L, "json");
    lua_getfield(L, -1, "encode"); // json.encode

    lua_pushvalue(L, -3); // push AST again
    lua_newtable(L);
    lua_pushstring(L, "indent");
    lua_pushinteger(L, 2);
    lua_settable(L, -3);

    if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        std::string err = lua_tostring(L, -1);
        lua_pop(L, 1);
        return "{\"error\":\"" + err + "\"}";
    }

    std::string jsonResult = lua_tostring(L, -1);
    lua_pop(L, 1);

    return jsonResult;
}
