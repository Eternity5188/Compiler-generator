# Compiler Generator

A practical compiler frontend project targeting a subset of the C language, featuring:

- Lexical and syntax parser generators (`generator`)
- Semantic analysis and intermediate representation generation (`analyzer`)
- Layered testing framework with structured report outputs (`tests`)

---

# Quick Start

## 1. Enter the Project Directory

```powershell
cd compiler-generator
```

## 2. One-Click Build and Full Test Suite (Recommended)

```powershell
.\build_all.bat
```

---

# Workflow Diagram



---

# Common Commands

## Step-by-Step Build and Test

```powershell
cmake -B build
cmake --build build -j
.\tests\run_all_tests.bat
```

## Run Specific Test Suites

```powershell
.\tests\lex_yacc\run_lex_yacc_tests.bat
.\tests\ir\run_ir_unit_tests.bat
.\tests\system\run_system_tests.bat
```

## Run the Analyzer Directly

```powershell
.\target\analyzer\Debug\analyzer.exe --unit
.\target\analyzer\Debug\analyzer.exe --system
.\target\analyzer\Debug\analyzer.exe .\tests\lex_yacc\lex_yacc_smoke.c
```

---

# Repository Structure

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

---

# Module Overview

- `generator/lexical`  
  Components related to lexical analyzer generation.

- `generator/syntax`  
  Syntax parser generator implementation. Reads grammar rules from `resource/rule/syntax_rule.txt`.

- `analyzer`  
  Main frontend pipeline entry, including parsing, semantic analysis, and IR generation.

- `tests/lex_yacc`  
  Batch regression tests for integrated lexical and syntax analysis.

- `tests/ir`  
  IR unit tests and parser-based file test cases.

- `tests/system`  
  System-level regression tests, including both functional and stress test suites.

---

# Test Report Outputs

After running the test suite, detailed reports are generated under `tests/output`:

- `tests/output/lex_yacc`  
  Reports for lexical and syntax analysis test cases

- `tests/output/ir_parser`  
  Reports for IR parser file-based test cases

- `tests/output/system_basic`  
  Functional regression test reports

- `tests/output/system_stress`  
  Stress test reports

Each report file is named after its corresponding source file, for example:

```text
core_03_decl_no_init.txt
```

Each report contains the following sections:

- `CASE_NAME`
- `SOURCE_CODE`
- `LEX_OUTPUT`
- `YACC_OUTPUT`
- `IR_INPUT_AST`
- `ANALYZER_OUTPUT`
- `IR_QUADRUPLES`
- `PIPELINE_ERROR`

---

# Grammar Rules and Supported Features

Grammar rules are defined in:

```text
resource/rule/syntax_rule.txt
```

The currently supported subset of the C language includes:

- Primitive types (`int`, `float`, `char`, `void`)
- Variable declarations and assignments
- Arithmetic, relational, and logical expressions
- Basic control flow constructs
- Function calls (within the currently implemented grammar subset)
- Intermediate representation (IR) generation pipeline
