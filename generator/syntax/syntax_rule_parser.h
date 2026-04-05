#pragma once


#include "symbol.h"
#include "production.h"
#include <filesystem>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>
#include <memory>
#include <optional>


class SyntaxRuleParser
{
public:
    SyntaxRuleParser();

    bool init(const std::filesystem::path& file_path);
    bool parse();
    void show() const;

    std::optional<Symbol> get_symbol(const std::string_view name) const;
private:
    bool read_lines(std::vector<std::string>& lines);
    bool parse_declaration(const std::vector<std::string>& lines, std::size_t begin, std::size_t size);
    bool parse_rule(const std::vector<std::string>& lines, std::size_t begin, std::size_t size);
private:
    bool inited_;
    std::filesystem::path file_path_;
    Symbol start_symbol;
    std::unordered_set<Symbol> symbols_;
    std::unordered_set<Production> productions_;
};