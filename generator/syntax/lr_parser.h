#pragma once


#include "symbol.h"
#include "production.h"
#include "lr_item.h"
#include "lr_state.h"
#include "action.h"
#include "ast_tree.h"
#include <cstdint>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace export_space
{
    // 文法结构（仅用于导出）
    struct Production
    {
        std::string left;
        std::vector<std::string> right;
    };

    // ACTION/GOTO 表项
    struct TableEntry
    {
        uint32_t state;
        std::string symbol;
        std::string type;
        uint32_t next_state;
    };
}


struct Token;
class Grammar;

class LRParser
{
public:
    LRParser(const Grammar* grammar);

    std::vector<export_space::Production> get_export_grammar();
    std::vector<export_space::TableEntry> get_export_action_table();
    std::vector<export_space::TableEntry> get_export_goto_table();
    const ASTNode* get_ast_root() const;

    bool build_states();
    bool construct_tables();
    void merge_states();
    bool parse(const std::vector<Token>& token_strings);
    const ASTTree& get_ast_tree() const;
    void show_states() const;
    void show_tables() const;

private:
    LRState get_closure(const LRState& state);
    LRState get_next_state(const LRState& state, const Symbol* next_symbol);
    int32_t find_state_by_items(const std::unordered_set<const LRItem*>& items) const;
    Action resolve_action_conflict(const Action& existing, const Action& incoming, const Symbol* terminal) const;
    const Symbol* get_production_precedence_symbol(const Production* production) const;

private:
    const Grammar* grammar_;
    Symbol start_symbol_;
    Production start_production_;
    std::unordered_set<LRItem> items_;
    uint32_t current_state_id_;
    std::vector<LRState> states_;
    std::unordered_map<uint32_t, std::unordered_map<const Symbol*, Action>> action_table_;
    std::unordered_map<uint32_t, std::unordered_map<const Symbol*, uint32_t>> goto_table_;
    ASTTree ast_tree_;
    std::vector<const ASTNode*> ast_nodes_stack_;
};