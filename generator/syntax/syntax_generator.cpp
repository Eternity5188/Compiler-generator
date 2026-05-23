#include "grammar.h"
#include "syntax_rule_parser.h"
#include "ast_tree.h"
#include "lr_parser.h"
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>


int main()
{
    std::filesystem::path rule_file{"./resource/rule/syntax/syntax_rule.txt"};

    Grammar grammar;
    SyntaxRuleParser::parse(rule_file, grammar);

    grammar.compute_first_sets();
    grammar.compute_follow_sets();

    LRParser parser{&grammar};
    parser.build_states();
    parser.construct_tables();

    std::filesystem::path h_file{"./resource/source/syntax_parser.h"};
    std::filesystem::path cpp_file{"./resource/source/syntax_parser.cpp"};
    std::filesystem::create_directories(h_file.parent_path());
    std::ofstream h{h_file};
    std::ofstream cpp{cpp_file};

    // generate .h file
    h << R"(
#pragma once
#include "token.h"
#include <cstdint>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector>
)";

    h << R"(
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
    const TableEntry* find_action(uint32_t state, const std::string& symbol) const;
    const TableEntry* find_goto(uint32_t state, const std::string& symbol) const;

private:
    std::vector<Production> grammar;
    std::vector<TableEntry> action_table;
    std::vector<TableEntry> goto_table;
    std::unordered_map<uint32_t, std::unordered_map<std::string, std::size_t>> action_index;
    std::unordered_map<uint32_t, std::unordered_map<std::string, std::size_t>> goto_index;
    ASTNode* ast_tree;
};
)";

    // generate .cpp file
    cpp << R"(
#include "syntax_parser.h"
#include <stack>
#include <algorithm>
#include <functional>
#include <iostream>
)";

    cpp << R"(
namespace
{
struct RawTableEntry
{
    uint32_t state;
    const char* symbol;
    const char* type;
    uint32_t next_state;
};

const RawTableEntry action_table_data[] = {
)";
    for (auto& e : parser.get_export_action_table())
        cpp << "    {" << e.state << ", \"" << e.symbol << "\", \"" << e.type << "\", " << e.next_state << "},\n";
    cpp << R"(
};

const RawTableEntry goto_table_data[] = {
)";
    for (auto& e : parser.get_export_goto_table())
        cpp << "    {" << e.state << ", \"" << e.symbol << "\", \"" << e.type << "\", " << e.next_state << "},\n";
    cpp << R"(
};
}
)";

    cpp << R"(
SyntaxParser::SyntaxParser()
    :ast_tree{nullptr}
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
    cpp << R"(
    action_table.reserve(sizeof(action_table_data) / sizeof(action_table_data[0]));
    for (const RawTableEntry& entry : action_table_data)
        action_table.push_back({entry.state, entry.symbol, entry.type, entry.next_state});

    goto_table.reserve(sizeof(goto_table_data) / sizeof(goto_table_data[0]));
    for (const RawTableEntry& entry : goto_table_data)
        goto_table.push_back({entry.state, entry.symbol, entry.type, entry.next_state});

    action_index.reserve(action_table.size());
    for (std::size_t i = 0; i < action_table.size(); ++i)
        action_index[action_table[i].state].emplace(action_table[i].symbol, i);

    goto_index.reserve(goto_table.size());
    for (std::size_t i = 0; i < goto_table.size(); ++i)
        goto_index[goto_table[i].state].emplace(goto_table[i].symbol, i);
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


        const TableEntry* action = find_action(cur_state, cur_sym);

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

            uint32_t top_state = state_stack.top();
            const TableEntry* goto_entry = find_goto(top_state, rule.left);
            if (!goto_entry)
                return false;

            state_stack.push(goto_entry->next_state);
        }

        // ====================== Accept
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

const TableEntry* SyntaxParser::find_action(uint32_t state, const std::string& symbol) const
{
    const auto state_it = action_index.find(state);
    if (state_it == action_index.end())
        return nullptr;

    const auto symbol_it = state_it->second.find(symbol);
    if (symbol_it == state_it->second.end())
        return nullptr;

    return &action_table[symbol_it->second];
}

const TableEntry* SyntaxParser::find_goto(uint32_t state, const std::string& symbol) const
{
    const auto state_it = goto_index.find(state);
    if (state_it == goto_index.end())
        return nullptr;

    const auto symbol_it = state_it->second.find(symbol);
    if (symbol_it == state_it->second.end())
        return nullptr;

    return &goto_table[symbol_it->second];
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
