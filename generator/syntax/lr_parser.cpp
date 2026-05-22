#include "lr_parser.h"


#include "token.h"
#include "production.h"
#include "grammar.h"
#include <algorithm>
#include <iostream>


LRParser::LRParser(const Grammar* grammar)
    :grammar_{grammar}
    ,start_symbol_{Symbol::Type::NonTerminal, "S'"}
    ,start_production_{}
    ,current_state_id_{0}
{}

std::vector<export_space::Production> LRParser::get_export_grammar()
{
    std::vector<export_space::Production> export_grammar;
    const auto& productions = grammar_->get_productions();
    for (const Production& prod : productions)
    {
        export_space::Production export_prod;
        export_prod.left = prod.get_left()->get_name();
        for (const Symbol* sym : prod.get_right())
            export_prod.right.push_back(sym->get_name());
        export_grammar.push_back(export_prod);
    }
    return export_grammar;
}
std::vector<export_space::TableEntry> LRParser::get_export_action_table()
{
    std::vector<export_space::TableEntry> export_action_table;
    for (const auto& row : action_table_)
    {
        uint32_t state_id = row.first;
        for (const auto& [symbol, action] : row.second)
        {
            export_space::TableEntry entry;
            entry.state = state_id;
            entry.symbol = symbol->get_name();
            switch (action.get_type())
            {
            case Action::Type::Shift:
                entry.type = "Shift";
                break;
            case Action::Type::Reduce:
                entry.type = "Reduce";
                break;
            case Action::Type::Accept:
                entry.type = "Accept";
                break;
            default:
                entry.type = "Error";
                break;
            }
            entry.next_state = action.get_arg();
            export_action_table.push_back(entry);
        }
    }
    return export_action_table;
}
std::vector<export_space::TableEntry> LRParser::get_export_goto_table()
{
    std::vector<export_space::TableEntry> export_goto_table;
    for (const auto& row : goto_table_)
    {
        uint32_t state_id = row.first;
        for (const auto& [symbol, next_state] : row.second)
        {
            export_space::TableEntry entry;
            entry.state = state_id;
            entry.symbol = symbol->get_name();
            entry.type = "Goto";
            entry.next_state = next_state;
            export_goto_table.push_back(entry);
        }
    }
    return export_goto_table;
}
const ASTNode* LRParser::get_ast_root() const
{
    return ast_tree_.get_root();
}

