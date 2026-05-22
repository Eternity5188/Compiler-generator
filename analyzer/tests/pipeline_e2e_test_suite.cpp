#include "pipeline_e2e_test_suite.h"

#include "../ir/ir_pipeline_api.h"

#include <cctype>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

struct CaseResult {
    bool passed = false;
    std::string detail;
};

struct Case {
    std::string name;
    std::string source;
    std::vector<std::string> expected_ops;
    bool expect_success = true;
};

bool contains_op(const std::vector<ir::Quadruple>& code, const std::string& op) {
    for (std::size_t i = 0; i < code.size(); ++i) {
        if (code[i].op == op) {
            return true;
        }
    }
    return false;
}

std::vector<Token> lex_minimal_c_like(const std::string& src) {
    std::vector<Token> out;
    std::size_t i = 0;

    auto push_kw_or_ident = [&](const std::string& word) {
        if (word == "int") {
            out.push_back({"INT", word});
        } else if (word == "float") {
            out.push_back({"FLOAT", word});
        } else if (word == "char") {
            out.push_back({"CHAR", word});
        } else if (word == "void") {
            out.push_back({"VOID", word});
        } else if (word == "if") {
            out.push_back({"IF", word});
        } else if (word == "else") {
            out.push_back({"ELSE", word});
        } else if (word == "while") {
            out.push_back({"WHILE", word});
        } else if (word == "return") {
            out.push_back({"RETURN", word});
        } else if (word == "break") {
            out.push_back({"BREAK", word});
        } else if (word == "continue") {
            out.push_back({"CONTINUE", word});
        } else {
            out.push_back({"IDENTIFIER", word});
        }
    };

    auto two_char_token = [&](char a, char b, const std::string& t, const std::string& v) -> bool {
        if (i + 1 < src.size() && src[i] == a && src[i + 1] == b) {
            out.push_back({t, v});
            i += 2;
            return true;
        }
        return false;
    };

    while (i < src.size()) {
        const char ch = src[i];

        if (std::isspace(static_cast<unsigned char>(ch))) {
            ++i;
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
            std::size_t j = i + 1;
            while (j < src.size()) {
                const char c = src[j];
                if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
                    break;
                }
                ++j;
            }
            push_kw_or_ident(src.substr(i, j - i));
            i = j;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(ch))) {
            std::size_t j = i + 1;
            while (j < src.size() && std::isdigit(static_cast<unsigned char>(src[j]))) {
                ++j;
            }
            out.push_back({"NUMBER", src.substr(i, j - i)});
            i = j;
            continue;
        }

        if (two_char_token('=', '=', "EQ", "==") ||
            two_char_token('!', '=', "NE", "!=") ||
            two_char_token('<', '=', "LE", "<=") ||
            two_char_token('>', '=', "GE", ">=") ||
            two_char_token('&', '&', "AND", "&&") ||
            two_char_token('|', '|', "OR", "||") ||
            two_char_token('+', '+', "INC", "++") ||
            two_char_token('-', '-', "DEC", "--")) {
            continue;
        }

        switch (ch) {
            case '=': out.push_back({"ASSIGN", "="}); break;
            case '+': out.push_back({"ADD", "+"}); break;
            case '-': out.push_back({"SUB", "-"}); break;
            case '*': out.push_back({"MUL", "*"}); break;
            case '/': out.push_back({"DIV", "/"}); break;
            case '%': out.push_back({"MOD", "%"}); break;
            case '<': out.push_back({"LT", "<"}); break;
            case '>': out.push_back({"GT", ">"}); break;
            case '!': out.push_back({"NOT", "!"}); break;
            case '&': out.push_back({"AMPER", "&"}); break;
            case '(': out.push_back({"LPAREN", "("}); break;
            case ')': out.push_back({"RPAREN", ")"}); break;
            case '{': out.push_back({"LBRACE", "{"}); break;
            case '}': out.push_back({"RBRACE", "}"}); break;
            case '[': out.push_back({"LBRACKET", "["}); break;
            case ']': out.push_back({"RBRACKET", "]"}); break;
            case ';': out.push_back({"SEMICOLON", ";"}); break;
            case ',': out.push_back({"COMMA", ","}); break;
            case '?': out.push_back({"QUESTION", "?"}); break;
            case ':': out.push_back({"COLON", ":"}); break;
            default:
                // Keep going to maximize parser-side diagnostics for unknown chars.
                break;
        }
        ++i;
    }

    out.push_back({"$", "$"});
    return out;
}

CaseResult run_case(const Case& c) {
    const std::vector<Token> tokens = lex_minimal_c_like(c.source);
    const ir::PipelineResult r = ir::generate_ir_from_tokens(tokens);

    if (c.expect_success && !r.success) {
        return {false, "流程失败: " + r.error_text};
    }
    if (!c.expect_success && r.success) {
        return {false, "预期失败但成功，IR: " + ir::dump_quadruples(r.code)};
    }

    if (!c.expect_success) {
        return {true, "已正确失败: " + r.error_text};
    }

    for (std::size_t i = 0; i < c.expected_ops.size(); ++i) {
        if (!contains_op(r.code, c.expected_ops[i])) {
            return {false, "缺少操作符 " + c.expected_ops[i] + "，IR: " + ir::dump_quadruples(r.code)};
        }
    }

    return {true, "IR: " + ir::dump_quadruples(r.code)};
}

}  // namespace

int run_pipeline_e2e_tests() {
    const std::vector<Case> cases = {
        {"E2E-声明初始化", "int a = 1;", {":="}, true},
        {"E2E-多声明初始化", "int a = 1; int b = 2;", {":="}, true},
        {"E2E-声明无初始化", "int a;", {}, true},
        {"E2E-非法声明", "int = 1;", {}, false},
        {"E2E-不在当前子集的语句", "while (1) break;", {}, false},
    };

    int passed = 0;
    std::cout << "\n========== 端到端流程测试（源码->Token->Parser->IR）==========\n";
    for (std::size_t i = 0; i < cases.size(); ++i) {
        const CaseResult r = run_case(cases[i]);
        if (r.passed) {
            ++passed;
            std::cout << "E2E测试用例" << (i + 1) << "已通过：（" << cases[i].name << "；" << r.detail << "）\n";
        } else {
            std::cout << "E2E测试用例" << (i + 1) << "未通过：（" << cases[i].name << "；" << r.detail << "）\n";
        }
    }

    std::cout << "端到端测试汇总：通过 " << passed << " / " << static_cast<int>(cases.size()) << "\n";
    return passed == static_cast<int>(cases.size()) ? 0 : 1;
}
