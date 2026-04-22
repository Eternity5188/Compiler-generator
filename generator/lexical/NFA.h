#pragma once
#ifndef NFA_H
#define NFA_H
#include <iostream>
#include "Infix2Postfix.h"
using namespace std;

// 表示一个 NFA 状态
struct NFAState {
    int id;  // 状态编号
    unordered_map<char, vector<NFAState*>> transitions;  // 边：从当前状态出发，读入某字符到达目标状态集合
    vector<NFAState*> epsilonTransitions;  // epsilon-转移（空边）
    bool isAccept = false; // 是否是终止状态
    int tokenID = -1;  // 对应哪条规则（优先级较高的）
};

// 表示一个 NFA 片段（方便用于构造时连接）
struct NFAFragment {
    NFAState* start;  // 起始状态
    NFAState* end; // 接受状态
};

// 整个 NFA 用一个类统一管理状态分配
class NFA {
public:
    int startid; // NFA的起始编号，合并nfa使用，让所有nfa节点的编号唯一
    vector<NFAState*> states;  // 管理所有状态，防止内存泄漏

    // 创建一个节点
    NFAState* createState() {
        NFAState* s = new NFAState{ startid };
        startid++;
        states.push_back(s);
        return s;
    }

    // 创建一个传递
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

    // 根据每一个规则的postfix创建nfa, 返回子nfa的头节点
    int createNfa(string& postfix, int ruleid);

    // 连接到桩
    void merge2head(int start);

    // 在两个state间添加边
    void add_char_edge(int start, int end, char c);
    void add_epsilon_edge(int start, int end);

    int size() {
        return states.size();
    }

    NFA(int start_id = 0) :startid(start_id) {}
    ~NFA() {
        for (auto s : states)
            delete s;
    }

    static void createNcombineNfa(NFA& nfa, vector<Rule>& rules);
};
#endif // !NFA_H

