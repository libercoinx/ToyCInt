//C://ToyC_Int//User//Antelope693//3112989263@qq.com
#include "semantic.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <memory>

class SemanticContext {
public:
    std::unordered_map<std::string, std::string> funcRetType; // 函数名->返回类型
    std::unordered_map<std::string, int> funcDeclaredOrder; // 函数名->声明顺序
    std::vector<std::unordered_map<std::string, bool>> varScopes; // 变量作用域栈
    std::unordered_set<std::string> funcNames;
    int funcOrder = 0;
    bool inLoop = false;
    std::string curFunc;
    std::string curFuncRetType;
    bool hasReturn = false;
    void enterScope() { varScopes.push_back({}); }
    void leaveScope() { varScopes.pop_back(); }
    void declareVar(const std::string& name) { varScopes.back()[name] = true; }
    bool isVarDeclared(const std::string& name) {
        for (int i = varScopes.size() - 1; i >= 0; --i) {
            if (varScopes[i].count(name)) return true;
        }
        return false;
    }
};

void checkFuncDef(const std::shared_ptr<FunctionDef>& func, SemanticContext& ctx, int order);
void checkStmt(const ASTNodePtr& node, SemanticContext& ctx);
void checkExpr(const ASTNodePtr& node, SemanticContext& ctx, bool allowVoidCall = false);
bool checkAllPathsReturn(const ASTNodePtr& node);

void SemanticAnalyzer::analyze(const ASTNodePtr& root) {
    auto prog = std::dynamic_pointer_cast<Program>(root);
    if (!prog) error("AST根节点不是Program");
    SemanticContext ctx;
    // 1. 函数名唯一、不能嵌套、不能作为值，main唯一、参数为空、返回int
    int mainCount = 0;
    int order = 0;
    for (auto& f : prog->functions) {
        auto func = std::dynamic_pointer_cast<FunctionDef>(f);
        if (!func) continue;
        if (ctx.funcNames.count(func->name)) error("函数名重复: " + func->name);
        ctx.funcNames.insert(func->name);
        ctx.funcRetType[func->name] = func->retType;
        ctx.funcDeclaredOrder[func->name] = order++;
        if (func->name == "main") {
            ++mainCount;
            if (func->retType != "int") error("main函数必须返回int");
            if (!func->params.empty()) error("main函数参数必须为空");
        }
    }
    if (mainCount != 1) error("必须有且只有一个main函数");
    // 2. 检查所有函数体
    order = 0;
    for (auto& f : prog->functions) {
        auto func = std::dynamic_pointer_cast<FunctionDef>(f);
        ctx.curFunc = func->name;
        ctx.curFuncRetType = func->retType;
        ctx.hasReturn = false;
        ctx.enterScope();
        // 形参声明
        for (auto& p : func->params) {
            if (ctx.isVarDeclared(p.second)) error("形参名与局部变量冲突: " + p.second);
            ctx.declareVar(p.second);
        }
        checkStmt(func->body, ctx);
        ctx.leaveScope();
        // int函数所有路径必须return int
        if (func->retType == "int" && !checkAllPathsReturn(func->body)) error("int函数所有路径必须return int: " + func->name);
        // void函数不能return带值
        // 已在checkStmt中处理
        ++order;
    }
}

void checkStmt(const ASTNodePtr& node, SemanticContext& ctx) {
    if (!node) return;
    if (auto block = std::dynamic_pointer_cast<Block>(node)) {
        ctx.enterScope();
        for (auto& stmt : block->stmts) checkStmt(stmt, ctx);
        ctx.leaveScope();
    } else if (auto decl = std::dynamic_pointer_cast<VarDecl>(node)) {
        if (ctx.varScopes.back().count(decl->name)) error("变量重复声明: " + decl->name);
        checkExpr(decl->initExpr, ctx);
        ctx.declareVar(decl->name);
    } else if (auto assign = std::dynamic_pointer_cast<Assign>(node)) {
        if (!ctx.isVarDeclared(assign->name)) error("变量未声明: " + assign->name);
        checkExpr(assign->expr, ctx);
    } else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(node)) {
        ctx.hasReturn = true;
        if (ctx.curFuncRetType == "void" && ret->expr) error("void函数不能return带值: " + ctx.curFunc);
        if (ctx.curFuncRetType == "int" && !ret->expr) error("int函数return必须带值: " + ctx.curFunc);
        if (ret->expr) checkExpr(ret->expr, ctx);
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExprStmt>(node)) {
        checkExpr(exprStmt->expr, ctx, true);
    } else if (auto ifs = std::dynamic_pointer_cast<IfStmt>(node)) {
        checkExpr(ifs->cond, ctx);
        checkStmt(ifs->thenStmt, ctx);
        if (ifs->elseStmt) checkStmt(ifs->elseStmt, ctx);
    } else if (auto wh = std::dynamic_pointer_cast<WhileStmt>(node)) {
        checkExpr(wh->cond, ctx);
        bool oldInLoop = ctx.inLoop;
        ctx.inLoop = true;
        checkStmt(wh->body, ctx);
        ctx.inLoop = oldInLoop;
    } else if (std::dynamic_pointer_cast<BreakStmt>(node) || std::dynamic_pointer_cast<ContinueStmt>(node)) {
        if (!ctx.inLoop) error("break/continue只能出现在循环中");
    }
}

void checkExpr(const ASTNodePtr& node, SemanticContext& ctx, bool allowVoidCall) {
    if (!node) return;
    if (auto lit = std::dynamic_pointer_cast<IntLiteral>(node)) {
        // ok
    } else if (auto var = std::dynamic_pointer_cast<VarRef>(node)) {
        if (!ctx.isVarDeclared(var->name)) error("变量未声明: " + var->name);
    } else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(node)) {
        checkExpr(bin->lhs, ctx);
        checkExpr(bin->rhs, ctx);
        if (bin->op == "/" || bin->op == "%") {
            if (auto rhs = std::dynamic_pointer_cast<IntLiteral>(bin->rhs)) {
                if (rhs->value == 0) error("除数不能为零");
            }
        }
    } else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(node)) {
        checkExpr(un->expr, ctx);
    } else if (auto call = std::dynamic_pointer_cast<FuncCall>(node)) {
        if (!ctx.funcRetType.count(call->name)) error("函数未声明: " + call->name);
        if (ctx.funcDeclaredOrder[call->name] > ctx.funcDeclaredOrder[ctx.curFunc]) error("函数调用必须在声明后: " + call->name);
        if (ctx.funcRetType[call->name] == "void" && !allowVoidCall) error("void函数调用不能作为条件或右值: " + call->name);
        for (auto& arg : call->args) checkExpr(arg, ctx);
    }
}

bool checkAllPathsReturn(const ASTNodePtr& node) {
    if (!node) return false;
    if (std::dynamic_pointer_cast<ReturnStmt>(node)) return true;
    if (auto block = std::dynamic_pointer_cast<Block>(node)) {
        for (auto& stmt : block->stmts) {
            if (checkAllPathsReturn(stmt)) return true;
        }
        return false;
    }
    if (auto ifs = std::dynamic_pointer_cast<IfStmt>(node)) {
        if (!ifs->elseStmt) return false;
        return checkAllPathsReturn(ifs->thenStmt) && checkAllPathsReturn(ifs->elseStmt);
    }
    if (auto wh = std::dynamic_pointer_cast<WhileStmt>(node)) {
        // conservatively assume while may not execute
        return false;
    }
    return false;
}

void SemanticAnalyzer::visit(const ASTNodePtr& node) {
    // 简单演示，实际应递归遍历AST并检查
} 