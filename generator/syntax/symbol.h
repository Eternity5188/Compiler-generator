#pragma once


#include "associativity.h"
#include <cstdint>
#include <string>
#include <string_view>


class Symbol
{
public:
    enum class Type
    {
        Terminal, NonTerminal, Epsilon, End
    };

public:
    static const Symbol get_epsilon();
    static const Symbol get_end();

    Symbol(Type type, const std::string_view name);
    Symbol(Type type, const std::string_view name, uint32_t precedence, Associativity associativity);

    void set_name(const std::string_view name);

    const Type get_type() const;
    const std::string get_name() const;

    bool operator==(const Symbol& other) const
    {
        return name_ == other.name_;
    }

private:
    Type type_;
    std::string name_;
    bool is_operator_;
    uint32_t precedence_;
    Associativity associativity_;
};

namespace std {
    template<>
    struct hash<Symbol>
    {
        size_t operator()(const Symbol& symbol) const
        {
            return hash<std::string>{}(symbol.get_name());
        }
    };
}