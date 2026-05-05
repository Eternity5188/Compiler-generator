#include "grammar.h"


#include <iostream>
#include <algorithm>


Grammar::Grammar()
    :start_symbol_{nullptr}, current_production_id_{0}
{}

bool Grammar::set_start_symbol(const std::string_view name)
{
    auto it = non_terminals_.find(Symbol{Symbol::Type::NonTerminal, name});
    if (it == non_terminals_.end())
        return false;
    start_symbol_ = &(*it);
    return true;
}
const Symbol* Grammar::add_terminal(const std::string_view name)
{
    auto [it, success] = terminals_.emplace(Symbol::Type::Terminal, name);
    return &(*it);
}
const Symbol* Grammar::add_operator(const std::string_view name, uint32_t precedence, Associativity associativity)
{
    auto [it, success] = terminals_.emplace(Symbol::Type::Terminal, name, precedence, associativity);
    return &(*it);
}
bool Grammar::set_operator(const std::string_view name, uint32_t precedence, Associativity associativity)
{
    auto it = terminals_.find(Symbol{Symbol::Type::Terminal, name});
    if (it == terminals_.end())
        return false;

    it->set_operator(precedence, associativity);
    return true;
}
const Symbol* Grammar::add_non_terminal(const std::string_view name)
{
    auto [it, success] = non_terminals_.emplace(Symbol::Type::NonTerminal, name);
    return &(*it);
}
bool Grammar::add_production(const std::string_view left_name, const std::vector<std::string>& right_names)
{
    const Symbol* left_symbol = get_non_terminal(left_name);
    if (left_symbol == nullptr)
        return false;

    if (right_names.empty())
        return false;

    std::vector<const Symbol*> right_symbols;
    if (right_names.front() == "epsilon")
    {
        right_symbols.push_back(&Symbol::get_epsilon());
        productions_.emplace_back(current_production_id_, left_symbol, right_symbols);
        ++current_production_id_;
        return true;
    }

    for (const std::string_view right_name : right_names)
    {
        const Symbol* right_symbol = get_symbol(right_name);
        if (right_symbol == nullptr)
            return false;

        right_symbols.push_back(right_symbol);
    }

    productions_.emplace_back(current_production_id_, left_symbol, right_symbols);
    ++current_production_id_;
    return true;
}
void Grammar::compute_first_sets()
{
    first_set_map_.clear();

    for (const Symbol& terminal : terminals_)
        first_set_map_[terminal.get_name()].insert(&terminal);
    first_set_map_[Symbol::get_epsilon().get_name()].insert(&Symbol::get_epsilon());
    for (const Symbol& non_terminal : non_terminals_)
        first_set_map_[non_terminal.get_name()] = {};

    bool updated = true;
    while (updated)
    {
        updated = false;

        for (const Production& production : productions_)
        {
            const Symbol* left_symbol = production.get_left();
            const std::vector<const Symbol*>& right_symbols = production.get_right();

            // A -> /epsilon
            if (right_symbols.front() == &Symbol::get_epsilon())
            {
                auto [it, success] = first_set_map_[left_symbol->get_name()].insert(&Symbol::get_epsilon());
                if (success)
                    updated = true;
                continue;
            }
            // A -> X1 X2 ... Xn
            bool all_nullable = true;
            for (const Symbol* symbol : right_symbols)
            {
                // 添加First(symbol)的非/epsilon符号
                const std::unordered_set<const Symbol*>& first_set = first_set_map_[symbol->get_name()];
                for (const Symbol* f : first_set)
                {
                    if (f != &Symbol::get_epsilon())
                    {
                        auto [it, success] = first_set_map_[left_symbol->get_name()].insert(f);
                        if (success)
                            updated = true;
                    }
                }
                // First(symbol)不含/epsilon
                auto it = first_set.find(&Symbol::get_epsilon());
                if (it == first_set.end())
                {
                    all_nullable = false;
                    break;
                }
            }
            // 添加/epsilon
            if (all_nullable)
            {
                auto [it, success] = first_set_map_[left_symbol->get_name()].insert(&Symbol::get_epsilon());
                if (success)
                    updated = true;
            }
        }
    }
}
void Grammar::compute_follow_sets()
{
    follow_set_map_.clear();

    for (const Symbol& non_terminal : non_terminals_)
        follow_set_map_[non_terminal.get_name()];
    if (start_symbol_)
        follow_set_map_[start_symbol_->get_name()].insert(&Symbol::get_end());

    bool updated = true;
    while (updated)
    {
        updated = false;

        for (const Production& production : productions_)
        {
            const Symbol* left_symbol = production.get_left();
            const std::vector<const Symbol*>& right_symbols = production.get_right();

            for (std::size_t i = 0; i < right_symbols.size(); ++i)
            {
                // 遍历非终结符
                const Symbol* symbol = right_symbols[i];
                if (symbol->get_type() != Symbol::Type::NonTerminal)
                    continue;

                // 计算当前符号后面序列的First集
                std::vector<const Symbol*> after_symbol(right_symbols.begin() + i + 1, right_symbols.end());
                std::unordered_set<const Symbol*> first_of_sequence = compute_first_of_sequence(after_symbol);
                // 添加非'/epsilon'符号
                for (const Symbol* f : first_of_sequence)
                {
                    if (f != &Symbol::get_epsilon())
                    {
                        auto[it, success] = follow_set_map_[symbol->get_name()].insert(f);
                        if (success)
                            updated = true;
                    }
                }

                // symbol之后存在'/epsilon'
                auto it = first_of_sequence.find(&Symbol::get_epsilon());
                if (it != first_of_sequence.end())
                {
                    const std::unordered_set<const Symbol*>& follow_set_of_left = follow_set_map_[left_symbol->get_name()];
                    for (const Symbol* f : follow_set_of_left)
                    {
                        auto [it, success] = follow_set_map_[symbol->get_name()].insert(f);
                        if (success)
                            updated = true;
                    }
                }
            }
        }
    }
}

