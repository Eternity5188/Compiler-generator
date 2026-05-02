#pragma once


#include "associativity.h"
#include "symbol.h"
#include "production.h"
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <string_view>
#include <optional>


class Grammar
{
public:
    Grammar();

    bool set_start_symbol(const std::string_view name);
    const Symbol* add_terminal(const std::string_view name);
    const Symbol* add_terminal(const std::string_view name, uint32_t precedence, Associativity associativity);
    const Symbol* add_non_terminal(const std::string_view name);
    bool add_production(const std::string_view left_name, const std::vector<std::string>& right_names);

    const Symbol* get_terminal(const std::string_view name) const;
    const Symbol* get_non_terminal(const std::string_view name) const;
    const Symbol* get_symbol(const std::string_view name) const;

    void show() const;

private:
    const Symbol* start_symbol_;
    std::unordered_set<Symbol> terminals_;
    std::unordered_set<Symbol> non_terminals_;
    uint32_t current_production_id_;
    std::vector<Production> productions_;
    Symbol epsilon_;
};