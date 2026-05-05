#include "action.h"


Action::Action(Type type, uint32_t state_id)
    :type_{type}, state_id_{state_id}
{}

const Action::Type Action::get_type() const
{
    return type_;
}
const uint32_t Action::get_state_id() const
{
    return state_id_;
}