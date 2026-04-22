# Gitee 上传清单

## 需要上传的文件/目录

### 1. 核心源代码（必须上传）
```
├── CMakeLists.txt          # CMake 构建配置
├── main.cpp                # 主程序入口
├── ReadLex.cpp
├── ReadLex.h
├── NormaliseRE.cpp
├── NormaliseRE.h
├── Infix2Postfix.cpp
├── Infix2Postfix.h
├── NFA.cpp
├── NFA.h
├── DFA.cpp
├── DFA.h
├── GenCode.cpp
└── GenCode.h
```

### 2. 词法规则文件（必须上传）
```
├── c99.l                   # C99 词法规则定义文件
```

### 3. 测试文件（建议上传）
```
├── test.txt                # 默认测试输入
└── bin/
    ├── test.txt            # 基础测试用例
    └── test_sample.c       # C 语言示例代码
```

### 4. 文档文件（必须上传）
```
├── README.md               # 项目说明文档
├── .gitignore              # Git 忽略文件配置
└── UPLOAD_LIST.md          # 本清单文件（可选）
```

### 5. 生成的词法分析器（可选上传）
```
├── lexer.cpp               # 生成的词法分析器源代码（可选）
└── lexer.exe               # 生成的可执行文件（Windows 可选）
```

---

## 不需要上传的文件/目录

### 编译产物（自动排除）
- `build/` 目录下的所有文件
- `*.exe` 可执行文件
- `*.o`, `*.obj` 目标文件
- `*.a`, `*.lib` 静态库文件
- `*.d` 依赖文件

### IDE 配置文件
- `.vscode/` 目录
- `.idea/` 目录
- Visual Studio 项目文件

---

## 上传步骤

### 方法一：使用 Git 命令行

```bash
# 1. 进入项目目录
cd seulex

# 2. 初始化 Git 仓库
git init

# 3. 添加所有文件（自动排除 .gitignore 中指定的文件）
git add .

# 4. 提交更改
git commit -m "Initial commit: SeuLex C99 Lexical Analyzer Generator"

# 5. 添加远程仓库
git remote add origin https://gitee.com/你的用户名/seulex.git

# 6. 推送到 Gitee
git push -u origin master
```

### 方法二：使用 Gitee Web 界面上传

1. 在 Gitee 创建新仓库 `seulex`
2. 在本地整理需要上传的文件（参考上面的清单）
3. 打包为 zip 文件
4. 在 Gitee 仓库页面选择 "上传文件"
5. 上传 zip 文件并解压

---

## 文件大小统计

### 源代码（约 30KB）
- 头文件 (.h): ~5KB
- 源文件 (.cpp): ~25KB

### 文档（约 10KB）
- README.md: ~8KB
- 其他文档: ~2KB

### 测试文件（约 2KB）
- 测试用例: ~2KB

### 总计（约 40-50KB）
- 纯源代码: ~40KB
- 包含生成文件: ~50KB

---

## 注意事项

1. **编码问题**：确保所有源代码文件使用 UTF-8 编码保存
2. **换行符**：Windows 用户使用 CRLF，建议统一使用 LF
3. **文件权限**：确保 `.sh` 脚本文件有执行权限（如有）
4. **README**：确保 README.md 中的截图路径正确

---

## 推荐的仓库结构

```
seulex/                         # 仓库根目录
├── .gitignore                  # Git 忽略配置
├── README.md                   # 项目说明
├── CMakeLists.txt              # 构建配置
├── c99.l                       # 词法规则文件
├── test.txt                    # 默认测试输入
├── bin/                        # 示例目录
│   ├── test.txt
│   └── test_sample.c
├── src/                        # 源代码目录（可选整理）
│   ├── main.cpp
│   ├── ReadLex.cpp
│   ├── ReadLex.h
│   ├── NormaliseRE.cpp
│   ├── NormaliseRE.h
│   ├── Infix2Postfix.cpp
│   ├── Infix2Postfix.h
│   ├── NFA.cpp
│   ├── NFA.h
│   ├── DFA.cpp
│   ├── DFA.h
│   ├── GenCode.cpp
│   └── GenCode.h
└── doc/                        # 文档目录（可选）
    └── design.md
```

如果不想整理目录结构，可以直接放在根目录下。
