#include "action.h"


Action::Action()
    :type_{Type::Error}, arg_{0}
{}
Action::Action(Type type, uint32_t arg)
    :type_{type}, arg_{arg}
{}

const std::string Action::to_string() const
{
    std::string result;
    std::string space{" "};
    switch (type_)
    {
    case Type::Shift:
        result += "Shift" + space;
        break;
    case Type::Reduce:
        result += "Reduce" + space;
        break;
    case Type::Accept:
        result += "Accept" + space;
        break;
    case Type::Error:
        result += "Error" + space;
        break;
    }
    result += std::to_string(arg_) + space;
    return result;
}

const Action::Type Action::get_type() const
{
    return type_;
}
const uint32_t Action::get_arg() const
{
    return arg_;
}