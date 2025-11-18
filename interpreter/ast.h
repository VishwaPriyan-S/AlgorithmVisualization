#pragma once
#include <string>
#include <vector>
#include <memory>

struct Expr {
    virtual ~Expr() = default;
};
struct NumberExpr : Expr { long long value; NumberExpr(long long v):value(v){} };
struct VarExpr : Expr { std::string name; VarExpr(std::string n):name(std::move(n)){} };
struct BinaryExpr : Expr { char op; std::unique_ptr<Expr> lhs, rhs; BinaryExpr(char o, std::unique_ptr<Expr>a, std::unique_ptr<Expr>b):op(o),lhs(std::move(a)),rhs(std::move(b)){} };
struct CallExpr : Expr { std::string func; std::vector<std::unique_ptr<Expr>> args; };

struct Stmt {
    virtual ~Stmt() = default;
};
struct ExprStmt : Stmt { std::unique_ptr<Expr> expr; };
struct IfStmt : Stmt { std::unique_ptr<Expr> cond; std::vector<std::unique_ptr<Stmt>> thenStmts, elseStmts; };
struct ReturnStmt : Stmt { std::unique_ptr<Expr> expr; };
struct BlockStmt : Stmt { std::vector<std::unique_ptr<Stmt>> stmts; };
struct ForStmt : Stmt { std::string var; std::unique_ptr<Expr> start, end; std::vector<std::unique_ptr<Stmt>> body; };
struct WhileStmt : Stmt { std::unique_ptr<Expr> cond; std::vector<std::unique_ptr<Stmt>> body; };

struct Function {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<Stmt>> body;
};
