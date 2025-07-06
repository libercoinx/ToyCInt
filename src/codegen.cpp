#include "codegen.h"
#include <iostream>
#include <sstream>

CodeGenerator::CodeGenerator(const std::string& outputFile) 
    : currentFunction(nullptr), labelCounter(0), tempVarCounter(0) {
    output.open(outputFile);
    if (!output.is_open()) {
        throw std::runtime_error("无法创建输出文件: " + outputFile);
    }
    
    // 输出汇编文件头部
    emitComment("RISC-V 32位汇编代码");
    emitComment("由ToyC编译器生成");
    emit("");
    emit(".text");
    emit(".globl main");
    emit("");
}

CodeGenerator::~CodeGenerator() {
    if (output.is_open()) {
        output.close();
    }
}

void CodeGenerator::generate(const ASTNodePtr& node) {
    visit(node);
}

void CodeGenerator::visit(const ASTNodePtr& node) {
    if (!node) return;
    
    // 根据节点类型分发到相应的处理函数
    if (auto program = std::dynamic_pointer_cast<Program>(node)) {
        visitProgram(program.get());
    } else if (auto funcDef = std::dynamic_pointer_cast<FunctionDef>(node)) {
        visitFunctionDef(funcDef.get());
    } else if (auto block = std::dynamic_pointer_cast<Block>(node)) {
        visitBlock(block.get());
    } else if (auto varDecl = std::dynamic_pointer_cast<VarDecl>(node)) {
        visitVarDecl(varDecl.get());
    } else if (auto assign = std::dynamic_pointer_cast<Assign>(node)) {
        visitAssign(assign.get());
    } else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(node)) {
        visitIfStmt(ifStmt.get());
    } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(node)) {
        visitWhileStmt(whileStmt.get());
    } else if (auto breakStmt = std::dynamic_pointer_cast<BreakStmt>(node)) {
        visitBreakStmt(breakStmt.get());
    } else if (auto continueStmt = std::dynamic_pointer_cast<ContinueStmt>(node)) {
        visitContinueStmt(continueStmt.get());
    } else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStmt>(node)) {
        visitReturnStmt(returnStmt.get());
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExprStmt>(node)) {
        visitExprStmt(exprStmt.get());
    } else if (auto binaryExpr = std::dynamic_pointer_cast<BinaryExpr>(node)) {
        visitBinaryExpr(binaryExpr.get());
    } else if (auto unaryExpr = std::dynamic_pointer_cast<UnaryExpr>(node)) {
        visitUnaryExpr(unaryExpr.get());
    } else if (auto intLiteral = std::dynamic_pointer_cast<IntLiteral>(node)) {
        visitIntLiteral(intLiteral.get());
    } else if (auto varRef = std::dynamic_pointer_cast<VarRef>(node)) {
        visitVarRef(varRef.get());
    } else if (auto funcCall = std::dynamic_pointer_cast<FuncCall>(node)) {
        visitFuncCall(funcCall.get());
    }
}

void CodeGenerator::visitProgram(const Program* node) {
    emitComment("程序开始");
    for (const auto& func : node->functions) {
        visit(func);
    }
    emitComment("程序结束");
}

void CodeGenerator::visitFunctionDef(const FunctionDef* node) {
    emit("");
    emitComment("函数定义: " + node->name);
    
    // 创建函数信息
    FunctionInfo funcInfo(node->name);
    funcInfo.returnType = node->retType;
    for (const auto& param : node->params) {
        funcInfo.paramNames.push_back(param.second);
        funcInfo.paramTypes.push_back(param.first);
    }
    functions[node->name] = funcInfo;
    currentFunction = &functions[node->name];
    
    // 生成函数标签
    emitLabel(node->name);
    
    // 生成函数序言
    generateFunctionPrologue(node);
    
    // 生成函数体
    visit(node->body);
    
    // 生成函数尾声
    generateFunctionEpilogue(node);
    
    currentFunction = nullptr;
}

void CodeGenerator::visitBlock(const Block* node) {
    enterScope();
    for (const auto& stmt : node->stmts) {
        visit(stmt);
    }
    exitScope();
}

void CodeGenerator::visitVarDecl(const VarDecl* node) {
    emitComment("变量声明: " + node->name);
    
    // 分配栈空间
    int offset = allocateStackSpace(4); // int类型占4字节
    symbolTable[node->name] = Symbol(node->name, node->type, offset);
    
    // 如果有初始值，生成赋值代码
    if (node->initExpr) {
        visit(node->initExpr);
        // 这里需要将表达式结果存储到变量位置
        // 简化实现，假设表达式结果在a0寄存器中
        emit("sw a0, " + std::to_string(offset) + "(sp)");
    }
}

void CodeGenerator::visitAssign(const Assign* node) {
    emitComment("赋值: " + node->name);
    
    // 计算表达式值
    visit(node->expr);
    
    // 存储到变量
    auto it = symbolTable.find(node->name);
    if (it != symbolTable.end()) {
        emit("sw a0, " + std::to_string(it->second.offset) + "(sp)");
    }
}

void CodeGenerator::visitIfStmt(const IfStmt* node) {
    std::string elseLabel = generateLabel("else");
    std::string endLabel = generateLabel("endif");
    
    emitComment("if语句开始");
    
    // 计算条件表达式
    visit(node->cond);
    
    // 如果条件为假，跳转到else分支
    emit("beqz a0, " + elseLabel);
    
    // then分支
    visit(node->thenStmt);
    emit("j " + endLabel);
    
    // else分支
    emitLabel(elseLabel);
    if (node->elseStmt) {
        visit(node->elseStmt);
    }
    
    emitLabel(endLabel);
    emitComment("if语句结束");
}

