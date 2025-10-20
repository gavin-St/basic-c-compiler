#include "asm.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <map>
#include <vector>

/** Prints an error to stderr with an "ERROR: " prefix, and newline suffix.
 *
 * @param message The error to print
 */
void formatError(const std::string & message)
{
    std::cerr << "ERROR: " << message << std::endl;
}

/** Convert a string representation of a number to an unsigned integer.
 *
 * If the string is "0", then 0 is returned.  If the string starts with "0x", the string is
 * interpreted as an unsigned hexidecimal number.  If the string starts with a "0", the string is
 * interpreted as an unsigned octal number.  Otherwise, the string is interpreted as a signed
 * decimal number.
 *
 * The function name is read as "string to uint64".
 *
 * @param s The string to parse
 * @return The uint64_t representation of the string
 */
uint64_t stouint64(const std::string & s)
{
    if(s == "0")
    {
        return 0;
    }
    if(s.starts_with("0x"))
    {
        return std::stol(s.substr(2), nullptr, 16);
    }

    if(s.starts_with("0"))
    {
        return std::stol(s.substr(1), nullptr, 8);
    }

    return std::stod(s);
}


/** For a given instruction, prints the instruction in the ascii representation of a call to
 *  compileLine().
 *
 * @param instruction The name of the instruction
 * @param one The value of the first parameter
 * @param two The value of the second parameter, 0 if the instruction has < 2 parameter
 * @param three The value of the third parameter, 0 if the instruction has < 3 parameter
 */
// Global variable for output capture
static std::ostringstream* g_output_stream = nullptr;

bool compileLine(const std::string & instruction,
                 uint64_t one,
                 uint64_t two,
                 uint64_t three)
{
    if (g_output_stream) {
        *g_output_stream << instruction << " " << one << " " << two << " " << three << std::endl;
    } else {
        std::cout << instruction << " " << one << " " << two << " " << three << std::endl;
    }

    return true;
}

bool compileLine2(const std::string & instruction,
                 uint64_t one,
                 uint64_t two,
                 uint16_t three)
{
    std::cout << instruction << " " << one << " " << two << " " << three << std::endl;

    return true;
}

enum TokenType {
    // Not a real token type we output: a unique value for initializing a TokenType when the
    // actual value is unknown
    NONE,

    DIRECTIVE,
    LABEL,
    ID,
    HEXINT,
    REG,
    DEC,
    COMMA,
    LPAREN,
    RPAREN,
    NEWLINE,
};

class Token
{
public:
    const TokenType type;
    const std::string lexeme;

    Token(TokenType type, std::string lexeme);
};

Token::Token(TokenType t, std::string l)
  : type(t), lexeme(l)
{
    // Nothing
}

#define TOKEN_TYPE_PRINTER(t) case t: return #t
const char * tokenTypeString(TokenType t)
{
    switch(t)
    {
        TOKEN_TYPE_PRINTER(NONE);
        TOKEN_TYPE_PRINTER(LABEL);
        TOKEN_TYPE_PRINTER(DIRECTIVE);
        TOKEN_TYPE_PRINTER(ID);
        TOKEN_TYPE_PRINTER(HEXINT);
        TOKEN_TYPE_PRINTER(REG);
        TOKEN_TYPE_PRINTER(DEC);
        TOKEN_TYPE_PRINTER(COMMA);
        TOKEN_TYPE_PRINTER(LPAREN);
        TOKEN_TYPE_PRINTER(RPAREN);
        TOKEN_TYPE_PRINTER(NEWLINE);
    }

    // We will never get here
    return "";
}
#undef TOKEN_TYPE_PRINTER

#define TOKEN_TYPE_READER(s, t) if(s == #t) return t
TokenType stringToTokenType(const std::string & s)
{
    TOKEN_TYPE_READER(s, NONE);
    TOKEN_TYPE_READER(s, LABEL);
    TOKEN_TYPE_READER(s, DIRECTIVE);
    TOKEN_TYPE_READER(s, ID);
    TOKEN_TYPE_READER(s, HEXINT);
    TOKEN_TYPE_READER(s, REG);
    TOKEN_TYPE_READER(s, DEC);
    TOKEN_TYPE_READER(s, COMMA);
    TOKEN_TYPE_READER(s, LPAREN);
    TOKEN_TYPE_READER(s, RPAREN);
    TOKEN_TYPE_READER(s, NEWLINE);
    return NONE;
}
#undef TOKEN_TYPE_READER

