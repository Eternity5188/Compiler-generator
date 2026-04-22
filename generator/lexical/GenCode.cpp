#include "GenCode.h"
void GenCode::printDeclaration()
{
    // 1. Include 头文件
    out << "#include <iostream>\n";
    out << "#include <vector>\n";
    out << "#include <unordered_map>\n";
    out << "#include <string>\n";
    out << "#include <fstream>\n";
    out << "#include <cstdio>\n";
    out << "#include <cstring>\n";
    out << "using namespace std;\n";

    // 2. 定义全局变量
    out << "string yytext;\n";
    out << "int yylval;\n";
    out << "int column = 0;\n\n";
    for (auto& s : user_declarations)
        out << s << "\n";
}

void GenCode::printSubroutines()
{
    for (auto& s : subroutines)
        out << s << "\n";
}

void GenCode::printActions()
{
    out << "int action(int tokenID) {\n";
    out << "    switch (tokenID) {\n";
    for (size_t i = 0; i < rules.size(); ++i) {
        out << "    case " << i << ":\n";
        // 处理return语句
        string action = rules[i].actions;
        if (action.find("return(") != string::npos) {
            // 替换return语句为返回值
            size_t returnPos = action.find("return(");
            size_t endPos = action.find(")", returnPos);
            if (endPos != string::npos) {
                out << "        " << action << "\n";
                out << "        break;\n";
                continue;
            }
        }
        out << "        " << action << "\n";
        out << "        return 0;\n";
        out << "        break;\n";
    }
    out << "    }\n";
    out << "    return 0;\n";
    out << "}\n";
}

static string handleEscape(char c)
{
    switch (c)
    {
    case '\n':
        return "\\n";
    case '\t':
        return "\\t";
    case '\r':
        return "\\r";
    case '\v':
        return "\\v";
    case '\f':
        return "\\f";
    case '\\':
        return "\\\\";
    }
    return string(1,c);
}

void GenCode::printMinDFA()
{
    // 3. 定义 minDFA 数据
    out << "// DFA 转移表\n";
    out << "vector<unordered_map<char, int>> transitions = {\n";
    for (const auto& state : dfa.states) {
        out << "    {";
        bool first = true;
        for (const auto& [ch, target] : state->transitions) {
            if (!first) {
                out << ", ";
            }
            out << "{'" << handleEscape(ch) << "', " << target << "}";
            first = false;
        }
        out << "},\n";
    }
    out << "};\n\n";

    out << "// 接受状态及其 tokenID\n";
    out << "unordered_map<int, int> acceptStates = {\n";
    for (const auto& state : dfa.states) {
        if (state->isAccept) {
            out << "    {" << state->id << ", " << state->tokenID << "},\n";
        }
    }
    out << "};\n\n";

    out << "int startState = " << dfa.minDFAsid << ";\n\n";

    // 4. yylex() 函数
    out << "int yylex() {\n";
    out << "    static int currentState = startState;\n";
    out << "    static string lexeme = \"\";\n";
    out << "    char ch;\n";
    out << "    while (true) {\n";
    out << "        ch = cin.get();\n";
    out << "        if (cin.eof()) {\n";
    out << "            if (acceptStates.count(currentState)) {\n";
    out << "                int tokenID = acceptStates[currentState];\n";
    out << "                yytext = lexeme;\n";
    out << "                // 执行动作\n";
    out << "                int token = action(tokenID);\n";
    out << "                lexeme = \"\";\n";
    out << "                currentState = startState;\n";
    out << "                return token;\n";
    out << "            }\n";
    out << "            return 0;  // EOF\n";
    out << "        }\n";
    out << "        auto it = transitions[currentState].find(ch);\n";
    out << "        if (it != transitions[currentState].end()) {\n";
    out << "            currentState = it->second;\n";
    out << "            lexeme += ch;\n";
    out << "        } else {\n";
    out << "            if (acceptStates.count(currentState)) {\n";
    out << "                int tokenID = acceptStates[currentState];\n";
    out << "                yytext = lexeme;\n";
    out << "                // 执行动作\n";
    out << "                int token = action(tokenID);\n";
    out << "                lexeme = \"\";\n";
    out << "                currentState = startState;\n";
    out << "                cin.putback(ch);\n";
    out << "                return token;\n";
    out << "            } else {\n";
    out << "                cerr << \"Lexical error at char: \" << ch << std::endl;\n";
    out << "                return -1;\n";
    out << "            }\n";
    out << "        }\n";
    out << "    }\n";
    out << "}\n";

}

void GenCode::printMainFuc()
{
    out << "int main()\n";
    out << "{\n"; 
    out << "    ifstream file(\"test.txt\");\n";
    out << "    // 备份原始 cin\n";
    out << "    streambuf* origCin = cin.rdbuf();\n";
    out << "    cin.rdbuf(file.rdbuf()); // 替换 cin 为 file\n";
    out << "    cout << \"Lexical Analysis Result : \\n\";\n";
    out << "    int token;\n";
    out << "    while ((token = yylex()) != 0) {}\n";
    out << "    cin.rdbuf(origCin);  // 恢复 cin\n";
    out << "    return 0;\n";
    out << "}\n";
}

void GenCode::genLexer()
{
	printDeclaration();
	printSubroutines();
	printActions();
	printMinDFA();
	printMainFuc();
}
