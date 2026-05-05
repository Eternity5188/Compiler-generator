#include "lr_state.h"


LRState::LRState(uint32_t id, const std::unordered_set<const LRItem*> items)
    :id_{id}, items_{items}
{}

const LRItem* LRState::add_item(const LRItem* item)
{
    auto [it, success] = items_.insert(item);
    return success ? *it : nullptr;
}

void LRState::set_id(uint32_t id)
{
    id_ = id;   
}
const uint32_t LRState::get_id() const
{
    return id_;
}
const std::unordered_set<const LRItem*>& LRState::get_items() const
{
    return items_;
}

bool LRState::operator==(const LRState& other) const
{
    return id_ == other.id_;
}