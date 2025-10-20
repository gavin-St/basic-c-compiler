#include "gen.h"
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
    vector<string> params;
    unordered_map<string, int> symbols;
    Procedure() {}
};

static vector<string> mc, mainc;
static int stack_off, proc_count;

static unordered_map<string, Procedure> tables;
static unordered_set<string> non_terminals;
static string cur_procedure;
static bool return_mode, param_list;
static deque<vector<Type>> args;
static deque<string> called_procedure;

int correctOffset(int i) {
    if (i <= 0) return i;
    else return tables[cur_procedure].params.size() - i + 1;
}

void add_code(string s) {
    if (cur_procedure == "wain") {
        mainc.emplace_back(s);
    }
    else {
        mc.emplace_back(s);
    }
}

void push(int reg) {
    string s = "sw $" + to_string(reg) + ", -4($30)";
    add_code(s);
    s = "sub $30, $30, $4";
    add_code(s);    
}

void loadWord(int reg, int diff) {
    diff = 4*correctOffset(diff);
    string s = "lw $" + to_string(reg) + ", " + to_string(diff) + "($29)";
    add_code(s);
}

void lis(int reg, int load) {
    string s = "lis $" + to_string(reg);
    add_code(s);
    s = ".word " + to_string(load);
    add_code(s);
}

void pop(int reg) {
    string s = "add $30, $30, $4";
    add_code(s);    
    s = "lw $" + to_string(reg) + ", -4($30)";
    add_code(s);    
}

void storeWord(int reg, int diff) {
    diff = 4*correctOffset(diff);
    string s = "sw $" + to_string(reg) + ", " + to_string(diff) + "($29)";
    add_code(s);
}


void comment(string s) {
    add_code("");
    s = ";;;  " + s;
    add_code(s);
    add_code("");
}

struct Node {
    string lhs;
    string rhs;
    Type type;
    vector<Node *> children;
    Node *parent;

    Node(string l, string r = "", Node *p = nullptr, Type t = UNDEF) : lhs{l}, rhs{r}, type{t}, parent{p} {}
    ~Node() {
        for (u_int64_t i = 0; i < children.size(); i++) {
            delete children[i];
        }
    };

    string getId() {
        if (rhs == "ID") {
            return children[0]->rhs;
        }
        else if (rhs == "LPAREN lvalue RPAREN") {
            return children[1]->getId();
        }
        else { // STAR factor
            return "POINTER";
        }
    }