std::ostream & operator<<(std::ostream & out, const Token token)
{
    out << tokenTypeString(token.type) << " " << token.lexeme;
    return out;
}

/** OLD MAIN - Kept for reference, but commented out since we now use assemble() function
 *  Entrypoint for the assembler.  The first parameter (optional) is a mips assembly file to
 *  read.  If no parameter is specified, read assembly from stdin.  Prints machine code to stdout.
 *  If invalid assembly is found, prints an error to stderr, stops reading assembly, and return a
 *  non-0 value.
 *
 * If the file is not found, print an error and returns a non-0 value.
 *
 * @return 0 on success, non-0 on error
 */
#if 0
int main(int argc, char * argv[])
{
    if(argc > 2)
    {
        std::cerr << "Usage:" << std::endl
                  << "\tasm [$FILE]" << std::endl
                  << std::endl
                  << "If $FILE is unspecified or if $FILE is `-`, read the assembly from standard "
                  << "in. Otherwise, read the assembly from $FILE." << std::endl;
        return 1;
    }

    std::ifstream fp;
    std::istream &in =
        (argc > 1 && std::string(argv[1]) != "-")
      ? [&]() -> std::istream& {
            fp.open(argv[1]);
            return fp;
        }()
      : std::cin;

    if(!fp && argc > 1)
    {
        formatError((std::stringstream() << "file '" << argv[1] << "' not found!").str());
        return 1;
    }

    std::vector<Token> tokens;
    std::map<std::string, uint32_t> labelMap;
    std::vector<std::string> labels;
    uint32_t instr_num = 0;
    bool instr_added = false;

    while(!in.eof())
    {
        std::string line;
        std::getline( in, line );
        if(line == "")
        {
                continue;
        }

        std::string tokenType;
        std::string lexeme;

        std::stringstream lineParser(line);
        lineParser >> tokenType;
        if(tokenType != "NEWLINE")
        {
            lineParser >> lexeme;
            if (tokenType == "LABEL") {
                lexeme.pop_back();
                labelMap[lexeme] = instr_num;
                labels.emplace_back(lexeme);
            }
            else if (!instr_added) {
                instr_added = true;
                instr_num += 4;
            }
        }   
        else {  // new line
            instr_added = false;
        }


        tokens.push_back(Token(stringToTokenType(tokenType), lexeme));
    }

    for (const std::string& s : labels) {
        std::cerr << s << " " << labelMap[s] << std::endl;
    }

    int pc = 0;
    for (std::vector<Token>::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == NEWLINE || it->type == LABEL)
            continue;
        if (it->type == ID) {
            std::string instruction = it->lexeme;
            // $d, $s, $t
            uint64_t d = 0, s = 0, t = 0;
            if (instruction == "add" || instruction == "sub" || instruction == "slt" || instruction == "sltu") {
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                d = stouint64(it->lexeme.substr(1));

                ++it;
                if (it == tokens.end() || it->type != COMMA) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                s = stouint64(it->lexeme.substr(1));

                ++it;
                if (it == tokens.end() || it->type != COMMA) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }

                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                t = stouint64(it->lexeme.substr(1));

                compileLine(instruction, d, s, t);

                ++it;
                if (it == tokens.end()) break;
                if (it->type != NEWLINE) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
            }
            // $d, $s, i
            else if (instruction == "beq" || instruction == "bne") {
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                d = stouint64(it->lexeme.substr(1));

                ++it;
                if (it == tokens.end() || it->type != COMMA) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                s = stouint64(it->lexeme.substr(1));

                ++it;
                if (it == tokens.end() || it->type != COMMA) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }

                ++it;
                if (it == tokens.end() || (it->type != DEC && it->type != ID)) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                if (it->type == ID) {
                    if (labelMap.count(it->lexeme) == 0) {
                        formatError("LABEL NOT FOUND");
                        return 1;
                    }
                    t = (labelMap[it->lexeme] - pc) / 4 - 1;
                }
                else {
                    t = stouint64(it->lexeme);
                }

                compileLine2(instruction, d, s, t);

                ++it;
                if (it == tokens.end()) break;
                if (it->type != NEWLINE) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
            }
            // $t, i($s)
            else if (instruction == "lw" || instruction == "sw") {
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                d = stouint64(it->lexeme.substr(1));

                ++it;
                if (it == tokens.end() || it->type != COMMA) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }

                ++it;
                if (it == tokens.end() || it->type != DEC) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                s = stouint64(it->lexeme);

                ++it;
                if (it == tokens.end() || it->type != LPAREN) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }

                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                t = stouint64(it->lexeme.substr(1));

                ++it;
                if (it == tokens.end() || it->type != RPAREN) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }

                compileLine(instruction, d, s, t);

                ++it;
                if (it == tokens.end()) break;
                if (it->type != NEWLINE) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
            }
            // $s, $t
            else if (instruction == "mult" || instruction == "multu" || instruction == "div" || instruction == "divu") {
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                d = stouint64(it->lexeme.substr(1));
                
                ++it;
                if (it == tokens.end() || it->type != COMMA) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                s = stouint64(it->lexeme.substr(1));
                
                compileLine(instruction, d, s, t);

                ++it;
                if (it == tokens.end()) break;
                if (it->type != NEWLINE) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
            }
            // $d
            else if (instruction == "mfhi" || instruction == "mflo") {
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                d = stouint64(it->lexeme.substr(1));

                compileLine(instruction, d, s, t);

                ++it;
                if (it == tokens.end()) break;
                if (it->type != NEWLINE) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
            }
            // $d/label
            else if (instruction == "jr" || instruction == "jalr") {
                ++it;
                if (it == tokens.end() || (it->type != REG && it->type != ID)) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                if (it->type == ID) {
                    if (labelMap.count(it->lexeme) == 0) {
                        formatError("LABEL NOT FOUND");
                        return 1;
                    }
                    d = labelMap[it->lexeme];
                }
                else {
                    d = stouint64(it->lexeme.substr(1));
                }

                compileLine(instruction, d, s, t);

                ++it;
                if (it == tokens.end()) break;
                if (it->type != NEWLINE) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
            }
            // lis
            else if (instruction == "lis") {
                ++it;
                if (it == tokens.end() || it->type != REG) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
                d = stouint64(it->lexeme.substr(1));

                compileLine(instruction, d, s, t);

                ++it;
                if (it == tokens.end()) break;
                if (it->type != NEWLINE) {
                    formatError("GENERAL SEMANTIC");
                    return 1;
                }
            }
            else {
                formatError("BAD ID");
                return 1;
            }
        }
        else if (it->type == DIRECTIVE) {
            if (it->lexeme != ".word") {
                formatError("NOT WORD DIRECTIVE");
                return 1;
            }
            ++it;
            if (it == tokens.end()) {
                formatError("GENERAL SEMANTIC");
                return 1;
            }
            if (it->type != ID && it->type != HEXINT && it->type != DEC) {
                formatError("GENERAL SEMANTIC");
                return 1;
            }
            if (it->type == ID) {
                if (labelMap.count(it->lexeme) == 0) {
                    formatError("LABEL NOT FOUND");
                    return 1;
                }
                compileLine("word", labelMap[it->lexeme], 0 , 0);
            }
            else {
                compileLine("word", stouint64(it->lexeme), 0 , 0);
            }
        }
        else {
            formatError("GENERAL SEMANTIC");
            return 1;
        }
        pc += 4;
    }
    
    return 0;  
}
#endif

