#pragma once
#ifndef Infix2Suffix_H
#define Infix2Suffix_H
#include <iostream>
#include <stack>
#include <queue>
#include "ReadLex.h"
#include "NormaliseRE.h"
using namespace std;

// Operator precedence: larger value means higher precedence
extern unordered_map<char, int> precedence;

class Infix2Postfix
{
public:
	// Processing helpers
	void add_dot(string& s);
	void infix2postfix(string& infix);
	void add_dot4rules(vector<Rule>& rules);

	// Top-level API
	void prepare4nfa(vector<Rule>& rules);
};
#endif // !Infix2Suffix_H



