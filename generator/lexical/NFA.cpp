#include "NFA.h"
#include <cassert>
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {
constexpr bool kNfaDebug = false;
}

char convertEscapeChar(char c) {
    switch (c) {
    case 'n': return '\n';
    case 't': return '\t';
    case 'r': return '\r';
    case 'v': return '\v';
    case 'f': return '\f';
    case '\\': return '\\';
    default: return c;
    }
}

static string escapeDotText(const string& text)
{
    string escaped;
    escaped.reserve(text.size());
    for (char c : text) {
        switch (c) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\t':
            escaped += "\\t";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\v':
            escaped += "\\v";
            break;
        case '\f':
            escaped += "\\f";
            break;
        default:
            escaped += c;
            break;
        }
    }
    return escaped;
}

static string escapeDotChar(char c)
{
    return escapeDotText(string(1, c));
}

int NFA::createNfa(string& postfix, int ruleid)
{
    stack<NFAFragment> st;
    bool escape = false;
    for (int i = 0; i < postfix.size(); i++) {
        char c = postfix[i];
        if (escape)
        {
            escape = false;
            st.push(buildChar(convertEscapeChar(c)));
            continue;
        }

        if (c == '\\')
            escape = true;
        else if (c == '#') {
            if (st.size() < 2) {
                cout << "Error: insufficient elements in stack for '#' operator" << endl;
                cout << "Stack size: " << st.size() << endl;
                cout << "Current character: " << c << endl;
                cout << "Current position: " << i << endl;
                cout << "Postfix: " << postfix << endl;
                exit(1);
            }
            auto right = st.top(); st.pop();
            auto left = st.top(); st.pop();
            add_epsilon_edge(left.end->id, right.start->id);
            st.push({ left.start, right.end });
        }
        else if (c == '|') {
            if (st.size() < 2) {
                cout << "Error: insufficient elements in stack for '|' operator" << endl;
                cout << "Stack size: " << st.size() << endl;
                cout << "Current character: " << c << endl;
                cout << "Current position: " << i << endl;
                cout << "Postfix: " << postfix << endl;
                exit(1);
            }
            auto b = st.top(); st.pop();
            auto a = st.top(); st.pop();
            NFAState* start = createState();
            NFAState* end = createState();
            add_epsilon_edge(start->id, a.start->id);
            add_epsilon_edge(start->id, b.start->id);
            add_epsilon_edge(a.end->id, end->id);
            add_epsilon_edge(b.end->id, end->id);
            st.push({ start, end });
        }
        else if (c == '*') {
            if (st.size() < 1) {
                cout << "Error: insufficient elements in stack for '*' operator" << endl;
                cout << "Stack size: " << st.size() << endl;
                cout << "Current character: " << c << endl;
                cout << "Current position: " << i << endl;
                cout << "Postfix: " << postfix << endl;
                exit(1);
            }
            auto frag = st.top(); st.pop();
            NFAState* start = createState();
            NFAState* end = createState();
            add_epsilon_edge(start->id, frag.start->id);
            add_epsilon_edge(start->id, end->id);
            add_epsilon_edge(frag.end->id, end->id);
            add_epsilon_edge(frag.end->id, frag.start->id);
            st.push({ start, end });
        }
        else if (c == '+')
        {
            if (st.size() < 1) {
                cout << "Error: insufficient elements in stack for '+' operator" << endl;
                cout << "Stack size: " << st.size() << endl;
                cout << "Current character: " << c << endl;
                cout << "Current position: " << i << endl;
                cout << "Postfix: " << postfix << endl;
                exit(1);
            }
            auto frag = st.top(); st.pop();
            NFAState* start = createState();
            NFAState* end = createState();
            add_epsilon_edge(start->id, frag.start->id);
            add_epsilon_edge(frag.end->id, end->id);
            add_epsilon_edge(frag.end->id, frag.start->id);
            st.push({ start, end });
        }
        else if (c == '?')
        {
            if (st.size() < 1) {
                cout << "Error: insufficient elements in stack for '?' operator" << endl;
                cout << "Stack size: " << st.size() << endl;
                cout << "Current character: " << c << endl;
                cout << "Current position: " << i << endl;
                cout << "Postfix: " << postfix << endl;
                exit(1);
            }
            auto frag = st.top(); st.pop();
            NFAState* start = createState();
            NFAState* end = createState();
            add_epsilon_edge(start->id, frag.start->id);
            add_epsilon_edge(start->id, end->id);
            add_epsilon_edge(frag.end->id, end->id);
            st.push({ start, end });
        }
        else
            st.push(buildChar(c));
    }

    st.top().end->isAccept = true;
    st.top().end->tokenID = ruleid;

    return st.top().start->id;
}

void NFA::merge2head(int end)
{
    if (size() == 0)
    {
        cout << "Nfa merge ERR : nfa is empty" << endl;
        exit(1);
    }

    add_epsilon_edge(0, end);
}

