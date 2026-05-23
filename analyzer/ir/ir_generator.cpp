#include "ir/ir_generator.h"

#include <cctype>
#include <utility>

namespace ir {

namespace {

std::string normalize_key(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(s[i]);
        if (std::isalnum(ch) || ch == '_') {
            out.push_back(static_cast<char>(std::tolower(ch)));
        }
    }
    return out;
}

const ASTNode* find_first_by_source_type(const ASTNode* node, const std::string& wanted_normalized) {
    if (node == nullptr) {
        return nullptr;
    }

    if (normalize_key(node->source_type) == wanted_normalized) {
        return node;
    }

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        const ASTNode* found = find_first_by_source_type(node->children[i].get(), wanted_normalized);
        if (found != nullptr) {
            return found;
        }
    }
    return nullptr;
}

std::string find_decl_identifier(const ASTNode* node) {
    if (node == nullptr) {
        return "";
    }

    if (node->type == ASTNodeType::IDENT && !node->lexeme.empty()) {
        return node->lexeme;
    }

    if (normalize_key(node->source_type) == "identifier" && !node->lexeme.empty()) {
        return node->lexeme;
    }

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        std::string name = find_decl_identifier(node->children[i].get());
        if (!name.empty()) {
            return name;
        }
    }

    return "";
}

std::string find_last_identifier(const ASTNode* node) {
    if (node == nullptr) {
        return "";
    }

    std::string best;
    if (node->type == ASTNodeType::IDENT && !node->lexeme.empty()) {
        best = node->lexeme;
    } else if (normalize_key(node->source_type) == "identifier" && !node->lexeme.empty()) {
        best = node->lexeme;
    }

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        std::string cur = find_last_identifier(node->children[i].get());
        if (!cur.empty()) {
            best = cur;
        }
    }

    return best;
}

std::string find_decl_identifier_prefer_declarator(const ASTNode* node) {
    if (node == nullptr) {
        return "";
    }

    const ASTNode* declarator = find_first_by_source_type(node, "declarator");
    if (declarator != nullptr) {
        std::string n = find_last_identifier(declarator);
        if (!n.empty()) {
            return n;
        }
    }

    const ASTNode* direct = find_first_by_source_type(node, "directdeclarator");
    if (direct != nullptr) {
        std::string n = find_last_identifier(direct);
        if (!n.empty()) {
            return n;
        }
    }

    return find_last_identifier(node);
}

void collect_nodes_by_type(const ASTNode* node, ASTNodeType wanted_type, std::vector<const ASTNode*>& out) {
    if (node == nullptr) {
        return;
    }

    if (node->type == wanted_type) {
        out.push_back(node);
    }

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        collect_nodes_by_type(node->children[i].get(), wanted_type, out);
    }
}

void collect_nodes_by_source_type(
    const ASTNode* node,
    const std::string& wanted_normalized,
    std::vector<const ASTNode*>& out) {
    if (node == nullptr) {
        return;
    }

    if (normalize_key(node->source_type) == wanted_normalized) {
        out.push_back(node);
    }

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        collect_nodes_by_source_type(node->children[i].get(), wanted_normalized, out);
    }
}

DataType infer_decl_type(const ASTNode* node) {
    const ASTNode* type_node = find_first_by_source_type(node, "typespecifier");
    if (type_node == nullptr) {
        return DataType::UNKNOWN;
    }

    const ASTNode* int_node = find_first_by_source_type(type_node, "int");
    if (int_node != nullptr) {
        return DataType::INT;
    }

    const ASTNode* float_node = find_first_by_source_type(type_node, "float");
    if (float_node != nullptr) {
        return DataType::FLOAT;
    }

    const ASTNode* char_node = find_first_by_source_type(type_node, "char");
    if (char_node != nullptr) {
        return DataType::CHAR;
    }

    const ASTNode* void_node = find_first_by_source_type(type_node, "void");
    if (void_node != nullptr) {
        return DataType::VOID_TYPE;
    }

    return DataType::UNKNOWN;
}

}  // namespace

IRGenerator::IRGenerator()
    : global_scope_("global", nullptr), current_scope_(&global_scope_) {}

const std::vector<Quadruple>& IRGenerator::generate(const ASTNode* root) {
    reset();
    if (root == nullptr) {
        return emitter_.get_code();
    }

    visit_stmt(root);
    return emitter_.get_code();
}

void IRGenerator::reset() {
    errors_.clear();
    emitter_.clear();
    global_scope_ = SymbolTable("global", nullptr);
    current_scope_ = &global_scope_;
}

const ErrorHandler& IRGenerator::errors() const {
    return errors_;
}

const CodeEmitter& IRGenerator::emitter() const {
    return emitter_;
}

