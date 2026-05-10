#include "syntax_parser.h"
#include <iostream>
#include <vector>

// AST 打印函数（递归缩进输出）
void print_ast(ASTNode* node, int indent = 0)
{
    if (!node)
        return;

    // 缩进
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";

    // 打印节点
    std::cout << "[" << node->type << "]";
    if (!node->value.empty())
        std::cout << " (" << node->value << ")";
    std::cout << "\n";

    // 递归打印子节点
    for (ASTNode* child : node->children)
    {
        print_ast(child, indent + 1);
    }
}

// 测试用例：int a;
std::vector<Token> create_test_tokens()
{
    return {
        {"INT", "int"},
        {"IDENTIFIER", "a"},
        {"SEMICOLON", ";"}
    };
}

int main()
{
    // 1. 创建解析器
    SyntaxParser parser;

    // 2. 准备测试 Token
    std::vector<Token> tokens = create_test_tokens();

    // 3. 解析
    std::cout << "Start parsing...\n";
    bool ok = parser.parse(tokens);

    if (!ok)
    {
        std::cerr << "Parse failed!\n";
        return 1;
    }

    std::cout << "Parse success!\n\n";

    // 4. 获取并打印 AST
    ASTNode* root = parser.get_ast_tree();
    std::cout << "AST Tree:\n";
    print_ast(root);

    return 0;
}