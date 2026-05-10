#include "ast_node.h"


ASTNode::ASTNode(const std::string_view type)
    :type_{type}
{}
ASTNode::ASTNode(const std::string_view type, const std::string_view value)
    :type_{type}, value_{value}
{}

void ASTNode::add_child(const ASTNode* child) const
{
    children_.push_back(child);
}
const std::string& ASTNode::get_type() const
{
    return type_;
}
const std::string& ASTNode::get_value() const
{
    return value_;
}
const std::vector<const ASTNode*>& ASTNode::get_children() const
{
    return children_;
}