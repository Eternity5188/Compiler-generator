#include "pipeline_api.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct FullFlowCase {
    std::string name;
    std::string file_path;
    std::vector<std::string> expected_ops;
    bool expect_success;
    std::string expected_error_substr;
};

std::string sanitize_filename(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            out.push_back(c);
        } else if (c == '-' || c == '_' || c == '.') {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    return out.empty() ? "case" : out;
}

std::string read_text_file(const std::string& file_path) {
    std::ifstream ifs(file_path);
    if (!ifs.is_open()) {
        return "<failed to open source file>";
    }
    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void write_case_report(const std::string& category,
                       const std::string& case_name,
                       const std::string& source_path,
                       const ir::PipelineDebugResult& debug) {
    const std::filesystem::path out_dir = std::filesystem::path("tests") / "output" / category;
    std::error_code ec;
    std::filesystem::create_directories(out_dir, ec);

    const std::string stem = std::filesystem::path(source_path).stem().string();
    const std::filesystem::path out_file = out_dir / (sanitize_filename(stem) + ".txt");

    std::ofstream ofs(out_file.string(), std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        return;
    }

    ofs << "CASE_NAME:\n" << case_name << "\n\n";
    ofs << "SOURCE_CODE:\n";
    ofs << "----------------------------------------\n";
    ofs << read_text_file(source_path) << "\n";
    ofs << "----------------------------------------\n\n";

    ofs << "LEX_OUTPUT:\n" << (debug.lexer_output.empty() ? "(none)" : debug.lexer_output) << "\n\n";
    ofs << "YACC_OUTPUT:\n" << (debug.yacc_output.empty() ? "(none)" : debug.yacc_output) << "\n\n";
    ofs << "IR_INPUT_AST:\n" << (debug.ir_input_ast_output.empty() ? "(none)" : debug.ir_input_ast_output) << "\n\n";
    ofs << "PIPELINE_SUCCESS:\n" << (debug.pipeline.success ? "true" : "false") << "\n\n";
    ofs << "PIPELINE_ERROR:\n" << (debug.pipeline.error_text.empty() ? "(none)" : debug.pipeline.error_text) << "\n\n";
    ofs << "IR_QUADRUPLES:\n" << ir::dump_quadruples(debug.pipeline.code) << "\n";
}

std::vector<std::string> collect_c_files_sorted(const std::string& dir_path) {
    std::vector<std::string> files;
    const std::filesystem::path root(dir_path);
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
        return files;
    }

    for (const auto& entry : std::filesystem::directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() == ".c") {
            files.push_back(entry.path().generic_string());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

bool contains_op(const std::vector<ir::Quadruple>& code, const std::string& op) {
    return std::any_of(code.begin(), code.end(), [&](const ir::Quadruple& q) { return q.op == op; });
}

int run_basic_fullflow_tests() {
    const std::vector<FullFlowCase> cases = {
        {"声明初始化", "tests/system/basic/core_01_decl_init.c", {":="}, true, ""},
        {"连续声明初始化", "tests/system/basic/core_02_multi_decl_init.c", {":="}, true, ""},
        {"声明无初始化", "tests/system/basic/core_03_decl_no_init.c", {}, true, ""},
        {"非法声明", "tests/system/basic/core_04_invalid_decl.c", {}, false, "parser.parse(tokens) failed"},
        {"重复声明（语义）", "tests/system/basic/core_05_redefinition.c", {}, false, "redefinition of identifier"},
        {"未声明变量（语义）", "tests/system/basic/core_06_undeclared_rhs.c", {}, false, "[SemanticError]"},
        {"错误赋值左值（语义）", "tests/system/basic/core_07_invalid_lhs_assign.c", {}, false, "[SemanticError]"},
        {"当前子集外语句", "tests/system/basic/core_08_out_of_subset_while_break.c", {}, false, "parser.parse(tokens) failed"}
    };

    int passed = 0;
    std::cout << "\n========== 系统测试A：功能回归（原 basic + corpus）==========" << "\n";

    for (std::size_t i = 0; i < cases.size(); ++i) {
        if (!std::filesystem::exists(cases[i].file_path)) {
            std::cout << "系统用例A-" << (i + 1) << "未通过：（" << cases[i].name
                      << "；文件不存在：" << cases[i].file_path << "）\n";
            continue;
        }

        const ir::PipelineDebugResult dbg = ir::analyze_source_file_with_debug(cases[i].file_path);
        const ir::PipelineResult& r = dbg.pipeline;
        write_case_report("system_basic", cases[i].name, cases[i].file_path, dbg);

        bool ok = true;
        std::string detail;

        if (cases[i].expect_success && !r.success) {
            ok = false;
            detail = "流程失败: " + r.error_text;
        } else if (!cases[i].expect_success && r.success) {
            ok = false;
            detail = "预期失败但成功";
        } else if (!cases[i].expect_success && !r.success) {
            if (!cases[i].expected_error_substr.empty() &&
                r.error_text.find(cases[i].expected_error_substr) == std::string::npos) {
                ok = false;
                detail = "失败类型不匹配，期望包含: " + cases[i].expected_error_substr + "；实际: " + r.error_text;
            } else {
                detail = "已正确失败: " + r.error_text;
            }
        } else {
            for (std::size_t j = 0; j < cases[i].expected_ops.size(); ++j) {
                if (!contains_op(r.code, cases[i].expected_ops[j])) {
                    ok = false;
                    detail = "缺少操作符 " + cases[i].expected_ops[j];
                    break;
                }
            }
            if (ok) {
                detail = "流程成功";
            }
        }

        if (ok) {
            ++passed;
            std::cout << "系统用例A-" << (i + 1) << "已通过：（" << cases[i].name << "；" << detail << "）\n";
        } else {
            std::cout << "系统用例A-" << (i + 1) << "未通过：（" << cases[i].name << "；" << detail << "）\n";
        }
    }

    const std::vector<std::string> merged_regression_files = {
        "tests/system/basic/flow_01_decl_assign_return.c",
        "tests/system/basic/flow_02_if_else.c",
        "tests/system/basic/flow_03_while_break.c",
        "tests/system/basic/flow_04_while_continue.c",
        "tests/system/basic/flow_05_function_call.c",
        "tests/system/basic/flow_06_array_call.c",
        "tests/system/basic/flow_07_logic_compare.c"
    };

    int merged_passed = 0;
    int merged_checked = 0;
    for (std::size_t i = 0; i < merged_regression_files.size(); ++i) {
        if (!std::filesystem::exists(merged_regression_files[i])) {
            std::cout << "系统用例A-C" << (i + 1) << "未通过：（" << merged_regression_files[i] << "；文件不存在）\n";
            continue;
        }

        ++merged_checked;
        const ir::PipelineDebugResult dbg = ir::analyze_source_file_with_debug(merged_regression_files[i]);
        const ir::PipelineResult& r = dbg.pipeline;
        write_case_report("system_basic", "A-C" + std::to_string(i + 1), merged_regression_files[i], dbg);
        if (r.success) {
            ++merged_passed;
            std::cout << "系统用例A-C" << (i + 1) << "已通过：（" << merged_regression_files[i] << "）\n";
        } else {
            std::cout << "系统用例A-C" << (i + 1) << "未通过：（" << merged_regression_files[i]
                      << "；" << r.error_text << "）\n";
        }
    }

    const int total = static_cast<int>(cases.size()) + merged_checked;
    const int total_passed = passed + merged_passed;
    std::cout << "系统测试A汇总：通过 " << total_passed << " / " << total << "\n";
    return (total > 0 && total_passed == total) ? 0 : 1;
}

int run_stress_fullflow_tests() {
    const std::vector<std::string> stress_files = collect_c_files_sorted("tests/system/stress");
    if (stress_files.empty()) {
        std::cout << "\n========== 系统测试B：扩展压力全流程（外部 C 用例）==========\n";
        std::cout << "系统测试B汇总：通过 0 / 0（tests/system/stress 下未发现 .c 文件）\n";
        return 1;
    }

    int passed = 0;
    std::cout << "\n========== 系统测试B：扩展压力全流程（外部 C 用例）==========\n";
    for (std::size_t i = 0; i < stress_files.size(); ++i) {
        const ir::PipelineDebugResult dbg = ir::analyze_source_file_with_debug(stress_files[i]);
        const ir::PipelineResult& r = dbg.pipeline;
        write_case_report("system_stress", "B-" + std::to_string(i + 1), stress_files[i], dbg);
        if (r.success) {
            ++passed;
            std::cout << "系统用例B-" << (i + 1) << "已通过：（" << stress_files[i] << "）\n";
        } else {
            std::cout << "系统用例B-" << (i + 1) << "未通过：（" << stress_files[i]
                      << "；" << r.error_text << "）\n";
        }
    }

    std::cout << "系统测试B汇总：通过 " << passed << " / " << stress_files.size() << "\n";
    return (passed == static_cast<int>(stress_files.size())) ? 0 : 1;
}

}  // namespace

int run_system_integration_tests() {
    const int basic_ret = run_basic_fullflow_tests();
    const int stress_ret = run_stress_fullflow_tests();

    std::cout << "\n========== 系统测试总汇 ==========" << "\n";
    std::cout << "功能回归：" << (basic_ret == 0 ? "PASS" : "FAIL") << "\n";
    std::cout << "扩展压力：" << (stress_ret == 0 ? "PASS" : "FAIL") << "\n";

    return (basic_ret == 0 && stress_ret == 0) ? 0 : 1;
}
