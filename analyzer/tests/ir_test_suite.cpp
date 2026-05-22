#include "ir_test_suite.h"

#include "syntax_parser.h"
#include "../ir/ir_generator.h"
#include "../ir/syntax_ast_adapter.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

using ir::ASTNode;
using ir::ASTNodeType;
using ir::OpType;
using ir::Quadruple;

struct TestResult {
    bool passed = false;
    std::string detail;
};

struct IRRunResult {
    std::vector<Quadruple> code;
    bool has_error = false;
    std::string error_text;
};

struct Case {
    std::string name;
    std::function<TestResult()> run;
};

struct Category {
    std::string name;
    std::vector<Case> cases;
};

std::string g_last_ir_dump = "(none)";

std::string to_chinese_number(int n) {
    static const std::vector<std::string> digits = {
        "零", "一", "二", "三", "四", "五", "六", "七", "八", "九"
    };
    if (n < 10) {
        return digits[n];
    }
    if (n == 10) {
        return "十";
    }
    if (n < 20) {
        return "十" + digits[n - 10];
    }
    if (n < 100) {
        const int tens = n / 10;
        const int ones = n % 10;
        if (ones == 0) {
            return digits[tens] + "十";
        }
        return digits[tens] + "十" + digits[ones];
    }
    return std::to_string(n);
}

std::string format_quad(const Quadruple& q) {
    std::ostringstream oss;
    oss << "(" << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << ")";
    return oss.str();
}

std::string dump_code(const std::vector<Quadruple>& code) {
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
        oss << format_quad(code[i]);
    }
    if (first) {
        return "(none)";
    }
    return oss.str();
}

void reset_last_ir_dump() {
    g_last_ir_dump = "(none)";
}

void record_last_ir_dump(const std::vector<Quadruple>& code) {
    g_last_ir_dump = dump_code(code);
}

const std::string& get_last_ir_dump() {
    return g_last_ir_dump;
}

bool contains_op(const std::vector<Quadruple>& code, const std::string& op) {
    return std::any_of(code.begin(), code.end(), [&](const Quadruple& q) { return q.op == op; });
}

bool contains_quad(const std::vector<Quadruple>& code,
                   const std::string& op,
                   const std::string& arg1,
                   const std::string& arg2,
                   const std::string& result) {
    return std::any_of(code.begin(), code.end(), [&](const Quadruple& q) {
        return q.op == op && q.arg1 == arg1 && q.arg2 == arg2 && q.result == result;
    });
}

std::unique_ptr<ASTNode> make_node(ASTNodeType type, const std::string& lexeme = "") {
    return std::make_unique<ASTNode>(type, lexeme);
}

std::unique_ptr<ASTNode> make_ident(const std::string& name) {
    auto n = make_node(ASTNodeType::IDENT, name);
    n->data_type = ir::DataType::INT;
    return n;
}

std::unique_ptr<ASTNode> make_int_literal(int value) {
    auto n = make_node(ASTNodeType::INT_LITERAL, std::to_string(value));
    n->data_type = ir::DataType::INT;
    return n;
}

std::unique_ptr<ASTNode> make_decl(const std::string& name, std::unique_ptr<ASTNode> init_expr = nullptr) {
    auto decl = make_node(ASTNodeType::DECL, name);
    decl->data_type = ir::DataType::INT;
    if (init_expr) {
        auto init = make_node(ASTNodeType::UNKNOWN);
        init->source_type = "initializer";
        init->add_child(std::move(init_expr));
        decl->add_child(std::move(init));
    }
    return decl;
}

std::unique_ptr<ASTNode> make_binop(OpType op, std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs) {
    auto n = make_node(ASTNodeType::BINOP);
    n->op = op;
    n->add_child(std::move(lhs));
    n->add_child(std::move(rhs));
    return n;
}

std::unique_ptr<ASTNode> make_unop(OpType op, std::unique_ptr<ASTNode> arg) {
    auto n = make_node(ASTNodeType::UNOP);
    n->op = op;
    n->add_child(std::move(arg));
    return n;
}

std::unique_ptr<ASTNode> make_assign(std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs) {
    auto n = make_node(ASTNodeType::ASSIGN);
    n->add_child(std::move(lhs));
    n->add_child(std::move(rhs));
    return n;
}

std::unique_ptr<ASTNode> make_if(std::unique_ptr<ASTNode> cond,
                                 std::unique_ptr<ASTNode> then_stmt,
                                 std::unique_ptr<ASTNode> else_stmt = nullptr) {
    auto n = make_node(ASTNodeType::IF_STMT);
    n->add_child(std::move(cond));
    n->add_child(std::move(then_stmt));
    if (else_stmt) {
        n->add_child(std::move(else_stmt));
    }
    return n;
}

