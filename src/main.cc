#include "scan.h"
#include "parse.h"
#include "type.h"
#include "gen.h"
#include "asm.h"
#include <iostream>
#include <sstream>
#include <string>

int main() {
    // Read WLP4 source code from stdin
    std::ostringstream inputBuffer;
    std::string line;
    while (std::getline(std::cin, line)) {
        inputBuffer << line << "\n";
    }
    std::string sourceCode = inputBuffer.str();

    // Stage 1: Scan (lexical analysis)
    std::vector<Token> tokens = scanTokens(sourceCode);
    if (tokens.empty() && !sourceCode.empty()) {
        std::cerr << "Compilation failed at scanning stage" << std::endl;
        return 1;
    }

    // Stage 2: Parse (syntax analysis)
    ParseNode* parseTree = buildParseTree(tokens);
    if (!parseTree) {
        std::cerr << "Compilation failed at parsing stage" << std::endl;
        return 1;
    }

    std::string parseTreeStr = parseTreeToString(parseTree);
    delete parseTree;

    if (parseTreeStr.empty()) {
        std::cerr << "Compilation failed at parse tree generation" << std::endl;
        return 1;
    }

    // Stage 3: Type checking (semantic analysis)
    std::string typedParseTree = typeCheck(parseTreeStr);
    if (typedParseTree.empty()) {
        std::cerr << "Compilation failed at type checking stage" << std::endl;
        return 1;
    }

    // Stage 4: Code generation
    std::string assemblyCode = generateCode(typedParseTree);
    if (assemblyCode.empty()) {
        std::cerr << "Compilation failed at code generation stage" << std::endl;
        return 1;
    }

    // Stage 5: Assembly
    std::string finalOutput = assemble(assemblyCode);
    if (finalOutput.empty()) {
        std::cerr << "Compilation failed at assembly stage" << std::endl;
        return 1;
    }

    // Output the final MIPS code
    std::cout << finalOutput;

    return 0;
}

