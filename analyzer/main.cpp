#include "syntax_parser.h"
#include "ir/ir_generator.h"
#include "ir/syntax_ast_adapter.h"
#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

void print_ast(ASTNode* node, std::unordered_set<const ASTNode*>& visited, int indent = 0)
{
    if (!node)
        return;

    if (visited.find(node) != visited.end())
    {
        for (int i = 0; i < indent; ++i)
            std::cout << "  ";
        std::cout << "[cycle detected]\n";
        return;
    }
    visited.insert(node);

    for (int i = 0; i < indent; ++i)
        std::cout << "  ";

    std::cout << "[" << node->type << "]";
    if (!node->value.empty())
        std::cout << " (" << node->value << ")";
    std::cout << "\n";

    for (ASTNode* child : node->children)
    {
        print_ast(child, visited, indent + 1);
    }
}

void print_ir(const std::vector<ir::Quadruple>& code)
{
    std::cout << "\nIR Quadruples:\n";
    int shown = 0;
    for (std::size_t i = 0; i < code.size(); ++i)
    {
        const ir::Quadruple& q = code[i];
        if (q.op == "nop")
            continue;

        std::cout << shown++ << ": (" << q.op << ", " << q.arg1 << ", "
                  << q.arg2 << ", " << q.result << ")\n";
    }

    if (shown == 0)
        std::cout << "(none)\n";
}

// Test case: int a = 1;
std::vector<Token> create_test_tokens()
{
    return {
        {"INT", "int"},
        {"IDENTIFIER", "a"},
        {"ASSIGN", "="},
        {"NUMBER", "1"},
        {"SEMICOLON", ";"}
    };
}

int main()
{
    // 1) Create parser
    SyntaxParser parser;

    // 2) Prepare test tokens
    std::vector<Token> tokens = create_test_tokens();

    // 3) Parse
    std::cout << "Start parsing...\n" << std::flush;
    bool ok = parser.parse(tokens);

    if (!ok)
    {
        std::cerr << "Parse failed!\n";
        return 1;
    }

    std::cout << "Parse success!\n\n" << std::flush;

    // 4) Print AST
    ASTNode* root = parser.get_ast_tree();
    std::cout << "AST Tree:\n";
    std::unordered_set<const ASTNode*> visited;
    print_ast(root, visited);

    // 5) AST -> IR
    std::unique_ptr<ir::ASTNode> ir_ast = ir::SyntaxASTAdapter::convert(root);
    ir::IRGenerator ir_generator;
    const std::vector<ir::Quadruple>& ir_code = ir_generator.generate(ir_ast.get());

    if (ir_generator.errors().has_error())
    {
        std::cerr << "\nSemantic errors found during IR generation:\n";
        ir_generator.errors().print_all(std::cerr);
        return 1;
    }

    print_ir(ir_code);

    return 0;
}