void IRGenerator::visit_stmt(const ASTNode* node) {
    if (node == nullptr) {
        return;
    }

    switch (node->type) {
        case ASTNodeType::PROGRAM:
            visit_program(node);
            break;
        case ASTNodeType::BLOCK:
            visit_block(node);
            break;
        case ASTNodeType::DECL:
            visit_decl(node);
            break;
        case ASTNodeType::FUNC_DEF:
            visit_func_def(node);
            break;
        case ASTNodeType::ASSIGN:
            visit_assign(node);
            break;
        case ASTNodeType::RETURN_STMT:
            visit_return(node);
            break;
        case ASTNodeType::IF_STMT:
            visit_if(node);
            break;
        case ASTNodeType::WHILE_STMT:
            visit_while(node);
            break;
        case ASTNodeType::BREAK_STMT:
            visit_break(node);
            break;
        case ASTNodeType::CONTINUE_STMT:
            visit_continue(node);
            break;
        case ASTNodeType::EXPR_STMT:
            if (!node->children.empty()) {
                static_cast<void>(visit_expr(node->children[0].get()));
            }
            break;
        case ASTNodeType::EMPTY_STMT:
            break;
        default:
            // Unhandled statement node kinds can still contain executable children.
            for (const auto& child : node->children) {
                visit_stmt(child.get());
            }
            break;
    }
}

IRGenerator::ExprResult IRGenerator::visit_expr(const ASTNode* node) {
    if (node == nullptr) {
        return {};
    }

    switch (node->type) {
        case ASTNodeType::IDENT: {
            SymbolEntry* entry = current_scope_->lookup(node->lexeme);
            if (entry == nullptr) {
                report(ErrorType::UNDECLARED, "use of undeclared identifier: " + node->lexeme, node);
                return {};
            }
            return ExprResult{entry->name, entry->type, true};
        }
        case ASTNodeType::INT_LITERAL:
        case ASTNodeType::FLOAT_LITERAL:
        case ASTNodeType::CHAR_LITERAL:
        case ASTNodeType::STRING_LITERAL:
            return ExprResult{node->lexeme, node->data_type, false};

        case ASTNodeType::BINOP: {
            std::vector<ExprResult> operands;
            operands.reserve(node->children.size());
            for (std::size_t i = 0; i < node->children.size(); ++i) {
                ExprResult cur = visit_expr(node->children[i].get());
                if (!cur.place.empty()) {
                    operands.push_back(cur);
                }
            }

            if (node->op == OpType::UNKNOWN) {
                if (operands.empty()) {
                    return {};
                }
                return operands.back();
            }

            if (operands.size() == 1) {
                return operands[0];
            }

            if (operands.size() < 2) {
                report(ErrorType::TYPE_MISMATCH, "binary op must have exactly 2 operands", node);
                return {};
            }

            ExprResult acc = operands[0];
            for (std::size_t i = 1; i < operands.size(); ++i) {
                std::string tmp = emitter_.new_temp();
                emitter_.emit(op_to_ir(node->op), acc.place, operands[i].place, tmp);
                acc = ExprResult{tmp, acc.type, false};
            }
            return acc;
        }

        case ASTNodeType::UNOP: {
            std::vector<ExprResult> operands;
            operands.reserve(node->children.size());
            for (std::size_t i = 0; i < node->children.size(); ++i) {
                ExprResult cur = visit_expr(node->children[i].get());
                if (!cur.place.empty()) {
                    operands.push_back(cur);
                }
            }

            if (node->op == OpType::UNKNOWN) {
                if (operands.empty()) {
                    return {};
                }
                return operands.back();
            }

            if (operands.empty()) {
                report(ErrorType::TYPE_MISMATCH, "unary op must have exactly 1 operand", node);
                return {};
            }

            ExprResult arg = operands.back();
            std::string tmp = emitter_.new_temp();
            emitter_.emit(op_to_ir(node->op), arg.place, "-", tmp);
            return ExprResult{tmp, arg.type, false};
        }

        case ASTNodeType::ASSIGN:
            visit_assign(node);
            if (!node->children.empty()) {
                return visit_expr(node->children[0].get());
            }
            return {};

        case ASTNodeType::FUNC_CALL: {
            std::string callee = find_decl_identifier(node);
            if (callee.empty()) {
                callee = "<anon>";
            }

            int arg_count = 0;
            for (std::size_t i = 0; i < node->children.size(); ++i) {
                const ASTNode* child = node->children[i].get();
                if (child == nullptr) {
                    continue;
                }
                const std::string src = normalize_key(child->source_type);
                if (src == "identifier" && child->lexeme == callee) {
                    continue;
                }
                ExprResult arg = visit_expr(child);
                if (!arg.place.empty()) {
                    ++arg_count;
                }
            }

            std::string tmp = emitter_.new_temp();
            emitter_.emit("call", callee, std::to_string(arg_count), tmp);
            return ExprResult{tmp, DataType::INT, false};
        }

        default:
            if (!node->children.empty()) {
                return visit_expr(node->children.front().get());
            }
            return {};
    }
}

