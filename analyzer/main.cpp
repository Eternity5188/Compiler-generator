#include "token.h"
#include <iostream>


int main()
{
    std::cout << "Analyzer" << '\n';

    Token token1{Token::Type::Identifier, "Hello", 1, 2};
    std::cout << token1.get_value<std::string>() << '\n';

    Token token2{Token::Type::Number, 12, 1, 2};
    std::cout << token2.get_value<int>() << '\n';

    return 0;
}