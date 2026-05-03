#pragma once


#include <filesystem>
#include <vector>
#include <string>


class Grammar;

class SyntaxRuleParser
{
public:
    static bool parse(const std::filesystem::path& file_path, Grammar& grammar);
private:
    static bool read_lines(std::vector<std::string>& lines, const std::filesystem::path& file_path);
    static bool parse_declaration(const std::vector<std::string>& lines, std::size_t begin, std::size_t size, Grammar& grammar);
    static bool parse_rule(const std::vector<std::string>& lines, std::size_t begin, std::size_t size, Grammar& grammar);
};