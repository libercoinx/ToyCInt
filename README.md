# ToyCC - Toy C语言编译器

一个用C++实现的简单C语言子集编译器，将Toy C代码编译为RISC-V 32位汇编代码。

## 功能特性

- **词法分析**: 支持关键字、标识符、字面量、运算符和分隔符
- **语法分析**: 递归下降解析器，构建抽象语法树(AST)
- **语义分析**: 类型检查和符号表管理
- **代码生成**: 生成RISC-V 32位汇编代码

## 支持的语法

### 数据类型
- `int` - 整数类型
- `void` - 空类型

### 语句
- 变量声明: `int x = 10;`
- 赋值: `x = 20;`
- 条件语句: `if (x > 0) { ... } else { ... }`
- 循环语句: `while (x > 0) { ... }`
- 返回语句: `return x;`
- 中断语句: `break;`, `continue;`
- 表达式语句: `x + y;`

### 表达式
- 算术运算: `+`, `-`, `*`, `/`, `%`
- 比较运算: `==`, `!=`, `<`, `<=`, `>`, `>=`
- 逻辑运算: `&&`, `||`, `!`
- 函数调用: `func(x, y)`

### 函数
- 函数定义和调用
- 参数传递
- 返回值

## 构建方法

### 使用Makefile

```bash
# 编译
make

# 编译测试
make compile-test

# 清理
make clean
```

## 使用方法

```bash
# 编译Toy C程序为RISC-V汇编
./bin/toycc <输入文件>

# 示例
./bin/toycc test/example.toyc
# 输出: example.s (RISC-V汇编文件)
```

## 输出格式

编译器会生成标准的RISC-V 32位汇编代码，包含：

- `.text` 段：代码段
- 函数标签和调用约定
- 栈帧管理
- 寄存器分配
- 控制流指令

生成的汇编文件可以直接用RISC-V工具链汇编和链接。

## 项目结构

```
ToyCC/
├── include/           # 头文件
│   ├── ast.h         # 抽象语法树定义
│   ├── lexer.h       # 词法分析器
│   ├── parser.h      # 语法分析器
│   ├── codegen.h     # 代码生成器
│   └── semantic.h    # 语义分析器
├── src/              # 源文件
│   ├── main.cpp      # 主程序入口
│   ├── lexer.cpp     # 词法分析器实现
│   ├── parser.cpp    # 语法分析器实现
│   ├── codegen.cpp   # 代码生成器实现
│   ├── ast.cpp       # AST辅助函数
│   └── semantic.cpp  # 语义分析器实现
├── test/             # 测试文件
│   └── example.toyc  # 示例程序
├── Makefile          # Make构建文件
└── README.md         # 项目说明
```

## 开发计划

- [ ] 实现词法分析器 (`src/lexer.cpp`)
- [ ] 实现语法分析器 (`src/parser.cpp`)
- [ ] 完善代码生成器 (`src/codegen.cpp`)
- [ ] 添加更多测试用例
- [ ] 支持更多数据类型 (float, char)
- [ ] 添加优化pass
- [ ] 支持标准库函数
- [ ] 添加调试信息生成

## 示例

输入文件 `test/example.toyc`:
```c
int main() {
    int x = 10;
    int y = 20;
    return x + y;
}
```

输出文件 `example.s`:
```assembly
    # RISC-V 32位汇编代码
    # 由ToyC编译器生成

    .text
    .globl main

    # 程序开始
    # 函数定义: main
    main:
        # 函数序言
        addi sp, sp, -16
        sw ra, 12(sp)
        sw s0, 8(sp)
        addi s0, sp, 16
        # 变量声明: x
        li a0, 10
        sw a0, -4(sp)
        # 变量声明: y
        li a0, 20
        sw a0, -8(sp)
        # 二元表达式: +
        lw a0, -4(sp)
        mv t0, a0
        lw a0, -8(sp)
        mv t1, a0
        add a0, t0, t1
        # return语句
        # 函数尾声
        lw ra, 12(sp)
        lw s0, 8(sp)
        addi sp, sp, 16
        ret
    # 程序结束
```

## 许可证

MIT License