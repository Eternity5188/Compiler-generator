#include "production.h"


#include "symbol.h"


Production::Production(uint32_t id, const Symbol* left, const std::vector<const Symbol*>& right)
    :id_{id}, left_{left}, right_{right}
{}

const std::string Production::to_string() const
{
    std::string result;
    std::string space{" "};
    result += std::to_string(id_) + space;
    result += left_->get_name() + space;
    result.append("->");
    result += space;
    for (auto& symbol : right_)
    {
        result += symbol->get_name() + space;
    }
    return result;
}

const uint32_t Production::get_id() const
{
    return id_;
}
const Symbol* Production::get_left() const
{
    return left_;
}
const std::vector<const Symbol*>& Production::get_right() const
{
    return right_;
}