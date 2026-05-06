#pragma once


#include "symbol.h"
#include "production.h"
#include "lr_item.h"
#include "lr_state.h"
#include "action.h"
#include <cstdint>
#include <vector>
#include <unordered_set>
#include <unordered_map>


class Grammar;

class LRParser
{
public:
    LRParser(const Grammar* grammar);

    bool build_states();
    bool construct_tables();
    void show_states() const;
    void show_tables() const;

private:
    LRState get_closure(const LRState& state);
    LRState get_next_state(const LRState& state, const Symbol* next_symbol);
    void merge_states();

private:
    const Grammar* grammar_;
    Symbol start_symbol_;
    Production start_production_;
    std::unordered_set<LRItem> items_;
    uint32_t current_state_id_;
    std::vector<LRState> states_;
    std::unordered_map<uint32_t, std::unordered_map<const Symbol*, Action>> action_table_;
    std::unordered_map<uint32_t, std::unordered_map<const Symbol*, uint32_t>> goto_table_;
};