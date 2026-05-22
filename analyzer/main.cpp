#include "tests/ir_test_suite.h"
#include "tests/pipeline_e2e_test_suite.h"

int main() {
    const int ir_ret = run_all_ir_tests();
    const int e2e_ret = run_pipeline_e2e_tests();
    return (ir_ret == 0 && e2e_ret == 0) ? 0 : 1;
}