#ifndef ASM_H
#define ASM_H

#include <string>

// Assemble MIPS assembly code
// Returns assembled code as string
// Returns empty string on error
std::string assemble(const std::string& assemblyCode);

#endif // ASM_H

