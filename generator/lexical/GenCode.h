#pragma once
#ifndef GENCODE_H
#define GENCODE_H

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "DFA.h"

using namespace std;

class GenCode
{
public:
    DFA& dfa;
    vector<string>& user_declarations;
    vector<Rule>& rules;
    vector<string>& subroutines;

    string sourceFileName;
    string headerFileName;
    fstream sourceOut;
    fstream headerOut;

    GenCode(DFA& dfa,
            vector<string>& user_declarations,
            vector<Rule>& rules,
            vector<string>& subroutines,
            string sourceFileName = "lexical_parser.cpp",
            string headerFileName = "lexical_parser.h");
    ~GenCode();

    void printHeader();
    void printDeclaration();
    void printTokenMapping();
    void printRuleActions();
    void printMinDFA();
    void genLexer();

private:
    string getStandardTokenType(const string& action) const;
};

#endif // !GENCODE_H