bool LRParser::build_states()
{
    items_.clear();
    states_.clear();
    action_table_.clear();
    goto_table_.clear();
    
    std::cout << "Starting build_states" << '\n';

    const Symbol* original_start_symbol = grammar_->get_start_symbol();
    if (original_start_symbol == nullptr)
        return false;
    
    uint32_t start_production_id = grammar_->get_current_production_id();
    start_production_.reset(start_production_id, &start_symbol_, std::vector<const Symbol*>{original_start_symbol});
    LRItem new_item{&start_production_, 0, &Symbol::get_end()};
    auto [it, success] = items_.insert(new_item);
    const LRItem* start_item = &(*it);

    LRState start_state = get_closure(LRState{current_state_id_, std::unordered_set<const LRItem*>{start_item}});
    states_.push_back(start_state);
    ++current_state_id_;

    for (std::size_t i = 0; i < states_.size(); ++i)
    {   
        const LRState current_state = states_[i];
        // 记录可能输入符号
        const auto& items = current_state.get_items();
        std::unordered_set<const Symbol*> symbols;
        for (const LRItem* item : items)
        {
            if (item->is_dot_at_end() == false)
                symbols.insert(item->get_next_symbol());
        }

        for (const Symbol* symbol : symbols)
        {
            LRState next_state = get_next_state(current_state, symbol);
            // 添加状态
            if (next_state.get_items().empty())
                continue;
            // 检查是否已存在
            uint32_t target_state_id = 0;
            auto it = std::find_if(
                states_.begin(), states_.end(),
                [&next_state](const LRState& s) { return s.get_items() == next_state.get_items(); }
            );
            if (it == states_.end())
            {
                states_.push_back(next_state);
                target_state_id = current_state_id_;
                ++current_state_id_;
            }
            else
            {
                target_state_id = it->get_id();
            }
            // 更新action, goto
            uint32_t source_state_id = current_state.get_id();
            if (symbol->get_type() == Symbol::Type::Terminal)
            {
                Action action{Action::Type::Shift, target_state_id};
                auto& row = action_table_[source_state_id];
                auto act_it = row.find(symbol);
                if (act_it == row.end())
                    row[symbol] = action;
                else
                    act_it->second = resolve_action_conflict(act_it->second, action, symbol);
            }
            else if (symbol->get_type() == Symbol::Type::NonTerminal)
            {
                goto_table_[source_state_id][symbol] = target_state_id;
            }
        }
    }

    std::cout << "Total states created: " << states_.size() << "\n";
    return true;
}
bool LRParser::construct_tables()
{
    if (states_.empty())
        return true;
    
    std::cout << "Starting construct_tables" << '\n';

    for (const LRState& state : states_)
    {
        uint32_t state_id = state.get_id();

        for (const LRItem* item : state.get_items())
        {
            const Production* production = item->get_production();
            uint32_t dot_pos = item->get_dot_pos();
            const Symbol* lookahead = item->get_lookahead();

            // 可规约项
            if (dot_pos == production->get_right().size())
            {
                Action action;
                if (production->get_left() == &start_symbol_)
                {
                    action = Action{Action::Type::Accept, 0};
                }
                else
                {
                    action = Action{Action::Type::Reduce, production->get_id()};
                }

                auto& row = action_table_[state_id];
                auto it = row.find(lookahead);
                if (it == row.end())
                    row[lookahead] = action;
                else
                    it->second = resolve_action_conflict(it->second, action, lookahead);
                continue;
            }
        }
    }

    std::cout << "Construct_tables completed\n";
    return true;
}
bool LRParser::parse(const std::vector<Token>& tokens)
{
    std::vector<const Symbol*> symbols;
    for (const Token& token : tokens)
    {
        const Symbol* symbol = grammar_->get_symbol(token.type);
        if (symbol == nullptr || symbol->get_type() != Symbol::Type::Terminal)
        {
            std::cerr << "Unknown token: " << token.type << '\n';
            return false;
        }
        symbols.push_back(symbol);
    }
    symbols.push_back(&Symbol::get_end());

    std::vector<uint32_t> state_stack;
    std::vector<const Symbol*> symbol_stack;
    std::vector<std::string> value_stack;
    state_stack.push_back(0);
    value_stack.push_back("");

    ast_tree_.clear();
    ast_nodes_stack_.clear();
    
    size_t lookahead_idx = 0;
    while (true)
    {
        uint32_t cur_state = state_stack.back();
        const Symbol* cur_token = symbols[lookahead_idx];
        std::string cur_value = cur_token == &Symbol::get_end() ? "" : tokens[lookahead_idx].value;

        auto state_it = action_table_.find(cur_state);
        if (state_it == action_table_.end())
        {
            std::cerr << "State " << cur_state << " has no action" << '\n';
            return false;
        }

        auto act_it = state_it->second.find(cur_token);
        if (act_it == state_it->second.end())
        {
            std::cerr << "State " << cur_state << " has no action for token " << cur_token->get_name() << '\n';
            return false;
        }

        Action action = act_it->second;

        switch (action.get_type())
        {
        case Action::Type::Accept:
            {
                std::cout << "Successful" << '\n';

                if (ast_nodes_stack_.size() == 1)
                {
                    const ASTNode* root = ast_nodes_stack_.back();
                    ast_tree_.set_root(root);
                }

                return true;
            }
        case Action::Type::Shift:
            {
                uint32_t next_state = action.get_arg();
                state_stack.push_back(next_state);
                symbol_stack.push_back(cur_token);
                value_stack.push_back(cur_value);

                const ASTNode* node = ast_tree_.add_node(cur_token->get_name(), cur_value);
                ast_nodes_stack_.push_back(node);

                lookahead_idx++;
                break;
            }
        case Action::Type::Reduce:
            {
                uint32_t prod_id = action.get_arg();
                const Production* prod = grammar_->get_production(prod_id); // 你必须实现
                if (!prod)
                {
                    std::cerr << "Unknow production: " << prod_id << '\n';
                    return false;
                }

                // 弹出右部长度个元素
                size_t pop_len = prod->get_right().size();
                std::vector<const ASTNode*> children;
                for (size_t i = 0; i < pop_len; i++)
                {
                    children.push_back(ast_nodes_stack_.back());
                    ast_nodes_stack_.pop_back();
                }
                for (size_t i = 0; i < pop_len; i++)
                {
                    state_stack.pop_back();
                    symbol_stack.pop_back();
                    value_stack.pop_back();
                }
                // GOTO 跳转
                uint32_t top_state = state_stack.back();
                const Symbol* left = prod->get_left();
                auto goto_it = goto_table_.find(top_state);
                if (goto_it == goto_table_.end() || goto_it->second.find(left) == goto_it->second.end())
                {
                    std::cerr << "State " << top_state << " has no goto for non-terminal " << left->get_name() << '\n';
                    return false;
                }

                uint32_t next_state = goto_it->second[left];
                symbol_stack.push_back(left);
                state_stack.push_back(next_state);
                value_stack.push_back("");

                const ASTNode* parent = ast_tree_.add_node(left->get_name());
                std::reverse(children.begin(), children.end());
                for (const ASTNode* child : children)
                    parent->add_child(child);

                ast_nodes_stack_.push_back(parent);
                break;
            }
        case Action::Type::Error:
            {
                std::cerr << "Failed" << '\n';
                return false;
            }
        default:
            break;
        }
    }
    return false;
}
const ASTTree& LRParser::get_ast_tree() const
{
    return ast_tree_;
}