std::unique_ptr<ASTNode> make_while(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> body) {
    auto n = make_node(ASTNodeType::WHILE_STMT);
    n->add_child(std::move(cond));
    n->add_child(std::move(body));
    return n;
}

void add_children(ASTNode*) {}

template <typename First, typename... Rest>
void add_children(ASTNode* parent, First&& first, Rest&&... rest) {
    parent->add_child(std::forward<First>(first));
    add_children(parent, std::forward<Rest>(rest)...);
}

template <typename... Children>
std::unique_ptr<ASTNode> make_program(Children&&... children) {
    auto n = make_node(ASTNodeType::PROGRAM);
    add_children(n.get(), std::forward<Children>(children)...);
    return n;
}

template <typename... Children>
std::unique_ptr<ASTNode> make_block(Children&&... children) {
    auto n = make_node(ASTNodeType::BLOCK);
    add_children(n.get(), std::forward<Children>(children)...);
    return n;
}

IRRunResult run_ir(std::unique_ptr<ASTNode> root) {
    ir::IRGenerator generator;
    const std::vector<Quadruple>& code_ref = generator.generate(root.get());

    std::ostringstream errors;
    if (generator.errors().has_error()) {
        generator.errors().print_all(errors);
    }

    IRRunResult ret;
    ret.code = code_ref;
    ret.has_error = generator.errors().has_error();
    ret.error_text = errors.str();
    record_last_ir_dump(ret.code);
    return ret;
}

std::vector<Token> create_parser_smoke_tokens() {
    return {
        {"INT", "int"},
        {"IDENTIFIER", "a"},
        {"ASSIGN", "="},
        {"NUMBER", "1"},
        {"SEMICOLON", ";"}
    };
}

Category make_parser_category() {
    Category cat;
    cat.name = "语法链路与适配";

    cat.cases.push_back({"Parser+Adapter 冒烟", []() -> TestResult {
        SyntaxParser parser;
        const auto tokens = create_parser_smoke_tokens();
        if (!parser.parse(tokens)) {
            return TestResult{false, "parser 未通过"};
        }

        auto* root = parser.get_ast_tree();
        if (root == nullptr) {
            return TestResult{false, "parser AST 为空"};
        }

        std::unique_ptr<ir::ASTNode> ir_ast = ir::SyntaxASTAdapter::convert(root);
        ir::IRGenerator gen;
        const auto& code = gen.generate(ir_ast.get());
        record_last_ir_dump(code);
        if (gen.errors().has_error()) {
            std::ostringstream oss;
            gen.errors().print_all(oss);
            return TestResult{false, "IR 报错: " + oss.str()};
        }

        if (!contains_quad(code, ":=", "1", "-", "a")) {
            return TestResult{false, "未看到 (:=, 1, -, a)，实际: " + dump_code(code)};
        }
        return TestResult{true, "语法链路和 AST 适配正常"};
    }});

    cat.cases.push_back({"空程序 IR 生成", []() -> TestResult {
        auto program = make_program();
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "空程序不应报错: " + r.error_text};
        }
        return TestResult{true, "空程序可正常生成空 IR"};
    }});

    return cat;
}

