#pragma once

#include "token.h"
#include "ir/ast.h"
#include "ir/code_emitter.h"

#include <istream>
#include <string>
#include <vector>

struct ASTNode;

namespace ir {

struct PipelineResult {
    bool success = false;
    std::vector<Quadruple> code;
    std::string error_text;
};

struct PipelineDebugResult {
    PipelineResult pipeline;
    std::string lexer_output;
    std::string yacc_output;
    std::string ir_input_ast_output;
};

PipelineResult generate_ir_from_tokens(const std::vector<Token>& tokens);
PipelineResult generate_ir_from_source_stream(std::istream& input);
PipelineResult generate_ir_from_source_file(const std::string& file_path);
PipelineResult generate_ir_from_parser_ast(const ::ASTNode* parser_root);
PipelineResult generate_ir_from_ir_ast(const ir::ASTNode* ir_root);
PipelineDebugResult analyze_source_file_with_debug(const std::string& file_path);
std::string dump_quadruples(const std::vector<Quadruple>& code);

}  // namespace ir
