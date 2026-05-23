#pragma once
#ifndef NormalizeRE_H
#define NormalizeRE_H
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include "ReadLex.h"
using namespace std;

//lexRE reserved characters
const unordered_set<char> lex_RC{ '\\','.','|','*','(',')','+','?','{','}','[',']','#' };

// Full ASCII character set used by this normalizer
const string ALLSET("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#%'()*+,-./:;<=>\?[\\]^{|}_ \n\t\v\f~&");

// Normalize LEX regular expression into internal regular expression form
class NormalizeRE
{
public:
	 void HandleBrace(string& s, unordered_map<string, string>& mp);

	 void HandleLexRC(string& s);

	 void getSet(unordered_set<char>& charSet, string& s, bool flag);

	 void HandleBrackets(string& s);

	 void HandleQuote(string& s);

	 void HandleDot(string& s);

	 void RENormalization(vector<Rule>& re, unordered_map<string, string>& mp);
};
#endif // !NormalizeRE_H