LRState LRParser::get_closure(const LRState& state)
{
    LRState result = state;

    bool updated = true;
    while (updated)
    {
        updated = false;

        const std::unordered_set<const LRItem*> items{result.get_items().begin(), result.get_items().end()};
        for (const LRItem* item : items)
        {
            if (item->is_dot_at_end())
                continue;

            const Symbol* next_symbol = item->get_next_symbol();
            if (next_symbol->get_type() != Symbol::Type::NonTerminal)
                continue;

            // 计算lookahead
            // beta
            std::vector<const Symbol*> beta;
            const std::vector<const Symbol*>& right_symbols = item->get_production()->get_right();
            for (std::size_t i = item->get_dot_pos() + 1; i < right_symbols.size(); ++i)
                beta.push_back(right_symbols[i]);
            std::unordered_set<const Symbol*> lookaheads = grammar_->compute_first_of_sequence(beta);
            // 当前lookahead
            auto epsilon_it = lookaheads.find(&Symbol::get_epsilon());
            if (epsilon_it != lookaheads.end())
            {
                lookaheads.erase(&Symbol::get_epsilon());
                lookaheads.insert(item->get_lookahead());
            }

            const auto& productions = grammar_->get_productions();
            for (const Production& production : productions)
            {
                if (production.get_left() != next_symbol)
                    continue;

                for (const Symbol* lookahead : lookaheads)
                {
                    LRItem new_item{&production, 0, lookahead};
                    auto [it, success] = items_.insert(new_item);

                    const LRItem* i = result.add_item(&(*it));
                    if (i)
                        updated = true;
                }
            }
        }
    }

    return result;
}
LRState LRParser::get_next_state(const LRState& state, const Symbol* next_symbol)
{  
    std::unordered_set<const LRItem*> next_items;
    std::unordered_set<const LRItem*> items = state.get_items();
    for (const LRItem* item : items)
    {
        if (item->is_dot_at_end() == false && item->get_next_symbol() == next_symbol)
        {
            LRItem new_item{item->get_production(), item->get_dot_pos() + 1, item->get_lookahead()};
            auto [it, success] = items_.insert(new_item);
            next_items.insert(&(*it));
        }
    }

    if (next_items.empty())
        return LRState{current_state_id_, {}};

    return get_closure(LRState{current_state_id_, next_items});
}
void LRParser::merge_states()
{
    std::vector<LRState> merged_states;

    for (const LRState& state : states_)
    {
        // 是否已经合并
        bool found = false;
        for (LRState& merged_state : merged_states)
        {
            if (merged_state.get_items().size() != state.get_items().size())
                continue;
            // 是否同心
            bool core_same = true;
            for (const LRItem* m_item : merged_state.get_items())
            {
                bool has_same_core = false;
                for (const LRItem* s_item : state.get_items())
                {
                    if (s_item->get_production() == m_item->get_production() && s_item->get_dot_pos() == m_item->get_dot_pos())
                    {
                        has_same_core = true;
                        break;
                    }
                }
                if (has_same_core == false)
                {
                    core_same = false;
                    break;
                }
            }
            // 合并
            if (core_same)
            {
                for (const LRItem* s_item : state.get_items())
                    merged_state.add_item(s_item);

                found = true;
                break;
            }
        }

        if (found == false)
            merged_states.push_back(state);
    }

    states_ = std::move(merged_states);
    for (std::size_t i = 0; i < states_.size(); ++i)
        states_[i].set_id(i);
}
int32_t LRParser::find_state_by_items(const std::unordered_set<const LRItem*>& items) const
{
    for (const LRState& state : states_)
    {
        if (state.get_items() == items)
            return static_cast<int32_t>(state.get_id());
    }
    return -1;
}
Action LRParser::resolve_action_conflict(const Action& existing, const Action& incoming, const Symbol* terminal) const
{
    if (existing.get_type() == incoming.get_type())
        return existing;

    if (existing.get_type() == Action::Type::Error)
        return incoming;
    if (incoming.get_type() == Action::Type::Error)
        return existing;

    if (existing.get_type() == Action::Type::Accept || incoming.get_type() == Action::Type::Accept)
        return existing.get_type() == Action::Type::Accept ? existing : incoming;

    const Action* shift_action = nullptr;
    const Action* reduce_action = nullptr;
    if (existing.get_type() == Action::Type::Shift)
        shift_action = &existing;
    else if (existing.get_type() == Action::Type::Reduce)
        reduce_action = &existing;

    if (incoming.get_type() == Action::Type::Shift)
        shift_action = &incoming;
    else if (incoming.get_type() == Action::Type::Reduce)
        reduce_action = &incoming;

    if (shift_action && reduce_action)
    {
        uint32_t shift_prec = terminal->get_precedence();
        const Production* reduce_prod = grammar_->get_production(reduce_action->get_arg());
        const Symbol* reduce_symbol = get_production_precedence_symbol(reduce_prod);
        uint32_t reduce_prec = reduce_symbol ? reduce_symbol->get_precedence() : 0;
        Associativity assoc = terminal->get_associativity();

        if (shift_prec > reduce_prec)
            return *shift_action;
        if (shift_prec < reduce_prec)
            return *reduce_action;

        if (assoc == Associativity::Left)
            return *reduce_action;
        if (assoc == Associativity::Right)
            return *shift_action;

        return existing;
    }

    if (existing.get_type() == Action::Type::Reduce && incoming.get_type() == Action::Type::Reduce)
    {
        const Production* prod_existing = grammar_->get_production(existing.get_arg());
        const Production* prod_incoming = grammar_->get_production(incoming.get_arg());
        const Symbol* symbol_existing = get_production_precedence_symbol(prod_existing);
        const Symbol* symbol_incoming = get_production_precedence_symbol(prod_incoming);
        uint32_t prec_existing = symbol_existing ? symbol_existing->get_precedence() : 0;
        uint32_t prec_incoming = symbol_incoming ? symbol_incoming->get_precedence() : 0;

        if (prec_existing > prec_incoming)
            return existing;
        if (prec_incoming > prec_existing)
            return incoming;
        return existing;
    }

    return existing;
}
const Symbol* LRParser::get_production_precedence_symbol(const Production* production) const
{
    if (production == nullptr)
        return nullptr;

    return production->get_precedence_symbol();
}

void LRParser::show_states() const
{
    std::cout << "\n==========\n";
    std::cout << "Item: " << '\n';
    for (const LRItem& item : items_)
        std::cout << item.to_string() << '\n';

    std::cout << "\n==========\n";
    std::cout << "State: " << '\n';
    for (const LRState& state : states_)
    {
        std::cout << "Id: " << state.get_id() << '\n';
        std::unordered_set<const LRItem*> items = state.get_items();
        for (const LRItem* item : items)
            std::cout << "Item: " << item->to_string() << '\n';
    }
}

void LRParser::show_tables() const
{
    std::cout << "\n==========\n";
    std::cout << "Action table: " << '\n';
    for (const auto& row : action_table_)
    {
        std::cout << "State: " << row.first << '\n';
        for (const auto& cell : row.second)
            std::cout << cell.first->get_name() << ": " << cell.second.to_string() << '\n';
        std::cout << std::endl;
    }

    std::cout << "\n==========\n";
    std::cout << "Goto table: " << '\n';
    for (const auto& row : goto_table_)
    {
        std::cout << "State: " << row.first << '\n';
        for (const auto& cell : row.second)
            std::cout << cell.first->get_name() << ": " << cell.second << '\n';
        std::cout << std::endl;
    }
}