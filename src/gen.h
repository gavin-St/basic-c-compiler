#ifndef GEN_H
#define GEN_H

#include <string>

// Generate MIPS assembly code from typed parse tree
// Returns assembly code as string
// Returns empty string on error
std::string generateCode(const std::string& typedParseTreeString);

#endif // GEN_H