    void generateCode() {
        if (lhs == "main" && rhs == "INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
            cur_procedure = "wain";
            comment("WAIN");
            add_code("wain:");

            comment("HEHE REGISTERS PUSH");

            comment("INNNIT");
                Type t1 = children[3]->children[1]->type;
                if (t1 == INT) {
                    add_code("add $5, $2, $0");
                    lis(2, 0);
                }
                push(31);
                add_code("lis $3");
                add_code(".word init");
                add_code("jalr $3");
                if (t1 == INT) {
                    add_code("add $2, $5, $0");
                }
                pop(31);

            param_list = true;            
            comment("START PARAMS");
            stack_off = 0;
            
        }

        else if (lhs == "procedure") {
            cur_procedure = children[1]->rhs;
            comment("PROC");
            param_list = true;
            stack_off = 0;  

            string temp = "P" + cur_procedure + ":";
            add_code(temp);

            add_code("sub $29, $30, $4");

            comment("HEHE REGISTERS PUSH");
    
            comment("START PARAMS");
        }
        
        else if (lhs == "dcl" && param_list) {
            string id = children[1]->rhs;
            stack_off += 1;
            tables[cur_procedure].symbols[id] = stack_off;
            tables[cur_procedure].params.emplace_back(id);
            
            if (cur_procedure == "wain")
                push(stack_off);
            
        }
       
        else if (lhs == "RPAREN" && param_list) {
            param_list = false;
            if (cur_procedure == "wain")
                add_code("sub $29, $30, $4");
            comment("END PARAMS");
            stack_off = 0;
        }
        
        else if (lhs == "SEMI" && return_mode) {
            return_mode = false;
            comment("END RETURN");

            comment("HEHE REGISTERS POP");

            lis(5, -stack_off*4);
            add_code("add $30, $30, $5");
            
            add_code("jr $31");
        }
        
        else if (lhs == "expr") {
            if (parent->lhs == "main" || parent->lhs == "procedure") {
                comment("START RETURN");
                return_mode = true;
            }
            if (rhs == "term") {
                children[0]->generateCode();
            }
            else {
                children[0]->generateCode();
                push(3);
                children[2]->generateCode();
                pop(5);
                Type t0 = children[0]->type, t2 = children[2]->type;
                if (rhs == "expr PLUS term") {
                    if (t0 == INT && t2 == INT) {
                        add_code("add $3, $5, $3");
                    }
                    else if (t0 == INTSTAR) {
                        add_code("mult $3, $4");
                        add_code("mflo $3");
                        add_code("add $3, $5, $3");
                    }
                    else {
                        add_code("mult $5, $4");
                        add_code("mflo $5");
                        add_code("add $3, $5, $3");
                    }
                }
                else {
                    if (t0 == INTSTAR && t2 == INTSTAR) {
                        add_code("sub $3, $5, $3");
                        add_code("div $3, $4");
                        add_code("mflo $3");
                    }
                    else if (t0 == INTSTAR) {
                        add_code("mult $3, $4");
                        add_code("mflo $3");
                        add_code("sub $3, $5, $3");
                    }
                    else {
                        add_code("sub $3, $5, $3");
                    }
                }
            }
            return;
        }

        else if (lhs == "term") {
            if (rhs == "factor") {
                children[0]->generateCode();
            }
            else {
                children[0]->generateCode();
                push(3);
                children[2]->generateCode();
                pop(5);
                if (rhs == "term STAR factor") {
                    add_code("mult $3, $5");
                    add_code("mflo $3");
                }
                else if (rhs == "term SLASH factor") {
                    add_code("div $5, $3");
                    add_code("mflo $3");
                }
                else {
                    add_code("div $5, $3");
                    add_code("mfhi $3");
                }
            }
            return;
        }

        else if (lhs == "factor") {
            if (rhs == "ID") {
                string id = children[0]->rhs;
                loadWord(3, tables[cur_procedure].symbols[id]);
            }
            else if (rhs == "NUM") {
                int num = stoi(children[0]->rhs);
                lis(3, num);
            }
            else if (rhs == "NULL") {
                lis(3, 1);
            }
            else if (rhs == "LPAREN expr RPAREN") {
                children[1]->generateCode();
            }
            else if (rhs == "AMP lvalue") {
                string id = children[1]->getId();
                if (id == "POINTER") {
                    children[1]->children[1]->generateCode();
                }
                else {
                    int diff = tables[cur_procedure].symbols[id];
                    diff = 4*correctOffset(diff);
                    lis(3, diff);
                    add_code("add $3, $3, $29");
                }
            }
            else if (rhs == "STAR factor") {
                children[1]->generateCode();
                add_code("lw $3, 0($3)");
            }
            else if (rhs == "ID LPAREN RPAREN") {
                push(31);
                string id = children[0]->rhs;
                add_code("lis $3");
                id = ".word P" + id;
                add_code(id);

                add_code("jalr $3");
                pop(31);
                return;
            }
            else if (rhs == "ID LPAREN arglist RPAREN") {
                push(31);
                push(29);
                string id = children[0]->rhs;
                int size = tables[id].params.size();
                comment("START ARGS");
                children[2]->generateCode();

                add_code("lis $3");
                id = ".word P" + id;
                add_code(id);

                add_code("jalr $3");

                comment("END ARGS");

                for (int i = 0; i < size; i++) {
                    pop(5);
                }
                pop(29);
                pop(31);

                return;
            }
            else if (rhs == "GETCHAR LPAREN RPAREN") {
                add_code("lis $5");
                add_code(".word 0xffff0004");
                add_code("lw $3, 0($5)");
            }
            else if (rhs == "NEW INT LBRACK expr RBRACK") {
                children[3]->generateCode();
                add_code("add $1, $3, $0");
                
                push(31);
                add_code("lis $5");
                add_code(".word new");
                add_code("jalr $5");
                pop(31);

                string temp = "beq $3, $0, Tnull" + to_string(++proc_count);
                add_code(temp);
                temp = "beq $0, $0, Tnewend" + to_string(proc_count);
                add_code(temp);
                temp = "Tnull" + to_string(proc_count) + ":";
                add_code(temp);
                lis(3, 1);
                temp = "Tnewend" + to_string(proc_count) + ":";
                add_code(temp);
            }
            return;
        }

        else if (lhs == "arglist") {
            children[0]->generateCode();
            push(3);
            if (rhs != "expr") {
                children[2]->generateCode();
            }
            return;
        }

        else if (lhs == "dcls") {
            if (rhs == "dcls dcl BECOMES NUM SEMI") {
                string id = children[1]->children[1]->rhs;
                int num = stoi(children[3]->rhs);
                tables[cur_procedure].symbols[id] = stack_off--;
                lis(3, num);
                push(3);
            }
            else if (rhs == "dcls dcl BECOMES NULL SEMI") {
                string id = children[1]->children[1]->rhs;
                tables[cur_procedure].symbols[id] = stack_off--;
                lis(3, 1);
                push(3);
            }
        }

        else if (lhs == "statement") {
            if (rhs == "lvalue BECOMES expr SEMI") {
                children[2]->generateCode();
                string id = children[0]->getId();
                if (id == "POINTER") {
                    push(3);
                    children[0]->children[1]->generateCode();
                    pop(5);
                    add_code("sw $5, 0($3)");
                } else {
                    storeWord(3, tables[cur_procedure].symbols[id]);
                }
            }
            else if (rhs == "IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
                comment("IF CONDITION");
                children[2]->generateCode();
                string else_label = "else" + to_string(proc_count++);
                string temp = "beq $3, $0, " + else_label;
                add_code(temp);

                comment("START IF");
                children[5]->generateCode();
                temp = "beq $0, $0, end" + else_label;
                add_code(temp);
                
                comment("ELSE");
                temp = else_label + ":";
                add_code(temp);
                children[9]->generateCode();

                comment("END IF");
                temp = "end" + else_label + ":";
                add_code(temp);
            }
            else if (rhs == "WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
                string while_label = "while" + to_string(proc_count++);
                string temp = while_label + ":";
                add_code(temp);

                comment("START WHILE CONDITION");
                children[2]->generateCode();
                temp = "beq $3, $0, end" + while_label;
                add_code(temp);
                comment("START WHILE");
                
                children[5]->generateCode();
                temp = "beq $0, $0, " + while_label;
                add_code(temp);

                comment("END WHILE");
                temp = "end" + while_label + ":";
                add_code(temp);
            }
            else if (rhs == "PUTCHAR LPAREN expr RPAREN SEMI") {
                children[2]->generateCode();
                add_code("lis $5");
                add_code(".word 0xffff000c");
                add_code("sw $3, 0($5)");
            }
            else if (rhs == "PRINTLN LPAREN expr RPAREN SEMI") {
                children[2]->generateCode();
                add_code("add $1, $3, $0");
                push(31);
                add_code("lis $5");
                add_code(".word print");
                add_code("jalr $5");
                pop(31);
            }
            else if (rhs == "DELETE LBRACK RBRACK expr SEMI") {
                children[3]->generateCode();
                add_code("add $1, $3, $0");
                lis(5,1);

                string temp = "beq $3, $5, Tnodelete" + to_string(++proc_count);
                add_code(temp);
                
                push(31);
                add_code("lis $5");
                add_code(".word delete");
                add_code("jalr $5");
                pop(31);
                
                temp = "Tnodelete" + to_string(proc_count) + ":";
                add_code(temp);
            }
            return;
        }

        else if (lhs == "test") {
            children[0]->generateCode();
            push(3);
            children[2]->generateCode();
            pop(5);
            if (rhs == "expr EQ expr") {
                add_code("sub $3, $5, $3");

                string proc = "test" + to_string(proc_count++);
                string temp = "beq $3, $0, " + proc;
                add_code(temp);
                add_code("add $3, $0, $0");

                temp = "beq $0, $0, end" + proc;
                add_code(temp);

                temp = proc + ":";
                add_code(temp);
                
                add_code("lis $3");
                add_code(".word 1");
                
                temp = "end" + proc + ":";
                add_code(temp);
            }
            else if (rhs == "expr NE expr") {
                add_code("sub $3, $5, $3");
            }
            else if (rhs == "expr LT expr") {
                if (children[0] -> type == INT)
                    add_code("slt $3, $5, $3");
                else
                    add_code("sltu $3, $5, $3");
            }
            else if (rhs == "expr LE expr") {
                if (children[0] -> type == INT)
                    add_code("slt $3, $3, $5");
                else
                    add_code("sltu $3, $3, $5");
                lis(5, 1);
                add_code("sub $3, $5, $3");
            }
            else if (rhs == "expr GE expr") {
                if (children[0] -> type == INT)
                    add_code("slt $3, $5, $3");
                else
                    add_code("sltu $3, $5, $3");
                lis(5, 1);
                add_code("sub $3, $5, $3");
            }
            else if (rhs == "expr GT expr") {
                if (children[0] -> type == INT)
                    add_code("slt $3, $3, $5");
                else
                    add_code("sltu $3, $3, $5");
            }
            return;
        }

        for (u_int64_t i = 0; i < children.size(); i++) {
            children[i]->generateCode();
        }
    }

