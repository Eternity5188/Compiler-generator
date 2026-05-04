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
    static const Symbol& get_epsilon();
    static const Symbol& get_end();

    Symbol(Type type, const std::string_view name);
    Symbol(Type type, const std::string_view name, uint32_t precedence, Associativity associativity);

    const std::string to_string() const;

    void set_name(const std::string_view name);
    void set_operator(uint32_t precedence, Associativity associativity) const;
    const Type get_type() const;
    const std::string get_name() const;

    bool operator==(const Symbol& other) const
    {
        return name_ == other.name_;
    }

private:
    static Symbol epsilon;
    static Symbol end;

    Type type_;
    std::string name_;
    mutable bool is_operator_;
    mutable uint32_t precedence_;
    mutable Associativity associativity_;
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