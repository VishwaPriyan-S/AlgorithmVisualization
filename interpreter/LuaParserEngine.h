#ifndef LUAPARSERENGINE_H
#define LUAPARSERENGINE_H

#include <string>
#include <lua.hpp>

class LuaParserEngine {
public:
    LuaParserEngine();
    ~LuaParserEngine();

    // Parses Lua source code → JSON (AST)
    std::string parse(const std::string &code);

private:
    lua_State* L;

    void loadLuaParser();
    void loadJsonLibrary();
};

#endif
