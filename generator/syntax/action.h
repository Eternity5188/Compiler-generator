#pragma once


#include <cstdint>


class Action
{
public:
    enum class Type
    {
        Shift, Reduce, Accept, Error
    };
public:
    Action(Type type, uint32_t state_id);

    const Type get_type() const;
    const uint32_t get_state_id() const;

private:
    Type type_;
    uint32_t state_id_;
};