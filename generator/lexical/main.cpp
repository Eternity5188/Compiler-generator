#include <iostream>
#include "ReadLex.h"
#include "NormaliseRE.h"
#include "Infix2Postfix.h"
#include "NFA.h"
#include "DFA.h"
#include "GenCode.h"
using namespace std;

void printMinDfa(DFA& dfa, vector<Rule>& rules);
void printNfa(NFA& nfa, vector<Rule>& rules);
void printDfa(DFA& nfa, vector<Rule>& rules);


int main()
{
    ReadLex rl;
    NormalizeRE nre;
    Infix2Postfix i2s;
    vector<string> user_declarations;
    unordered_map<string, string> elements;
    vector<Rule> rules;
    vector<string> subroutines;
    rl.read_lex_file(user_declarations, elements, rules, subroutines);
    nre.RENormalization(rules, elements);

    NFA nfa;
    i2s.prepare4nfa(rules);
    NFA::createNcombineNfa(nfa, rules);
    printNfa(nfa, rules);

    DFA dfa(nfa);
    dfa.buildDFA();
    printDfa(dfa, rules);

    dfa.printDFAinfo();
    dfa.hopcroftMinDFA();
    dfa.printDFAinfo();
    printMinDfa(dfa, rules);
    
    GenCode gencode(dfa, user_declarations, rules, subroutines);
    gencode.genLexer();
    return 0;
}


void printMinDfa(DFA& dfa, vector<Rule>& rules)
{
    cout << "MinDFA----------------------------------MinDFA" << endl;
    cout << "begin state : " << dfa.minDFAsid << endl;
    for (auto& s : dfa.states)
    {
        cout << "cur sid : " << s->id << "--";
        if (s->isAccept && s->tokenID != -1)
            cout << "accept : " << rules[s->tokenID].actions << endl;
        else
            cout << "nonaccept" << endl;
    }
    cout << "MinDFA----------------------------------MinDFA" << endl;
}

void printNfa(NFA& nfa, vector<Rule>& rules)
{
    cout << "NFA----------------------------------NFA" << endl;
    cout << "begin state : " << 0 << endl;
    int count = 0;

    for (auto& s : nfa.states)
    {

        if (s->isAccept && s->tokenID != -1)
        {
            cout << "cur sid : " << s->id << "--";
            cout << "accept : " << "rule-id " << s->tokenID << endl
                << "action :" << rules[s->tokenID].actions << endl;
            count++;
        }
    }
    cout << "nfa, remain rules count = " << count << endl;
    cout << "NFA----------------------------------NFA" << endl;
}
void printDfa(DFA& nfa, vector<Rule>& rules)
{
    cout << "DFA----------------------------------DFA" << endl;
    cout << "begin state : " << 0 << endl;
    int count = 0;
    for (auto& s : nfa.states)
    {
        if (s->isAccept)
        {
            /*cout << "cur sid : " << s->id << "--";
            cout << "accept : " << "rule-id " << s->tokenID << endl
                << "action :" << rules[s->tokenID].actions << endl;*/
            count++;
        }
    }
    cout << "after nfa -> dfa, ac states count = " << count << endl;
    cout << "DFA----------------------------------DFA" << endl;
}
