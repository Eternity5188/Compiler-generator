#pragma once


#include <cstdint>
#include <string>
#include <vector>


class Symbol;

class Production
{
public:
    Production();
    Production(uint32_t id, const Symbol* left, const std::vector<const Symbol*>& right);
    
    const std::string to_string() const;
    
    void reset(uint32_t id, const Symbol* left, const std::vector<const Symbol*>& right);
    const uint32_t get_id() const;
    const Symbol* get_left() const;
    const std::vector<const Symbol*>& get_right() const;

    bool operator==(const Production& other) const
    {
        return id_ == other.id_;
    }
private:
    uint32_t id_;
    const Symbol* left_;
    std::vector<const Symbol*> right_;
};

namespace std {
    template<>
    struct hash<Production>
    {
        size_t operator()(const Production& production) const
        {
            return hash<uint32_t>{}(production.get_id());
        }
    };
}