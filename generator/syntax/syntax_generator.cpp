#include "grammar.h"
#include "syntax_rule_parser.h"
#include "lr_parser.h"
#include <filesystem>
#include <iostream>


int main()
{
    std::filesystem::path file{"./resource/rule/syntax_rule.txt"};

    Grammar grammar;
    SyntaxRuleParser::parse(file, grammar);
    grammar.show();

    grammar.compute_first_sets();
    grammar.compute_follow_sets();
    grammar.show_first_follow();

    LRParser parser{&grammar};
    parser.build_states();
    // parser.show_states();
    // parser.merge_states();
    parser.construct_tables();
    // parser.show_tables();

    std::vector<std::vector<std::string>> tokens_list {
        {"INT", "IDENTIFIER", "SEMICOLON"},
        {"INT", "IDENTIFIER", "LPAREN", "RPAREN", "LBRACE", "RBRACE"},
        {"INT", "IDENTIFIER", "LPAREN", "RPAREN", "LBRACE", "RETURN", "NUMBER", "SEMICOLON", "RBRACE"},
        {"INT", "IDENTIFIER", "LPAREN", "RPAREN", "LBRACE", "IDENTIFIER", "ASSIGN", "NUMBER", "SEMICOLON", "RBRACE"},
        {"INT", "IDENTIFIER", "LPAREN", "RPAREN", "LBRACE", "IF", "LPAREN", "IDENTIFIER", "RPAREN", "RETURN", "NUMBER", "SEMICOLON", "RBRACE"},
    };

    int counter = 0;
    for (auto& tokens : tokens_list)
    {
        std::cout << counter << ": ";
        std::cout << (parser.parse(tokens) ? "Passed" : "Failed") << '\n';
        ++counter;
    }

    return 0;
}