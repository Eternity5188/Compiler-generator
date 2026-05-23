#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "ReadLex.h"
#include "NormaliseRE.h"
#include "Infix2Postfix.h"
#include "NFA.h"
#include "DFA.h"
#include "GenCode.h"

using namespace std;
namespace fs = std::filesystem;

static string getLexRulePath(int argc, char* argv[])
{
    if (argc > 1)
        return argv[1];
    return "resource/rule/lexical/lexical_rule.txt";
}

static string getGeneratedLexerSourcePath(int argc, char* argv[])
{
    if (argc > 2)
        return argv[2];
    return "resource/source/lexical_parser.cpp";
}

static string getGeneratedLexerHeaderPath(int argc, char* argv[])
{
    if (argc > 3)
        return argv[3];
    return "resource/source/lexical_parser.h";
}

int main(int argc, char* argv[])
{
    const string lexRulePath = getLexRulePath(argc, argv);
    const string generatedLexerSourcePath = getGeneratedLexerSourcePath(argc, argv);
    const string generatedLexerHeaderPath = getGeneratedLexerHeaderPath(argc, argv);

    cout << "Lexical generator" << endl;
    cout << "rule file: " << lexRulePath << endl;
    cout << "source output file: " << generatedLexerSourcePath << endl;
    cout << "header output file: " << generatedLexerHeaderPath << endl;

    vector<string> user_declarations;
    unordered_map<string, string> elements;
    vector<Rule> rules;
    vector<string> subroutines;

    ReadLex rl(lexRulePath);
    rl.read_lex_file(user_declarations, elements, rules, subroutines);

    NormalizeRE nre;
    nre.RENormalization(rules, elements);

    Infix2Postfix i2s;
    i2s.prepare4nfa(rules);

    NFA nfa;
    NFA::createNcombineNfa(nfa, rules);

    const char* nfaDotDir = std::getenv("SEULEX_NFA_DOT_DIR");
    if (nfaDotDir != nullptr && nfaDotDir[0] != '\0')
        nfa.writeRuleDotFiles(rules, nfaDotDir);

    DFA dfa(nfa);
    dfa.buildDFA();
    dfa.hopcroftMinDFA();

    const fs::path sourcePath(generatedLexerSourcePath);
    if (sourcePath.has_parent_path())
        fs::create_directories(sourcePath.parent_path());

    const fs::path headerPath(generatedLexerHeaderPath);
    if (headerPath.has_parent_path())
        fs::create_directories(headerPath.parent_path());

    GenCode gencode(dfa, user_declarations, rules, subroutines, generatedLexerSourcePath, generatedLexerHeaderPath);
    gencode.genLexer();

    cout << "Generated lexer saved to: " << generatedLexerSourcePath << " and " << generatedLexerHeaderPath << endl;
    return 0;
}
