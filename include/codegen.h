#pragma once
#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>

// RISC-V寄存器
enum class Register {
    ZERO, RA, SP, GP, TP, T0, T1, T2, S0, S1,
    A0, A1, A2, A3, A4, A5, A6, A7,
    S2, S3, S4, S5, S6, S7, S8, S9, S10, S11,
    T3, T4, T5, T6
};

// 符号表项
struct Symbol {
    std::string name;
    std::string type;
    int offset;  // 相对于栈帧的偏移
    bool isParam;
    
    Symbol(const std::string& name, const std::string& type, int offset, bool isParam = false)
        : name(name), type(type), offset(offset), isParam(isParam) {}
};

// 函数信息
struct FunctionInfo {
    std::string name;
    std::vector<std::string> paramNames;
    std::vector<std::string> paramTypes;
    std::string returnType;
    int localVarCount;
    int stackSize;
    
    FunctionInfo(const std::string& name) : name(name), localVarCount(0), stackSize(0) {}
};

class CodeGenerator {
public:
    CodeGenerator(const std::string& outputFile);
    ~CodeGenerator();
    
    void generate(const ASTNodePtr& node);
    
private:
    std::ofstream output;
    std::unordered_map<std::string, Symbol> symbolTable;
    std::unordered_map<std::string, FunctionInfo> functions;
    FunctionInfo* currentFunction;
    int labelCounter;
    int tempVarCounter;
    
    // 访问者模式实现
    void visit(const ASTNodePtr& node);
    void visitProgram(const Program* node);
    void visitFunctionDef(const FunctionDef* node);
    void visitBlock(const Block* node);
    void visitVarDecl(const VarDecl* node);
    void visitAssign(const Assign* node);
    void visitIfStmt(const IfStmt* node);
    void visitWhileStmt(const WhileStmt* node);
    void visitBreakStmt(const BreakStmt* node);
    void visitContinueStmt(const ContinueStmt* node);
    void visitReturnStmt(const ReturnStmt* node);
    void visitExprStmt(const ExprStmt* node);
    void visitBinaryExpr(const BinaryExpr* node);
    void visitUnaryExpr(const UnaryExpr* node);
    void visitIntLiteral(const IntLiteral* node);
    void visitVarRef(const VarRef* node);
    void visitFuncCall(const FuncCall* node);
    
    // 代码生成辅助函数
    void emit(const std::string& instruction);
    void emitLabel(const std::string& label);
    void emitComment(const std::string& comment);
    
    std::string allocateRegister();
    void freeRegister(const std::string& reg);
    
    std::string generateLabel(const std::string& prefix);
    std::string generateTempVar();
    
    void loadVariable(const std::string& name, const std::string& reg);
    void storeVariable(const std::string& name, const std::string& reg);
    
    void generateFunctionPrologue(const FunctionDef* node);
    void generateFunctionEpilogue(const FunctionDef* node);
    
    void generateArithmeticOp(const std::string& op, const std::string& result, 
                             const std::string& lhs, const std::string& rhs);
    void generateComparisonOp(const std::string& op, const std::string& result,
                             const std::string& lhs, const std::string& rhs);
    void generateLogicalOp(const std::string& op, const std::string& result,
                          const std::string& lhs, const std::string& rhs);
    
    void generateFunctionCall(const std::string& funcName, 
                             const std::vector<ASTNodePtr>& args,
                             const std::string& resultReg);
    
    // 栈管理
    void enterScope();
    void exitScope();
    int allocateStackSpace(int size);
    void deallocateStackSpace(int size);
}; 