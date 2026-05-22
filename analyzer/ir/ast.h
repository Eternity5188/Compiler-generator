#pragma once

#include "symbol_table.h"

#include <memory>
#include <string>
#include <vector>

namespace ir {

enum class ASTNodeType {
    UNKNOWN,
    PROGRAM,
    FUNC_DEF,
    DECL,
    PARAM_DECL,
    BLOCK,
    ASSIGN,
    EXPR_STMT,
    EMPTY_STMT,
    RETURN_STMT,
    BREAK_STMT,
    CONTINUE_STMT,
    IF_STMT,
    WHILE_STMT,
    DO_WHILE_STMT,
    FOR_STMT,
    BINOP,
    UNOP,
    INC_DEC,
    CAST,
    COND_EXPR,
    COMMA_EXPR,
    FUNC_CALL,
    ARG_LIST,
    ARRAY_ACCESS,
    ARRAY_INIT,
    IDENT,
    INT_LITERAL,
    FLOAT_LITERAL,
    CHAR_LITERAL,
    STRING_LITERAL,
};

enum class OpType {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LT,
    LE,
    GT,
    GE,
    EQ,
    NE,
    AND,
    OR,
    NOT,
    UMINUS,
    UPLUS,
    ADDR,
    DEREF,
    INC,
    DEC,
    ASSIGN,
    CAST,
    UNKNOWN,
};

struct ASTNode {
    ASTNodeType type = ASTNodeType::UNKNOWN;
    std::string lexeme;
    std::string source_type;
    OpType op = OpType::UNKNOWN;
    DataType data_type = DataType::UNKNOWN;
    int line_no = 0;
    int col_no = 0;
    std::vector<std::unique_ptr<ASTNode>> children;

    std::string place;
    std::vector<int> true_list;
    std::vector<int> false_list;
    std::vector<int> break_list;
    std::vector<int> continue_list;
    int width = 0;
    bool is_lvalue = false;

    DataType array_elem_type = DataType::UNKNOWN;
    int array_size = 0;

    bool is_prefix = true;
    int mid_label = -1;

    ASTNode() = default;
    explicit ASTNode(ASTNodeType t) : type(t) {}
    ASTNode(ASTNodeType t, std::string text) : type(t), lexeme(std::move(text)) {}

    ASTNode* add_child(std::unique_ptr<ASTNode> child) {
        children.push_back(std::move(child));
        return children.back().get();
    }
};

}  // namespace ir
