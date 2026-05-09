#include "lr_parser.h"


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

bool LRParser::build_states()
{
    items_.clear();
    states_.clear();
    state_set_.clear();

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
    state_set_.insert(start_state);
    ++current_state_id_;

    for (std::size_t i = 0; i < states_.size(); ++i)
    {
        if (i % 10 == 0) {
            std::cout << "Processing state " << i << " of " << states_.size() << "...\n";
        }
        
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
            // 使用unordered_set快速查找
            if (state_set_.find(next_state) == state_set_.end())
            {
                states_.push_back(next_state);
                state_set_.insert(next_state);
                ++current_state_id_;
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
    action_table_.clear();
    goto_table_.clear();

    std::cout << "Starting construct_tables with " << states_.size() << " states\n";

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
                {
                    row[lookahead] = action;
                }
                else
                {
                    it->second = resolve_action_conflict(it->second, action, lookahead);
                }
                continue;
            }

            const Symbol* next_symbol = item->get_next_symbol();
            LRState next_state = get_next_state(state, next_symbol);
            if (next_state.get_items().empty())
                continue;

            int32_t target_state_index = find_state_by_items(next_state.get_items());
            if (target_state_index < 0)
                continue;
            uint32_t target_state_id = static_cast<uint32_t>(target_state_index);

            if (next_symbol->get_type() == Symbol::Type::Terminal)
            {
                Action action{Action::Type::Shift, target_state_id};
                auto& row = action_table_[state_id];
                auto it = row.find(next_symbol);
                if (it == row.end())
                {
                    row[next_symbol] = action;
                }
                else
                {
                    it->second = resolve_action_conflict(it->second, action, next_symbol);
                }
            }
            else if (next_symbol->get_type() == Symbol::Type::NonTerminal)
            {
                goto_table_[state_id][next_symbol] = target_state_id;
            }
        }
    }

    std::cout << "construct_tables completed\n";
    return true;
}

const Symbol* LRParser::get_production_precedence_symbol(const Production* production) const
{
    if (production == nullptr)
        return nullptr;

    const auto& right_symbols = production->get_right();
    for (auto it = right_symbols.rbegin(); it != right_symbols.rend(); ++it)
    {
        const Symbol* symbol = *it;
        if (symbol && symbol->get_type() == Symbol::Type::Terminal && symbol->is_operator())
            return symbol;
    }
    return nullptr;
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
        uint32_t shift_prec = terminal ? terminal->get_precedence() : 0;
        const Production* reduce_prod = grammar_->get_production(reduce_action->get_state_id());
        const Symbol* reduce_symbol = get_production_precedence_symbol(reduce_prod);
        uint32_t reduce_prec = reduce_symbol ? reduce_symbol->get_precedence() : 0;
        Associativity assoc = terminal ? terminal->get_associativity() : Associativity::None;

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
        const Production* prod_existing = grammar_->get_production(existing.get_state_id());
        const Production* prod_incoming = grammar_->get_production(incoming.get_state_id());
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
bool LRParser::parse(const std::vector<std::string>& token_strings)
{
    std::vector<const Symbol*> tokens;
    for (const std::string& s : token_strings)
    {
        const Symbol* symbol = grammar_->get_symbol(s);
        if (symbol == nullptr || symbol->get_type() != Symbol::Type::Terminal)
        {
            std::cerr << "Unknown token: " << s << '\n';
            return false;
        }
        tokens.push_back(symbol);
    }
    tokens.push_back(&Symbol::get_end());

    std::vector<uint32_t> state_stack;
    std::vector<const Symbol*> symbol_stack;
    state_stack.push_back(0);
    size_t lookahead_idx = 0;

    while (true)
    {
        uint32_t cur_state = state_stack.back();
        const Symbol* cur_token = tokens[lookahead_idx];

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
                return true;
            }
        case Action::Type::Shift:
            {
                uint32_t next_state = action.get_state_id();
                symbol_stack.push_back(cur_token);
                state_stack.push_back(next_state);
                lookahead_idx++;

                break;
            }
        case Action::Type::Reduce:
            {
                uint32_t prod_id = action.get_state_id();
                const Production* prod = grammar_->get_production(prod_id); // 你必须实现
                if (!prod)
                {
                    std::cerr << "Unknow production: " << prod_id << '\n';
                    return false;
                }

                // 弹出右部长度个元素
                size_t pop_len = prod->get_right().size();
                for (size_t i = 0; i < pop_len; i++)
                {
                    state_stack.pop_back();
                    symbol_stack.pop_back();
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

LRState LRParser::get_closure(const LRState& state)
{
    LRState result = state;

    bool updated = true;
    int iteration_count = 0;
    while (updated)
    {
        updated = false;
        iteration_count++;
        if (iteration_count > 1000) {
            std::cout << "Warning: get_closure() iteration count exceeded 1000, possible infinite loop\n";
            break;
        }
        if (iteration_count % 100 == 0) {
            std::cout << "  get_closure iteration " << iteration_count << ", items: " << result.get_items().size() << "\n";
        }

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