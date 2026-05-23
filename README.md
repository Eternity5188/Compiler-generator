# Compiler Generator

面向 C 语言子集的编译前端实践项目，包含：
- 词法/语法生成器（generator）
- 分析与中间代码生成入口（analyzer）
- 分层测试与结果落盘（tests）

## 快速开始

### 1) 进入项目目录

```powershell
cd compiler-generator
```

### 2) 一键构建 + 全量测试（推荐）

```powershell
.\build_all.bat
```

## 常用运行命令

### 分步执行

```powershell
cmake -B build
cmake --build build -j
.\tests\run_all_tests.bat
```

### 单独执行某类测试

```powershell
.\tests\lex_yacc\run_lex_yacc_tests.bat
.\tests\ir\run_ir_unit_tests.bat
.\tests\system\run_system_tests.bat
```

### 直接运行 analyzer

```powershell
.\target\analyzer\Debug\analyzer.exe --unit
.\target\analyzer\Debug\analyzer.exe --system
.\target\analyzer\Debug\analyzer.exe .\tests\lex_yacc\lex_yacc_smoke.c
```

## 仓库结构

```text
compiler-generator/
├── CMakeLists.txt
├── build_all.bat
├── analyzer/
├── common/
├── generator/
├── resource/
├── tests/
│   ├── run_all_tests.bat
│   ├── lex_yacc/
│   ├── ir/
│   ├── system/
│   └── output/
└── doc/
```

## 模块说明

- `generator/lexical`：词法生成器相关代码。
- `generator/syntax`：语法生成器相关代码，读取 `resource/rule/syntax_rule.txt`。
- `analyzer`：主入口与 IR 生成流程。
- `tests/lex_yacc`：词法与语法综合语料的批量回归测试。
- `tests/ir`：IR 单元测试与文件型 parser 用例。
- `tests/system`：系统功能回归（basic）和压力用例（stress）。

## 测试报告输出

执行测试后，会在 `tests/output` 生成详细报告：

- `tests/output/lex_yacc`：lex_yacc 用例报告
- `tests/output/ir_parser`：IR 文件型用例报告
- `tests/output/system_basic`：系统功能回归报告
- `tests/output/system_stress`：系统压力报告

每个报告文件名使用对应源码文件名（如 `core_03_decl_no_init.txt`）。

报告包含以下字段：
- `CASE_NAME`
- `SOURCE_CODE`
- `LEX_OUTPUT`
- `YACC_OUTPUT`
- `IR_INPUT_AST`
- `ANALYZER_OUTPUT` 或 `IR_QUADRUPLES / PIPELINE_ERROR`

## 语法规则与能力范围

语法规则文件位于 `resource/rule/syntax_rule.txt`。

当前覆盖的 C 子集能力包括：
- 基本类型（`int`、`float`、`char`、`void`）
- 声明、赋值、表达式
- 关系/逻辑/算术运算
- 基本控制流与函数调用（按当前语法子集实现范围）