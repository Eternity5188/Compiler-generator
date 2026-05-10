#include "ast_tree.h"

#include <queue>
#include <iostream>


ASTTree::ASTTree()
    :root_{nullptr}
{}

void ASTTree::clear()
{
    nodes_.clear();
    root_ = nullptr;
}

const ASTNode* ASTTree::add_node(const std::string_view type)
{
    nodes_.emplace_back(type);
    return &nodes_.back();
}
const ASTNode* ASTTree::add_node(const std::string_view type, const std::string_view value)
{
    nodes_.emplace_back(type, value);
    return &nodes_.back();
}
void ASTTree::set_root(const ASTNode* root)
{
    root_ = const_cast<ASTNode*>(root);
}
const ASTNode* ASTTree::get_root() const
{
    return root_;
}

void ASTTree::show() const
{
    if (root_ == nullptr)
    {
        std::cout << "Empty AST" << '\n';
        return;
    }

    std::cout << "\n==========\n";
    std::cout << "AST tree: " << '\n';
    std::queue<const ASTNode*> q;
    q.push(root_);
    std::size_t level = 0;
    while (q.empty() == false)
    {
        std::size_t level_size = q.size();
        std::cout << "Level " << level << ":\n";

        for (int i = 0; i < level_size; ++i)
        {
            const ASTNode* node = q.front();
            q.pop();

            std::cout << "[ " << node->get_type() << " ]";
            if (node->get_value().empty() == false)
                std::cout << "(" << node->get_value() << ")";
            std::cout << " ";

            for (const ASTNode* child : node->get_children())
                q.push(child);
        }

        std::cout << '\n';
        ++level;
    }
}