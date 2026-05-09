#include "symbol.h"


const Symbol& Symbol::get_epsilon()
{
    return epsilon;
}
const Symbol& Symbol::get_end()
{
    return end;
}

Symbol::Symbol(Type type, const std::string_view name)
    :type_{type}, name_{name}, is_operator_{false}, precedence_{0}, associativity_{Associativity::None}
{}
Symbol::Symbol(Type type, const std::string_view name, uint32_t precedence, Associativity associativity)
    :type_{type}, name_{name}, is_operator_{true}, precedence_{precedence}, associativity_{associativity}
{}

const std::string Symbol::to_string() const
{
    std::string result;
    std::string space{" "};
    result += name_ + space;
    if (is_operator_)
    {
        result += std::string{"operator"} + space;
        result += std::to_string(precedence_) + space;
        switch (associativity_)
        {
        case Associativity::Left:
            result += std::string{"left"} + space; break;
        case Associativity::Right:
            result += std::string{"right"} + space; break;
        case Associativity::None:
            break;
        default:
            break;
        }
    }

    return result;
}

void Symbol::set_name(const std::string_view name)
{
    name_ = name;
}
void Symbol::set_operator(uint32_t precedence, Associativity associativity) const
{
    is_operator_ = true;
    precedence_ = precedence;
    associativity_ = associativity;
}
const Symbol::Type Symbol::get_type() const
{
    return type_;
}
const std::string Symbol::get_name() const
{
    return name_;
}
bool Symbol::is_operator() const
{
    return is_operator_;
}
uint32_t Symbol::get_precedence() const
{
    return precedence_;
}
Associativity Symbol::get_associativity() const
{
    return associativity_;
}

Symbol Symbol::epsilon{Symbol::Type::Epsilon, "epsilon"};
Symbol Symbol::end{Symbol::Type::End, "$"};