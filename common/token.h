#pragma once


#include <variant>
#include <string>


class Token
{
public:
    enum class Type
    {
        Identifier, Number
    };

    template<typename Value_Type>
    Token(Type type, Value_Type value, unsigned int line, unsigned int column)
        :type_{type}, value_{std::move(value)}, line_{line}, column_{column}
    {}
    Token(Type type, const char* value, unsigned int line, unsigned int column)
        :type_{type}, value_{std::string{value}}, line_{line}, column_{column}
    {}
    Token(const Token& other)
        :type_{other.type_}, value_{other.value_}, line_{other.line_}, column_{other.column_}
    {}


    Type get_type() const;
    void set_type(Type type);

    template<typename Value_Type>
    Value_Type get_value() const
    {
        if (has_value<Value_Type>() == false)
            throw std::bad_variant_access{};
        
        return std::get<Value_Type>(value_);
    }
    template<typename Value_Type>
    Value_Type set_value(Value_Type value)
    {
        if (has_value<Value_Type>() == false)
            throw std::bad_variant_access{};

        value_ = value;
    }
    template<typename Value_Type>
    bool has_value() const
    {
        return std::holds_alternative<Value_Type>(value_);
    }

    unsigned int get_line() const;
    void set_line(unsigned int line);

    unsigned int get_column() const;
    void set_column(unsigned int column);

private:
    Type type_;
    std::variant<int, std::string> value_;
    unsigned int line_;
    unsigned int column_;
};