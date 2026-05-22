#include "Infix2Postfix.h"

namespace {
constexpr bool kRegexDebug = false;
}

void Infix2Postfix::add_dot(string& s)
{
    string res;
    int n = static_cast<int>(s.size());

    auto readToken = [&](int& i) {
        string token;
        if (s[i] == '\\' && i + 1 < n) {
            token += s[i++];
            token += s[i];
            return token;
        }

        token += s[i];
        return token;
    };

    auto canEndAtom = [](const string& token) {
        if (token.size() > 1 && token[0] == '\\')
            return true;

        char c = token[0];
        return c != '(' && c != '|';
    };

    auto canStartAtom = [](const string& token) {
        if (token.size() > 1 && token[0] == '\\')
            return true;

        char c = token[0];
        return c != ')' && c != '|' && c != '*' && c != '+' && c != '?';
    };

    bool prevCanEndAtom = false;
    for (int i = 0; i < n; i++) {
        string token = readToken(i);
        if (prevCanEndAtom && canStartAtom(token))
            res += '#';

        res += token;
        prevCanEndAtom = canEndAtom(token);
    }

    s = move(res);
    if (kRegexDebug)
        cout << "inffix : " << s << endl;
}

unordered_map<char, int> precedence = {
    {'*', 3}, {'+', 3}, {'?', 3},
    {'#', 2},
    {'|', 1},
};

bool isOperator(char c)
{
    return precedence.count(c);
}

void Infix2Postfix::infix2postfix(string& expr)
{
    string postfix;
    stack<char> opStack;
    bool escape = false;

    for (size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];

        if (escape) {
            postfix += '\\';
            postfix += c;
            escape = false;
            continue;
        }

        if (c == '\\') {
            escape = true;
            continue;
        }

        if (c == '(') {
            opStack.push(c);
        }
        else if (c == ')') {
            while (!opStack.empty() && opStack.top() != '(') {
                postfix += opStack.top();
                opStack.pop();
            }
            if (!opStack.empty())
                opStack.pop();
        }
        else if (isOperator(c)) {
            while (!opStack.empty() && opStack.top() != '(' && precedence[opStack.top()] >= precedence[c]) {
                postfix += opStack.top();
                opStack.pop();
            }
            opStack.push(c);
        }
        else {
            postfix += c;
        }
    }

    while (!opStack.empty()) {
        postfix += opStack.top();
        opStack.pop();
    }

    expr = move(postfix);
    if (kRegexDebug)
        cout << "postfix : " << expr << endl;
}

void Infix2Postfix::add_dot4rules(vector<Rule>& rules)
{
    for (auto& rule : rules) {
        add_dot(rule.pattern);
        infix2postfix(rule.pattern);
        if (kRegexDebug)
            cout << endl;
    }
}

void Infix2Postfix::prepare4nfa(vector<Rule>& rules)
{
    add_dot4rules(rules);
}
