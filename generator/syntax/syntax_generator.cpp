#include "grammar.h"
#include "syntax_rule_parser.h"
#include "lr_parser.h"
#include <filesystem>


int main()
{
    std::filesystem::path file{"./resource/rule/syntax_test.txt"};

    Grammar grammar;
    SyntaxRuleParser::parse(file, grammar);
    grammar.show();

    grammar.compute_first_sets();
    grammar.compute_follow_sets();
    grammar.show_first_follow();

    LRParser parser{&grammar};
    parser.build_states();
    parser.show_states();

    return 0;
}