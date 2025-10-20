#ifndef TYPE_H
#define TYPE_H

#include "parse.h"
#include <string>

// Type check the parse tree and annotate with types
// Returns the typed parse tree as a string (for gen to consume)
// Returns empty string on error
std::string typeCheck(const std::string& parseTreeString);

#endif // TYPE_H