void IRGenerator::visit_program(const ASTNode* node) {
    for (const auto& child : node->children) {
        visit_stmt(child.get());
    }
}

void IRGenerator::visit_block(const ASTNode* node) {
    SymbolTable* previous = current_scope_;
    current_scope_ = current_scope_->enter_scope("block");

    for (const auto& child : node->children) {
        visit_stmt(child.get());
    }

    current_scope_ = current_scope_->exit_scope();
    if (current_scope_ == nullptr) {
        current_scope_ = previous;
    }
}

void IRGenerator::visit_decl(const ASTNode* node) {
    std::string decl_name = node->lexeme;
    if (decl_name.empty()) {
        decl_name = find_decl_identifier_prefer_declarator(node);
    }

    if (decl_name.empty()) {
        report(ErrorType::REDEFINED, "declaration has empty identifier", node);
        return;
    }

    SymbolEntry entry;
    entry.name = decl_name;
    entry.type = node->data_type;
    if (entry.type == DataType::UNKNOWN) {
        entry.type = infer_decl_type(node);
    }
    if (entry.type == DataType::UNKNOWN) {
        entry.type = DataType::INT;
    }
    entry.width = get_type_width(node->data_type);
    if (entry.width <= 0) {
        entry.width = get_type_width(entry.type);
    }
    if (entry.width <= 0) {
        entry.width = 4;
    }
    entry.offset = current_scope_->allocate(entry.width);
    entry.line_no = node->line_no;

    if (!current_scope_->insert(entry)) {
        report(ErrorType::REDEFINED, "redefinition of identifier: " + decl_name, node);
        return;
    }

    const ASTNode* initializer = find_first_by_source_type(node, "initializer");
    if (initializer != nullptr && !initializer->children.empty()) {
        ExprResult init_value = visit_expr(initializer->children[0].get());
        if (!init_value.place.empty()) {
            emitter_.emit(":=", init_value.place, "-", decl_name);
        }
    }
}

void IRGenerator::visit_func_def(const ASTNode* node) {
    std::string func_name = find_decl_identifier(node);
    if (func_name.empty()) {
        func_name = "<anonymous_func>";
    }

    SymbolEntry fn_entry;
    fn_entry.name = func_name;
    fn_entry.type = DataType::INT;
    fn_entry.width = 4;
    fn_entry.offset = global_scope_.allocate(fn_entry.width);
    fn_entry.line_no = node->line_no;
    static_cast<void>(global_scope_.insert(fn_entry));

    SymbolTable* previous = current_scope_;
    current_scope_ = current_scope_->enter_scope("func");

    std::vector<const ASTNode*> params;
    collect_nodes_by_type(node, ASTNodeType::PARAM_DECL, params);
    if (params.empty()) {
        collect_nodes_by_source_type(node, "parameterdeclaration", params);
    }
    for (std::size_t i = 0; i < params.size(); ++i) {
        std::string param_name = find_decl_identifier_prefer_declarator(params[i]);
        if (param_name.empty()) {
            continue;
        }

        SymbolEntry p;
        p.name = param_name;
        p.type = infer_decl_type(params[i]);
        if (p.type == DataType::UNKNOWN) {
            p.type = DataType::INT;
        }
        p.width = get_type_width(p.type);
        if (p.width <= 0) {
            p.width = 4;
        }
        p.offset = current_scope_->allocate(p.width);
        p.line_no = params[i]->line_no;
        static_cast<void>(current_scope_->insert(p));
    }

    bool visited_body = false;
    for (std::size_t i = 0; i < node->children.size(); ++i) {
        const ASTNode* child = node->children[i].get();
        if (child == nullptr) {
            continue;
        }
        if (child->type == ASTNodeType::BLOCK || normalize_key(child->source_type) == "compoundstatement") {
            visit_stmt(child);
            visited_body = true;
            break;
        }
    }

    if (!visited_body) {
        for (std::size_t i = 0; i < node->children.size(); ++i) {
            visit_stmt(node->children[i].get());
        }
    }

    current_scope_ = current_scope_->exit_scope();
    if (current_scope_ == nullptr) {
        current_scope_ = previous;
    }
}

