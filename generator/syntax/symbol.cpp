#include "symbol.h"


Symbol::Symbol(Type type, const std::string_view name)
    :type_{type}, name_{name}, is_operator_{false}, precedence_{0}, associativity_{Associativity::None}
{}
Symbol::Symbol(Type type, const std::string_view name, unsigned int precedence, Associativity associativity)
    :type_{type}, name_{name}, is_operator_{true}, precedence_{precedence}, associativity_{associativity}
{}

void Symbol::set_name(const std::string_view name)
{
    name_ = name;
}
std::string Symbol::get_name() const
{
    return name_;
}