#include "scan.h"
#include <iostream>
#include <sstream>
#include <cctype>

std::vector<Token> scanTokens(const std::string& input) {
    std::vector<Token> tokens;
    std::istringstream iss(input);
    std::string s;

    while (std::getline(iss, s)) {
        if (s == ".ERROR") {
            std::cerr << "ERROR: DFA ERROR" << std::endl;
            return {};
        }
        else if (s == ".NEWLINE" || s == ".SPACE") {
            continue;
        }
        else if (s.starts_with("//")) {
            continue;
        }
        else if (s == "(") {
            tokens.push_back({"LPAREN", s});
        }
        else if (s == ")") {
            tokens.push_back({"RPAREN", s});
        }
        else if (s == "{") {
            tokens.push_back({"LBRACE", s});
        }
        else if (s == "}") {
            tokens.push_back({"RBRACE", s});
        }
        else if (s == "=") {
            tokens.push_back({"BECOMES", s});
        }
        else if (s == "==") {
            tokens.push_back({"EQ", s});
        }
        else if (s == "return") {
            tokens.push_back({"RETURN", s});
        }
        else if (s == "if") {
            tokens.push_back({"IF", s});
        }
        else if (s == "else") {
            tokens.push_back({"ELSE", s});
        }
        else if (s == "while") {
            tokens.push_back({"WHILE", s});
        }
        else if (s == "println") {
            tokens.push_back({"PRINTLN", s});
        }
        else if (s == "putchar") {
            tokens.push_back({"PUTCHAR", s});
        }
        else if (s == "getchar") {
            tokens.push_back({"GETCHAR", s});
        }
        else if (s == "wain") {
            tokens.push_back({"WAIN", s});
        }
        else if (s == "int") {
            tokens.push_back({"INT", s});
        }
        else if (s == "!=") {
            tokens.push_back({"NE", s});
        }
        else if (s == "<") {
            tokens.push_back({"LT", s});
        }
        else if (s == ">") {
            tokens.push_back({"GT", s});
        }
        else if (s == "<=") {
            tokens.push_back({"LE", s});
        }
        else if (s == ">=") {
            tokens.push_back({"GE", s});
        }
        else if (s == "+") {
            tokens.push_back({"PLUS", s});
        }
        else if (s == "-") {
            tokens.push_back({"MINUS", s});
        }
        else if (s == "*") {
            tokens.push_back({"STAR", s});
        }
        else if (s == "/") {
            tokens.push_back({"SLASH", s});
        }
        else if (s == "%") {
            tokens.push_back({"PCT", s});
        }
        else if (s == ",") {
            tokens.push_back({"COMMA", s});
        }
        else if (s == ";") {
            tokens.push_back({"SEMI", s});
        }
        else if (s == "new") {
            tokens.push_back({"NEW", s});
        }
        else if (s == "delete") {
            tokens.push_back({"DELETE", s});
        }
        else if (s == "[") {
            tokens.push_back({"LBRACK", s});
        }
        else if (s == "]") {
            tokens.push_back({"RBRACK", s});
        }
        else if (s == "&") {
            tokens.push_back({"AMP", s});
        }
        else if (s == "NULL") {
            tokens.push_back({"NULL", s});
        }
        else if (!s.empty() && std::isdigit(s[0])) {
            double temp = std::stod(s);
            if (temp > 2147483647) {
                std::cerr << "ERROR: NUMBER TOO BIG" << std::endl;
                return {};
            }
            tokens.push_back({"NUM", s});
        }
        else if (!s.empty()) {
            tokens.push_back({"ID", s});
        }
    }

    return tokens;
}
