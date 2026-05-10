
#pragma once
#include <cstdint>
#include <string>
#include <filesystem>
#include <vector>

struct Token
{
    std::string type;
    std::string value;
};
struct Production
{
    std::string left;
    std::vector<std::string> right;
};
struct TableEntry
{
    uint32_t state;
    std::string symbol;
    std::string type;
    uint32_t next_state;
};
struct ASTNode
{
    std::string type;
    std::string value;
    std::vector<ASTNode*> children;
};

class SyntaxParser
{
public:
    SyntaxParser();
    ~SyntaxParser();

    bool parse(const std::vector<Token>& tokens);

    std::vector<Production> get_grammar();
    std::vector<TableEntry> get_action_table();
    std::vector<TableEntry> get_goto_table();
    ASTNode* get_ast_tree();
private:
    void clear_ast_tree();

private:
    std::vector<Production> grammar;
    std::vector<TableEntry> action_table;
    std::vector<TableEntry> goto_table;
    ASTNode* ast_tree;
};
