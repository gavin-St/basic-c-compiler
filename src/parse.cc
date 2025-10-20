#include "parse.h"
#include "wlp4data.h"
#include <iostream>
#include <sstream>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <memory>

using namespace std;

struct rule {
    string lhs;
    string r;
    int n;
    rule(const string& t, const string& p, int n) : lhs(t), r(p),  n(n) {}
};

struct state {
    unordered_map<string, string> transitions;
    unordered_map<string, int> reductions;
};

ParseNode::ParseNode(const ParseNode& other) : value(other.value) {
    for (const auto& child : other.children) {
        children.push_back(new ParseNode(*child));
    }
}

static string getFirstPart(const string& str) {
    istringstream iss(str);
    string firstPart;
    iss >> firstPart;
    return firstPart;
}

static int countWords(const string& str) {
    istringstream iss(str);
    string word;
    int count = -1;

    while (iss >> word) {
        count++;
    }

    if (word == ".EMPTY") {
        return 0;
    }

    return count;
}

ParseNode* buildParseTree(const std::vector<Token>& tokens) {
    istringstream iss0(WLP4_COMBINED);
    string line;
    getline(iss0, line);
    vector<rule> rules;
    
    // .CFG
    while (getline(iss0, line) && line != ".TRANSITIONS") {  
        rules.emplace_back(rule(getFirstPart(line), line, countWords(line)));
    }

    // dfa initialization
    deque<string> states;
    deque<ParseNode*> symbols;
    unordered_map<string, state> dfa;
    bool first = true;

    // .TRANSITIONS
    while (getline(iss0, line) && line != ".REDUCTIONS") {
        istringstream iss(line);
        string a, b, c;
        iss >> a;
        iss >> b;
        iss >> c;
        dfa[a].transitions[b] = c;
        if (first) {
            states.push_back(getFirstPart(line));
            first = false;
        }
    }

    // .REDUCTIONS
    while (getline(iss0, line) && line != ".END") {
        istringstream iss(line);
        int b; string a, c;
        iss >> a;
        iss >> b;
        iss >> c;
        dfa[a].reductions[c] = b;
    }

    // .INPUT - convert tokens to input format
    deque<string> input;
    vector<string> terminals;
    terminals.emplace_back("BOF");

    for (const auto& token : tokens) {
        input.push_back(token.type);
        terminals.push_back(token.lexeme);
    }
    terminals.emplace_back("EOF");

    // DFA
    int shift_count = 0;
    input.push_front("BOF");
    input.push_back("EOF");
    
    while (!input.empty()) {  
        string next = input.front();

        while (dfa[states.back()].reductions.count(next) > 0) { // reduce
            int rulen = dfa[states.back()].reductions[next];
            ParseNode* tempnode = new ParseNode("");
            for (int i = 0; i < rules[rulen].n; i++) {
                tempnode->children.insert(tempnode->children.begin(), symbols.back());
                symbols.pop_back();
                states.pop_back();
            }
            if (rules[rulen].n==0) {
                ParseNode* tempnode1 = new ParseNode(".EMPTY");
                tempnode->children.insert(tempnode->children.begin(), tempnode1);
            }
            string B = rules[rulen].lhs;
            tempnode->value = B;
            symbols.push_back(tempnode);

            if (dfa[states.back()].transitions.count(B) == 0) {
                cerr << "ERROR at " << shift_count << endl;
                // Clean up
                for (auto sym : symbols) delete sym;
                return nullptr;
            }
            states.push_back(dfa[states.back()].transitions[B]);
        }
        
        // shift
        ParseNode* tempnode = new ParseNode(next);
        ParseNode* tempnode1 = new ParseNode(terminals[shift_count]);
        tempnode->addChild(tempnode1);
        symbols.push_back(tempnode);
        input.pop_front();
        
        if (dfa[states.back()].transitions.count(next) == 0) {
            cerr << "ERROR at " << shift_count << endl;
            // Clean up
            for (auto sym : symbols) delete sym;
            return nullptr;
        }
        
        states.push_back(dfa[states.back()].transitions[next]);
        ++shift_count;
    }

    ParseNode* tempnode = new ParseNode("");
    for (int i = 0; i < rules[0].n; i++) {
        tempnode->children.insert(tempnode->children.begin(), symbols.back());
        symbols.pop_back();
        states.pop_back();
    }
    tempnode->value = rules[0].lhs;
    
    return tempnode;
}

void parseTreeToStringHelper(const ParseNode* node, ostringstream& oss) {
    if (!node) return;
    
    if (node->children.empty()) {
        return;
    }
    
    oss << node->value;
    for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) {
        oss << " " << (*it)->value;
    }
    oss << "\n";

    for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) {
        parseTreeToStringHelper(*it, oss);
    }
}

string parseTreeToString(const ParseNode* root) {
    ostringstream oss;
    parseTreeToStringHelper(root, oss);
    return oss.str();
}
