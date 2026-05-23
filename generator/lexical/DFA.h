#pragma once
#ifndef DFA_H
#define DFA_H
#include "NFA.h"
#include <set>
using namespace std;

#define DEBUG

struct DFAState {
    int id;
    unordered_set<int> nfaStates;  // NFA state subset represented by this DFA state
    unordered_map<char, int> transitions; // input char -> DFAState.id
    bool isAccept = false;
    int tokenID = -1;  // selected token id for accept state (smaller id = higher priority)
};

class DFA
{
public:
    // Hash for unordered_set<int> used as key in unordered_map
    struct SetHash {
        size_t operator()(const unordered_set<int>& s) const {
            size_t hash = 0;
            for (int x : s) {
                hash ^= std::hash<int>()(x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };

public:
    int startid; // Unique DFA state id allocator
    int minDFAsid; // Start id after minimization
    NFA& nfa;
    vector<DFAState*> states;
    unordered_map<unordered_set<int>, int, SetHash> subset2id;  // key: NFA subset, value: DFA state id

#ifdef DEBUG
    vector<int> nfaidInUse;
#endif // DEBUG


    DFA(NFA& nfa, int start_id = 0) :startid(start_id), nfa(nfa) {}
    ~DFA()
    {
        for (auto& p : states)
            delete p;
        states.clear();
    }

    // Create a new DFA state from epsilon-closure subset
    DFAState* createState(unordered_set<int>&& closure) {
        DFAState* s = new DFAState{ startid };
        startid++;
        s->nfaStates = std::move(closure);
        // Add to state list
        states.push_back(s);
        // Build reverse mapping
        subset2id[s->nfaStates] = s->id;
        // Check accept property and resolve token priority
        // Rule: choose the smallest token id among matched accept states.
        int minTokenId = INT_MAX;
        for (int id : s->nfaStates)
        {
            NFAState* ns = nfa.getState(id);
            if (ns->isAccept)
            {
                s->isAccept = true;
                minTokenId = min(minTokenId, ns->tokenID);
            }
        }
        s->tokenID = minTokenId == INT_MAX ? -1 : minTokenId;
        return s;
    }

    // Epsilon closure helpers
    unordered_set<int> epsilonClosure(NFAState* s);
    void epsilonClosure(unordered_set<int>& states); // In-place closure expansion

    // move(T, c): transition from subset T on character c
    unordered_set<int> move(const unordered_set<int>& states, char symbol);

    void buildDFA();

    void checksum(vector<unordered_set<int>>& newStates);

    void hopcroftMinDFA();

    void printDFAinfo()
    {
        cout << endl << endl << "----------------------------------------------" << endl;
        cout << "cur DFA size = " << states.size() << endl
            << "startid = " << startid << endl
            << "----------------------------------------------" << endl;
    }

    void clearOldStates()
    {
        for (auto& p : states)
            delete p;
        states.clear();
    }
};
#endif // !DFA_H



