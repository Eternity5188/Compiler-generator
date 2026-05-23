#include "pipeline_api.h"

#include "lexical_parser.h"
#include "syntax_parser.h"
#include "ir/ir_generator.h"
#include "ir/syntax_ast_adapter.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

int run_ir_unit_tests();
int run_system_integration_tests();

namespace ir {

namespace {

bool is_tag_keyword(const std::string& type)
{
    return type == "STRUCT" || type == "UNION" || type == "ENUM";
}

bool is_typedef_declarator_boundary(const std::string& type)
{
    return type == "COMMA" || type == "SEMICOLON" || type == "ASSIGN"
        || type == "LBRACKET" || type == "LPAREN";
}

std::string dump_tokens(const std::vector<Token>& tokens)
{
    std::ostringstream oss;
    if (tokens.empty())
        return "(none)";

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        oss << i << ": [" << tokens[i].type << "] " << tokens[i].value;
        if (i + 1 < tokens.size())
            oss << "\n";
    }
    return oss.str();
}

void dump_parser_ast_impl(const ::ASTNode* node, std::ostringstream& oss, int depth)
{
    if (node == nullptr)
    {
        oss << std::string(depth * 2, ' ') << "(null)";
        return;
    }

    oss << std::string(depth * 2, ' ') << node->type;
    if (!node->value.empty())
        oss << ": " << node->value;
    oss << "\n";

    for (std::size_t i = 0; i < node->children.size(); ++i)
        dump_parser_ast_impl(node->children[i], oss, depth + 1);
}

std::string dump_parser_ast(const ::ASTNode* root)
{
    std::ostringstream oss;
    if (root == nullptr)
        return "(none)";
    dump_parser_ast_impl(root, oss, 0);
    return oss.str();
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

}  // namespace

std::string dump_quadruples(const std::vector<Quadruple>& code)
{
    std::ostringstream oss;
    bool first = true;
    for (std::size_t i = 0; i < code.size(); ++i)
    {
        if (code[i].op == "nop")
            continue;
        if (!first)
            oss << " | ";
        first = false;
        oss << "(" << code[i].op << ", " << code[i].arg1 << ", " << code[i].arg2 << ", " << code[i].result << ")";
    }
    return first ? "(none)" : oss.str();
}

PipelineResult generate_ir_from_ir_ast(const ir::ASTNode* ir_root)
{
    PipelineResult result;
    if (ir_root == nullptr)
    {
        result.error_text = "IR AST is null";
        return result;
    }

    IRGenerator generator;
    const std::vector<Quadruple>& code_ref = generator.generate(ir_root);
    result.code = code_ref;

    if (generator.errors().has_error())
    {
        std::ostringstream oss;
        generator.errors().print_all(oss);
        result.error_text = oss.str();
        result.success = false;
        return result;
    }

    result.success = true;
    return result;
}

PipelineResult generate_ir_from_parser_ast(const ::ASTNode* parser_root)
{
    PipelineResult result;
    if (parser_root == nullptr)
    {
        result.error_text = "parser AST is null";
        return result;
    }

    std::unique_ptr<ir::ASTNode> ir_ast = ir::SyntaxASTAdapter::convert(parser_root);
    if (!ir_ast)
    {
        result.error_text = "failed to convert parser AST to IR AST";
        return result;
    }

    return generate_ir_from_ir_ast(ir_ast.get());
}

PipelineResult generate_ir_from_tokens(const std::vector<Token>& tokens)
{
    PipelineResult result;
    SyntaxParser parser;
    if (!parser.parse(tokens))
    {
        result.error_text = "parser.parse(tokens) failed";
        return result;
    }

    ::ASTNode* root = parser.get_ast_tree();
    if (root == nullptr)
    {
        result.error_text = "parser produced null AST";
        return result;
    }

    return generate_ir_from_parser_ast(root);
}

PipelineResult generate_ir_from_source_stream(std::istream& input)
{
    PipelineResult result;
    std::vector<Token> tokens = classify_typedef_names(tokenize(input));

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == "ERROR")
        {
            result.error_text = "lexical error near: " + tokens[i].value;
            return result;
        }
    }

    return generate_ir_from_tokens(tokens);
}

PipelineResult generate_ir_from_source_file(const std::string& file_path)
{
    PipelineResult result;
    std::ifstream input(file_path);
    if (!input.is_open())
    {
        result.error_text = "failed to open source file: " + file_path;
        return result;
    }
    return generate_ir_from_source_stream(input);
}

PipelineDebugResult analyze_source_file_with_debug(const std::string& file_path)
{
    PipelineDebugResult debug;

    std::ifstream input(file_path);
    if (!input.is_open())
    {
        debug.pipeline.error_text = "failed to open source file: " + file_path;
        return debug;
    }

    std::vector<Token> tokens = classify_typedef_names(tokenize(input));
    debug.lexer_output = dump_tokens(tokens);

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == "ERROR")
        {
            debug.pipeline.error_text = "lexical error near: " + tokens[i].value;
            debug.yacc_output = "(parser skipped due to lexical error)";
            debug.ir_input_ast_output = "(none)";
            return debug;
        }
    }

    SyntaxParser parser;
    if (!parser.parse(tokens))
    {
        debug.pipeline.error_text = "parser.parse(tokens) failed";
        debug.yacc_output = "(parse failed)";
        debug.ir_input_ast_output = "(none)";
        return debug;
    }

    ::ASTNode* root = parser.get_ast_tree();
    debug.yacc_output = dump_parser_ast(root);
    debug.ir_input_ast_output = debug.yacc_output;

    if (root == nullptr)
    {
        debug.pipeline.error_text = "parser produced null AST";
        return debug;
    }

    debug.pipeline = generate_ir_from_parser_ast(root);
    return debug;
}

}  // namespace ir

namespace {

int run_source_file(const char* path)
{
    const ir::PipelineResult ir_result = ir::generate_ir_from_source_file(path);
    if (!ir_result.success)
    {
        std::cerr << "Pipeline failed: " << ir_result.error_text << "\n";
        return 1;
    }

    std::cout << "IR Quadruples:\n" << ir::dump_quadruples(ir_result.code) << "\n";
    return 0;
}

int run_source_file_trace(const char* path)
{
    const ir::PipelineDebugResult debug = ir::analyze_source_file_with_debug(path);

    std::cout << "LEX Output:\n" << debug.lexer_output << "\n";
    std::cout << "YACC Output (Parser AST):\n" << debug.yacc_output << "\n";
    std::cout << "IR Input AST:\n" << debug.ir_input_ast_output << "\n";
    std::cout << "IR Quadruples:\n" << ir::dump_quadruples(debug.pipeline.code) << "\n";

    if (!debug.pipeline.success)
    {
        std::cerr << "Pipeline failed: " << debug.pipeline.error_text << "\n";
        return 1;
    }
    return 0;
}

}  // namespace

int main(int argc, char* argv[])
{
    if (argc > 1) {
        if (std::strcmp(argv[1], "--unit") == 0) {
            return run_ir_unit_tests();
        }
        if (std::strcmp(argv[1], "--system") == 0) {
            return run_system_integration_tests();
        }
        if (argc > 2 && std::strcmp(argv[1], "--trace") == 0) {
            return run_source_file_trace(argv[2]);
        }
        return run_source_file(argv[1]);
    }

    return run_system_integration_tests();
}
