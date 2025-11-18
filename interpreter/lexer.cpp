#include "lexer.h"
#include <cctype>
#include <unordered_map>

Lexer::Lexer(const std::string &s): src(s), i(0) {}
char Lexer::peek(){ return i < src.size() ? src[i] : '\0'; }
char Lexer::get(){ return i < src.size() ? src[i++] : '\0'; }
void Lexer::skipWhitespaceAndComments(){
    for(;;){
        while(std::isspace(peek())) get();
        if(peek()=='/' && i+1 < src.size() && src[i+1]=='/'){
            while(peek() && peek()!='\n') get();
        } else break;
    }
}
Token Lexer::number(){
    Token t; t.type = TokenType::Number;
    std::string s;
    while(std::isdigit(peek())) s.push_back(get());
    t.text = s;
    t.numberValue = std::stoi(s);
    return t;
}
Token Lexer::identifierOrKeyword(){
    std::string s;
    while(std::isalnum(peek()) || peek()=='_') s.push_back(get());
    static std::unordered_map<std::string, TokenType> kw{
        {"if", TokenType::If}, {"else", TokenType::Else},
        {"for", TokenType::For}, {"while", TokenType::While},
        {"return", TokenType::Return}
    };
    Token t;
    t.text = s;
    auto it = kw.find(s);
    t.type = (it==kw.end()) ? TokenType::Identifier : it->second;
    return t;
}

std::vector<Token> Lexer::tokenize(){
    std::vector<Token> out;
    while(true){
        skipWhitespaceAndComments();
        char c = peek();
        if(c=='\0'){ out.push_back({TokenType::EndOfFile,""}); break; }
        if(std::isdigit(c)){ out.push_back(number()); continue; }
        if(std::isalpha(c) || c=='_'){ out.push_back(identifierOrKeyword()); continue; }
        // symbols
        switch(get()){
        case '(' : out.push_back({TokenType::LParen,"("}); break;
        case ')' : out.push_back({TokenType::RParen,")"}); break;
        case '{' : out.push_back({TokenType::LBrace,"{"}); break;
        case '}' : out.push_back({TokenType::RBrace,"}"}); break;
        case ';' : out.push_back({TokenType::Semicolon,";"}); break;
        case ',' : out.push_back({TokenType::Comma,","}); break;
        case '+' : out.push_back({TokenType::Plus,"+"}); break;
        case '-' : out.push_back({TokenType::Minus,"-"}); break;
        case '*' : out.push_back({TokenType::Star,"*"}); break;
        case '/' : out.push_back({TokenType::Slash,"/"}); break;
        case '%' : out.push_back({TokenType::Percent,"%"}); break;
        case '=' :
            if(peek()=='='){ get(); out.push_back({TokenType::EqEq,"=="}); }
            else out.push_back({TokenType::Assign,"="});
            break;
        case '!' :
            if(peek()=='='){ get(); out.push_back({TokenType::NotEq,"!="}); }
            else out.push_back({TokenType::Unknown,"!"});
            break;
        case '<' :
            if(peek()=='='){ get(); out.push_back({TokenType::LessEq,"<="}); }
            else out.push_back({TokenType::Less,"<"});
            break;
        case '>' :
            if(peek()=='='){ get(); out.push_back({TokenType::GreaterEq,">="}); }
            else out.push_back({TokenType::Greater,">"});
            break;
        default:
            out.push_back({TokenType::Unknown, std::string(1, peek())});
            break;
        }
    }
    return out;
}
