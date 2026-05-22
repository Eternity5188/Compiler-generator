#pragma once

#include "ast.h"
#include "code_emitter.h"
#include "semantic_error.h"
#include "symbol_table.h"

#include <string>
#include <vector>

namespace ir {

class IRGenerator {
public:
    IRGenerator();

    const std::vector<Quadruple>& generate(const ASTNode* root);
    void reset();

    const ErrorHandler& errors() const;
    const CodeEmitter& emitter() const;

private:
    struct ExprResult {
        std::string place;
        DataType type = DataType::UNKNOWN;
        bool is_lvalue = false;
    };

    void visit_stmt(const ASTNode* node);
    ExprResult visit_expr(const ASTNode* node);

    void visit_program(const ASTNode* node);
    void visit_block(const ASTNode* node);
    void visit_decl(const ASTNode* node);
    void visit_assign(const ASTNode* node);
    void visit_return(const ASTNode* node);
    void visit_if(const ASTNode* node);
    void visit_while(const ASTNode* node);
    void visit_break(const ASTNode* node);
    void visit_continue(const ASTNode* node);

    void report(ErrorType type, const std::string& message, const ASTNode* node) const;
    std::string op_to_ir(OpType op) const;

private:
    mutable ErrorHandler errors_;
    CodeEmitter emitter_;
    SymbolTable global_scope_;
    SymbolTable* current_scope_;
};

}  // namespace ir
