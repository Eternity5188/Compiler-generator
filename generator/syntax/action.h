#pragma once


#include <cstdint>
#include <string>


class Action
{
public:
    enum class Type
    {
        Shift, Reduce, Accept, Error
    };
public:
    Action();
    Action(Type type, uint32_t arg);

    const std::string to_string() const;

    const Type get_type() const;
    const uint32_t get_arg() const;

private:
    Type type_;
    uint32_t arg_;
};