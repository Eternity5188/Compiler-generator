#pragma once


#include "ast_node.h"
#include <string_view>
#include <vector>
#include <deque>


class ASTTree
{
public:
    ASTTree();

    void clear();

    const ASTNode* add_node(const std::string_view type);
    const ASTNode* add_node(const std::string_view type, const std::string_view value);
    void set_root(const ASTNode* root);
    const ASTNode* get_root() const;

    void show() const;

private:
    std::deque<ASTNode> nodes_; 
    ASTNode* root_;
};