std::string assemble(const std::string& assemblyCode) {
    std::ostringstream output;
    g_output_stream = &output;

    std::istringstream in(assemblyCode);
    std::vector<Token> tokens;
    std::map<std::string, uint32_t> labelMap;
    std::vector<std::string> labels;
    uint32_t instr_num = 0;
    bool instr_added = false;

    while(!in.eof())
    {
        std::string line;
        std::getline( in, line );
        if(line == "")
        {
                continue;
        }

        std::string tokenType;
        std::string lexeme;

        std::stringstream lineParser(line);
        lineParser >> tokenType;
        if(tokenType != "NEWLINE")
        {
            lineParser >> lexeme;
            if (tokenType == "LABEL") {
                lexeme.pop_back();
                labelMap[lexeme] = instr_num;
                labels.emplace_back(lexeme);
            }
            else if (!instr_added) {
                ++instr_num;
                instr_added = true;
            }

            std::string next_token;
            while (lineParser >> next_token) {
                lexeme += " " + next_token;
            }

            tokens.emplace_back(stringToTokenType(tokenType), lexeme);
        }
        else {
            tokens.emplace_back(stringToTokenType(tokenType), "");
            instr_added = false;
        }
    }

    uint32_t pc = 0;

    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        if (it->type == LABEL) {
            continue;
        }

        std::string instruction = it->lexeme;
        ++it;

        if (instruction == ".word") {
            if (it->type != ID && it->type != HEXINT && it->type != DEC) {
                std::cerr << "ERROR: GENERAL SEMANTIC" << std::endl;
                g_output_stream = nullptr;
                return "";
            }
            if (it->type == ID) {
                if (labelMap.count(it->lexeme) == 0) {
                    std::cerr << "ERROR: LABEL NOT FOUND" << std::endl;
                    g_output_stream = nullptr;
                    return "";
                }
                compileLine("word", labelMap[it->lexeme], 0 , 0);
            }
            else {
                compileLine("word", stouint64(it->lexeme), 0 , 0);
            }
        }
        else {
            uint64_t d = 0, s = 0, t = 0;

            if (instruction == "add" || instruction == "sub" || instruction == "slt" || instruction == "sltu") {
                d = stouint64(it->lexeme.substr(1));
                ++it; ++it;
                s = stouint64(it->lexeme.substr(1));
                ++it; ++it;
                t = stouint64(it->lexeme.substr(1));

                compileLine(instruction, d, s, t);
                ++it;
            }
            else if (instruction == "beq" || instruction == "bne") {
                s = stouint64(it->lexeme.substr(1));
                ++it; ++it;
                t = stouint64(it->lexeme.substr(1));
                ++it; ++it;

                if (it->type == ID) {
                    if (labelMap.count(it->lexeme) == 0) {
                        std::cerr << "ERROR: LABEL NOT FOUND" << std::endl;
                        g_output_stream = nullptr;
                        return "";
                    }
                    int32_t offset = labelMap[it->lexeme] - pc - 4;
                    if (offset % 4 != 0) {
                        std::cerr << "ERROR: MISALIGNED OFFSET" << std::endl;
                        g_output_stream = nullptr;
                        return "";
                    }
                    d = offset / 4;
                }
                else {
                    d = stouint64(it->lexeme);
                }

                compileLine(instruction, d, s, t);
                ++it;
            }
            else if (instruction == "lis" || instruction == "mflo" || instruction == "mfhi") {
                d = stouint64(it->lexeme.substr(1));
                compileLine(instruction, d, s, t);
                ++it;
            }
            else if (instruction == "mult" || instruction == "multu" || instruction == "div" || instruction == "divu") {
                s = stouint64(it->lexeme.substr(1));
                ++it; ++it;
                t = stouint64(it->lexeme.substr(1));
                compileLine(instruction, d, s, t);
                ++it;
            }
            else if (instruction == "jr" || instruction == "jalr") {
                s = stouint64(it->lexeme.substr(1));
                compileLine(instruction, d, s, t);
                ++it;
            }
            else if (instruction == "lw" || instruction == "sw") {
                t = stouint64(it->lexeme.substr(1));
                ++it; ++it;
                std::regex pattern(R"((-?\d+)\((\$\d+)\))");
                std::smatch matches;
                std::string offset_reg = it->lexeme;

                if (std::regex_search(offset_reg, matches, pattern)) {
                    d = stouint64(matches[1].str());
                    s = stouint64(matches[2].str().substr(1));
                }

                compileLine(instruction, d, s, t);
                ++it;
            }
            else {
                std::cerr << "ERROR: UNKNOWN INSTRUCTION" << std::endl;
                g_output_stream = nullptr;
                return "";
            }
        }
        pc += 4;
    }

    g_output_stream = nullptr;
    return output.str();
}
