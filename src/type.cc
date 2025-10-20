#include "type.h"
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <utility>
#include "wlp4data.h"

using namespace std;

enum Type {
    UNDEF,
    NONE,
    INT,
    INTSTAR
};

struct Rule {
    string lhs;
    string r;
    int n;
    Rule(const string& t, const string& p, int n) : lhs(t), r(p),  n(n) {}
};

struct Procedure {
    vector<Type> params;
    unordered_map<string, Type> symbols;
    Procedure() {}
};

static unordered_map<string, Procedure> tables;
static unordered_set<string> non_terminals;
static string cur_procedure;
static bool error, param_list;
static deque<vector<Type>> args;
static deque<string> called_procedure;
static string error_reason;

struct Node {
    string lhs;
    string rhs;
    Type type;
    vector<Node *> children;
    Node *parent;

    Node(std::string l, std::string r = "", Node *p = nullptr, Type t = UNDEF) : lhs{l}, rhs{r}, type{t}, parent{p} {}
    ~Node() {
        for (u_int64_t i = 0; i < children.size(); i++) {
            delete children[i];
        }
    };

    void idCheck() {
        if (error) {
            return;
        }

        if (lhs == "main") {
            cur_procedure = "main";
            param_list = true;
            if (tables.count(cur_procedure)) {
                error = true;
                error_reason = "MAIN ALREADY EXISTS";
                return;
            }
            tables["main"];
            // check second dcl->type
            if (children[5]->children[0]->rhs != " INT") {
                error = true;
                error_reason = "WAIN SECOND PARAM NOT INT";
                return;
            }
        }

        else if (lhs == "procedure") {
            cur_procedure = children[1]->rhs;
            param_list = true;
            if (tables.count(cur_procedure)) {
                error = true;
                error_reason = "PROCEDURE: " + cur_procedure + " ALREADY EXISTS";
                return;
            }
            tables[cur_procedure];
        }

        else if (lhs == "RPAREN") {
            if (!args.empty()) {
                // cout << called_procedure.back();
                // for (Type t:args.back()) {
                //     cout << t << " ";
                // }
                // cout << endl;
                vector<Type> cur_args = args.back();
                if (cur_args.size() != tables[called_procedure.back()].params.size()) {
                        error = true;
                        error_reason = "WRONG NUMBER OF ARGS TO CALL: " + called_procedure.back();
                        return;
                    }

                    for (size_t i = 0; i < cur_args.size(); ++i) {
                        if (cur_args[i] != tables[called_procedure.back()].params[i]) {
                            error = true;
                            error_reason = "BAD CALL TO PROCEDURE: " + called_procedure.back();
                            return;
                        }
                    }
                args.pop_back();
                called_procedure.pop_back();
                if (!args.empty()){
                    vector<Type> cur_args = args.back();
                    args.pop_back();
                    cur_args.emplace_back(INT);
                    args.push_back(cur_args);
                }
            }
            
            param_list = false;
        }

        else if (lhs == "dcl") {
            string id = children[1]->rhs;
            string ty = children[0]->rhs;
            if (tables[cur_procedure].symbols.count(id)) {
                error = true;
                error_reason = "ID: " + id + " ALREADY EXISTS IN PROCEDURE: " + cur_procedure;
                return;
            }
            if (ty == " INT") {
                tables[cur_procedure].symbols[id] = INT;
                if (param_list) tables[cur_procedure].params.emplace_back(INT);
            }
            else {
                tables[cur_procedure].symbols[id] = INTSTAR;
                if (param_list) tables[cur_procedure].params.emplace_back(INTSTAR);
            }
        }

        else if (lhs == "factor" && rhs.find(" ID LPAREN") == 0) {
            called_procedure.push_back(children[0]->rhs);
            if (tables[cur_procedure].symbols.count(called_procedure.back())) {
                error = true;
                error_reason = "ID: " + called_procedure.back() + " IS CALLED WRONGLY AS PROCEDURE";
                return;
            }
            if (!tables.count(called_procedure.back())) {
                error = true;
                error_reason = "PROCEDURE: " + called_procedure.back() + " DOESNT EXIST";
                return;
            }
            if (rhs == " ID LPAREN RPAREN" && !tables[called_procedure.back()].params.empty()) {
                error = true;
                error_reason = "PROCEDURE: " + called_procedure.back() + " EXPECTED ARGUMENTS BUT RECEIVED NONE";
                return;
            }
            vector<Type> temp;
            args.push_back(temp);
        }

        else if (lhs == "factor" && rhs == " ID") {
            string var = children[0]->rhs;
            if (!tables[cur_procedure].symbols.count(var)) {
                error = true;
                error_reason = "ID: " + var + " IS READ FROM BEFORE DECLARATION";
                return;
            }
            
            if (!args.empty()){
                Type t = tables[cur_procedure].symbols[var];
                vector<Type> cur_args = args.back();
                args.pop_back();
                cur_args.emplace_back(t);
                args.push_back(cur_args);
            }
        }

        else if (lhs == "factor" && rhs == " NUM") { 
            if (!args.empty()){
                vector<Type> cur_args = args.back();
                args.pop_back();
                cur_args.emplace_back(INT);
                args.push_back(cur_args);
            }
        }

        else if (lhs == "lvalue" && rhs == " ID") {
            string var = children[0]->rhs;
            if (!tables[cur_procedure].symbols.count(var)) {
                error = true;
                error_reason = "ID: " + var + " IS USED AS LVALUE BEFORE DECLARATION";
                return;
            }
        }

        for (u_int64_t i = 0; i < children.size(); i++) {
            children[i]->idCheck();
        }
    }

