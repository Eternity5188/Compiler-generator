#pragma once


#include <cstdint>
#include <unordered_set>
#include <functional>


class LRItem;

class LRState
{
public:
    LRState(uint32_t id, const std::unordered_set<const LRItem*>& items);

    const LRItem* add_item(const LRItem* item);

    void set_id(uint32_t id);
    const uint32_t get_id() const;
    const std::unordered_set<const LRItem*>& get_items() const;

    bool operator==(const LRState& other) const;

private:
    uint32_t id_;
    std::unordered_set<const LRItem*> items_;
};

namespace std{
    template<>
    struct hash<LRState>
    {
        size_t operator()(const LRState& state) const
        {
            return hash<uint32_t>{}(state.get_id());
        }
    };
}