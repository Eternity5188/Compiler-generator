#include "DFA.h"

unordered_set<int> DFA::epsilonClosure(NFAState* s)
{
    unordered_set<int> closure;
    queue<NFAState*> q;
    q.push(s);
    closure.insert(s->id);
    while (!q.empty())
    {
        NFAState* cur_state = q.front();
        q.pop();
        for (auto& next : cur_state->epsilonTransitions)
        {
            if (!closure.count(next->id))
            {
                closure.insert(next->id);
                q.push(next);
            }
        }
    }
    return closure;
}

void DFA::epsilonClosure(unordered_set<int>& states)
{
    queue<NFAState*> q;

    for (int id : states) 
        q.push(nfa.getState(id));


    while (!q.empty()) {
        NFAState* s = q.front(); q.pop();
        for (NFAState* next : s->epsilonTransitions) {
            if (!states.count(next->id)) {
                states.insert(next->id);
                q.push(next);
            }
        }
    }
}

unordered_set<int> DFA::move(const unordered_set<int>& states, char symbol)
{
    unordered_set<int> result;
    for (int id : states) {
        NFAState* s = nfa.getState(id);
        //unordered_map<char, vector<NFAState*>> transitions;
        auto it = s->transitions.find(symbol);
        if (it != s->transitions.end()) {
            for (auto& target : it->second) {
                result.insert(target->id);
            }
        }
    }
    return result;
}

void DFA::buildDFA()
{
    queue<DFAState*> q; 
    //由nfa初始状态构造第一个子集
    DFAState* initState = createState(epsilonClosure(nfa.states[0]));
    q.push(initState);
    while (!q.empty())
    {
        DFAState* cur_dfaState = q.front(); q.pop();
        //unordered_set<int> next_state;
        unordered_set<char> tran_char;// 可转移字符
        //找到所有的可转移字符
        for (int nfaid : cur_dfaState->nfaStates)
        {
            NFAState* nfa_state = nfa.getState(nfaid);
            // 将当前nfa_state中的字符加入集合中
            for (auto& [c, states] : nfa_state->transitions)
                tran_char.insert(c);
#ifdef DEBUG
            //nfaidInUse[nfaid]++;
#endif // DEBUG
        }

        for (char c : tran_char) {
            //移动并计算闭包
            auto moveSet = move(cur_dfaState->nfaStates, c);
            epsilonClosure(moveSet);

            if (moveSet.empty())
                continue;

            //新状态
            int id = 0;
            if (subset2id.find(moveSet) == subset2id.end()) {
                DFAState* new_state = createState(std::move(moveSet));
                id = new_state->id;
                q.push(new_state);
            }
            else 
                id = subset2id[moveSet];

            //添加转移
            cur_dfaState->transitions[c] = id;
        }
    }

#ifdef DEBUG
    cout << "NFADEBUG----------------------------------NFADEBUG" << endl;
    nfaidInUse.resize(nfa.size(), 0);
    for (int i = 0; i < states.size(); i++)
    {
        for (int id : states[i]->nfaStates)
            nfaidInUse[id]++;
    }
    for (int i = 0; i < nfa.size(); i++)
    {
        if (nfaidInUse[i] == 0)
        {
            cout << "nfa-id : " << i << " not use" << endl
                << "accept : " << nfa.states[i]->isAccept << "; tokenid : " << nfa.states[i]->tokenID;
        }
    }
    cout << "NFADEBUG----------------------------------NFADEBUG" << endl;

    vector<int> totalUsedToken(95, 0);
    vector<int> finalUsedToken(95, 0);
    vector<set<int>> each_stoken(states.size());
    int acceptnum = 0;
    for (auto& s : states)
    {
        /*int minTokenId = INT_MAX;
        for (int id : s->nfaStates)
        {
            NFAState* ns = nfa.getState(id);
            if (ns->isAccept)
            {
                s->isAccept = true;
                minTokenId = min(minTokenId, ns->tokenID);
                totalUsedToken[ns->tokenID]++;
                each_stoken[s->id].insert(ns->tokenID);
            }
        }
        s->tokenID = minTokenId == INT_MAX ? -1 : minTokenId;*/
        if (s->isAccept)
        {
            finalUsedToken[s->tokenID]++;
            acceptnum++;
        }
    }
    cout << "total ac dfa states = " << acceptnum << endl;

    cout << "breakpoint" << endl;
#endif
}

