#pragma once
#ifndef READLEX_H
#define READLEX_H
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

#define DEFINITIONS 0
#define ELEMENTS 1
#define RULES 2
#define SUBROUTINES 3

struct Rule {
	string pattern;
	string actions;
};

class ReadLex
{
public:
	string filename;
	ifstream file;
public:
	ReadLex(string fname = "c99.l");
	~ReadLex();

	//逐行读取lex文件
	void read_lex_file(vector<string>& user_declarations,unordered_map<string, string>& elements, vector<Rule>& rules, vector<string>& subroutines);

	//去除line两端的空格和制表符
	string& trim(string& s);
};
#endif