    Type getType() {
        if (error) {
            return UNDEF;
        }
        if (lhs == "main") {
            cur_procedure = "main";
            Type temp = children[11]->getType();
            if (temp != INT) {
                error = true;
                error_reason = "MAIN DOES NOT HAVE RETURN TYPE INT";
                return UNDEF;
            }
        }

        else if (lhs == "procedure") {
            cur_procedure = children[1]->rhs;
            Type temp = children[9]->getType();
            if (temp != INT) {
                error = true;
                error_reason = "PROCEDURE DOES NOT HAVE RETURN TYPE INT: " + cur_procedure;
                return UNDEF;
            }
        }

        else if (lhs == "LPAREN") {
            param_list = false;
        }

        // cout << lhs << "   " << param_list << endl;
        for (u_int64_t i = 0; i < children.size(); i++) {
            children[i]->getType();
        }
        if (type != UNDEF) {
            return type;
        }
        if (lhs == "NUM") {
            type = INT;
            return type;
        }
        if (lhs == "NULL") {
            type = INTSTAR;
            return type;
        }
        if (lhs == "ID") {
            if (param_list) return NONE;
            type = tables[cur_procedure].symbols[rhs];
            return type;
        }
        if (lhs == "factor") {
            if (rhs == " LPAREN expr RPAREN") {
                type = children[1]->getType(); 
            }
            else if (rhs == " AMP lvalue") {
                type = children[1]->getType();
                if (type != INT) {
                    error = true;
                    error_reason = "ATTEMPT TO POINT TO NON-INT";
                    return type;
                }
                type = INTSTAR;
            }
            else if (rhs == " STAR factor") {
                type = children[1]->getType();
                // cout << "STAR FACTOR  " << children[1]->lhs << children[1]->rhs << " " << type << endl;
                if (type != INTSTAR) {
                    error = true;
                    error_reason = "ATTEMPT TO DEREFERENCE NON-POINTER";
                    return type;
                }
                type = INT;
            }
            else if (rhs == " NEW INT LBRACK expr RBRACK") {
                type = children[3]->getType();
                if (type != INT) {
                    error = true;
                    error_reason = "ARRAY DECLARATION EXPECTS INT SIZE";
                    return type;
                }
                type = INTSTAR;
            }
            else if (rhs == " ID LPAREN RPAREN") {
                type = INT;
            }
            else if (rhs == " ID LPAREN arglist RPAREN") {
                type = INT;
            }
            else if (rhs == " GETCHAR LPAREN RPAREN") {
                type = INT;
            }
            else { // NUM/NULL/ID
                type = children[0]->getType();
            }
            // cout << type << endl;
            return type;
        }
        if (lhs == "term") {
            if (rhs == " factor") {
                type = children[0]->getType(); 
                // cout << " FACTOR  " << children[0]->lhs << children[0]->rhs << type << endl;
            }
            else { // STAR / DIV / PCT
                type = children[2]->getType();
                if (type != children[0]->getType()) {
                    error = true;
                    error_reason = "TERM HAS MISMATCH TYPES: " + rhs;
                    return type;
                }
                if (type != INT) {
                    error = true;
                    error_reason = "TERM IS NOT TYPE INT: " + rhs;
                }
            }
            return type;
        }
        if (lhs == "expr") {
            if (rhs == " term") {
                type = children[0]->getType(); 
            }
            else if (rhs == " expr PLUS term"){
                type = children[2]->getType();
                Type type2 = children[0]->getType();
                if (type == INTSTAR && type2 == INTSTAR) {
                    error = true;
                    error_reason = "ATTEMPTED ADDITION BETWEEN TWO POINTERS";
                    return type;
                }
                if (type2 == INTSTAR) {
                    type = INTSTAR;
                } // otherwise type is already INT
            }
            else { // expr MINUS term
                type = children[0]->getType();
                Type type2 = children[2]->getType();
                if (type == INT && type2 == INTSTAR) {
                    error = true;
                    error_reason = "ATTEMPTED SUBTRACTION OF POINTER FROM INT";
                    return type;
                }
                if (type == INTSTAR && type2 == INTSTAR) {
                    type = INT;
                } // otherwise type is already INT/INTSTAR
            }
            return type;
        }
        if (lhs == "lvalue") {
            if (rhs == " ID") {
                type = children[0]->getType();
            }
            else if (rhs == " STAR factor") {
                type = children[1]->getType();
                if (type != INTSTAR) {
                    error = true;
                    error_reason = "ATTEMPT TO DEREFERENCE NON-POINTER";
                    return UNDEF;
                }
                type = INT;
            }
            else { // LPAREN lvalue RPAREN
                type = children[1]->getType();
            }
            return type;
        }
        if (lhs == "statement") {
            if (rhs.find(" PRINTLN") == 0 || rhs.find(" PUTCHAR") == 0) {
                if (children[2]->getType() != INT) {
                    error = true;
                    error_reason = "PRINTLN/PUTCHAR PARAM MUST BE INT";
                    return UNDEF;
                }
            }
            if (rhs.find(" DELETE") == 0) {
                if (children[3]->getType() != INTSTAR) {
                    error = true;
                    error_reason = "DELETE PARAM MUST BE INTSTAR";
                    return UNDEF;
                }
            }
            if (rhs.find(" lvalue") == 0) {
                Type type0 = children[0]->getType();
                Type type2 = children[2]->getType();
                if (type0 != type2) {
                    error = true;
                    error_reason = "ATTEMPTED ASSIGN WITH TWO DIFFERENT TYPES: " + rhs;
                    return UNDEF;
                }
            }
            if (rhs.find(" IF") == 0 || rhs.find(" WHILE") == 0) {
                Type type0 = children[2]->children[0]->getType();
                Type type2 = children[2]->children[2]->getType();
                if (type0 != type2) {
                    error = true;
                    error_reason = "ATTEMPTED TEST WITH TWO DIFFERENT TYPES: " + rhs;
                    return UNDEF;
                }
            }
        }
        if (lhs == "dcls")
        {
            if (rhs == " dcls dcl BECOMES NUM SEMI") {
                Type type0 = children[1]->children[1]->getType();
                if (type0 != INT) {
                    error = true;
                    error_reason = "DECLARED NON-INT VAR AS NUM";
                    return UNDEF;
                }
            }
            if (rhs == " dcls dcl BECOMES NULL SEMI") {
                Type type0 = children[1]->children[1]->getType();
                if (type0 != INTSTAR) {
                    error = true;
                    error_reason = "DECLARED NON-POINTER VAR AS NULL";
                    return UNDEF;
                }
            }
        }
        
        type = NONE;
        return NONE;
    }