void NFA::add_char_edge(int start, int end, char c)
{
    if (end >= startid || start >= startid)
    {
        cout << "unknown NFA state" << endl;
        exit(1);
    }

    NFAState* A = getState(start);
    NFAState* B = getState(end);
    A->transitions[c].push_back(B);
}

void NFA::add_epsilon_edge(int start, int end)
{
    if (end >= startid || start >= startid)
    {
        cout << "unknown NFA state" << endl;
        exit(1);
    }

    NFAState* A = getState(start);
    NFAState* B = getState(end);
    A->epsilonTransitions.push_back(B);
}

void NFA::createNcombineNfa(NFA& nfa, vector<Rule>& rules)
{
    if (nfa.size() != 0)
    {
        cout << "nfa is not clean" << endl;
        exit(1);
    }

    nfa.createState();

    for (int i = 0; i < rules.size(); i++)
    {
        if (kNfaDebug)
            cout << "cur startid = " << nfa.startid << ";  cur pattern is :" << rules[i].pattern << endl;
        try {
            int son = nfa.createNfa(rules[i].pattern, i);
            nfa.merge2head(son);
        }
        catch (const exception& e) {
            cout << "Error processing rule " << i << ": " << e.what() << endl;
            exit(1);
        }
    }

    assert(nfa.states[0]->transitions.size() == 0);
}

void NFA::writeRuleDotFiles(const vector<Rule>& rules, const string& outputDir) const
{
    if (states.empty())
    {
        cout << "NFA dot export ERR : nfa is empty" << endl;
        exit(1);
    }

    if (states[0]->epsilonTransitions.size() < rules.size())
    {
        cout << "NFA dot export ERR : missing rule start states" << endl;
        exit(1);
    }

    fs::create_directories(outputDir);

    for (size_t ruleIndex = 0; ruleIndex < rules.size(); ++ruleIndex)
    {
        const NFAState* ruleStart = states[0]->epsilonTransitions[ruleIndex];
        unordered_set<int> visited;
        queue<const NFAState*> work;
        work.push(ruleStart);
        visited.insert(ruleStart->id);

        while (!work.empty())
        {
            const NFAState* current = work.front();
            work.pop();

            for (const auto* next : current->epsilonTransitions)
            {
                if (visited.insert(next->id).second)
                    work.push(next);
            }

            for (const auto& [symbol, nextStates] : current->transitions)
            {
                (void)symbol;
                for (const auto* next : nextStates)
                {
                    if (visited.insert(next->id).second)
                        work.push(next);
                }
            }
        }

        vector<int> orderedStates(visited.begin(), visited.end());
        sort(orderedStates.begin(), orderedStates.end());

        const fs::path dotPath = fs::path(outputDir) / ("nfa_rule_" + to_string(ruleIndex) + ".dot");
        ofstream out(dotPath);
        if (!out.is_open())
        {
            cerr << "Error opening dot output file: " << dotPath << endl;
            exit(1);
        }

        out << "digraph NFA {\n";
        out << "    rankdir=LR;\n";
        out << "    labelloc=\"t\";\n";
        out << "    label=\"rule " << ruleIndex << ": " << escapeDotText(rules[ruleIndex].pattern) << "\";\n";
        out << "    node [shape=circle];\n";
        out << "    __start [shape=point];\n";
        out << "    __start -> " << ruleStart->id << ";\n";

        for (int stateId : orderedStates)
        {
            const NFAState* state = getState(stateId);
            out << "    " << stateId << " [shape=" << (state->isAccept ? "doublecircle" : "circle");
            if (state->isAccept)
                out << ", label=\"" << stateId << " / token " << state->tokenID << "\"";
            out << "];\n";
        }

        struct Edge {
            int from;
            int to;
            string label;
        };

        vector<Edge> edges;
        for (int stateId : orderedStates)
        {
            const NFAState* state = getState(stateId);
            for (const auto* next : state->epsilonTransitions)
            {
                if (visited.count(next->id))
                    edges.push_back({ stateId, next->id, "epsilon" });
            }
            for (const auto& [symbol, nextStates] : state->transitions)
            {
                for (const auto* next : nextStates)
                {
                    if (visited.count(next->id))
                        edges.push_back({ stateId, next->id, escapeDotChar(symbol) });
                }
            }
        }

        sort(edges.begin(), edges.end(), [](const Edge& lhs, const Edge& rhs) {
            if (lhs.from != rhs.from)
                return lhs.from < rhs.from;
            if (lhs.to != rhs.to)
                return lhs.to < rhs.to;
            return lhs.label < rhs.label;
        });

        for (const auto& edge : edges)
        {
            out << "    " << edge.from << " -> " << edge.to
                << " [label=\"" << edge.label << "\"];\n";
        }

        out << "}\n";
    }
}
