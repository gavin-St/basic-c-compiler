#ifndef PARSE_H
#define PARSE_H

#include "scan.h"
#include <string>
#include <vector>

struct ParseNode {
    std::string value;
    std::vector<ParseNode*> children;
    
    ParseNode(const std::string& val) : value(val) {}
    
    // Deep copy constructor
    ParseNode(const ParseNode& other);
    
    ~ParseNode() {
        for (auto child : children) {
            delete child;
        }
    }
    
    void addChild(ParseNode* child) {
        children.push_back(child);
    }
};

// Parse tokens into a parse tree
// Returns nullptr on error
ParseNode* buildParseTree(const std::vector<Token>& tokens);

// Convert parse tree to string representation
std::string parseTreeToString(const ParseNode* root);

#endif // PARSE_H

