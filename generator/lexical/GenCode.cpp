#include "GenCode.h"

#include <cctype>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

static string trimCopy(string value)
{
    size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])))
        ++begin;

    size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])))
        --end;

    return value.substr(begin, end - begin);
}

static string extractReturnExpression(const string& action)
{
    size_t returnPos = action.find("return");
    if (returnPos == string::npos)
        return "";

    size_t open = action.find('(', returnPos);
    if (open == string::npos)
        return "";

    int depth = 1;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    bool escaped = false;

    for (size_t i = open + 1; i < action.size(); ++i) {
        char c = action[i];

        if (escaped) {
            escaped = false;
            continue;
        }
        if (c == '\\') {
            escaped = true;
            continue;
        }
        if (inSingleQuote) {
            if (c == '\'')
                inSingleQuote = false;
            continue;
        }
        if (inDoubleQuote) {
            if (c == '"')
                inDoubleQuote = false;
            continue;
        }
        if (c == '\'') {
            inSingleQuote = true;
            continue;
        }
        if (c == '"') {
            inDoubleQuote = true;
            continue;
        }
        if (c == '(') {
            ++depth;
            continue;
        }
        if (c == ')') {
            --depth;
            if (depth == 0)
                return trimCopy(action.substr(open + 1, i - open - 1));
        }
    }

    return "";
}

static string extractActionBlock(const string& action)
{
    size_t open = action.find('{');
    if (open == string::npos)
        return "{}";

    int depth = 0;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    bool escaped = false;

    for (size_t i = open; i < action.size(); ++i) {
        char c = action[i];

        if (escaped) {
            escaped = false;
            continue;
        }
        if (c == '\\') {
            escaped = true;
            continue;
        }
        if (inSingleQuote) {
            if (c == '\'')
                inSingleQuote = false;
            continue;
        }
        if (inDoubleQuote) {
            if (c == '"')
                inDoubleQuote = false;
            continue;
        }
        if (c == '\'') {
            inSingleQuote = true;
            continue;
        }
        if (c == '"') {
            inDoubleQuote = true;
            continue;
        }
        if (c == '{') {
            ++depth;
            continue;
        }
        if (c == '}') {
            --depth;
            if (depth == 0)
                return action.substr(open, i - open + 1);
        }
    }

    return action.substr(open);
}

static string escapeGeneratedString(const string& text)
{
    string result;
    for (char c : text) {
        switch (c) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\t': result += "\\t"; break;
        case '\r': result += "\\r"; break;
        default: result += c; break;
        }
    }
    return result;
}

static string mapReturnToStandardToken(const string& expr)
{
    if (expr.empty()) return "";
    if (expr == "check_type()") return "IDENTIFIER";
    return expr;
}

GenCode::GenCode(DFA& dfa,
                 vector<string>& user_declarations,
                 vector<Rule>& rules,
                 vector<string>& subroutines,
                 string sourceFileName,
                 string headerFileName)
    : dfa(dfa),
      user_declarations(user_declarations),
      rules(rules),
      subroutines(subroutines),
      sourceFileName(sourceFileName),
      headerFileName(headerFileName)
{
    sourceOut.open(this->sourceFileName, ios::out | ios::trunc);
    if (!sourceOut.is_open()) {
        cerr << "Error opening lexer source output file." << endl;
        exit(1);
    }

    headerOut.open(this->headerFileName, ios::out | ios::trunc);
    if (!headerOut.is_open()) {
        cerr << "Error opening lexer header output file." << endl;
        exit(1);
    }
}

GenCode::~GenCode()
{
    sourceOut.close();
    headerOut.close();
}

string GenCode::getStandardTokenType(const string& action) const
{
    return mapReturnToStandardToken(extractReturnExpression(action));
}

void GenCode::printHeader()
{
    headerOut << "#pragma once\n\n";
    headerOut << "#include \"token.h\"\n";
    headerOut << "#include <istream>\n";
    headerOut << "#include <vector>\n\n";
    headerOut << "Token yylex(std::istream& input);\n";
    headerOut << "std::vector<Token> tokenize(std::istream& input);\n";
}

void GenCode::printDeclaration()
{
    const string headerName = fs::path(headerFileName).filename().string();
    sourceOut << "#include \"" << escapeGeneratedString(headerName) << "\"\n";
    sourceOut << "#include <cstdio>\n";
    sourceOut << "#include <cstdlib>\n";
    sourceOut << "#include <iostream>\n";
    sourceOut << "#include <string>\n";
    sourceOut << "#include <unordered_map>\n";
    sourceOut << "#include <vector>\n\n";
    sourceOut << "using namespace std;\n\n";
    sourceOut << "static istream* yyin = nullptr;\n";
    sourceOut << "static string yytext_storage;\n";
    sourceOut << "static const char* yytext = \"\";\n";
    sourceOut << "static int yylval = 0;\n";
    sourceOut << "static int column = 0;\n\n";
    sourceOut << "struct LexToken\n";
    sourceOut << "{\n";
    sourceOut << "    string type;\n";
    sourceOut << "    string value;\n";
    sourceOut << "};\n\n";
}

