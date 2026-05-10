#pragma once


#include <string>
#include <string_view>
#include <vector>


class ASTNode
{
public:
    ASTNode(const std::string_view type);
    ASTNode(const std::string_view type, const std::string_view value);

    void add_child(const ASTNode* child) const;
    const std::string& get_type() const;
    const std::string& get_value() const;
    const std::vector<const ASTNode*>& get_children() const;

private:
    std::string type_;
    std::string value_;
    mutable std::vector<const ASTNode*> children_;
};