Category make_decl_and_assign_category() {
    Category cat;
    cat.name = "声明与赋值";

    cat.cases.push_back({"声明无初始化", []() -> TestResult {
        auto program = make_program(make_decl("a"));
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (contains_op(r.code, ":=")) {
            return TestResult{false, "无初始化不应生成 :=，实际: " + dump_code(r.code)};
        }
        return TestResult{true, "无初始化声明行为正确"};
    }});

    cat.cases.push_back({"声明带初始化", []() -> TestResult {
        auto program = make_program(make_decl("a", make_int_literal(7)));
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "7", "-", "a")) {
            return TestResult{false, "缺少 (:=, 7, -, a)，实际: " + dump_code(r.code)};
        }
        return TestResult{true, "声明初始化正确"};
    }});

    cat.cases.push_back({"多变量声明与初始化", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b", make_int_literal(2)),
            make_decl("c", make_int_literal(3))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "1", "-", "a") ||
            !contains_quad(r.code, ":=", "2", "-", "b") ||
            !contains_quad(r.code, ":=", "3", "-", "c")) {
            return TestResult{false, "多变量初始化不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "多变量初始化完整"};
    }});

    cat.cases.push_back({"简单赋值", []() -> TestResult {
        auto program = make_program(
            make_decl("a"),
            make_assign(make_ident("a"), make_int_literal(9))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "9", "-", "a")) {
            return TestResult{false, "缺少 (:=, 9, -, a): " + dump_code(r.code)};
        }
        return TestResult{true, "简单赋值正确"};
    }});

    cat.cases.push_back({"链式运算后赋值", []() -> TestResult {
        auto expr = make_binop(OpType::MUL,
            make_binop(OpType::ADD, make_ident("a"), make_int_literal(2)),
            make_binop(OpType::SUB, make_ident("b"), make_int_literal(1)));

        auto program = make_program(
            make_decl("a", make_int_literal(3)),
            make_decl("b", make_int_literal(6)),
            make_decl("c"),
            make_assign(make_ident("c"), std::move(expr))
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "+") || !contains_op(r.code, "-") || !contains_op(r.code, "*")) {
            return TestResult{false, "缺少链式运算四元式: " + dump_code(r.code)};
        }
        return TestResult{true, "链式运算赋值正确"};
    }});

    cat.cases.push_back({"赋值给自身", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(5)),
            make_assign(make_ident("a"), make_ident("a"))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "a", "-", "a")) {
            return TestResult{false, "缺少自赋值四元式: " + dump_code(r.code)};
        }
        return TestResult{true, "自赋值路径正确"};
    }});

    cat.cases.push_back({"声明初始化使用表达式", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(2)),
            make_decl("b", make_int_literal(3)),
            make_decl("c", make_binop(OpType::ADD, make_ident("a"), make_ident("b")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "+") || !contains_quad(r.code, ":=", "t0", "-", "c")) {
            return TestResult{false, "表达式初始化不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "表达式初始化声明正确"};
    }});

    cat.cases.push_back({"同变量多次覆盖赋值", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_assign(make_ident("a"), make_int_literal(2)),
            make_assign(make_ident("a"), make_int_literal(3)),
            make_assign(make_ident("a"), make_int_literal(4))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "4", "-", "a")) {
            return TestResult{false, "最终覆盖赋值缺失: " + dump_code(r.code)};
        }
        return TestResult{true, "覆盖赋值链路正确"};
    }});

    cat.cases.push_back({"中间结果再赋值", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(5)),
            make_decl("b", make_int_literal(6)),
            make_decl("c"),
            make_decl("d"),
            make_assign(make_ident("c"), make_binop(OpType::ADD, make_ident("a"), make_ident("b"))),
            make_assign(make_ident("d"), make_ident("c"))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "+") || !contains_quad(r.code, ":=", "c", "-", "d")) {
            return TestResult{false, "中间结果赋值链路不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "中间结果再赋值正常"};
    }});

    return cat;
}

