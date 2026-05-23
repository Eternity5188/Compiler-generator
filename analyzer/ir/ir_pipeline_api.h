#pragma once

#include "token.h"
#include "ir/ast.h"
#include "ir/code_emitter.h"

#include <string>
#include <vector>

struct ASTNode;

namespace ir {

struct PipelineResult {
    bool success = false;
    std::vector<Quadruple> code;
    std::string error_text;
};

PipelineResult generate_ir_from_tokens(const std::vector<Token>& tokens);
PipelineResult generate_ir_from_parser_ast(const ::ASTNode* parser_root);
PipelineResult generate_ir_from_ir_ast(const ir::ASTNode* ir_root);
std::string dump_quadruples(const std::vector<Quadruple>& code);

}  // namespace ir
