#pragma once
#include <string>
#include <vector>
#include <memory>

struct ASTNode {
    virtual ~ASTNode() = default;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

struct Program : ASTNode {
    std::vector<ASTNodePtr> functions;
};

struct FunctionDef : ASTNode {
    std::string retType;
    std::string name;
    std::vector<std::pair<std::string, std::string>> params; // (type, name)
    ASTNodePtr body;
    FunctionDef() = default;
    FunctionDef(const std::string& retType,
                const std::string& name,
                const std::vector<std::pair<std::string, std::string>>& params,
                ASTNodePtr body)
        : retType(retType), name(name), params(params), body(body) {}
};

struct Block : ASTNode {
    std::vector<ASTNodePtr> stmts;
    Block(const std::vector<ASTNodePtr>& stmts) : stmts(stmts) {}
};

struct VarDecl : ASTNode {
    std::string type;
    std::string name;
    ASTNodePtr initExpr;
    VarDecl(const std::string& type, const std::string& name, ASTNodePtr initExpr)
        : type(type), name(name), initExpr(initExpr) {}
};

struct Assign : ASTNode {
    std::string name;
    ASTNodePtr expr;
    Assign(const std::string& name, ASTNodePtr expr) : name(name), expr(expr) {}
};

struct IfStmt : ASTNode {
    ASTNodePtr cond;
    ASTNodePtr thenStmt;
    ASTNodePtr elseStmt;
    IfStmt(ASTNodePtr cond, ASTNodePtr thenStmt, ASTNodePtr elseStmt)
        : cond(cond), thenStmt(thenStmt), elseStmt(elseStmt) {}
};

struct WhileStmt : ASTNode {
    ASTNodePtr cond;
    ASTNodePtr body;
    WhileStmt(ASTNodePtr cond, ASTNodePtr body) : cond(cond), body(body) {}
};

struct BreakStmt : ASTNode {};
struct ContinueStmt : ASTNode {};

struct ReturnStmt : ASTNode {
    ASTNodePtr expr; // 可为nullptr，表示return;
    ReturnStmt(ASTNodePtr expr = nullptr) : expr(expr) {}
};

struct ExprStmt : ASTNode {
    ASTNodePtr expr;
    ExprStmt(ASTNodePtr expr) : expr(expr) {}
};

struct BinaryExpr : ASTNode {
    std::string op;
    ASTNodePtr lhs, rhs;
    BinaryExpr(const std::string& op, ASTNodePtr lhs, ASTNodePtr rhs)
        : op(op), lhs(lhs), rhs(rhs) {}
};

struct UnaryExpr : ASTNode {
    std::string op;
    ASTNodePtr expr;
    UnaryExpr(const std::string& op, ASTNodePtr expr) : op(op), expr(expr) {}
};

struct IntLiteral : ASTNode {
    int value;
    IntLiteral(int value) : value(value) {}
};

struct VarRef : ASTNode {
    std::string name;
    VarRef(const std::string& name) : name(name) {}
};

struct FuncCall : ASTNode {
    std::string name;
    std::vector<ASTNodePtr> args;
    FuncCall(const std::string& name, const std::vector<ASTNodePtr>& args)
        : name(name), args(args) {}
};

// 可扩展更多AST节点类型 