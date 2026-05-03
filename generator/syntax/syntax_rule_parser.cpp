#include "syntax_rule_parser.h"


#include "symbol.h"
#include "production.h"
#include "grammar.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>


bool SyntaxRuleParser::parse(const std::filesystem::path& file_path, Grammar& grammar)
{
    if (std::filesystem::is_regular_file(file_path) == false)
        return false;

    // 读取所有的行
    std::vector<std::string> lines;
    if (read_lines(lines, file_path) == false)
        return false;

    // 查找 "%%"
    std::size_t index = 0;
    while (index < lines.size() && lines[index].find("%%") == std::string::npos)
        ++index;

    if (parse_declaration(lines, 0, index, grammar) == false)
        return false;
    if (parse_rule(lines, index + 1, lines.size() - index - 1, grammar) == false)
        return false;

    return true;
}

bool SyntaxRuleParser::read_lines(std::vector<std::string>& lines, const std::filesystem::path& file_path)
{
    std::ifstream file_in(file_path);
    if (file_in.is_open() == false)
        return false;

    std::string line;
    bool in_comment { false };

    while (std::getline(file_in, line))
    {
        std::size_t index = 0;
        std::string data;
        while (index < line.size())
        {
            if (in_comment)
            {
                if (index + 1 < line.size() && line[index] == '*' && line[index + 1] == '/')
                {
                    in_comment = false;
                    index += 2;
                }
                else
                    ++index;
            }
            else
            {
                if (index + 1 < line.size() && line[index] == '/' && line[index + 1] == '*')
                {
                    in_comment = true;
                    index += 2;
                }
                else
                {
                    data.push_back(line[index]);
                    ++index;
                }
            }
        }

        if (data.empty() == false)
            lines.push_back(data);
    }
    file_in.close();

    return true;
}
bool SyntaxRuleParser::parse_declaration(const std::vector<std::string>& lines, std::size_t begin, std::size_t size, Grammar& grammar)
{
    std::string token;
    unsigned int precedence = 1;
    for (std::size_t index = begin; index < begin + size; ++index)
    {
        std::string type;
        std::istringstream str_in(lines[index]);
        str_in >> type;

        if (str_in.fail())
        {
            str_in.clear();
            continue;
        }
        if (type == "%token")        
        {
            while (str_in >> token)
                grammar.add_terminal(token);
            continue;
        }
        if (type == "%left")
        {
            while (str_in >> token)
                grammar.set_operator(token, precedence, Associativity::Left);
            ++precedence;
            continue;
        }
        if (type == "%right")
        {
            while (str_in >> token)
                grammar.set_operator(token, precedence, Associativity::Right);
            ++precedence;
            continue;
        }
        if (type == "%start")
        {
            str_in >> token;
            grammar.add_non_terminal(token);
            grammar.set_start_symbol(token);
            continue;
        }
    }
    return true;
}
bool SyntaxRuleParser::parse_rule(const std::vector<std::string>& lines, std::size_t begin, std::size_t size, Grammar& grammar)
{
    std::string left_name;
    std::vector<std::string> right_names;
    
    unsigned int state = 0;
    for (std::size_t index = begin; index < begin + size; ++index)
    {   
        std::istringstream str_in{lines[index]};
        std::string token;

        while (str_in >> token)
        {
            switch (state)
            {
            case 0:
                if (token == ":" || token == ";")
                {
                    std::cerr << "Invalid production" << '\n';
                    return false;
                }
                left_name = token;
                if (grammar.get_non_terminal(token) == nullptr)
                    grammar.add_non_terminal(token);
                state = 1;
                break;
            case 1:
                if (token != ":")
                {
                    std::cerr << "Invalid production" << '\n';
                    return false;
                }
                state = 2;
                break;
            case 2:
                if (token == "|")
                {
                    grammar.add_production(left_name, right_names);
                    right_names.clear();
                }
                else if (token == ";")
                {
                    grammar.add_production(left_name, right_names);
                    right_names.clear();
                    state = 0;
                }
                else
                {
                    right_names.push_back(token);
                    if (grammar.get_symbol(token) == nullptr)
                        grammar.add_non_terminal(token);
                }
                break;
            }
        }
    }

    return true;
}