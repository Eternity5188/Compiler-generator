#include "Infix2Postfix.h"

// ����������ʽ֮���������ӷ�.��������Ȼ�󡱣��γ���׺����ʽ
void Infix2Postfix::add_dot(string& s)
{
	// �������� |, (, ), *, +, ?
	// ����ת��֮�⣬�κβ�����
	string res = "";
	int n = s.size();
	int bracketDepth = 0; // 括号深度
	int charsetDepth = 0; // 字符集深度
	bool inQuote = false; // 是否在字符串中
	bool inChar = false; // 是否在字符常量中
	for (int i = 0; i < n; i++)
	{
		res += s[i];
		//��ת���
		if (s[i] == '\\')
			continue;
		// 处理括号
		if (s[i] == '(' && (i == 0 || s[i - 1] != '\\'))
			bracketDepth++;
		else if (s[i] == ')' && (i == 0 || s[i - 1] != '\\'))
			bracketDepth--;
		// 处理字符集
		if (s[i] == '[' && (i == 0 || s[i - 1] != '\\'))
			charsetDepth++;
		else if (s[i] == ']' && (i == 0 || s[i - 1] != '\\'))
			charsetDepth--;
		// 处理字符串
		if (s[i] == '"' && (i == 0 || s[i - 1] != '\\'))
			inQuote = !inQuote;
		// 处理字符常量
		if (s[i] == '\'' && (i == 0 || s[i - 1] != '\\'))
			inChar = !inChar;
		// 如果在括号、字符集、字符串或字符常量内，不添加#
		if (bracketDepth > 0 || charsetDepth > 0 || inQuote || inChar)
			continue;
		//�Ƿ�ת��� (, |
		if ((s[i] == '(' || s[i] == '|') && (i == 0 || s[i - 1] != '\\'))
			continue;
		//���һ���ַ�
		if (i == n - 1)
			continue;
		//��ǰ�ַ��ĺ�һ���ַ��ǲ�����,��a*
		if (s[i + 1] == '|' || s[i + 1] == ')' || s[i + 1] == '*' || s[i + 1] == '+' || s[i + 1] == '?')
			continue;
		res += '#';
	}
	s = move(res);
	cout << "inffix : " << s << endl;
}

unordered_map<char, int> precedence = {
    {'*', 3}, {'+', 3}, {'?', 3},
    {'#', 2}, // ���ӷ���ԭ��ӦΪ��ʽ��
    {'|', 1},
};

// �ж��Ƿ��ǲ�����
bool isOperator(char c) {
    return precedence.count(c);
}


void Infix2Postfix::infix2postfix(string& expr)
{
    string postfix;
    stack<char> opStack;
    bool escape = false;
    int bracketDepth = 0; // 括号深度

    for (size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];

        if (escape) {
            postfix += '\\'; // ����ת���
            postfix += c;
            escape = false;
            continue;
        }

        if (c == '\\') {
            escape = true;
            continue;
        }

        if (c == '(') {
            bracketDepth++;
            opStack.push(c);
        }
        else if (c == ')') {
            bracketDepth--;
            while (!opStack.empty() && opStack.top() != '(') {
                postfix += opStack.top();
                opStack.pop();
            }
            if (!opStack.empty()) 
                opStack.pop(); // ����'('
        }
        else if (isOperator(c)) { // 处理所有运算符
            // 处理优先级相同或更高的运算符
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
    cout << "postfix : " << expr << endl;
}

void Infix2Postfix::add_dot4rules(vector<Rule>& rules)
{
	for (auto& rule : rules)
	{
		add_dot(rule.pattern);
		infix2postfix(rule.pattern);
        cout << endl;
	}
}

void Infix2Postfix::prepare4nfa(vector<Rule>& rules)
{
	add_dot4rules(rules);
}
