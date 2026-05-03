#pragma once


#include "associativity.h"
#include "symbol.h"
#include "production.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>


class Grammar
{
public:
    Grammar();

    bool set_start_symbol(const std::string_view name);
    const Symbol* add_terminal(const std::string_view name);
    const Symbol* add_operator(const std::string_view name, uint32_t precedence, Associativity associativity);
    bool set_operator(const std::string_view name, uint32_t precedence, Associativity associativity);
    const Symbol* add_non_terminal(const std::string_view name);
    bool add_production(const std::string_view left_name, const std::vector<std::string>& right_names);
    void compute_first_sets();
    void compute_follow_sets();

    const Symbol* get_terminal(const std::string_view name) const;
    const Symbol* get_non_terminal(const std::string_view name) const;
    const Symbol* get_symbol(const std::string_view name) const;
    const std::unordered_set<const Symbol*> get_first_set(const std::string_view name) const;
    const std::unordered_set<const Symbol*> get_follow_set(const std::string_view name) const;

    void show() const;
    void show_first_follow() const;

private:
    std::unordered_set<const Symbol*> compute_first_of_sequence(const std::vector<const Symbol*>& symbols);

private:
    const Symbol* start_symbol_;
    std::unordered_set<Symbol> terminals_;
    std::unordered_set<Symbol> non_terminals_;
    uint32_t current_production_id_;
    std::vector<Production> productions_;
    std::unordered_map<std::string, std::unordered_set<const Symbol*>> first_set_map_;
    std::unordered_map<std::string, std::unordered_set<const Symbol*>> follow_set_map_;
    Symbol epsilon_;
    Symbol end_;
};