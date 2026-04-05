#include "syntax_rule_parser.h"
#include <filesystem>


int main()
{
    std::filesystem::path file{"./resource/rule/syntax_rule.txt"};

    SyntaxRuleParser parser;
    parser.init(file);

    parser.parse();

    parser.show();

    return 0;
}