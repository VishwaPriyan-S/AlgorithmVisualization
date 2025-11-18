#pragma once
#include "lexer.h"
#include "ast.h"
#include <vector>
#include <string>

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<Function> parse(); // top-level parse
private:
    const std::vector<Token>& toks;
    size_t pos=0;
    const Token& peek();
    const Token& consume();
    bool accept(TokenType t);
    bool expect(TokenType t);
    Function parseFunction();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Expr> parseExpression();
    // ... small helpers
};
