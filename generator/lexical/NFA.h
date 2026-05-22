#pragma once
#ifndef NFA_H
#define NFA_H
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_set>
#include "Infix2Postfix.h"
using namespace std;

struct NFAState {
    int id;
    unordered_map<char, vector<NFAState*>> transitions;
    vector<NFAState*> epsilonTransitions;
    bool isAccept = false;
    int tokenID = -1;
};

struct NFAFragment {
    NFAState* start;
    NFAState* end;
};

class NFA {
public:
    int startid;
    vector<NFAState*> states;

    NFAState* createState() {
        NFAState* s = new NFAState{ startid };
        startid++;
        states.push_back(s);
        return s;
    }

    NFAFragment buildChar(char c) {
        NFAState* start = createState();
        NFAState* end = createState();
        start->transitions[c].push_back(end);
        return { start, end };
    }

    NFAState* getState(int id)
    {
        return states[id];
    }

    const NFAState* getState(int id) const
    {
        return states[id];
    }

    int createNfa(string& postfix, int ruleid);

    void merge2head(int start);

    void add_char_edge(int start, int end, char c);
    void add_epsilon_edge(int start, int end);

    int size() {
        return states.size();
    }

    void writeRuleDotFiles(const vector<Rule>& rules, const string& outputDir) const;

    NFA(int start_id = 0) :startid(start_id) {}
    ~NFA() {
        for (auto s : states)
            delete s;
    }

    static void createNcombineNfa(NFA& nfa, vector<Rule>& rules);
};
#endif // !NFA_H
