#include "grammar.h"


#include <iostream>
#include <algorithm>
#include <stdexcept>


Grammar::Grammar()
    :start_symbol_{nullptr}, current_production_id_{0}
    ,epsilon_{Symbol::get_epsilon()}
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
const Symbol* Grammar::add_terminal(const std::string_view name, uint32_t precedence, Associativity associativity)
{
    auto [it, success] = terminals_.emplace(Symbol::Type::Terminal, name, precedence, associativity);
    return &(*it);
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
        right_symbols.push_back(&epsilon_);
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

void Grammar::show() const
{
    std::cout << "Start: " << '\n';
    std::cout << start_symbol_->get_name() << '\n';
    
    std::cout << "\n==========\n";

    std::cout << "Terminal: " << '\n';
    for (const Symbol& terminal : terminals_)
        std::cout << terminal.get_name() << '\n';
    
    std::cout << "\n==========\n";

    std::cout << "Non-terminal: " << '\n';
    for (const Symbol& non_terminal : non_terminals_)
        std::cout << non_terminal.get_name() << '\n';

    std::cout << "\n==========\n";

    std::cout << "Production: " << '\n';
    for (auto& production : productions_)
        std::cout << production.to_string() << '\n';
}