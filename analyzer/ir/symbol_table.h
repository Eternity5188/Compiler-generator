#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ir {

enum class DataType {
    INT,
    FLOAT,
    CHAR,
    VOID_TYPE,
    POINTER,
    ARRAY,
    UNKNOWN,
};

struct SymbolEntry {
    std::string name;
    DataType type = DataType::UNKNOWN;
    int offset = 0;
    int width = 0;
    bool is_array = false;
    int array_size = 0;
    bool is_param = false;
    bool is_pointer = false;
    bool is_const = false;
    int line_no = 0;
    bool initialized = false;
};

class SymbolTable {
public:
    explicit SymbolTable(std::string scope_name, SymbolTable* parent = nullptr);

    bool insert(const SymbolEntry& entry);
    SymbolEntry* lookup(const std::string& name);
    const SymbolEntry* lookup(const std::string& name) const;

    SymbolTable* enter_scope(const std::string& scope_name);
    SymbolTable* exit_scope();

    bool is_in_loop() const;
    void enter_loop(const std::string& exit_label, const std::string& next_iter_label);
    void exit_loop();
    std::string loop_exit_label() const;
    std::string loop_next_iter_label() const;

    int allocate(int width);

    SymbolTable* parent() const;
    const std::string& scope_name() const;

private:
    SymbolTable* parent_ = nullptr;
    std::string scope_name_;
    std::unordered_map<std::string, SymbolEntry> entries_;
    int current_offset_ = 0;
    std::vector<std::pair<std::string, std::string>> loop_stack_;
    std::vector<SymbolTable> children_;
};

int get_type_width(DataType type);

}  // namespace ir
