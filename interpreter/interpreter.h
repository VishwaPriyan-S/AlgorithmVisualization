#pragma once
#include "ast.h"
#include "vis_event.h"
#include <functional>
#include <unordered_map>
#include <variant>

class Interpreter {
public:
    using EventCallback = std::function<void(const VisEvent&)>;

    // Accept functions by reference (NO COPY)
    Interpreter(const std::vector<Function>& fns);

    void setCallback(EventCallback cb);

    std::variant<long long, std::monostate>
    callFunction(const std::string &name, const std::vector<long long> &args);

private:

    // Store POINTERS to functions (NO COPY)
    std::unordered_map<std::string, const Function*> funcs;

    EventCallback cb;

    void fire(const VisEvent &e){ if(cb) cb(e); }

    long long evalExpr(Expr* e,
                       std::unordered_map<std::string,long long>& env);

    std::variant<long long, std::monostate>
    execBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
              std::unordered_map<std::string,long long>& env);
};