Category make_expression_category() {
    Category cat;
    cat.name = "算术与一元表达式";

    cat.cases.push_back({"加减乘除取模", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(8)),
            make_decl("b", make_int_literal(3)),
            make_decl("r1"), make_decl("r2"), make_decl("r3"), make_decl("r4"), make_decl("r5"),
            make_assign(make_ident("r1"), make_binop(OpType::ADD, make_ident("a"), make_ident("b"))),
            make_assign(make_ident("r2"), make_binop(OpType::SUB, make_ident("a"), make_ident("b"))),
            make_assign(make_ident("r3"), make_binop(OpType::MUL, make_ident("a"), make_ident("b"))),
            make_assign(make_ident("r4"), make_binop(OpType::DIV, make_ident("a"), make_ident("b"))),
            make_assign(make_ident("r5"), make_binop(OpType::MOD, make_ident("a"), make_ident("b")))
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "+") || !contains_op(r.code, "-") || !contains_op(r.code, "*") ||
            !contains_op(r.code, "/") || !contains_op(r.code, "%")) {
            return TestResult{false, "五种算术运算不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "五种算术运算齐全"};
    }});

    cat.cases.push_back({"一元负号", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b"),
            make_assign(make_ident("b"), make_unop(OpType::UMINUS, make_ident("a")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "uminus")) {
            return TestResult{false, "缺少 uminus: " + dump_code(r.code)};
        }
        return TestResult{true, "uminus 正常"};
    }});

    cat.cases.push_back({"一元正号", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b"),
            make_assign(make_ident("b"), make_unop(OpType::UPLUS, make_ident("a")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "uplus")) {
            return TestResult{false, "缺少 uplus: " + dump_code(r.code)};
        }
        return TestResult{true, "uplus 正常"};
    }});

    cat.cases.push_back({"逻辑非", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(0)),
            make_decl("b"),
            make_assign(make_ident("b"), make_unop(OpType::NOT, make_ident("a")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "!")) {
            return TestResult{false, "缺少 !: " + dump_code(r.code)};
        }
        return TestResult{true, "逻辑非正常"};
    }});

    cat.cases.push_back({"自增运算符映射", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b"),
            make_assign(make_ident("b"), make_unop(OpType::INC, make_ident("a")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "++")) {
            return TestResult{false, "缺少 ++: " + dump_code(r.code)};
        }
        return TestResult{true, "++ 映射正确"};
    }});

    cat.cases.push_back({"自减运算符映射", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b"),
            make_assign(make_ident("b"), make_unop(OpType::DEC, make_ident("a")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "--")) {
            return TestResult{false, "缺少 --: " + dump_code(r.code)};
        }
        return TestResult{true, "-- 映射正确"};
    }});

    cat.cases.push_back({"取地址运算符映射", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("p"),
            make_assign(make_ident("p"), make_unop(OpType::ADDR, make_ident("a")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "&")) {
            return TestResult{false, "缺少 &: " + dump_code(r.code)};
        }
        return TestResult{true, "& 映射正确"};
    }});

    cat.cases.push_back({"解引用运算符映射", []() -> TestResult {
        auto program = make_program(
            make_decl("p", make_int_literal(1)),
            make_decl("a"),
            make_assign(make_ident("a"), make_unop(OpType::DEREF, make_ident("p")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "=*")) {
            return TestResult{false, "缺少 =*: " + dump_code(r.code)};
        }
        return TestResult{true, "=* 映射正确"};
    }});

    cat.cases.push_back({"强制类型转换映射", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b"),
            make_assign(make_ident("b"), make_unop(OpType::CAST, make_ident("a")))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "cast")) {
            return TestResult{false, "缺少 cast: " + dump_code(r.code)};
        }
        return TestResult{true, "cast 映射正确"};
    }});

    cat.cases.push_back({"双层一元运算", []() -> TestResult {
        auto nested = make_unop(OpType::NOT, make_unop(OpType::UMINUS, make_ident("a")));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b"),
            make_assign(make_ident("b"), std::move(nested))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "uminus") || !contains_op(r.code, "!")) {
            return TestResult{false, "双层一元运算缺失: " + dump_code(r.code)};
        }
        return TestResult{true, "双层一元运算正确"};
    }});

    cat.cases.push_back({"UNKNOWN 一元透传", []() -> TestResult {
        auto passthrough = make_node(ASTNodeType::UNOP);
        passthrough->op = OpType::UNKNOWN;
        passthrough->add_child(make_ident("a"));

        auto program = make_program(
            make_decl("a", make_int_literal(9)),
            make_decl("b"),
            make_assign(make_ident("b"), std::move(passthrough))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "a", "-", "b")) {
            return TestResult{false, "UNKNOWN 一元透传失败: " + dump_code(r.code)};
        }
        return TestResult{true, "UNKNOWN 一元透传正常"};
    }});

    cat.cases.push_back({"单子节点 BINOP 透传", []() -> TestResult {
        auto pseudo_bin = make_node(ASTNodeType::BINOP);
        pseudo_bin->op = OpType::ADD;
        pseudo_bin->add_child(make_ident("a"));

        auto program = make_program(
            make_decl("a", make_int_literal(3)),
            make_decl("b"),
            make_assign(make_ident("b"), std::move(pseudo_bin))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (contains_op(r.code, "+") || !contains_quad(r.code, ":=", "a", "-", "b")) {
            return TestResult{false, "BINOP 单子节点透传异常: " + dump_code(r.code)};
        }
        return TestResult{true, "BINOP 单子节点透传正常"};
    }});

    cat.cases.push_back({"关系结果再参与算术", []() -> TestResult {
        auto mixed = make_binop(OpType::ADD,
            make_binop(OpType::LT, make_ident("a"), make_ident("b")),
            make_int_literal(1));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b", make_int_literal(2)),
            make_decl("c"),
            make_assign(make_ident("c"), std::move(mixed))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "<") || !contains_op(r.code, "+")) {
            return TestResult{false, "关系+算术混合不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "关系结果参与算术正常"};
    }});

    return cat;
}