void GenCode::printTokenMapping()
{
    sourceOut << R"(
#define ECHO ((void)0)

static constexpr int TOK_SKIP = 0;
static constexpr int AUTO = 256;
static constexpr int BOOL = 257;
static constexpr int BREAK = 258;
static constexpr int CASE = 259;
static constexpr int CHAR = 260;
static constexpr int COMPLEX = 261;
static constexpr int CONST = 262;
static constexpr int CONTINUE = 263;
static constexpr int DEFAULT = 264;
static constexpr int DO = 265;
static constexpr int DOUBLE = 266;
static constexpr int ELSE = 267;
static constexpr int ENUM = 268;
static constexpr int EXTERN = 269;
static constexpr int FLOAT = 270;
static constexpr int FOR = 271;
static constexpr int GOTO = 272;
static constexpr int IF = 273;
static constexpr int IMAGINARY = 274;
static constexpr int INLINE = 275;
static constexpr int INT = 276;
static constexpr int LONG = 277;
static constexpr int REGISTER = 278;
static constexpr int RESTRICT = 279;
static constexpr int RETURN = 280;
static constexpr int SHORT = 281;
static constexpr int SIGNED = 282;
static constexpr int SIZEOF = 283;
static constexpr int STATIC = 284;
static constexpr int STRUCT = 285;
static constexpr int SWITCH = 286;
static constexpr int TYPEDEF = 287;
static constexpr int UNION = 288;
static constexpr int UNSIGNED = 289;
static constexpr int VOID = 290;
static constexpr int VOLATILE = 291;
static constexpr int WHILE = 292;
static constexpr int IDENTIFIER = 293;
static constexpr int CONSTANT = 294;
static constexpr int STRING_LITERAL = 295;
static constexpr int ELLIPSIS = 296;
static constexpr int RIGHT_ASSIGN = 297;
static constexpr int LEFT_ASSIGN = 298;
static constexpr int ADD_ASSIGN = 299;
static constexpr int SUB_ASSIGN = 300;
static constexpr int MUL_ASSIGN = 301;
static constexpr int DIV_ASSIGN = 302;
static constexpr int MOD_ASSIGN = 303;
static constexpr int AND_ASSIGN = 304;
static constexpr int XOR_ASSIGN = 305;
static constexpr int OR_ASSIGN = 306;
static constexpr int RIGHT_OP = 307;
static constexpr int LEFT_OP = 308;
static constexpr int INC_OP = 309;
static constexpr int DEC_OP = 310;
static constexpr int PTR_OP = 311;
static constexpr int AND_OP = 312;
static constexpr int OR_OP = 313;
static constexpr int LE_OP = 314;
static constexpr int GE_OP = 315;
static constexpr int EQ_OP = 316;
static constexpr int NE_OP = 317;
static constexpr int TYPE_NAME = 318;

static string standardTokenTypeFromActionResult(int token)
{
    switch (token) {
    case AUTO: return "AUTO";
    case BOOL: return "BOOL";
    case BREAK: return "BREAK";
    case CASE: return "CASE";
    case CHAR: return "CHAR";
    case COMPLEX: return "COMPLEX";
    case CONST: return "CONST";
    case CONTINUE: return "CONTINUE";
    case DEFAULT: return "DEFAULT";
    case DO: return "DO";
    case DOUBLE: return "DOUBLE";
    case ELSE: return "ELSE";
    case ENUM: return "ENUM";
    case EXTERN: return "EXTERN";
    case FLOAT: return "FLOAT";
    case FOR: return "FOR";
    case GOTO: return "GOTO";
    case IF: return "IF";
    case IMAGINARY: return "IMAGINARY";
    case INLINE: return "INLINE";
    case INT: return "INT";
    case LONG: return "LONG";
    case REGISTER: return "REGISTER";
    case RESTRICT: return "RESTRICT";
    case RETURN: return "RETURN";
    case SHORT: return "SHORT";
    case SIGNED: return "SIGNED";
    case SIZEOF: return "SIZEOF";
    case STATIC: return "STATIC";
    case STRUCT: return "STRUCT";
    case SWITCH: return "SWITCH";
    case TYPEDEF: return "TYPEDEF";
    case UNION: return "UNION";
    case UNSIGNED: return "UNSIGNED";
    case VOID: return "VOID";
    case VOLATILE: return "VOLATILE";
    case WHILE: return "WHILE";
    case IDENTIFIER: return "IDENTIFIER";
    case CONSTANT: return "CONSTANT";
    case STRING_LITERAL: return "STRING_LITERAL";
    case ELLIPSIS: return "ELLIPSIS";
    case RIGHT_ASSIGN: return "RIGHT_ASSIGN";
    case LEFT_ASSIGN: return "LEFT_ASSIGN";
    case ADD_ASSIGN: return "ADD_ASSIGN";
    case SUB_ASSIGN: return "SUB_ASSIGN";
    case MUL_ASSIGN: return "MUL_ASSIGN";
    case DIV_ASSIGN: return "DIV_ASSIGN";
    case MOD_ASSIGN: return "MOD_ASSIGN";
    case AND_ASSIGN: return "AND_ASSIGN";
    case XOR_ASSIGN: return "XOR_ASSIGN";
    case OR_ASSIGN: return "OR_ASSIGN";
    case RIGHT_OP: return "RIGHT_OP";
    case LEFT_OP: return "LEFT_OP";
    case INC_OP: return "INC_OP";
    case DEC_OP: return "DEC_OP";
    case PTR_OP: return "PTR_OP";
    case AND_OP: return "AND_OP";
    case OR_OP: return "OR_OP";
    case LE_OP: return "LE_OP";
    case GE_OP: return "GE_OP";
    case EQ_OP: return "EQ_OP";
    case NE_OP: return "NE_OP";
    case TYPE_NAME: return "TYPE_NAME";
    case ';': return "';'";
    case ',': return "','";
    case ':': return "':'";
    case '=': return "'='";
    case '(': return "'('";
    case ')': return "')'";
    case '{': return "'{'";
    case '}': return "'}'";
    case '[': return "'['";
    case ']': return "']'";
    case '.': return "'.'";
    case '&': return "'&'";
    case '!': return "'!'";
    case '~': return "'~'";
    case '+': return "'+'";
    case '-': return "'-'";
    case '*': return "'*'";
    case '/': return "'/'";
    case '%': return "'%'";
    case '<': return "'<'";
    case '>': return "'>'";
    case '^': return "'^'";
    case '|': return "'|'";
    case '?': return "'?'";
    default: return "";
    }
}

static string lex_error;

static int input()
{
    if (yyin == nullptr)
        return 0;
    int ch = yyin->get();
    if (ch == EOF)
        return 0;
    return ch;
}

static void error(const char* msg)
{
    lex_error = msg;
}

static int yywrap(void)
{
    return 1;
}

static void comment(void)
{
    int c = 0;
    int prev = 0;
    while ((c = input()) != 0) {
        if (c == '/' && prev == '*')
            return;
        prev = c;
    }
    error("unterminated comment");
}

static void count(void)
{
    for (int i = 0; yytext[i] != '\0'; ++i) {
        char ch = yytext[i];
        if (ch == '\n')
            column = 0;
        else if (ch == '\t')
            column += 8 - (column % 8);
        else
            ++column;
    }
    ECHO;
}

static int check_type(void)
{
    return IDENTIFIER;
}

static string mapLexTokenTypeToParserTokenType(const string& type)
{
    if (type == "CONSTANT") return "NUMBER";
    if (type == "STRING_LITERAL") return "STRING";

    if (type == "INC_OP") return "INC";
    if (type == "DEC_OP") return "DEC";
    if (type == "AND_OP") return "AND";
    if (type == "OR_OP") return "OR";
    if (type == "PTR_OP") return "PTR";
    if (type == "LEFT_OP") return "LEFT_SHIFT";
    if (type == "RIGHT_OP") return "RIGHT_SHIFT";
    if (type == "LE_OP") return "LE";
    if (type == "GE_OP") return "GE";
    if (type == "EQ_OP") return "EQ";
    if (type == "NE_OP") return "NE";

    if (type == "';'") return "SEMICOLON";
    if (type == "','") return "COMMA";
    if (type == "':'") return "COLON";
    if (type == "'='") return "ASSIGN";
    if (type == "'('") return "LPAREN";
    if (type == "')'") return "RPAREN";
    if (type == "'{'") return "LBRACE";
    if (type == "'}'") return "RBRACE";
    if (type == "'['") return "LBRACKET";
    if (type == "']'") return "RBRACKET";
    if (type == "'.'") return "DOT";
    if (type == "'&'") return "AMPER";
    if (type == "'!'") return "NOT";
    if (type == "'~'") return "BIT_NOT";
    if (type == "'+'") return "ADD";
    if (type == "'-'") return "SUB";
    if (type == "'*'") return "MUL";
    if (type == "'/'") return "DIV";
    if (type == "'%'") return "MOD";
    if (type == "'<'") return "LT";
    if (type == "'>'") return "GT";
    if (type == "'^'") return "BIT_XOR";
    if (type == "'|'") return "BIT_OR";
    if (type == "'?'") return "QUESTION";

    return type;
}

static Token mapLexTokenToParserToken(const LexToken& token)
{
    return Token{mapLexTokenTypeToParserTokenType(token.type), token.value};
}
)";
}

