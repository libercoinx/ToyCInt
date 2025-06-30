#pragma once
#include "ast.h"
#include <string>
#include <unordered_map>

class SemanticAnalyzer {
public:
    void analyze(const ASTNodePtr& root);
private:
    std::unordered_map<std::string, std::string> symbolTable;
    void visit(const ASTNodePtr& node);
}; 