Category make_logic_and_compare_category() {
    Category cat;
    cat.name = "比较与逻辑";

    cat.cases.push_back({"小于与不等", []() -> TestResult {
        auto expr = make_binop(OpType::AND,
            make_binop(OpType::LT, make_ident("a"), make_int_literal(10)),
            make_binop(OpType::NE, make_ident("b"), make_int_literal(0)));

        auto program = make_program(
            make_decl("a", make_int_literal(3)),
            make_decl("b", make_int_literal(4)),
            make_decl("c"),
            make_assign(make_ident("c"), std::move(expr))
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "<") || !contains_op(r.code, "!=") || !contains_op(r.code, "&&")) {
            return TestResult{false, "缺少 <、!=、&&: " + dump_code(r.code)};
        }
        return TestResult{true, "< != && 正常"};
    }});

    cat.cases.push_back({"<= 与 >=", []() -> TestResult {
        auto expr = make_binop(OpType::OR,
            make_binop(OpType::LE, make_ident("a"), make_ident("b")),
            make_binop(OpType::GE, make_ident("a"), make_int_literal(0)));

        auto program = make_program(
            make_decl("a", make_int_literal(3)),
            make_decl("b", make_int_literal(4)),
            make_decl("c"),
            make_assign(make_ident("c"), std::move(expr))
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "<=") || !contains_op(r.code, ">=") || !contains_op(r.code, "||")) {
            return TestResult{false, "缺少 <=、>=、||: " + dump_code(r.code)};
        }
        return TestResult{true, "<= >= || 正常"};
    }});

    cat.cases.push_back({"== 与 >", []() -> TestResult {
        auto expr = make_binop(OpType::AND,
            make_binop(OpType::EQ, make_ident("a"), make_ident("b")),
            make_binop(OpType::GT, make_ident("a"), make_int_literal(1)));

        auto program = make_program(
            make_decl("a", make_int_literal(2)),
            make_decl("b", make_int_literal(2)),
            make_decl("c"),
            make_assign(make_ident("c"), std::move(expr))
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "==") || !contains_op(r.code, ">") || !contains_op(r.code, "&&")) {
            return TestResult{false, "缺少 ==、>、&&: " + dump_code(r.code)};
        }
        return TestResult{true, "== > && 正常"};
    }});

    cat.cases.push_back({"纯逻辑或", []() -> TestResult {
        auto expr = make_binop(OpType::OR,
            make_binop(OpType::GT, make_ident("a"), make_int_literal(0)),
            make_binop(OpType::LT, make_ident("b"), make_int_literal(0)));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b", make_int_literal(2)),
            make_decl("c"),
            make_assign(make_ident("c"), std::move(expr))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "||") || !contains_op(r.code, ">") || !contains_op(r.code, "<")) {
            return TestResult{false, "逻辑或链路不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "逻辑或链路正常"};
    }});

    cat.cases.push_back({"纯逻辑与", []() -> TestResult {
        auto expr = make_binop(OpType::AND,
            make_binop(OpType::GE, make_ident("a"), make_int_literal(1)),
            make_binop(OpType::LE, make_ident("b"), make_int_literal(3)));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b", make_int_literal(2)),
            make_decl("c"),
            make_assign(make_ident("c"), std::move(expr))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "&&") || !contains_op(r.code, ">=") || !contains_op(r.code, "<=")) {
            return TestResult{false, "逻辑与链路不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "逻辑与链路正常"};
    }});

    return cat;
}

