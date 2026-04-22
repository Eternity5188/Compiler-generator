#include "NFA.h"
#include <cassert>

char convertEscapeChar(char c) {
	switch (c) {
	case 'n': return '\n';
	case 't': return '\t';
	case 'r': return '\r';
	case 'v': return '\v';
	case 'f': return '\f';
	case '\\': return '\\';
	default: return c;
	}
}

// ����nfa��ʼ�ڵ�
int NFA::createNfa(string& postfix, int ruleid)
{
	stack<NFAFragment> st;
	bool escape = false;
	for (int i = 0; i < postfix.size(); i++) {
		char c = postfix[i];
		if (escape)
		{
			escape = false;
			st.push(buildChar(convertEscapeChar(c)));
			continue;
		}

		if (c == '\\')
			escape = true;
		else if (c == '#') {
			// epsilon连接操作
			if (st.size() < 2) {
				cout << "Error: insufficient elements in stack for '#' operator" << endl;
				cout << "Stack size: " << st.size() << endl;
				cout << "Current character: " << c << endl;
				cout << "Current position: " << i << endl;
				cout << "Postfix: " << postfix << endl;
				exit(1);
			}
			auto right = st.top(); st.pop();
			auto left = st.top(); st.pop();
			add_epsilon_edge(left.end->id, right.start->id);
			st.push({ left.start, right.end });
		}
		else if (c == '|') {
			// 或 操作合并
			if (st.size() < 2) {
				cout << "Error: insufficient elements in stack for '|' operator" << endl;
				cout << "Stack size: " << st.size() << endl;
				cout << "Current character: " << c << endl;
				cout << "Current position: " << i << endl;
				cout << "Postfix: " << postfix << endl;
				exit(1);
			}
			auto b = st.top(); st.pop();
			auto a = st.top(); st.pop();
			NFAState* start = createState();
			NFAState* end = createState();
			add_epsilon_edge(start->id, a.start->id);
			add_epsilon_edge(start->id, b.start->id);
			add_epsilon_edge(a.end->id, end->id);
			add_epsilon_edge(b.end->id, end->id);
			st.push({ start, end });
		}
		else if (c == '*') {
			// ��Ŀ����
			if (st.size() < 1) {
				cout << "Error: insufficient elements in stack for '*' operator" << endl;
				cout << "Stack size: " << st.size() << endl;
				cout << "Current character: " << c << endl;
				cout << "Current position: " << i << endl;
				cout << "Postfix: " << postfix << endl;
				exit(1);
			}
			auto frag = st.top(); st.pop();
			NFAState* start = createState();
			NFAState* end = createState();
			add_epsilon_edge(start->id, frag.start->id);// ��start->��start
			add_epsilon_edge(start->id, end->id); // ��start->��end������
			add_epsilon_edge(frag.end->id, end->id); // ��end->��end
			add_epsilon_edge(frag.end->id, frag.start->id);// ��end��ָ
			st.push({ start, end });
		}
		// ͬ��ʵ�� '+' '?' ��
		else if (c == '+')
		{
			if (st.size() < 1) {
				cout << "Error: insufficient elements in stack for '+' operator" << endl;
				cout << "Stack size: " << st.size() << endl;
				cout << "Current character: " << c << endl;
				cout << "Current position: " << i << endl;
				cout << "Postfix: " << postfix << endl;
				exit(1);
			}
			auto frag = st.top(); st.pop();
			NFAState* start = createState();
			NFAState* end = createState();
			add_epsilon_edge(start->id, frag.start->id);// ��start->��start
			add_epsilon_edge(frag.end->id, end->id); // ��end->��end
			add_epsilon_edge(frag.end->id, frag.start->id);// ��end��ָ
			st.push({ start, end });
		}
		else if (c == '?')
		{
			if (st.size() < 1) {
				cout << "Error: insufficient elements in stack for '?' operator" << endl;
				cout << "Stack size: " << st.size() << endl;
				cout << "Current character: " << c << endl;
				cout << "Current position: " << i << endl;
				cout << "Postfix: " << postfix << endl;
				exit(1);
			}
			auto frag = st.top(); st.pop();
			NFAState* start = createState();
			NFAState* end = createState();
			add_epsilon_edge(start->id, frag.start->id);// ��start->��start
			add_epsilon_edge(start->id, end->id); // ��start->��end������
			add_epsilon_edge(frag.end->id, end->id); // ��end->��end
			st.push({ start, end });
		}
		else
			st.push(buildChar(c));
	}

	// ��top.end����Ϊ�ɱ�����
	st.top().end->isAccept = true;
	st.top().end->tokenID = ruleid;

	return st.top().start->id;  // �������� NFA ��ͷ�ڵ�
}

void NFA::merge2head(int end)
{
	if (size() == 0)
	{
		cout << "Nfa merge ERR : nfa is empty" << endl;
		exit(1);
	}

	add_epsilon_edge(0, end);
}

void NFA::add_char_edge(int start, int end, char c)
{
	if (end >= startid || start >= startid)
	{
		cout << "unknown NFA state" << endl;
		exit(1);
	}

	NFAState* A = getState(start);
	NFAState* B = getState(end);
	A->transitions[c].push_back(B);
}

void NFA::add_epsilon_edge(int start, int end)
{
	if (end >= startid || start >= startid)
	{
		cout << "unknown NFA state" << endl;
		exit(1);
	}

	NFAState* A = getState(start);
	NFAState* B = getState(end);
	A->epsilonTransitions.push_back(B);
}

// ���ݽ�����rulesĬ���Ǻ�׺����ʽ
void NFA::createNcombineNfa(NFA& nfa, vector<Rule>& rules)
{
	if (nfa.size() != 0)
	{
		cout << "nfa is not clean" << endl;
		exit(1);
	}

	nfa.createState();//���建根节点，作为合并nfa

	for (int i = 0; i < rules.size(); i++)
	{
		cout << "cur startid = " << nfa.startid << ";  cur pattern is :" << rules[i].pattern << endl;
		try {
			int son = nfa.createNfa(rules[i].pattern, i);
			nfa.merge2head(son);
		} catch (const exception& e) {
			cout << "Error processing rule " << i << ": " << e.what() << endl;
			exit(1);
		}
	}

	// 根节点的转移应该是空，全是epsilon转移
	assert(nfa.states[0]->transitions.size() == 0);
}
