#include "lr_item.h"


#include "symbol.h"
#include "production.h"


LRItem LRItem::advance_dot(const LRItem& item)
{
    return LRItem{item.production_, item.dot_pos_ + 1, item.lookahead_};
}

LRItem::LRItem(const Production* production, uint32_t dot_pos, const Symbol* lookahead)
    :production_{production}, dot_pos_{dot_pos}, lookahead_{lookahead}
{}

const std::string LRItem::to_string() const
{
    std::string result;
    std::string space{" "};
    result += production_->get_left()->get_name() + space;
    result.append("->");
    result += space;
    auto& right_symbols = production_->get_right();
    for (std::size_t i = 0; i < right_symbols.size(); ++i)
    {
        if (i == dot_pos_)
            result.append(".");
        result += right_symbols[i]->get_name() + space;
    }
    if (dot_pos_ == right_symbols.size())
        result.append(".");
    result += std::string{", "} + lookahead_->get_name();

    return result;
}

const Production* LRItem::get_production() const
{
    return production_;
}
const uint32_t LRItem::get_dot_pos() const
{
    return dot_pos_;
}
const Symbol* LRItem::get_lookahead() const
{
    return lookahead_;
}
const Symbol* LRItem::get_next_symbol() const
{
    const auto& right_symbols = production_->get_right();
    return dot_pos_ < right_symbols.size() ? right_symbols[dot_pos_] : nullptr;
}
bool LRItem::is_dot_at_end() const
{
    return dot_pos_ == production_->get_right().size();
}

bool LRItem::operator==(const LRItem& other) const
{
    return production_ == other.production_
        && dot_pos_ == other.dot_pos_
        && lookahead_ == other.lookahead_;
}