void DFA::checksum(vector<unordered_set<int>>& newStates)
{
    int sum = 0;
    for (auto& uset : newStates)
        sum += uset.size();

    if (sum != states.size())
    {
        cout << "ERR in hopcroftMinDFA" << endl;
        exit(1);
    }
}


// 最小化dfa
void DFA::hopcroftMinDFA()
{
    vector<unordered_set<int>> newStates; //新状态集合表
    // 初始化将可接受和不可接受状态划分
    // 初始化时按 tokenID 分割接受状态
    unordered_map<int, unordered_set<int>> acceptGroups;
    unordered_set<int> nonaccept;
    for (DFAState* s : states) {
        if (s->isAccept)
            acceptGroups[s->tokenID].insert(s->id);
        else
            nonaccept.insert(s->id);
    }
    for (auto& [tokenID, group] : acceptGroups) {
        if (!group.empty()) {
            newStates.push_back(std::move(group));
        }
    }
    if (!nonaccept.empty())
        newStates.push_back(std::move(nonaccept));

    bool change = true;
    int round = 0;
    cout << "round : " << round++ << " -- newStates.size = " << newStates.size() << endl;
    checksum(newStates);
    while (change)
    {
        change = false;
        vector<unordered_set<int>> changeStates;
        // 对于每一个分组，进行转移切割
        int gnum = newStates.size(); // 前一个分组的个数
        for (auto& group : newStates) {
            unordered_map<string, unordered_set<int>> splitter;
            // 通过signature判断分组是否相同
            // 同一分组的转移应该相同
            for (int sid : group) {
                string signature;
                DFAState* s = states[sid];
                /* for (char c = 0; c <= 127; ++c)
                 * 无限执行，[-128,127] 127时溢出
                 */
                for (int c = 0; c <= 127; ++c) {
                    auto it = s->transitions.find(c);
                    if (it != s->transitions.end()) {
                        int target = it->second;
                        //cout << "for char : " << c << " goto id:" << target << endl;
                        for (int i = 0; i < gnum; ++i) {
                            if (newStates[i].count(target)) {
                                signature += to_string(i) + ",";
                                break;
                            }
                        }
                    }
                    else {
                        signature += "#,"; // no transition
                    }
                }
                splitter[signature].insert(sid);
            }

            if (splitter.size() == 1) {
                // 该组的所有转移相同
                changeStates.push_back(group);
                for (auto& [sig, _] : splitter)
                    cout << "signature " << " : " << sig << endl;
            }
            else {
                // 存在不同转移
                change = true;
                int k = 0;
                for (auto& [_, subset] : splitter)
                {
                    changeStates.push_back(subset);
                    cout << "signature " << k++ << " : " << _ << endl;
                }
            }
        }
        newStates = changeStates;
        cout << "round : " << round++ << " -- newStates.size = " << newStates.size() << endl;
        checksum(newStates);
    }

    // 开始构建minDFA
    vector<DFAState*> minStates(newStates.size());
    unordered_map<int, int> old2NewStateID; // 原dfaState属于哪个新dfa状态集合

    // 更新旧state到新state的映射
    for (int i = 0; i < newStates.size(); i++)
    {
        for (int id : newStates[i])
            old2NewStateID[id] = i;
    }

    // 记录minDFAstartid
    minDFAsid = old2NewStateID[0];

    // 创建新的dfaStates
    for (int i = 0; i < newStates.size(); ++i) {
        DFAState* s = new DFAState;
        s->id = i;

        // 如果该组中有任意一个是接受状态，则它是接受状态
        for (int oldID : newStates[i]) {
            DFAState* oldState = states[oldID];
            if (oldState->isAccept) {
                s->isAccept = true;
                s->tokenID = oldState->tokenID; // 你可以决定是否保留第一个 oldState 的 tokenID 或更优先的
                break;
            }
        }

        minStates[i] = s;
    }

    // 重建转移关系
    for (int i = 0; i < newStates.size(); ++i) {
        int representativeID = *newStates[i].begin(); // 任意一个旧状态
        DFAState* oldState = states[representativeID];
        for (auto& [c, targetOldID] : oldState->transitions) {
            int targetNewID = old2NewStateID[targetOldID];
            minStates[i]->transitions[c] = targetNewID;
        }
    }

    // 清楚旧节点
    clearOldStates();
    states = std::move(minStates);

    cout << "----------------------------" << endl
        << "minimize DFA done" << endl
        << "total node number : " << states.size() << endl
        << "----------------------------" << endl;
}