    void printTree() const {
        if (rhs == "") return;
        string type_output = "";
        if (type == INT) {
            type_output = " : int";
        }
        else if (type == INTSTAR) {
            type_output = " : int*";
        }
        std::cout << lhs << rhs << type_output << endl;
        // if (parent) std::cout << "parent:   " << parent->lhs << endl;
        for (auto it = children.begin(); it != children.end(); ++it) {
            (*it)->printTree();
        }
    }

    void printTreeToString(ostringstream& oss) const {
        if (rhs == "") return;
        string type_output = "";
        if (type == INT) {
            type_output = " : int";
        }
        else if (type == INTSTAR) {
            type_output = " : int*";
        }
        oss << lhs << rhs << type_output << endl;
        for (auto it = children.begin(); it != children.end(); ++it) {
            (*it)->printTreeToString(oss);
        }
    }
};

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

static bool isTerminal(const string& s) {
    return !non_terminals.count(s);
}

static Node *makeTree(Node *parent, bool terminal, istringstream& input) {
    string line, s;
    input >> s;
    getline(input, line);
    istringstream iss(line);

    Node* node = new Node(s, line, parent);

    if (terminal) {
        iss >> s;
        node->children.emplace_back(new Node(s, "", node, NONE));
        return node;
    }

    while (iss >> s) {
        if (s == ".EMPTY") {
            node->children.emplace_back(new Node(".EMPTY", "", node, NONE));
        }
        else {
            node->children.emplace_back(makeTree(node, isTerminal(s), input));
        }
    }
    return node;
}

