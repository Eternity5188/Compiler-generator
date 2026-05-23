#pragma once

#include "ir/ast.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>
#include <memory>
#include <string>
#include <unordered_map>

namespace ir {

// SyntaxASTAdapter converts parser-generated AST nodes into IR AST nodes.
// It is template-based to avoid hard dependency on generated syntax_parser.h.
class SyntaxASTAdapter {
public:
    template <typename ParserNode>
    static std::unique_ptr<ASTNode> convert(const ParserNode* parser_root) {
        std::unordered_set<const ParserNode*> visited;
        return convert_impl(parser_root, visited);
    }

private:
    template <typename ParserNode>
    static std::unique_ptr<ASTNode> convert_impl(
        const ParserNode* parser_root,
        std::unordered_set<const ParserNode*>& visited) {
        if (parser_root == nullptr) {
            return std::unique_ptr<ASTNode>();
        }

        if (visited.find(parser_root) != visited.end()) {
            std::unique_ptr<ASTNode> cycle_node(new ASTNode(ASTNodeType::UNKNOWN, "<cycle>"));
            cycle_node->source_type = parser_root->type;
            return cycle_node;
        }
        visited.insert(parser_root);

        std::unique_ptr<ASTNode> node(
            new ASTNode(classify_node_type(parser_root), parser_root->value));

        node->source_type = parser_root->type;
        node->op = map_op_type(parser_root->value);
        node->data_type = infer_literal_data_type(node->type, parser_root->value);

        for (std::size_t i = 0; i < parser_root->children.size(); ++i) {
            const ParserNode* child = parser_root->children[i];
            node->add_child(convert_impl(child, visited));
        }

        visited.erase(parser_root);

        return node;
    }
    // Map known grammar/token node names from syntax_rule.txt.
    static ASTNodeType map_known_node_type(const std::string& raw_type) {
        const std::string key = normalize(raw_type);

        static const std::unordered_map<std::string, ASTNodeType> table = {
            {"program", ASTNodeType::PROGRAM},
            {"function_definition", ASTNodeType::FUNC_DEF},
            {"declaration", ASTNodeType::DECL},
            {"parameter_declaration", ASTNodeType::PARAM_DECL},
            {"compound_statement", ASTNodeType::BLOCK},
            {"jump_statement", ASTNodeType::RETURN_STMT},
            {"expression_statement", ASTNodeType::EXPR_STMT},
            {"selection_statement", ASTNodeType::IF_STMT},
            {"iteration_statement", ASTNodeType::WHILE_STMT},
            {"assignment_expression", ASTNodeType::ASSIGN},
            {"conditional_expression", ASTNodeType::COND_EXPR},
            {"logical_or_expression", ASTNodeType::BINOP},
            {"logical_and_expression", ASTNodeType::BINOP},
            {"equality_expression", ASTNodeType::BINOP},
            {"relational_expression", ASTNodeType::BINOP},
            {"additive_expression", ASTNodeType::BINOP},
            {"multiplicative_expression", ASTNodeType::BINOP},
            {"unary_expression", ASTNodeType::UNOP},
            {"cast_expression", ASTNodeType::CAST},
            {"postfix_expression", ASTNodeType::FUNC_CALL},
            {"argument_expression_list", ASTNodeType::ARG_LIST},
            {"identifier", ASTNodeType::IDENT},
            {"number", ASTNodeType::INT_LITERAL},
            {"string_literal", ASTNodeType::STRING_LITERAL},
            {"string", ASTNodeType::STRING_LITERAL},
            // Token-level node names emitted by generated parser.
            {"if", ASTNodeType::IF_STMT},
            {"while", ASTNodeType::WHILE_STMT},
            {"for", ASTNodeType::FOR_STMT},
            {"do", ASTNodeType::DO_WHILE_STMT},
            {"return", ASTNodeType::RETURN_STMT},
            {"break", ASTNodeType::BREAK_STMT},
            {"continue", ASTNodeType::CONTINUE_STMT},
        };

        const std::unordered_map<std::string, ASTNodeType>::const_iterator it = table.find(key);
        if (it != table.end()) {
            return it->second;
        }

        if (key == "int" || key == "float" || key == "char" || key == "void" ||
            key == "type_specifier") {
            return ASTNodeType::DECL;
        }

        return ASTNodeType::UNKNOWN;
    }