Category make_control_flow_category() {
    Category cat;
    cat.name = "控制流";

    cat.cases.push_back({"if 无 else", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(0)),
            make_if(make_ident("a"), make_assign(make_ident("a"), make_int_literal(1)))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "if_false") || !contains_op(r.code, "label")) {
            return TestResult{false, "if 四元式不足: " + dump_code(r.code)};
        }
        return TestResult{true, "if 控制流正常"};
    }});

    cat.cases.push_back({"if-else", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(0)),
            make_if(
                make_ident("a"),
                make_assign(make_ident("a"), make_int_literal(1)),
                make_assign(make_ident("a"), make_int_literal(2))
            )
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "if_false") || !contains_op(r.code, "goto") || !contains_op(r.code, "label")) {
            return TestResult{false, "if-else 四元式不足: " + dump_code(r.code)};
        }
        return TestResult{true, "if-else 控制流正常"};
    }});

    cat.cases.push_back({"if 嵌套 if", []() -> TestResult {
        auto nested = make_if(make_ident("a"), make_assign(make_ident("a"), make_int_literal(3)));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_if(make_ident("a"), std::move(nested))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "if_false") || !contains_op(r.code, "label")) {
            return TestResult{false, "嵌套 if 四元式不足: " + dump_code(r.code)};
        }
        return TestResult{true, "嵌套 if 正常"};
    }});

    cat.cases.push_back({"while + continue", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(make_ident("a"), make_node(ASTNodeType::CONTINUE_STMT))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "if_false") || !contains_op(r.code, "goto") || !contains_op(r.code, "label")) {
            return TestResult{false, "while/continue 四元式不足: " + dump_code(r.code)};
        }
        return TestResult{true, "while+continue 正常"};
    }});

    cat.cases.push_back({"while + break", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(make_ident("a"), make_node(ASTNodeType::BREAK_STMT))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "if_false") || !contains_op(r.code, "goto") || !contains_op(r.code, "label")) {
            return TestResult{false, "while/break 四元式不足: " + dump_code(r.code)};
        }
        return TestResult{true, "while+break 正常"};
    }});

    cat.cases.push_back({"while 体内赋值", []() -> TestResult {
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(make_ident("a"), make_assign(make_ident("a"), make_int_literal(0)))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, ":=") || !contains_op(r.code, "goto") || !contains_op(r.code, "label")) {
            return TestResult{false, "while 体赋值四元式不足: " + dump_code(r.code)};
        }
        return TestResult{true, "while 体赋值正常"};
    }});

    cat.cases.push_back({"while 体内 if+break", []() -> TestResult {
        auto body_if = make_if(make_ident("a"), make_node(ASTNodeType::BREAK_STMT));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(make_ident("a"), std::move(body_if))
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "if_false") || !contains_op(r.code, "goto") || !contains_op(r.code, "label")) {
            return TestResult{false, "while-if-break 四元式不足: " + dump_code(r.code)};
        }
        return TestResult{true, "while-if-break 正常"};
    }});

    cat.cases.push_back({"return 无值", []() -> TestResult {
        auto ret = make_node(ASTNodeType::RETURN_STMT);
        auto program = make_program(std::move(ret));

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "return")) {
            return TestResult{false, "缺少 return: " + dump_code(r.code)};
        }
        return TestResult{true, "return 无值正常"};
    }});

    cat.cases.push_back({"return 有值", []() -> TestResult {
        auto ret = make_node(ASTNodeType::RETURN_STMT);
        ret->add_child(make_ident("a"));

        auto program = make_program(make_decl("a", make_int_literal(3)), std::move(ret));
        IRRunResult r = run_ir(std::move(program));

        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "return")) {
            return TestResult{false, "缺少 return: " + dump_code(r.code)};
        }
        return TestResult{true, "return 有值正常"};
    }});

    cat.cases.push_back({"双层 while + 内层 break", []() -> TestResult {
        auto inner = make_while(make_ident("a"), make_node(ASTNodeType::BREAK_STMT));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(make_ident("a"), std::move(inner))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "label") || !contains_op(r.code, "goto")) {
            return TestResult{false, "双层 while/break 控制流不足: " + dump_code(r.code)};
        }
        return TestResult{true, "双层 while + break 正常"};
    }});

    cat.cases.push_back({"双层 while + 内层 continue", []() -> TestResult {
        auto inner = make_while(make_ident("a"), make_node(ASTNodeType::CONTINUE_STMT));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(make_ident("a"), std::move(inner))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "label") || !contains_op(r.code, "goto")) {
            return TestResult{false, "双层 while/continue 控制流不足: " + dump_code(r.code)};
        }
        return TestResult{true, "双层 while + continue 正常"};
    }});

    cat.cases.push_back({"while 条件为比较表达式", []() -> TestResult {
        auto cond = make_binop(OpType::LT, make_ident("a"), make_int_literal(10));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(std::move(cond), make_node(ASTNodeType::BREAK_STMT))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "<") || !contains_op(r.code, "if_false")) {
            return TestResult{false, "比较条件 while 不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "比较条件 while 正常"};
    }});

    cat.cases.push_back({"if-else 两支都 return", []() -> TestResult {
        auto then_ret = make_node(ASTNodeType::RETURN_STMT);
        then_ret->add_child(make_int_literal(1));
        auto else_ret = make_node(ASTNodeType::RETURN_STMT);
        else_ret->add_child(make_int_literal(0));

        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_if(make_ident("a"), std::move(then_ret), std::move(else_ret))
        );
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, "return") || !contains_op(r.code, "if_false")) {
            return TestResult{false, "if-return 链路不完整: " + dump_code(r.code)};
        }
        return TestResult{true, "if-else return 链路正常"};
    }});

    return cat;
}

