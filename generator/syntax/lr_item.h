#pragma once


#include <cstdint>
#include <string>
#include <functional>


class Symbol;
class Production;

class LRItem
{
public:
    static LRItem advance_dot(const LRItem& item);

    LRItem(const Production* production, uint32_t dot_pos, const Symbol* lookahead);

    const std::string to_string() const;

    const Production* get_production() const;
    const uint32_t get_dot_pos() const;
    const Symbol* get_lookahead() const;
    const Symbol* get_next_symbol() const;
    bool is_dot_at_end() const;
    
    bool operator==(const LRItem& other) const;

private:
    const Production* production_;
    uint32_t dot_pos_;
    const Symbol* lookahead_;
};

namespace std {
    template<>
    struct hash<LRItem>
    {
        size_t operator()(const LRItem& item) const
        {
            size_t h1 = hash<const Production*>{}(item.get_production());
            size_t h2 = hash<uint32_t>{}(item.get_dot_pos());
            size_t h3 = hash<const Symbol*>{}(item.get_lookahead());
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}