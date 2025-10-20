#ifndef SCAN_H
#define SCAN_H

#include <string>
#include <vector>

struct Token {
    std::string type;  // e.g., "INT", "NUM", "ID"
    std::string lexeme; // e.g., "int", "42", "x"
};

// Scan WLP4 source code and return tokens
// Returns empty vector on error
std::vector<Token> scanTokens(const std::string& input);

#endif // SCAN_H

