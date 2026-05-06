#include "action.h"


Action::Action()
    :type_{Type::Error}, state_id_{0}
{}
Action::Action(Type type, uint32_t state_id)
    :type_{type}, state_id_{state_id}
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
    result += std::to_string(state_id_) + space;
    return result;
}

const Action::Type Action::get_type() const
{
    return type_;
}
const uint32_t Action::get_state_id() const
{
    return state_id_;
}