void CodeGenerator::visitWhileStmt(const WhileStmt* node) {
    std::string loopLabel = generateLabel("while");
    std::string endLabel = generateLabel("endwhile");
    
    emitComment("while循环开始");
    emitLabel(loopLabel);
    
    // 计算条件表达式
    visit(node->cond);
    
    // 如果条件为假，跳出循环
    emit("beqz a0, " + endLabel);
    
    // 循环体
    visit(node->body);
    
    // 跳回循环开始
    emit("j " + loopLabel);
    
    emitLabel(endLabel);
    emitComment("while循环结束");
}

void CodeGenerator::visitBreakStmt(const BreakStmt* node) {
    emitComment("break语句");
    // 这里需要跳转到循环结束标签
    // 简化实现，暂时注释掉
    // emit("j " + currentLoopEndLabel);
}

void CodeGenerator::visitContinueStmt(const ContinueStmt* node) {
    emitComment("continue语句");
    // 这里需要跳转到循环开始标签
    // 简化实现，暂时注释掉
    // emit("j " + currentLoopStartLabel);
}

void CodeGenerator::visitReturnStmt(const ReturnStmt* node) {
    emitComment("return语句");
    
    if (node->expr) {
        visit(node->expr);
        // 返回值已经在a0寄存器中
    }
    
    generateFunctionEpilogue(nullptr);
}

void CodeGenerator::visitExprStmt(const ExprStmt* node) {
    visit(node->expr);
    // 表达式语句的结果被忽略
}

void CodeGenerator::visitBinaryExpr(const BinaryExpr* node) {
    emitComment("二元表达式: " + node->op);
    
    // 计算左操作数
    visit(node->lhs);
    emit("mv t0, a0");  // 保存左操作数
    
    // 计算右操作数
    visit(node->rhs);
    emit("mv t1, a0");  // 保存右操作数
    
    // 执行运算
    generateArithmeticOp(node->op, "a0", "t0", "t1");
}

void CodeGenerator::visitUnaryExpr(const UnaryExpr* node) {
    emitComment("一元表达式: " + node->op);
    
    visit(node->expr);
    
    if (node->op == "-") {
        emit("neg a0, a0");
    } else if (node->op == "!") {
        emit("seqz a0, a0");
    }
}

void CodeGenerator::visitIntLiteral(const IntLiteral* node) {
    emitComment("整数字面量: " + std::to_string(node->value));
    emit("li a0, " + std::to_string(node->value));
}

void CodeGenerator::visitVarRef(const VarRef* node) {
    emitComment("变量引用: " + node->name);
    
    auto it = symbolTable.find(node->name);
    if (it != symbolTable.end()) {
        emit("lw a0, " + std::to_string(it->second.offset) + "(sp)");
    }
}

void CodeGenerator::visitFuncCall(const FuncCall* node) {
    emitComment("函数调用: " + node->name);
    
    // 计算参数并压栈
    for (size_t i = 0; i < node->args.size(); ++i) {
        visit(node->args[i]);
        // 将参数保存到临时位置
        emit("sw a0, " + std::to_string(-(i + 1) * 4) + "(sp)");
    }
    
    // 将参数加载到参数寄存器
    for (size_t i = 0; i < node->args.size() && i < 8; ++i) {
        emit("lw a" + std::to_string(i) + ", " + std::to_string(-(i + 1) * 4) + "(sp)");
    }
    
    // 调用函数
    emit("call " + node->name);
}

// 辅助函数实现
void CodeGenerator::emit(const std::string& instruction) {
    output << "    " << instruction << std::endl;
}

void CodeGenerator::emitLabel(const std::string& label) {
    output << label << ":" << std::endl;
}

void CodeGenerator::emitComment(const std::string& comment) {
    output << "    # " << comment << std::endl;
}

std::string CodeGenerator::generateLabel(const std::string& prefix) {
    return prefix + "_" + std::to_string(labelCounter++);
}

void CodeGenerator::generateFunctionPrologue(const FunctionDef* node) {
    emitComment("函数序言");
    emit("addi sp, sp, -16");  // 分配栈空间
    emit("sw ra, 12(sp)");     // 保存返回地址
    emit("sw s0, 8(sp)");      // 保存帧指针
    emit("addi s0, sp, 16");   // 设置新的帧指针
}

void CodeGenerator::generateFunctionEpilogue(const FunctionDef* node) {
    emitComment("函数尾声");
    emit("lw ra, 12(sp)");     // 恢复返回地址
    emit("lw s0, 8(sp)");      // 恢复帧指针
    emit("addi sp, sp, 16");   // 恢复栈指针
    emit("ret");               // 返回
}

void CodeGenerator::generateArithmeticOp(const std::string& op, const std::string& result, 
                                        const std::string& lhs, const std::string& rhs) {
    if (op == "+") {
        emit("add " + result + ", " + lhs + ", " + rhs);
    } else if (op == "-") {
        emit("sub " + result + ", " + lhs + ", " + rhs);
    } else if (op == "*") {
        emit("mul " + result + ", " + lhs + ", " + rhs);
    } else if (op == "/") {
        emit("div " + result + ", " + lhs + ", " + rhs);
    } else if (op == "%") {
        emit("rem " + result + ", " + lhs + ", " + rhs);
    }
}

void CodeGenerator::enterScope() {
    // 简化实现，暂时不处理作用域
}

void CodeGenerator::exitScope() {
    // 简化实现，暂时不处理作用域
}

int CodeGenerator::allocateStackSpace(int size) {
    if (currentFunction) {
        currentFunction->stackSize += size;
        return -currentFunction->stackSize;
    }
    return 0;
} 