#include "ir/ir_pipeline_api.h"
#include "lexical_parser.h"
#include "syntax_parser.h"
#include "tests/ir_test_suite.h"
#include "tests/pipeline_e2e_test_suite.h"

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

void print_ast(ASTNode* node, int indent = 0)
{
    if (!node)
        return;

    for (int i = 0; i < indent; ++i)
        std::cout << "  ";

    std::cout << "[" << node->type << "]";
    if (!node->value.empty())
        std::cout << " (" << node->value << ")";
    std::cout << "\n";

    for (ASTNode* child : node->children)
        print_ast(child, indent + 1);
}

void print_tokens(const std::vector<Token>& tokens)
{
    std::cout << "Tokens:\n";
    for (const Token& token : tokens)
    {
        std::cout << "  [" << token.type << "]";
        if (!token.value.empty())
            std::cout << " (" << token.value << ")";
        std::cout << "\n";
    }
}

bool is_tag_keyword(const std::string& type)
{
    return type == "STRUCT" || type == "UNION" || type == "ENUM";
}

bool is_typedef_declarator_boundary(const std::string& type)
{
    return type == "COMMA" || type == "SEMICOLON" || type == "ASSIGN"
        || type == "LBRACKET" || type == "LPAREN";
}

std::vector<Token> classify_typedef_names(std::vector<Token> tokens)
{
    std::unordered_set<std::string> typedef_names;
    std::vector<bool> in_typedef_declaration(tokens.size(), false);

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type != "TYPEDEF")
            continue;

        int brace_depth = 0;
        std::size_t end = i;
        for (; end < tokens.size(); ++end)
        {
            in_typedef_declaration[end] = true;
            if (tokens[end].type == "LBRACE")
                ++brace_depth;
            else if (tokens[end].type == "RBRACE" && brace_depth > 0)
                --brace_depth;
            else if (tokens[end].type == "SEMICOLON" && brace_depth == 0)
                break;
        }

        for (std::size_t j = i + 1; j <= end && j < tokens.size(); ++j)
        {
            if (tokens[j].type == "LBRACE")
                ++brace_depth;
            else if (tokens[j].type == "RBRACE" && brace_depth > 0)
                --brace_depth;

            if (brace_depth != 0 || tokens[j].type != "IDENTIFIER")
                continue;

            if (j > 0 && is_tag_keyword(tokens[j - 1].type))
                continue;

            const std::string next_type = (j + 1 < tokens.size()) ? tokens[j + 1].type : "";
            if (is_typedef_declarator_boundary(next_type))
                typedef_names.insert(tokens[j].value);
        }
    }

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        if (in_typedef_declaration[i] || tokens[i].type != "IDENTIFIER")
            continue;
        if (!typedef_names.count(tokens[i].value))
            continue;
        if (i > 0 && (tokens[i - 1].type == "DOT" || tokens[i - 1].type == "PTR"))
            continue;
        tokens[i].type = "TYPE_NAME";
    }

    return tokens;
}

int run_source_file(const char* path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        std::cerr << "Failed to open source file: " << path << "\n";
        return 1;
    }

    std::vector<Token> tokens = classify_typedef_names(tokenize(input));
    for (const Token& token : tokens)
    {
        if (token.type == "ERROR")
        {
            std::cerr << "Lexical error near: " << token.value << "\n";
            return 1;
        }
    }

    print_tokens(tokens);

    SyntaxParser parser;
    std::cout << "Start parsing...\n";
    if (!parser.parse(tokens))
    {
        std::cerr << "Parse failed!\n";
        return 1;
    }

    std::cout << "Parse success!\n\n";
    ASTNode* root = parser.get_ast_tree();
    std::cout << "AST Tree:\n";
    print_ast(root);

    const ir::PipelineResult ir_result = ir::generate_ir_from_parser_ast(root);
    if (!ir_result.success)
    {
        std::cerr << "\nIR generation warning: " << ir_result.error_text << "\n";
        return 0;
    }

    std::cout << "\nIR Quadruples:\n" << ir::dump_quadruples(ir_result.code) << "\n";
    return 0;
}

}  // namespace

int main(int argc, char* argv[])
{
    if (argc > 1)
        return run_source_file(argv[1]);

    const int ir_ret = run_all_ir_tests();
    const int e2e_ret = run_pipeline_e2e_tests();
    return (ir_ret == 0 && e2e_ret == 0) ? 0 : 1;
}