void GenCode::printRuleActions()
{
    sourceOut << "static int runRuleAction(int tokenID)\n";
    sourceOut << "{\n";
    sourceOut << "    switch (tokenID) {\n";
    for (size_t i = 0; i < rules.size(); ++i) {
        sourceOut << "    case " << i << ":\n";
        sourceOut << "        " << extractActionBlock(rules[i].actions) << "\n";
        sourceOut << "        return TOK_SKIP;\n";
    }
    sourceOut << "    default:\n";
    sourceOut << "        return TOK_SKIP;\n";
    sourceOut << "    }\n";
    sourceOut << "}\n\n";
}

static string handleEscape(char c)
{
    switch (c)
    {
    case '\n': return "\\n";
    case '\t': return "\\t";
    case '\r': return "\\r";
    case '\v': return "\\v";
    case '\f': return "\\f";
    case '\\': return "\\\\";
    case '\'': return "\\'";
    }
    return string(1, c);
}

void GenCode::printMinDFA()
{
    sourceOut << "static vector<unordered_map<char, int>> transitions = {\n";
    for (const auto& state : dfa.states) {
        sourceOut << "    {";
        bool first = true;
        for (const auto& [ch, target] : state->transitions) {
            if (!first)
                sourceOut << ", ";
            sourceOut << "{'" << handleEscape(ch) << "', " << target << "}";
            first = false;
        }
        sourceOut << "},\n";
    }
    sourceOut << "};\n\n";

    sourceOut << "static unordered_map<int, int> acceptStates = {\n";
    for (const auto& state : dfa.states) {
        if (state->isAccept)
            sourceOut << "    {" << state->id << ", " << state->tokenID << "},\n";
    }
    sourceOut << "};\n\n";

    sourceOut << "static int startState = " << dfa.minDFAsid << ";\n\n";

    sourceOut << R"(
Token yylex(istream& input)
{
    yyin = &input;

    while (true) {
        int currentState = startState;
        int lastAcceptToken = -1;
        size_t lastAcceptLength = 0;
        string lexeme;

        while (true) {
            int next = input.get();
            if (next == EOF)
                break;

            char ch = static_cast<char>(next);
            auto stateIt = transitions[currentState].find(ch);
            if (stateIt == transitions[currentState].end()) {
                input.putback(ch);
                break;
            }

            currentState = stateIt->second;
            lexeme += ch;

            auto acceptIt = acceptStates.find(currentState);
            if (acceptIt != acceptStates.end()) {
                lastAcceptToken = acceptIt->second;
                lastAcceptLength = lexeme.size();
            }
        }

        if (lastAcceptToken >= 0) {
            for (size_t i = lexeme.size(); i > lastAcceptLength; --i)
                input.putback(lexeme[i - 1]);

            string accepted = lexeme.substr(0, lastAcceptLength);
            yytext_storage = accepted;
            yytext = yytext_storage.c_str();
            lex_error.clear();

            int actionResult = runRuleAction(lastAcceptToken);
            if (!lex_error.empty())
                return Token{"ERROR", lex_error};

            string standardType = standardTokenTypeFromActionResult(actionResult);
            if (standardType.empty())
                continue;

            LexToken lexToken{standardType, accepted};
            return mapLexTokenToParserToken(lexToken);
        }

        if (lexeme.empty()) {
            int bad = input.get();
            if (bad == EOF)
                return Token{"", ""};
            return Token{"ERROR", string(1, static_cast<char>(bad))};
        }

        return Token{"ERROR", lexeme};
    }
}

vector<Token> tokenize(istream& input)
{
    vector<Token> tokens;
    while (true) {
        Token token = yylex(input);
        if (token.type.empty())
            break;
        tokens.push_back(token);
        if (token.type == "ERROR")
            break;
    }
    return tokens;
}
)";
}

void GenCode::genLexer()
{
    printHeader();
    printDeclaration();
    printTokenMapping();
    printRuleActions();
    printMinDFA();
}