const Symbol* Grammar::get_start_symbol() const
{
    return start_symbol_;
}
const Symbol* Grammar::get_terminal(const std::string_view name) const
{
    auto it = terminals_.find(Symbol{Symbol::Type::Terminal, name});
    return it != terminals_.end() ? &(*it) : nullptr;
}
const Symbol* Grammar::get_non_terminal(const std::string_view name) const
{
    auto it = non_terminals_.find(Symbol{Symbol::Type::NonTerminal, name});
    return it != non_terminals_.end() ? &(*it) : nullptr;
}
const Symbol* Grammar::get_symbol(const std::string_view name) const
{
    const Symbol* terminal = get_terminal(name);
    if (terminal)
        return terminal;

    return get_non_terminal(name);
}
const uint32_t Grammar::get_current_production_id() const
{
    return current_production_id_;
}
const Production* Grammar::get_production(uint32_t id) const
{
    auto it = std::find_if(productions_.begin(), productions_.end(),
        [id](const Production& production) {
            return production.get_id() == id;
        }
    );
    return it != productions_.end() ? &(*it) : nullptr;
}
const std::vector<Production>& Grammar::get_productions() const
{
    return productions_;
}
const std::unordered_set<const Symbol*> Grammar::get_first_set(const std::string_view name) const
{
    std::string key{name};
    auto it = first_set_map_.find(key);
    if (it == first_set_map_.end())
        return std::unordered_set<const Symbol*>{};

    return it->second;
}
const std::unordered_set<const Symbol*> Grammar::get_follow_set(const std::string_view name) const
{
    std::string key{name};
    auto it = follow_set_map_.find(key);
    if (it == follow_set_map_.end())
        return std::unordered_set<const Symbol*>{};

    return it->second;
}

void Grammar::show() const
{
    std::cout << "\n==========\n";
    std::cout << "Start: " << '\n';
    std::cout << start_symbol_->to_string() << '\n';
    
    std::cout << "\n==========\n";
    std::cout << "Terminal: " << '\n';
    for (const Symbol& terminal : terminals_)
        std::cout << terminal.to_string() << '\n';
    
    std::cout << "\n==========\n";
    std::cout << "Non-terminal: " << '\n';
    for (const Symbol& non_terminal : non_terminals_)
        std::cout << non_terminal.to_string() << '\n';

    std::cout << "\n==========\n";
    std::cout << "Production: " << '\n';
    for (auto& production : productions_)
        std::cout << production.to_string() << '\n';
}
void Grammar::show_first_follow() const
{
    std::cout << "\n==========\n";
    std::cout << "First: " << '\n';
    for (const auto& [name, set] : first_set_map_)
    {
        std::cout << "FIRST(" << name << ")" << " = " << " { ";
        for (const Symbol* s : set)
            std::cout << s->get_name() << " ";
        std::cout << "}" << '\n';
    }

    std::cout << "\n==========\n";
    std::cout << "Follow: " << '\n';
    for (const auto& [name, set] : follow_set_map_)
    {
        std::cout << "FOLLOW(" << name << ")" << " = " << " { ";
        for (const Symbol* s : set)
            std::cout << s->get_name() << " ";
        std::cout << "}" << '\n';
    }
}

std::unordered_set<const Symbol*> Grammar::compute_first_of_sequence(const std::vector<const Symbol*>& symbols) const
{
    if (symbols.empty())
        return { &Symbol::get_epsilon() };
    
    std::unordered_set<const Symbol*> result;
    bool all_nullable = true;
    for (const Symbol* symbol : symbols)
    {
        // 添加First(symbol)的非'/epsilon'符号
        auto first_set_it = first_set_map_.find(symbol->get_name());
        auto& current_first_set = (*first_set_it).second;
        
        for (const Symbol* f : current_first_set)
        {
            if (f != &Symbol::get_epsilon())
                result.insert(f);
        }
        // 检查First(symbol)是否含有'/epsilon'
        auto it = current_first_set.find(&Symbol::get_epsilon());
        if (it == current_first_set.end())
        {
            all_nullable = false;
            break;
        }
    }
    if (all_nullable)
        result.insert(&Symbol::get_epsilon());

    return result;
}