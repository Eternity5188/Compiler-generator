#include "token.h"


Token::Type Token::get_type() const
{
    return type_;
}
void Token::set_type(Type type)
{
    type_ = type;
}

unsigned int Token::get_line() const
{
    return line_;
}
void Token::set_line(unsigned int line)
{
    line_ = line;
}

unsigned int Token::get_column() const
{
    return column_;
}
void Token::set_column(unsigned int column)
{
    column_ = column;
}