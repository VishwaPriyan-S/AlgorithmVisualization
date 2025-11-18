#include "interpreter.h"
#include <stdexcept>

Interpreter::Interpreter(const std::vector<Function>& fns)
{
    // Store pointers to the functions, do NOT copy them
    for (const Function& fn : fns)
    {
        funcs[fn.name] = &fn;
    }
}

void Interpreter::setCallback(EventCallback cb_)
{
    cb = cb_;
}

long long Interpreter::evalExpr(Expr* e, std::unordered_map<std::string,long long>& env)
{
    if (auto ne = dynamic_cast<NumberExpr*>(e))
        return ne->value;

    if (auto ve = dynamic_cast<VarExpr*>(e))
    {
        if (env.count(ve->name))
            return env[ve->name];
        return 0;
    }

    if (auto be = dynamic_cast<BinaryExpr*>(e))
    {
        long long L = evalExpr(be->lhs.get(), env);
        long long R = evalExpr(be->rhs.get(), env);

        switch (be->op)
        {
        case '+': return L + R;
        case '-': return L - R;
        case '*': return L * R;
        case '/': return (R == 0 ? 0 : L / R);
        }
    }

    if (auto ce = dynamic_cast<CallExpr*>(e))
    {
        std::vector<long long> args;
        for (auto &a : ce->args)
            args.push_back(evalExpr(a.get(), env));

        fire(VisEvent{VisEvent::Call, QString::fromStdString(ce->func), -1, -1, 0});

        auto res = callFunction(ce->func, args);

        if (std::holds_alternative<long long>(res))
        {
            long long v = std::get<long long>(res);
            fire(VisEvent{VisEvent::Return, QString::fromStdString(ce->func), -1, -1, v});
            return v;
        }

        return 0;
    }

    return 0;
}

std::variant<long long, std::monostate>
Interpreter::execBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                       std::unordered_map<std::string,long long>& env)
{
    for (auto &s : stmts)
    {
        if (auto rs = dynamic_cast<ReturnStmt*>(s.get()))
        {
            long long v = evalExpr(rs->expr.get(), env);
            return v;
        }
        else if (auto is = dynamic_cast<IfStmt*>(s.get()))
        {
            long long cond = evalExpr(is->cond.get(), env);
            fire(VisEvent{VisEvent::Compare, QString("if"), -1, -1, cond});

            if (cond)
            {
                auto res = execBlock(is->thenStmts, env);
                if (std::holds_alternative<long long>(res))
                    return res;
            }
            else
            {
                auto res = execBlock(is->elseStmts, env);
                if (std::holds_alternative<long long>(res))
                    return res;
            }
        }
        else if (auto es = dynamic_cast<ExprStmt*>(s.get()))
        {
            // Expression or call
            evalExpr(es->expr.get(), env);
        }
    }

    return std::monostate{};
}

std::variant<long long, std::monostate>
Interpreter::callFunction(const std::string &name, const std::vector<long long> &args)
{
    if (!funcs.count(name))
        throw std::runtime_error("Function not found: " + name);

    // FIX: get pointer, not reference
    const Function* f = funcs[name];

    std::unordered_map<std::string,long long> env;

    for (size_t i = 0; i < f->params.size() && i < args.size(); ++i)
        env[f->params[i]] = args[i];

    fire(VisEvent{VisEvent::Call, QString::fromStdString(name), -1, -1, 0});
    auto res = execBlock(f->body, env);

    if (std::holds_alternative<long long>(res))
    {
        long long val = std::get<long long>(res);
        fire(VisEvent{VisEvent::Return, QString::fromStdString(name), -1, -1, val});
        return val;
    }

    fire(VisEvent{VisEvent::Return, QString::fromStdString(name), -1, -1, 0});
    return 0;
}
