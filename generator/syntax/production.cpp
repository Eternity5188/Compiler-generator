#include "production.h"


Production::Production(unsigned int id, const Symbol& left, const std::vector<Symbol>& right)
    :id_{id}, left_{left}, right_{right}
{}
std::string Production::to_string() const
{
    std::string result;
    std::string space{" "};
    result += std::to_string(id_) + space;
    result += left_.get_name() + space;
    result.append("->");
    result += space;
    for (auto& symbol : right_)
    {
        result += symbol.get_name() + space;
    }
    return result;
}
unsigned int Production::get_id() const
{
    return id_;
}