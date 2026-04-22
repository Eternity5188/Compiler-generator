# SeuLex - C99 词法分析器生成器

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

## 项目简介

SeuLex 是一个基于 C++ 实现的词法分析器生成器，能够根据用户定义的 `.l` 词法规则文件生成 C99 标准的词法分析器。

本项目实现了从正则表达式到 DFA（确定有限自动机）的完整转换流程，包括：
- 正则表达式解析与规范化
- NFA（非确定有限自动机）构建
- NFA 转 DFA（子集构造法）
- DFA 最小化
- 生成可执行的词法分析器代码

## 功能特性

- ✅ 支持完整的 C99 词法规则
- ✅ 支持正则表达式定义（字符类、范围、闭包等）
- ✅ 支持关键字、标识符、常量、运算符识别
- ✅ 支持字符常量和字符串字面量
- ✅ 支持注释处理（单行和多行）
- ✅ 生成独立的 C++ 词法分析器代码
- ✅ 跨平台支持（Windows/Linux）

## 项目结构

```
seulex/
├── CMakeLists.txt          # CMake 构建配置
├── main.cpp                # 生成器主程序入口
├── ReadLex.cpp/h           # 词法规则文件解析
├── NormaliseRE.cpp/h       # 正则表达式规范化
├── Infix2Postfix.cpp/h     # 中缀转后缀表达式
├── NFA.cpp/h               # NFA 构建与管理
├── DFA.cpp/h               # DFA 构建与最小化
├── GenCode.cpp/h           # 代码生成
├── c99.l                   # C99 词法规则定义文件
├── test.txt                # 测试输入文件
├── bin/                    # 示例代码目录
│   ├── test.txt            # 基础测试用例
│   └── test_sample.c       # C 语言示例代码
└── README.md               # 项目说明文档
```

## 编译与运行

### 环境要求

- C++11 或更高版本编译器
- CMake 3.10+
- Make 或 Visual Studio (Windows)

### 构建步骤

```bash
# 创建构建目录
mkdir build && cd build

# 生成构建文件
cmake ..

# 编译
cmake --build .
# 或 Linux 下使用 make
make
```

### 使用方法

#### 1. 生成词法分析器

```bash
# 运行生成器，处理词法规则文件
./main.exe c99.l
```

生成器将输出：
- `lexer.cpp` - 生成的词法分析器源代码
- 解析过程中的调试信息

#### 2. 编译生成的词法分析器

```bash
# 编译生成的词法分析器
g++ -o lexer lexer.cpp
```

#### 3. 运行词法分析

```bash
# 准备待分析的源代码文件（必须是 test.txt）
cp your_code.c test.txt

# 运行词法分析器
./lexer.exe
```

词法分析器将从 `test.txt` 读取输入，并输出识别到的 token 序列。

## 词法规则文件格式

词法规则文件（`.l`）采用类似 Flex 的格式：

```
%{
/* 声明部分 - C/C++ 代码 */
#include <iostream>
using namespace std;

// 全局变量和辅助函数
int line_num = 1;
%}

/* 正则定义 */
LETTER          [a-zA-Z_]
DIGIT           [0-9]
IDENTIFIER      {LETTER}({LETTER}|{DIGIT})*
INTEGER         {DIGIT}+

/* 规则部分 */
%%

"int"           { return INT; }
"return"        { return RETURN; }
{IDENTIFIER}    { return IDENTIFIER; }
{INTEGER}       { return CONSTANT; }
"+"             { return '+'; }
";"             { return ';'; }

%%
```

### 支持的语法

| 语法 | 说明 | 示例 |
|------|------|------|
| `x` | 匹配字符 x | `a` |
| `"..."` | 匹配字符串 | `"int"` |
| `[xyz]` | 字符类 | `[abc]` |
| `[a-z]` | 字符范围 | `[0-9]` |
| `[^...]` | 补集 | `[^\n]` |
| `r*` | 零次或多次 | `a*` |
| `r+` | 一次或多次 | `a+` |
| `r?` | 零次或一次 | `a?` |
| `r\|s` | 或运算 | `a\|b` |
| `(r)` | 分组 | `(ab)+` |
| `{name}` | 引用定义 | `{IDENTIFIER}` |

## 实现原理

### 1. 正则表达式规范化

将词法规则中的正则表达式转换为标准形式：
- 展开字符类 `[a-z]` → `(a|b|c|...|z)`
- 展开重复操作 `r+` → `rr*`
- 展开可选操作 `r?` → `(r|ε)`

### 2. NFA 构建

使用 Thompson 构造法将正则表达式转换为 NFA：
- 基本字符：创建两个状态，用该字符标记的边连接
- 连接运算：将两个 NFA 首尾相连
- 或运算：创建新的起始和接受状态
- 闭包运算：添加 ε 转移实现循环

### 3. DFA 构建

使用子集构造法将 NFA 转换为 DFA：
- 计算 ε 闭包
- 对每个可能的输入字符，计算状态转移
- 合并等价状态

### 4. DFA 最小化

使用划分法最小化 DFA：
- 初始划分：接受状态和非接受状态
- 迭代细分直到稳定
- 合并等价状态

## 示例输出

输入 C 代码：
```c
int main() {
    int a = 10;
    return 0;
}
```

词法分析输出：
```
Lexical Analysis Result :
INT int
IDENTIFIER main
(
)
{
INT int
IDENTIFIER a
=
CONSTANT 10
;
RETURN return
CONSTANT 0
;
}
```

## 技术栈

- **语言**：C++11
- **构建工具**：CMake
- **算法**：Thompson 构造法、子集构造法、DFA 最小化

## 文件说明

### 核心源代码

| 文件 | 说明 |
|------|------|
| `main.cpp` | 程序入口，协调各模块工作 |
| `ReadLex.cpp/h` | 解析 `.l` 词法规则文件 |
| `NormaliseRE.cpp/h` | 正则表达式规范化处理（字符类展开等） |
| `Infix2Postfix.cpp/h` | 中缀表达式转后缀表达式（调度场算法） |
| `NFA.cpp/h` | NFA 数据结构和 Thompson 构造法实现 |
| `DFA.cpp/h` | DFA 数据结构、子集构造法和最小化实现 |
| `GenCode.cpp/h` | 生成最终的词法分析器 C++ 代码 |

### 配置文件

| 文件 | 说明 |
|------|------|
| `CMakeLists.txt` | CMake 构建配置 |
| `c99.l` | C99 标准词法规则定义 |

### 测试文件

| 文件 | 说明 |
|------|------|
| `test.txt` | 默认测试输入 |
| `bin/test_sample.c` | C 语言示例代码 |

## 注意事项

1. 生成的 `lexer.cpp` 默认从 `test.txt` 读取输入
2. 词法规则文件需使用 UTF-8 编码
3. 中文字符可能导致词法错误，建议使用纯英文代码测试
4. 注释规则需正确定义，避免与除法运算符冲突

