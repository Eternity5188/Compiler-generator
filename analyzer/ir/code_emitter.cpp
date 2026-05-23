#include "ir/code_emitter.h"

#include <stdexcept>

namespace ir {

int CodeEmitter::emit(const std::string& op,
                      const std::string& arg1,
                      const std::string& arg2,
                      const std::string& result) {
    const int idx = static_cast<int>(code_.size());
    code_.push_back(Quadruple{op, arg1, arg2, result, idx});
    return idx;
}

std::string CodeEmitter::new_temp() {
    return "t" + std::to_string(temp_count_++);
}

std::string CodeEmitter::new_label() {
    return "L" + std::to_string(label_count_++);
}

void CodeEmitter::backpatch(const std::vector<int>& list, const std::string& label) {
    for (int idx : list) {
        if (idx < 0 || idx >= static_cast<int>(code_.size())) {
            throw std::out_of_range("backpatch index out of range");
        }
        code_[idx].result = label;
    }
}

std::vector<int> CodeEmitter::merge(const std::vector<int>& l1, const std::vector<int>& l2) const {
    std::vector<int> merged = l1;
    merged.insert(merged.end(), l2.begin(), l2.end());
    return merged;
}

int CodeEmitter::get_next_index() const {
    return static_cast<int>(code_.size());
}

const std::vector<Quadruple>& CodeEmitter::get_code() const {
    return code_;
}

void CodeEmitter::clear() {
    code_.clear();
    temp_count_ = 0;
    label_count_ = 0;
}

}  // namespace ir
