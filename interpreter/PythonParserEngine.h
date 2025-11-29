#ifndef PYTHONPARSERENGINE_H
#define PYTHONPARSERENGINE_H

#include <string>

class PythonParserEngine {
public:
    // pythonExecutable: e.g. "python" or "python3"
    explicit PythonParserEngine(const std::string &pythonExecutable = "python");

    // Takes Python source code and returns AST JSON (or error JSON)
    std::string parseToAstJson(const std::string &code);

private:
    std::string m_pythonExe;
};

#endif // PYTHONPARSERENGINE_H