    template <typename ParserNode>
    static bool has_child_type(const ParserNode* node, const std::string& expected_normalized_type) {
        for (std::size_t i = 0; i < node->children.size(); ++i) {
            const ParserNode* child = node->children[i];
            if (normalize(child->type) == expected_normalized_type) {
                return true;
            }
        }
        return false;
    }

    template <typename ParserNode>
    static ASTNodeType classify_node_type(const ParserNode* node) {
        const std::string key = normalize(node->type);
        ASTNodeType mapped = map_known_node_type(node->type);

        if (mapped != ASTNodeType::UNKNOWN) {
            if (key == "jump_statement") {
                if (has_child_type(node, "break")) {
                    return ASTNodeType::BREAK_STMT;
                }
                if (has_child_type(node, "continue")) {
                    return ASTNodeType::CONTINUE_STMT;
                }
                if (has_child_type(node, "return")) {
                    return ASTNodeType::RETURN_STMT;
                }
            }

            if (key == "iteration_statement") {
                if (has_child_type(node, "while")) {
                    return ASTNodeType::WHILE_STMT;
                }
                if (has_child_type(node, "for")) {
                    return ASTNodeType::FOR_STMT;
                }
                if (has_child_type(node, "do")) {
                    return ASTNodeType::DO_WHILE_STMT;
                }
            }

            if (key == "postfix_expression") {
                if (has_child_type(node, "inc") || has_child_type(node, "dec")) {
                    return ASTNodeType::INC_DEC;
                }
                if (has_child_type(node, "lbracket")) {
                    return ASTNodeType::ARRAY_ACCESS;
                }
                if (has_child_type(node, "lparen")) {
                    return ASTNodeType::FUNC_CALL;
                }
            }

            if (key == "expression" && has_child_type(node, "comma")) {
                return ASTNodeType::COMMA_EXPR;
            }

            if (key == "assignment_expression" && !has_child_type(node, "assign")) {
                return ASTNodeType::EXPR_STMT;
            }

            return mapped;
        }

        // Conservative fallback: keep structural nodes as containers.
        if (key == "statement" || key == "statement_list" || key == "external_declaration_list" ||
            key == "external_declaration" || key == "initializer" || key == "argument_list" ||
            key == "parameter_list" || key == "primary_expression") {
            return ASTNodeType::UNKNOWN;
        }
        return ASTNodeType::UNKNOWN;
    }

    static OpType map_op_type(const std::string& raw_op) {
        const std::string op = normalize(raw_op);
        if (op == "+") return OpType::ADD;
        if (op == "-") return OpType::SUB;
        if (op == "*") return OpType::MUL;
        if (op == "/") return OpType::DIV;
        if (op == "%") return OpType::MOD;
        if (op == "<") return OpType::LT;
        if (op == "<=") return OpType::LE;
        if (op == ">") return OpType::GT;
        if (op == ">=") return OpType::GE;
        if (op == "==") return OpType::EQ;
        if (op == "!=") return OpType::NE;
        if (op == "&&") return OpType::AND;
        if (op == "||") return OpType::OR;
        if (op == "!") return OpType::NOT;
        if (op == "++") return OpType::INC;
        if (op == "--") return OpType::DEC;
        if (op == "=") return OpType::ASSIGN;
        return OpType::UNKNOWN;
    }

    static DataType infer_literal_data_type(ASTNodeType type, const std::string& literal) {
        (void)literal;
        switch (type) {
            case ASTNodeType::INT_LITERAL:
                return DataType::INT;
            case ASTNodeType::FLOAT_LITERAL:
                return DataType::FLOAT;
            case ASTNodeType::CHAR_LITERAL:
                return DataType::CHAR;
            case ASTNodeType::STRING_LITERAL:
                return DataType::POINTER;
            default:
                return DataType::UNKNOWN;
        }
    }

    static std::string normalize(const std::string& s) {
        std::string out;
        out.reserve(s.size());

        for (std::size_t i = 0; i < s.size(); ++i) {
            const unsigned char ch = static_cast<unsigned char>(s[i]);
            if (std::isalnum(ch) || ch == '_' || is_operator_char(ch)) {
                out.push_back(static_cast<char>(std::tolower(ch)));
            }
        }
        return out;
    }

    static bool is_operator_char(unsigned char ch) {
        return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' ||
               ch == '<' || ch == '>' || ch == '=' || ch == '!' || ch == '&' || ch == '|';
    }
};

}  // namespace ir