// Debug operators - commented out to avoid unused function warnings
#if 0
// Overload the << operator for Procedure
static std::ostream& operator<<(std::ostream& os, const Procedure& procedure) {
    os << "Params: ";
    for (const auto& param : procedure.params) {
        os << param << " ";
    }
    os << "\nSymbols:\n";
    for (const auto& symbol : procedure.symbols) {
        os << "  " << symbol.first << " : " << symbol.second << "\n";
    }
    return os;
}

// Overload the << operator for the unordered_map<string, Procedure>
static std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string, Procedure>& tables) {
    for (const auto& entry : tables) {
        os << "Key: " << entry.first << "\n" << entry.second << "\n";
    }
    return os;
}
#endif

string typeCheck(const string& parseTreeString) {
    // INITIALIZE
    vector<Rule> rules;
    tables.clear();
    non_terminals.clear();
    args.clear();
    called_procedure.clear();

    // CFG
    istringstream iss0(WLP4_COMBINED);
    string line;
    getline(iss0, line);
    
    while (getline(iss0, line) && line != ".TRANSITIONS") {
        string lhs = getFirstPart(line);  
        rules.emplace_back(Rule(lhs, line, countWords(line)));
        non_terminals.insert(lhs);
    }

    istringstream input(parseTreeString);
    Node *tree = makeTree(nullptr, false, input);

    // ANALYSIS
    error = false;
    param_list = false;
    // SYMBOL TABLE / FIRST PASS
    tree->idCheck();

    if (error) {
        cerr << "ERROR: " << error_reason << endl;
        delete tree;
        return "";
    }

    // TYPE CHECK / SECOND PASS
    tree->getType();
    if (error) {
        cerr << "ERROR: " << error_reason << endl;
        delete tree;
        return "";
    }
    
    ostringstream output;
    tree->printTreeToString(output);
    
    delete tree;
    return output.str();
}