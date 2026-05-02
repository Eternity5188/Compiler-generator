#include "symbol.h"


const Symbol Symbol::get_epsilon()
{
    return Symbol{Symbol::Type::Epsilon, "epsilon"};
}
const Symbol Symbol::get_end()
{
    return Symbol{Symbol::Type::End, "$"};
}

Symbol::Symbol(Type type, const std::string_view name)
    :type_{type}, name_{name}, is_operator_{false}, precedence_{0}, associativity_{Associativity::None}
{}
Symbol::Symbol(Type type, const std::string_view name, uint32_t precedence, Associativity associativity)
    :type_{type}, name_{name}, is_operator_{true}, precedence_{precedence}, associativity_{associativity}
{}

void Symbol::set_name(const std::string_view name)
{
    name_ = name;
}
const Symbol::Type Symbol::get_type() const
{
    return type_;
}
const std::string Symbol::get_name() const
{
    return name_;
}