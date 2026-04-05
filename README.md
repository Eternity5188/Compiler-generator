# Compiler Generator

一个 C 语言编译器前端的代码生成工具集。

---

## 项目结构

```
compiler-generator/
├── CMakeLists.txt              # 顶层构建配置
├── build_all.bat              # 编译所有模块
├── run_generator.bat          # 运行生成器
├── run_analyzer.bat           # 运行分析器
│
├── common/                    # 公共模块
│   ├── token.h
│   └── token.cpp
│
├── generator/                 # 代码生成器
│   ├── CMakeLists.txt
│   ├── lexical/              # 词法分析器生成器
│   │   └── lexical_generator.cpp
│   └── syntax/               # 语法分析器生成器
│       ├── syntax_generator.cpp
│       ├── syntax_rule_parser.h
│       ├── syntax_rule_parser.cpp
│       ├── production.h
│       ├── production.cpp
│       ├── symbol.h
│       └── symbol.cpp
│
├── analyzer/                 # 分析器
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── resource/                # 资源文件
│   └── rule/
│       └── syntax_rule.txt   # 语法规则定义
│
└── doc/                     # 设计文档
    ├── 中间代码生成设计文档.md
    ├── 中间代码生成设计文档.pdf
    ├── lex设计文档.pdf
    ├── yacc.md
    └── yacc.pdf
```

---

## 模块说明

### common

公共模块，提供 Token 类型定义，供其他模块使用。

### generator

代码生成器模块，包含两个子模块：

**lexical/** - 词法分析器生成器
- 输入：正则表达式规则
- 输出：词法分析器代码

**syntax/** - 语法分析器生成器
- 依赖 `resource/rule/syntax_rule.txt` 中的语法规则
- 使用 `SyntaxRuleParser` 类解析规则文件
- 支持终结符声明、优先级/结合性定义、产生式规则

### analyzer

分析器模块，对源代码进行词法分析、语法分析处理。

### resource/rule

存放语法规则定义文件。

**syntax_rule.txt** 定义了 C 语言子集的语法规则：
- 终结符声明（token）
- 优先级和结合性
- 产生式规则

---

## 编译与运行

### 编译

```bash
cmake -B build
cmake --build build
```

或直接双击运行 `build_all.bat`。

### 运行生成器

```bash
./target/generator/lexical_generator.exe
./target/generator/syntax_generator.exe
```

或双击 `run_generator.bat`。

### 运行分析器

```bash
./target/analyzer/analyzer.exe
```

或双击 `run_analyzer.bat`。

---

## 语法规则

详见 `resource/rule/syntax_rule.txt`。

支持的 C 语言特性包括：
- 基本类型（int, float, char, void）
- 变量声明与赋值
- 算术与关系运算
- 逻辑运算
- 控制流语句（if-else, while, do-while, for, break, continue）
- 函数定义与调用
- 数组
- 指针操作