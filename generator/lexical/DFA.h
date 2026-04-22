#pragma once
#ifndef DFA_H
#define DFA_H
#include "NFA.h"
#include <set>
using namespace std;

#define DEBUG

struct DFAState {
    int id;
    unordered_set<int> nfaStates;  // 该DFA状态包含的NFA状态集合
    unordered_map<char, int> transitions; // 字符 -> DFAState.id
    bool isAccept = false;
    int tokenID = -1;  // 对应哪条规则（优先级较高的）
};

class DFA
{
public:
    // unordered_map<unordered_set<int>, int>哈希函数
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
    int startid; // 唯一化编号使用
    int minDFAsid; //最小化dfa开始id
    NFA& nfa;
    vector<DFAState*> states;
    unordered_map<unordered_set<int>, int, SetHash> subset2id;  // key: NFA状态集合, value: DFA状态编号

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

    // 创建一个节点, 右值参
    DFAState* createState(unordered_set<int>&& closure) {
        DFAState* s = new DFAState{ startid };
        startid++;
        s->nfaStates = std::move(closure);
        // 加入状态集
        states.push_back(s);
        // 绑定映射
        subset2id[s->nfaStates] = s->id;
        // 判断当前状态是否可被接受
        // 并且设置规则编号，**保留编号最小的规则**，否则会发生覆盖
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

    // epsilon闭包函数
    unordered_set<int> epsilonClosure(NFAState* s);
    void epsilonClosure(unordered_set<int>& states);//直接修改当前states

    // move(T,c),将状态集通过a进行转换
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



