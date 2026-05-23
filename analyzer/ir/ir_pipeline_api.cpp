#include "ir/ir_pipeline_api.h"

#include "syntax_parser.h"
#include "ir/ir_generator.h"
#include "ir/syntax_ast_adapter.h"

#include <sstream>

namespace ir {

std::string dump_quadruples(const std::vector<Quadruple>& code) {
    std::ostringstream oss;
    bool first = true;
    for (std::size_t i = 0; i < code.size(); ++i) {
        if (code[i].op == "nop") {
            continue;
        }
        if (!first) {
            oss << " | ";
        }
        first = false;
        oss << "(" << code[i].op << ", " << code[i].arg1 << ", " << code[i].arg2 << ", " << code[i].result << ")";
    }
    if (first) {
        return "(none)";
    }
    return oss.str();
}

PipelineResult generate_ir_from_ir_ast(const ir::ASTNode* ir_root) {
    PipelineResult result;
    if (ir_root == nullptr) {
        result.error_text = "IR AST is null";
        return result;
    }

    IRGenerator generator;
    const std::vector<Quadruple>& code_ref = generator.generate(ir_root);
    result.code = code_ref;

    if (generator.errors().has_error()) {
        std::ostringstream oss;
        generator.errors().print_all(oss);
        result.error_text = oss.str();
        result.success = false;
        return result;
    }

    result.success = true;
    return result;
}

PipelineResult generate_ir_from_parser_ast(const ::ASTNode* parser_root) {
    PipelineResult result;
    if (parser_root == nullptr) {
        result.error_text = "parser AST is null";
        return result;
    }

    std::unique_ptr<ir::ASTNode> ir_ast = ir::SyntaxASTAdapter::convert(parser_root);
    if (!ir_ast) {
        result.error_text = "failed to convert parser AST to IR AST";
        return result;
    }

    return generate_ir_from_ir_ast(ir_ast.get());
}

PipelineResult generate_ir_from_tokens(const std::vector<Token>& tokens) {
    PipelineResult result;
    SyntaxParser parser;
    if (!parser.parse(tokens)) {
        result.error_text = "parser.parse(tokens) failed";
        return result;
    }

    ::ASTNode* root = parser.get_ast_tree();
    if (root == nullptr) {
        result.error_text = "parser produced null AST";
        return result;
    }

    return generate_ir_from_parser_ast(root);
}

}  // namespace ir
