#include "syntax_rule_parser.h"


#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>


SyntaxRuleParser::SyntaxRuleParser()
    :inited_{false}, file_path_{}, start_symbol{Symbol::Type::NonTerminal, ""}
{}

bool SyntaxRuleParser::init(const std::filesystem::path& file_path)
{
    if (std::filesystem::is_regular_file(file_path) == false)
        return false;

    file_path_ = file_path;
    inited_ = true;
    return true;
}
bool SyntaxRuleParser::parse()
{
    if (inited_ == false)
        return false;

    // 读取所有的行
    std::vector<std::string> lines;
    if (read_lines(lines) == false)
        return false;
    
    // 查找 "%%"
    std::size_t index = 0;
    while (index < lines.size() && lines[index].find("%%") == std::string::npos)
        ++index;

    if (parse_declaration(lines, 0, index) == false)
        return false;
    if (parse_rule(lines, index + 1, lines.size() - index - 1) == false)
        return false;
    return true;
}
void SyntaxRuleParser::show() const
{
    std::cout << "Start: " << '\n';
    std::cout << start_symbol.get_name() << '\n';
    
    std::cout << "\n==========\n";

    std::cout << "Symbol: " << '\n';
    for (auto& symbol : symbols_)
    {
        std::cout << symbol.get_name() << '\n';
    }
    
    std::cout << "\n==========\n";

    std::cout << "Production: " << '\n';
    for (auto& production : productions_)
    {
        std::cout << production.to_string() << '\n';
    }
}
std::optional<Symbol> SyntaxRuleParser::get_symbol(const std::string_view name) const
{
    auto it = std::find_if(symbols_.begin(), symbols_.end(),
        [name](const Symbol& symbol) { return symbol.get_name() == name; }
    );
    if (it != symbols_.end())
        return *it;
    return std::nullopt;
}

bool SyntaxRuleParser::read_lines(std::vector<std::string>& lines)
{
    std::ifstream file_in(file_path_);
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

bool SyntaxRuleParser::parse_declaration(const std::vector<std::string>& lines, std::size_t begin, std::size_t size)
{
    std::string token;
    unsigned int precedence = 1;
    for (std::size_t index = begin; index < size; ++index)
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
                symbols_.emplace(Symbol::Type::Terminal, token);
            continue;
        }
        if (type == "%left")
        {
            while (str_in >> token)
                symbols_.emplace(Symbol::Type::Terminal, token, precedence, Associativity::Left);
            ++precedence;
            continue;
        }
        if (type == "%right")
        {
            while (str_in >> token)
                symbols_.emplace(Symbol::Type::Terminal, token, precedence, Associativity::Right);
            ++precedence;
            continue;
        }
        if (type == "%start")
        {
            str_in >> token;
            start_symbol.set_name(token);
            continue;
        }
    }
    return true;
}
bool SyntaxRuleParser::parse_rule(const std::vector<std::string>& lines, std::size_t begin, std::size_t size)
{
    unsigned int production_id = 0;
    bool in_production { false };
    Symbol left{Symbol::Type::NonTerminal, ""};
    std::vector<Symbol> right;
    unsigned int state = 0;

    auto get_or_create_symbol = [this](const std::string& name, Symbol::Type default_type = Symbol::Type::NonTerminal) -> Symbol {
        auto opt = get_symbol(name);
        if (opt)
            return *opt;
        Symbol new_sym(default_type, name);
        symbols_.insert(new_sym);
        return new_sym;
    };

    for (std::size_t index = begin; index < size; ++index)
    {   
        std::istringstream str_in{lines[index]};
        std::string token;

        while (str_in >> token)
        {
            if (token == "%prec")
            {
                str_in >> token;  // 暂时跳过优先级标记（如 UMINUS）
                continue;
            }

            switch (state)
            {
            case 0:
                if (token == ":" || token == ";")
                {
                    std::cerr << "Invalid production" << '\n';
                    return false;
                }
                left = get_or_create_symbol(token);
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
                if (token == "|" || token == ";")
                {
                    productions_.emplace(production_id, left, right);
                    ++production_id;
                    right.clear();
                    state = token == "|" ? 2 : 0;
                }
                else
                {
                    Symbol symbol = get_or_create_symbol(token);
                    right.push_back(symbol);
                }
                break;
            }
        }
    }

    return true;
}