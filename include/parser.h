#pragma once
#include "ast.h"
#include "lexer.h"
#include <memory>
#include <vector>

class Parser {
public:
    explicit Parser(const std::string& source);
    ASTNodePtr parse();
    
private:
    Lexer lexer;
    Token currentToken;
    
    void advance();
    bool match(TokenType type);
    bool check(TokenType type);
    void expect(TokenType type, const std::string& message);
    
    // 解析函数
    ASTNodePtr parseProgram();
    ASTNodePtr parseFunctionDef();
    ASTNodePtr parseBlock();
    ASTNodePtr parseStatement();
    ASTNodePtr parseVarDecl();
    ASTNodePtr parseIfStmt();
    ASTNodePtr parseWhileStmt();
    ASTNodePtr parseReturnStmt();
    ASTNodePtr parseBreakStmt();
    ASTNodePtr parseContinueStmt();
    ASTNodePtr parseExprStmt();
    ASTNodePtr parseExpression();
    ASTNodePtr parseAssignment();
    ASTNodePtr parseLogicalOr();
    ASTNodePtr parseLogicalAnd();
    ASTNodePtr parseEquality();
    ASTNodePtr parseComparison();
    ASTNodePtr parseAdditive();
    ASTNodePtr parseMultiplicative();
    ASTNodePtr parseUnary();
    ASTNodePtr parsePrimary();
    ASTNodePtr parseFuncCall();
    
    // 辅助函数
    std::vector<std::pair<std::string, std::string>> parseParams();
    std::vector<ASTNodePtr> parseArgs();
}; 