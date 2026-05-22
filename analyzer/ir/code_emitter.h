#pragma once

#include "quadruple.h"

#include <string>
#include <vector>

namespace ir {

class CodeEmitter {
public:
    int emit(const std::string& op,
             const std::string& arg1 = "-",
             const std::string& arg2 = "-",
             const std::string& result = "-");

    std::string new_temp();
    std::string new_label();

    void backpatch(const std::vector<int>& list, const std::string& label);
    std::vector<int> merge(const std::vector<int>& l1, const std::vector<int>& l2) const;

    int get_next_index() const;
    const std::vector<Quadruple>& get_code() const;

    void clear();

private:
    std::vector<Quadruple> code_;
    int temp_count_ = 0;
    int label_count_ = 0;
};

}  // namespace ir