void IRGenerator::visit_assign(const ASTNode* node) {
    ExprResult lhs;
    ExprResult rhs;
    bool lhs_set = false;

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        ExprResult cur = visit_expr(node->children[i].get());
        if (cur.place.empty()) {
            continue;
        }
        if (!lhs_set) {
            lhs = cur;
            lhs_set = true;
        } else {
            rhs = cur;
        }
    }

    if (!lhs_set || rhs.place.empty()) {
        report(ErrorType::TYPE_MISMATCH, "assignment must have lhs and rhs", node);
        return;
    }

    if (!lhs.is_lvalue) {
        const bool literal_like = !lhs.place.empty() &&
            (std::isdigit(static_cast<unsigned char>(lhs.place[0])) || lhs.place[0] == '\'' || lhs.place[0] == '"');
        if (literal_like) {
            report(ErrorType::NOT_LVALUE, "left side of assignment is not assignable", node->children[0].get());
            return;
        }
    }

    emitter_.emit(":=", rhs.place, "-", lhs.place);
}

void IRGenerator::visit_return(const ASTNode* node) {
    if (node->children.empty()) {
        emitter_.emit("return", "-", "-", "-");
        return;
    }

    ExprResult value = visit_expr(node->children[0].get());
    emitter_.emit("return", value.place, "-", "-");
}

void IRGenerator::visit_if(const ASTNode* node) {
    if (node->children.size() < 2) {
        report(ErrorType::TYPE_MISMATCH, "if statement must have condition and then-branch", node);
        return;
    }

    ExprResult cond = visit_expr(node->children[0].get());
    const std::string else_label = emitter_.new_label();
    const std::string end_label = emitter_.new_label();

    emitter_.emit("if_false", cond.place, "-", else_label);
    visit_stmt(node->children[1].get());
    emitter_.emit("goto", "-", "-", end_label);
    emitter_.emit("label", "-", "-", else_label);

    if (node->children.size() >= 3) {
        visit_stmt(node->children[2].get());
    }

    emitter_.emit("label", "-", "-", end_label);
}

void IRGenerator::visit_while(const ASTNode* node) {
    if (node->children.size() < 2) {
        report(ErrorType::TYPE_MISMATCH, "while statement must have condition and body", node);
        return;
    }

    const std::string begin_label = emitter_.new_label();
    const std::string end_label = emitter_.new_label();

    emitter_.emit("label", "-", "-", begin_label);
    ExprResult cond = visit_expr(node->children[0].get());
    emitter_.emit("if_false", cond.place, "-", end_label);

    current_scope_->enter_loop(end_label, begin_label);
    visit_stmt(node->children[1].get());
    current_scope_->exit_loop();

    emitter_.emit("goto", "-", "-", begin_label);
    emitter_.emit("label", "-", "-", end_label);
}

void IRGenerator::visit_break(const ASTNode* node) {
    const std::string break_target = current_scope_->loop_exit_label();
    if (break_target.empty()) {
        report(ErrorType::BREAK_OUTSIDE_LOOP, "break used outside of loop", node);
        return;
    }

    emitter_.emit("goto", "-", "-", break_target);
}

void IRGenerator::visit_continue(const ASTNode* node) {
    const std::string continue_target = current_scope_->loop_next_iter_label();
    if (continue_target.empty()) {
        report(ErrorType::CONTINUE_OUTSIDE_LOOP, "continue used outside of loop", node);
        return;
    }

    emitter_.emit("goto", "-", "-", continue_target);
}

void IRGenerator::report(ErrorType type, const std::string& message, const ASTNode* node) const {
    const int line = node == nullptr ? 0 : node->line_no;
    const int col = node == nullptr ? 0 : node->col_no;
    errors_.add_error(type, message, line, col);
}

std::string IRGenerator::op_to_ir(OpType op) const {
    switch (op) {
        case OpType::ADD:
            return "+";
        case OpType::SUB:
            return "-";
        case OpType::MUL:
            return "*";
        case OpType::DIV:
            return "/";
        case OpType::MOD:
            return "%";
        case OpType::LT:
            return "<";
        case OpType::LE:
            return "<=";
        case OpType::GT:
            return ">";
        case OpType::GE:
            return ">=";
        case OpType::EQ:
            return "==";
        case OpType::NE:
            return "!=";
        case OpType::AND:
            return "&&";
        case OpType::OR:
            return "||";
        case OpType::NOT:
            return "!";
        case OpType::UMINUS:
            return "uminus";
        case OpType::UPLUS:
            return "uplus";
        case OpType::ADDR:
            return "&";
        case OpType::DEREF:
            return "=*";
        case OpType::INC:
            return "++";
        case OpType::DEC:
            return "--";
        case OpType::ASSIGN:
            return ":=";
        case OpType::CAST:
            return "cast";
        case OpType::UNKNOWN:
        default:
            return "nop";
    }
}

}  // namespace ir
