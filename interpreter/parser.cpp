#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens): toks(tokens), pos(0) {}
const Token& Parser::peek(){ return toks[pos]; }
const Token& Parser::consume(){ return toks[pos++]; }
bool Parser::accept(TokenType t){ if(peek().type==t){ consume(); return true;} return false; }
bool Parser::expect(TokenType t){ if(peek().type==t){ consume(); return true;} throw std::runtime_error("Unexpected token"); }

std::vector<Function> Parser::parse(){
    std::vector<Function> fns;
    while(peek().type != TokenType::EndOfFile){
        fns.push_back(parseFunction());
    }
    return fns;
}

Function Parser::parseFunction(){
    // very small: expect "int name ( params ) { ... }"
    // accept return type ident (ignored)
    if(peek().type==TokenType::Identifier) consume(); // return type or type like int
    if(peek().type!=TokenType::Identifier) throw std::runtime_error("Expected function name");
    Function fn;
    fn.name = consume().text;
    expect(TokenType::LParen);
    // params: identifier (comma id)*
    while(!accept(TokenType::RParen)){
        if(peek().type==TokenType::Identifier){
            fn.params.push_back(consume().text);
        }
        accept(TokenType::Comma);
    }
    expect(TokenType::LBrace);
    // parse statements until '}'
    while(!accept(TokenType::RBrace)){
        fn.body.push_back(parseStatement());
    }
    return fn;
}

std::unique_ptr<Stmt> Parser::parseStatement(){
    if(accept(TokenType::If)){
        expect(TokenType::LParen);
        auto cond = parseExpression();
        expect(TokenType::RParen);
        expect(TokenType::LBrace);
        auto thenBlk = std::make_unique<BlockStmt>();
        while(!accept(TokenType::RBrace)){
            thenBlk->stmts.push_back(parseStatement());
        }
        auto ifs = std::make_unique<IfStmt>();
        ifs->cond = std::move(cond);
        ifs->thenStmts = std::move(thenBlk->stmts);
        // else?
        if(accept(TokenType::Else)){
            expect(TokenType::LBrace);
            while(!accept(TokenType::RBrace)){
                ifs->elseStmts.push_back(parseStatement());
            }
        }
        return ifs;
    } else if(accept(TokenType::Return)){
        auto r = std::make_unique<ReturnStmt>();
        r->expr = parseExpression();
        expect(TokenType::Semicolon);
        return r;
    } else {
        // expression or assignment
        auto e = parseExpression();
        expect(TokenType::Semicolon);
        auto exs = std::make_unique<ExprStmt>();
        exs->expr = std::move(e);
        return exs;
    }
}

std::unique_ptr<Expr> Parser::parseExpression(){
    // only support: numbers, vars, binary (left assoc) and calls: id(args)
    if(peek().type==TokenType::Number){
        int v = consume().numberValue;
        return std::make_unique<NumberExpr>(v);
    }
    if(peek().type==TokenType::Identifier){
        std::string id = consume().text;
        if(accept(TokenType::LParen)){
            auto call = std::make_unique<CallExpr>();
            call->func = id;
            while(!accept(TokenType::RParen)){
                call->args.push_back(parseExpression());
                accept(TokenType::Comma);
            }
            return call;
        } else {
            return std::make_unique<VarExpr>(id);
        }
    }
    // parenthesized or unary not required for minimal
    throw std::runtime_error("Expression parse error");
}
