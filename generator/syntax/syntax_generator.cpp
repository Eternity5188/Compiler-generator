#include "grammar.h"
#include "syntax_rule_parser.h"
#include "ast_tree.h"
#include "lr_parser.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


void generate_ast_recursive(std::ofstream& cpp, const ASTNode* node, int& node_id);
void generate_ast_tree(std::ofstream& cpp, const ASTNode* root);


int main()
{
    std::filesystem::path rule_file{"./resource/rule/syntax_rule.txt"};

    Grammar grammar;
    SyntaxRuleParser::parse(rule_file, grammar);

    grammar.compute_first_sets();
    grammar.compute_follow_sets();

    LRParser parser{&grammar};
    parser.build_states();
    parser.construct_tables();

    std::filesystem::path h_file{"./resource/source/syntax_parser.h"};
    std::filesystem::path cpp_file{"./resource/source/syntax_parser.cpp"};
    std::ofstream h{h_file};
    std::ofstream cpp{cpp_file};

    // 生成.h文件
    h << R"(
#pragma once
#include <cstdint>
#include <string>
#include <filesystem>
#include <vector>
)";

    h << R"(
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
)";

    h << R"(
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
)";

    // 生成.cpp文件
    cpp << R"(
#include "syntax_parser.h"
#include <stack>
#include <algorithm>
#include <functional>
#include <iostream>
)";

    cpp << R"(
SyntaxParser::SyntaxParser()
    :grammar{}, action_table{}, goto_table{}, ast_tree{nullptr}
{
)";

    for (auto& p : parser.get_export_grammar())
    {
        cpp << "grammar.push_back({\"" << p.left << "\"";
        cpp << ", {";
        for (std::size_t i = 0; i < p.right.size(); i++)
        {
            cpp << "\"" << p.right[i] << "\"";
            if (i < p.right.size() - 1)
                cpp << ", ";
        }
        cpp << "}});\n";
    }
    for (auto& e : parser.get_export_action_table())
        cpp << "    action_table.push_back({" << e.state << ", \"" << e.symbol << "\", \"" << e.type << "\", " << e.next_state << "});\n";
    for (auto& e : parser.get_export_goto_table())
        cpp << "    goto_table.push_back({" << e.state << ", \"" << e.symbol << "\", \"" << e.type << "\", " << e.next_state << "});\n";

    cpp << R"(
}
)";

    cpp << R"(
SyntaxParser::~SyntaxParser()
{
    clear_ast_tree();
}
)";

    cpp << R"(
bool SyntaxParser::parse(const std::vector<Token>& tokens)
{
    std::stack<uint32_t> state_stack;
    std::stack<ASTNode*> node_stack;

    state_stack.push(0);
    size_t pos = 0;

    clear_ast_tree();

    while (true)
    {
        uint32_t cur_state = state_stack.top();
        std::string cur_sym;

        if (pos < tokens.size())
            cur_sym = tokens[pos].type;
        else
            cur_sym = "$";


        // 查找 Action（完全用你的 action_table）
        const TableEntry* action = nullptr;
        for (const auto& entry : action_table)
        {
            if (entry.state == cur_state && entry.symbol == cur_sym)
            {
                action = &entry;
                break;
            }
        }

        if (!action)
        {
            std::cerr << "Parse error: state " << cur_state 
                      << " symbol " << cur_sym << '\n';
            return false;
        }

        // ====================== Shift
        if (action->type == "Shift")
        {
            state_stack.push(action->next_state);

            ASTNode* n = new ASTNode;
            n->type = cur_sym;
            n->value = tokens[pos].value;
            node_stack.push(n);

            pos++;
        }

        // ====================== Reduce
        else if (action->type == "Reduce")
        {
            uint32_t rule_idx = action->next_state;
            const Production& rule = grammar[rule_idx];

            std::vector<ASTNode*> children;
            for (size_t i = 0; i < rule.right.size(); i++)
            {
                children.push_back(node_stack.top());
                node_stack.pop();
                state_stack.pop();
            }

            std::reverse(children.begin(), children.end());

            ASTNode* parent = new ASTNode;
            parent->type = rule.left;
            parent->value = "";
            parent->children = children;

            node_stack.push(parent);

            // GOTO
            uint32_t top_state = state_stack.top();
            uint32_t goto_next = 0;
            for (const auto& entry : goto_table)
            {
                if (entry.state == top_state && entry.symbol == rule.left)
                {
                    goto_next = entry.next_state;
                    break;
                }
            }

            if (goto_next == 0)
                return false;

            state_stack.push(goto_next);
        }

        // ====================== Accept（你的表里一定有！）
        else if (action->type == "Accept")
        {
            ast_tree = node_stack.top();
            return true;
        }

        else
        {
            return false;
        }
    }
}
    )";

    cpp << R"(
std::vector<Production> SyntaxParser::get_grammar()
{
    return grammar;
}
std::vector<TableEntry> SyntaxParser::get_action_table()
{
    return action_table;
}
std::vector<TableEntry> SyntaxParser::get_goto_table()
{
    return goto_table;
}
ASTNode* SyntaxParser::get_ast_tree()
{
    return ast_tree;
}
)";

    cpp << R"(
)";

    cpp << R"(
void SyntaxParser::clear_ast_tree()
{
    if (ast_tree == nullptr)
        return;

    std::function<void(ASTNode*)> free = [&](ASTNode* n) {
        for (auto c : n->children) free(c);
        delete n;
    };
    free(ast_tree);
    ast_tree = nullptr;
}
)";

    std::cout << "Code generated: syntax_parser" << std::endl;
    return 0;
}