Category make_scope_category() {
    Category cat;
    cat.name = "作用域与符号解析";

    cat.cases.push_back({"嵌套作用域遮蔽", []() -> TestResult {
        auto inner = make_block(
            make_decl("a", make_int_literal(9)),
            make_assign(make_ident("a"), make_int_literal(10))
        );
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            std::move(inner),
            make_assign(make_ident("a"), make_int_literal(2))
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_op(r.code, ":=")) {
            return TestResult{false, "缺少赋值四元式: " + dump_code(r.code)};
        }
        return TestResult{true, "遮蔽场景正常"};
    }});

    cat.cases.push_back({"内层读取外层变量", []() -> TestResult {
        auto inner = make_block(make_assign(make_ident("a"), make_int_literal(5)));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            std::move(inner)
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "5", "-", "a")) {
            return TestResult{false, "缺少对外层变量赋值: " + dump_code(r.code)};
        }
        return TestResult{true, "内层访问外层变量正常"};
    }});

    cat.cases.push_back({"三层嵌套作用域", []() -> TestResult {
        auto deep = make_block(
            make_block(
                make_assign(make_ident("a"), make_int_literal(11))
            )
        );
        auto program = make_program(make_decl("a", make_int_literal(1)), std::move(deep));

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "11", "-", "a")) {
            return TestResult{false, "三层作用域赋值未命中: " + dump_code(r.code)};
        }
        return TestResult{true, "三层嵌套作用域正常"};
    }});

    cat.cases.push_back({"兄弟块同名声明互不冲突", []() -> TestResult {
        auto block1 = make_block(make_decl("x", make_int_literal(1)));
        auto block2 = make_block(make_decl("x", make_int_literal(2)));
        auto program = make_program(std::move(block1), std::move(block2));

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "兄弟块同名声明不应报错: " + r.error_text};
        }
        return TestResult{true, "兄弟块同名声明正常"};
    }});

    cat.cases.push_back({"内外同名变量连续赋值", []() -> TestResult {
        auto inner = make_block(
            make_decl("a", make_int_literal(2)),
            make_assign(make_ident("a"), make_int_literal(3))
        );
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            std::move(inner),
            make_assign(make_ident("a"), make_int_literal(4))
        );

        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "4", "-", "a")) {
            return TestResult{false, "外层最终赋值缺失: " + dump_code(r.code)};
        }
        return TestResult{true, "内外同名连续赋值正常"};
    }});

    cat.cases.push_back({"四层嵌套读取外层", []() -> TestResult {
        auto deep = make_block(
            make_block(
                make_block(
                    make_assign(make_ident("a"), make_int_literal(12))
                )
            )
        );
        auto program = make_program(make_decl("a", make_int_literal(1)), std::move(deep));
        IRRunResult r = run_ir(std::move(program));
        if (r.has_error) {
            return TestResult{false, "不应报错: " + r.error_text};
        }
        if (!contains_quad(r.code, ":=", "12", "-", "a")) {
            return TestResult{false, "四层嵌套赋值未命中: " + dump_code(r.code)};
        }
        return TestResult{true, "四层嵌套访问正常"};
    }});

    return cat;
}

