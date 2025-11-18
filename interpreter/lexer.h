#pragma once
#include <string>
#include <vector>

enum class TokenType {
    Identifier, Number,
    LParen, RParen, LBrace, RBrace, Semicolon, Comma,
    Plus, Minus, Star, Slash, Percent,
    Eq, EqEq, NotEq, Less, LessEq, Greater, GreaterEq,
    Assign,
    If, Else, For, While, Return,
    EndOfFile, Unknown
};

struct Token {
    TokenType type;
    std::string text;
    int numberValue = 0;
};

class Lexer {
public:
    Lexer(const std::string &src);
    std::vector<Token> tokenize();
private:
    const std::string src;
    size_t i=0;
    char peek();
    char get();
    void skipWhitespaceAndComments();
    Token number();
    Token identifierOrKeyword();
};
