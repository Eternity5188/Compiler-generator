#include "grammar.h"
#include "syntax_rule_parser.h"
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

    return 0;
}