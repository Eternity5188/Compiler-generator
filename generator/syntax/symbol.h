#pragma once


#include <string>
#include <string_view>


enum class Associativity
{
    Left, Right, None
};

class Symbol
{
public:
    enum class Type
    {
        Terminal, NonTerminal
    };

public:
    Symbol(Type type, const std::string_view name);
    Symbol(Type type, const std::string_view name, unsigned int precedence, Associativity associativity);

    void set_name(const std::string_view name);
    std::string get_name() const;

    bool operator==(const Symbol& other) const
    {
        return name_ == other.name_;
    }

private:
    Type type_;
    std::string name_;
    bool is_operator_;
    unsigned int precedence_;
    Associativity associativity_;
};

namespace std {
    template<>
    struct hash<Symbol>
    {
        size_t operator()(const Symbol& symbol) const
        {
            return hash<std::string>()(symbol.get_name());
        }
    };
}