#pragma once
#include <string>
#include <vector>
#include <memory>

enum class TokenType {
    // 关键字
    INT, VOID, IF, ELSE, WHILE, RETURN, BREAK, CONTINUE,
    
    // 标识符和字面量
    IDENTIFIER, INT_LITERAL,
    
    // 运算符
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    ASSIGN, EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    AND, OR, NOT,
    
    // 分隔符
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, COMMA,
    
    // 特殊
    END_OF_FILE, ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType type, const std::string& value, int line, int column)
        : type(type), value(value), line(line), column(column) {}
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    Token nextToken();
    Token peekToken();
    void reset();
    
private:
    std::string source;
    size_t position;
    size_t line;
    size_t column;
    
    char currentChar();
    char peekChar();
    void advance();
    void skipWhitespace();
    void skipComment();
    
    Token readNumber();
    Token readIdentifier();
    Token readOperator();
    
    bool isKeyword(const std::string& word);
    TokenType getKeywordType(const std::string& word);
}; 