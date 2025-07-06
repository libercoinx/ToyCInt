#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "semantic.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getOutputFilename(const std::string& inputFile) {
    std::filesystem::path path(inputFile);
    std::string stem = path.stem().string();
    return stem + ".s";
}

void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " <输入文件>" << std::endl;
    std::cout << "示例: " << programName << " test/example.toyc" << std::endl;
    std::cout << "输出: 生成对应的RISC-V汇编文件 (.s后缀)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = getOutputFilename(inputFile);
    
    try {
        // 读取源文件
        std::string source = readFile(inputFile);
        std::cout << "正在编译文件: " << inputFile << std::endl;
        
        // 词法分析
        Lexer lexer(source);
        std::cout << "词法分析完成" << std::endl;
        
        // 语法分析
        Parser parser(source);
        auto ast = parser.parse();
        if (!ast) {
            std::cerr << "语法分析失败" << std::endl;
            return 1;
        }
        std::cout << "语法分析完成" << std::endl;
        
        // 语义分析
        SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.analyze(ast);
        std::cout << "语义分析完成" << std::endl;
        
        // 代码生成
        CodeGenerator codegen(outputFile);
        codegen.generate(ast);
        std::cout << "代码生成完成" << std::endl;
        
        std::cout << "编译成功！输出文件: " << outputFile << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "编译错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 