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

	string outputFileName;
	fstream out;


	GenCode(DFA& dfa, vector<string>& user_declarations, vector<Rule>& rules, vector<string>& subroutines, string filename = "lexer.cpp")
		:dfa(dfa), user_declarations(user_declarations), rules(rules), subroutines(subroutines), outputFileName(filename) {
		out.open(outputFileName, ios::out | ios::trunc);
		if (!out.is_open()) {
			cerr << "Error opening output file." << std::endl;
			exit(1);
		}
	}
	~GenCode()
	{
		out.close();
	}

	void printDeclaration();
	void printSubroutines();
	void printActions();
	void printMinDFA();
	void printMainFuc();

	void genLexer();
};
#endif // !GENCODE_H



