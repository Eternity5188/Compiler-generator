#pragma once


#include "symbol.h"
#include <vector>
#include <string>


class Production
{
public:
    Production(unsigned int id, const Symbol& left, const std::vector<Symbol>& right);
    std::string to_string() const;
    unsigned int get_id() const;

    bool operator==(const Production& other) const
    {
        return id_ == other.id_;
    }
private:
    unsigned int id_;
    Symbol left_;
    std::vector<Symbol> right_;
};

namespace std {
    template<>
    struct hash<Production>
    {
        size_t operator()(const Production& production) const
        {
            return hash<unsigned int>()(production.get_id());   // 只哈希 id
        }
    };
}