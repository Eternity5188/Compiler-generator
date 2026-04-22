#pragma once
#ifndef Infix2Suffix_H
#define Infix2Suffix_H
#include <iostream>
#include <stack>
#include <queue>
#include "ReadLex.h"
#include "NormaliseRE.h"
using namespace std;

// 优先级表，数值越大优先级越高
extern unordered_map<char, int> precedence;

class Infix2Postfix
{
public:
	//给每一个规则添加
	void add_dot(string& s);
	void infix2postfix(string& infix);
	void add_dot4rules(vector<Rule>& rules);

	// 总调用函数
	void prepare4nfa(vector<Rule>& rules);
};
#endif // !Infix2Suffix_H



