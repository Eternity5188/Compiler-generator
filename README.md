## Code Structure

- resource

    - rule

        词法规则、语法规则文件

    - source

        源代码文件

- common

    token等通用类型

- generator

    - lexical

        词法分析程序生成器代码

    - syntax

        语法分析程序生成器代码

- analyzer

    最终程序，对源代码进行词法分析、语法分析、中间代码生成处理

- build.all.bat

    编译所有程序

- run_generator.bat

    执行词法生成器、语法生成器

- run_analyzer.bat

    执行代码分析程序