Category make_error_category() {
    Category cat;
    cat.name = "错误处理与边界";

    cat.cases.push_back({"未声明变量使用", []() -> TestResult {
        auto program = make_program(make_assign(make_ident("x"), make_int_literal(1)));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 UNDECLARED"};
        }
        return TestResult{true, "捕获未声明变量错误"};
    }});

    cat.cases.push_back({"同作用域重复声明", []() -> TestResult {
        auto program = make_program(make_decl("a"), make_decl("a"));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 REDEFINED"};
        }
        return TestResult{true, "捕获重复声明错误"};
    }});

    cat.cases.push_back({"break 在循环外", []() -> TestResult {
        auto program = make_program(make_node(ASTNodeType::BREAK_STMT));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 BREAK_OUTSIDE_LOOP"};
        }
        return TestResult{true, "捕获 break 越界错误"};
    }});

    cat.cases.push_back({"continue 在循环外", []() -> TestResult {
        auto program = make_program(make_node(ASTNodeType::CONTINUE_STMT));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 CONTINUE_OUTSIDE_LOOP"};
        }
        return TestResult{true, "捕获 continue 越界错误"};
    }});

    cat.cases.push_back({"赋值左值非法", []() -> TestResult {
        auto program = make_program(make_assign(make_int_literal(1), make_int_literal(2)));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 NOT_LVALUE"};
        }
        return TestResult{true, "捕获左值非法错误"};
    }});

    cat.cases.push_back({"声明缺少标识符", []() -> TestResult {
        auto program = make_program(make_decl(""));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 declaration empty identifier"};
        }
        return TestResult{true, "捕获空声明名错误"};
    }});

    cat.cases.push_back({"if 缺少 then 分支", []() -> TestResult {
        auto bad_if = make_node(ASTNodeType::IF_STMT);
        bad_if->add_child(make_ident("a"));

        auto program = make_program(make_decl("a", make_int_literal(1)), std::move(bad_if));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 if 参数不足错误"};
        }
        return TestResult{true, "捕获 if 参数不足"};
    }});

    cat.cases.push_back({"while 缺少 body", []() -> TestResult {
        auto bad_while = make_node(ASTNodeType::WHILE_STMT);
        bad_while->add_child(make_ident("a"));

        auto program = make_program(make_decl("a", make_int_literal(1)), std::move(bad_while));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 while 参数不足错误"};
        }
        return TestResult{true, "捕获 while 参数不足"};
    }});

    cat.cases.push_back({"assignment 子节点不足", []() -> TestResult {
        auto bad_assign = make_node(ASTNodeType::ASSIGN);
        bad_assign->add_child(make_ident("a"));

        auto program = make_program(make_decl("a", make_int_literal(1)), std::move(bad_assign));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 assignment 参数不足"};
        }
        return TestResult{true, "捕获 assignment 参数不足"};
    }});

    cat.cases.push_back({"binop 子节点不足", []() -> TestResult {
        auto bad_binop = make_node(ASTNodeType::BINOP);
        bad_binop->op = OpType::ADD;
        bad_binop->add_child(make_ident("a"));
        bad_binop->add_child(make_int_literal(1));
        bad_binop->add_child(make_int_literal(2));

        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b"),
            make_assign(make_ident("b"), std::move(bad_binop))
        );

        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 binary op 参数错误"};
        }
        return TestResult{true, "捕获 binary op 参数错误"};
    }});

    cat.cases.push_back({"unop 子节点不足", []() -> TestResult {
        auto bad_unop = make_node(ASTNodeType::UNOP);
        bad_unop->op = OpType::NOT;
        bad_unop->add_child(make_ident("a"));
        bad_unop->add_child(make_ident("a"));

        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_decl("b"),
            make_assign(make_ident("b"), std::move(bad_unop))
        );

        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 unary op 参数错误"};
        }
        return TestResult{true, "捕获 unary op 参数错误"};
    }});

    cat.cases.push_back({"while 体为 block + break（当前实现边界）", []() -> TestResult {
        auto body = make_block(make_node(ASTNodeType::BREAK_STMT));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(make_ident("a"), std::move(body))
        );
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "当前实现下应报 break outside loop"};
        }
        return TestResult{true, "捕获 while-block-break 边界行为"};
    }});

    cat.cases.push_back({"while 体为 block + continue（当前实现边界）", []() -> TestResult {
        auto body = make_block(make_node(ASTNodeType::CONTINUE_STMT));
        auto program = make_program(
            make_decl("a", make_int_literal(1)),
            make_while(make_ident("a"), std::move(body))
        );
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "当前实现下应报 continue outside loop"};
        }
        return TestResult{true, "捕获 while-block-continue 边界行为"};
    }});

    cat.cases.push_back({"if 条件使用未声明变量", []() -> TestResult {
        auto program = make_program(
            make_if(make_ident("not_declared"), make_assign(make_ident("x"), make_int_literal(1)))
        );
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 if 条件未声明变量错误"};
        }
        return TestResult{true, "捕获 if 条件未声明变量"};
    }});

    cat.cases.push_back({"while 条件使用未声明变量", []() -> TestResult {
        auto program = make_program(
            make_while(make_ident("not_declared"), make_node(ASTNodeType::BREAK_STMT))
        );
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 while 条件未声明变量错误"};
        }
        return TestResult{true, "捕获 while 条件未声明变量"};
    }});

    cat.cases.push_back({"return 使用未声明变量", []() -> TestResult {
        auto ret = make_node(ASTNodeType::RETURN_STMT);
        ret->add_child(make_ident("unknown"));
        auto program = make_program(std::move(ret));
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 return 未声明变量错误"};
        }
        return TestResult{true, "捕获 return 未声明变量"};
    }});

    cat.cases.push_back({"赋值右值未声明", []() -> TestResult {
        auto program = make_program(
            make_decl("a"),
            make_assign(make_ident("a"), make_ident("unknown"))
        );
        IRRunResult r = run_ir(std::move(program));
        if (!r.has_error) {
            return TestResult{false, "应报 RHS 未声明变量错误"};
        }
        return TestResult{true, "捕获 RHS 未声明变量"};
    }});

    return cat;
}

std::vector<Category> build_all_categories() {
    std::vector<Category> categories;
    categories.push_back(make_parser_category());
    categories.push_back(make_decl_and_assign_category());
    categories.push_back(make_expression_category());
    categories.push_back(make_logic_and_compare_category());
    categories.push_back(make_control_flow_category());
    categories.push_back(make_scope_category());
    categories.push_back(make_error_category());
    return categories;
}

}  // namespace

int run_all_ir_tests() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    const std::vector<Category> categories = build_all_categories();

    int passed = 0;
    int total = 0;

    for (std::size_t i = 0; i < categories.size(); ++i) {
        std::cout << "\n========== 分类" << (i + 1) << "：" << categories[i].name << " ==========" << '\n';
        for (std::size_t j = 0; j < categories[i].cases.size(); ++j) {
            ++total;
            const std::string idx_cn = to_chinese_number(total);
            reset_last_ir_dump();
            const TestResult r = categories[i].cases[j].run();
            if (r.passed) {
                ++passed;
                std::cout << "测试用例" << idx_cn << "已通过：（" << categories[i].cases[j].name << "；" << r.detail << "）" << '\n';
            } else {
                std::cout << "测试用例" << idx_cn << "未通过：（" << categories[i].cases[j].name << "；" << r.detail << "）" << '\n';
            }
            std::cout << "  四元式：" << get_last_ir_dump() << '\n';
        }
    }

    std::cout << "\n测试汇总：通过 " << passed << " / " << total << '\n';
    return passed == total ? 0 : 1;
}
