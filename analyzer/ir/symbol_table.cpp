#include "symbol_table.h"

namespace ir {

SymbolTable::SymbolTable(std::string scope_name, SymbolTable* parent)
    : parent_(parent), scope_name_(std::move(scope_name)) {}

bool SymbolTable::insert(const SymbolEntry& entry) {
    if (entries_.find(entry.name) != entries_.end()) {
        return false;
    }

    entries_.emplace(entry.name, entry);
    return true;
}

SymbolEntry* SymbolTable::lookup(const std::string& name) {
    auto it = entries_.find(name);
    if (it != entries_.end()) {
        return &it->second;
    }

    if (parent_ != nullptr) {
        return parent_->lookup(name);
    }

    return nullptr;
}

const SymbolEntry* SymbolTable::lookup(const std::string& name) const {
    auto it = entries_.find(name);
    if (it != entries_.end()) {
        return &it->second;
    }

    if (parent_ != nullptr) {
        return parent_->lookup(name);
    }

    return nullptr;
}

SymbolTable* SymbolTable::enter_scope(const std::string& scope_name) {
    children_.emplace_back(scope_name, this);
    return &children_.back();
}

SymbolTable* SymbolTable::exit_scope() {
    return parent_;
}

bool SymbolTable::is_in_loop() const {
    return !loop_stack_.empty();
}

void SymbolTable::enter_loop(const std::string& exit_label, const std::string& next_iter_label) {
    loop_stack_.push_back({exit_label, next_iter_label});
}

void SymbolTable::exit_loop() {
    if (!loop_stack_.empty()) {
        loop_stack_.pop_back();
    }
}

std::string SymbolTable::loop_exit_label() const {
    if (loop_stack_.empty()) {
        return "";
    }
    return loop_stack_.back().first;
}

std::string SymbolTable::loop_next_iter_label() const {
    if (loop_stack_.empty()) {
        return "";
    }
    return loop_stack_.back().second;
}

int SymbolTable::allocate(int width) {
    const int allocated = current_offset_;
    current_offset_ += width;
    return allocated;
}

SymbolTable* SymbolTable::parent() const {
    return parent_;
}

const std::string& SymbolTable::scope_name() const {
    return scope_name_;
}

int get_type_width(DataType type) {
    switch (type) {
        case DataType::CHAR:
            return 1;
        case DataType::INT:
            return 4;
        case DataType::FLOAT:
            return 8;
        case DataType::POINTER:
            return 8;
        case DataType::VOID_TYPE:
        case DataType::ARRAY:
        case DataType::UNKNOWN:
        default:
            return 0;
    }
}

}  // namespace ir