    void printTree(int indent) const {
        if (rhs == "") return;
        string type_output = "";
        if (type == INT) {
            type_output = " : int";
        }
        else if (type == INTSTAR) {
            type_output = " : int*";
        }
        for (int i = 0; i < indent; i++) {
            cout << "| ";
        }
        std::cout << lhs << " " << rhs << type_output << endl;
        // if (parent) std::cout << "parent:   " << parent->lhs << endl;
        for (auto it = children.begin(); it != children.end(); ++it) {
            (*it)->printTree(indent + 1);
        }
    }
};

static string getFirstPart(const string& str) {
    istringstream iss(str);
    string firstPart;
    iss >> firstPart;
    return firstPart;
}

// remove last 2 space separated words if it is type annotation
static string removeTypeAnnotation(const string& str) {
    stringstream ss(str);
    vector<string> words;
    string word;

    while (ss >> word) {
        if (word == ":") 
            break;
        words.push_back(word);
    }

    string result;
    for (size_t i = 0; i < words.size(); ++i) {
        result += words[i];
        if (i < words.size() - 1) {
            result += " ";
        }
    }

    return result;
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

    Node* node = new Node(s, removeTypeAnnotation(line), parent);

    if (terminal) {
        iss >> s;
        node->children.emplace_back(new Node(s, "", node, NONE));
        iss >> s;
        if (s == ":") {
            iss >> s;
            if (s == "int") 
                node->type = INT;
            else
                node->type = INTSTAR;
        }

        return node;
    }

    while (iss >> s) {
        if (s == ":") {
            iss >> s;
            if (s == "int") 
                node->type = INT;
            else
                node->type = INTSTAR;
            continue;
        }
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

static std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i] << endl;
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

string generateCode(const string& typedParseTreeString) {
    // INITIALIZE
    vector<Rule> rules;
    tables.clear();
    non_terminals.clear();
    mc.clear();
    mainc.clear();

    // CFG
    istringstream iss0(WLP4_COMBINED);
    string line;
    getline(iss0, line);
    
    while (getline(iss0, line) && line != ".TRANSITIONS") {
        string lhs = getFirstPart(line);  
        rules.emplace_back(Rule(lhs, line, countWords(line)));
        non_terminals.insert(lhs);
    }

    istringstream input(typedParseTreeString);
    Node *tree = makeTree(nullptr, false, input);

    mainc.emplace_back(".import print");
    mainc.emplace_back(".import init");
    mainc.emplace_back(".import new");
    mainc.emplace_back(".import delete");
    mainc.emplace_back("lis $4");
    mainc.emplace_back(".word 4");

    stack_off = 0;
    proc_count = 0;
    param_list = false;
    return_mode = false;
    tree->generateCode();
    
    ostringstream output;
    for (const auto& line : mainc) {
        output << line << endl;
    }
    for (const auto& line : mc) {
        output << line << endl;
    }

    delete tree;
    return output.str();
}
