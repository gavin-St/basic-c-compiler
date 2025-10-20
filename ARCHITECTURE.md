# WLP4 Compiler Architecture

## Overview

This compiler has been refactored from a pipeline of separate executables into a single modular executable with clean interfaces between components.

## Directory Structure

```
basic-c-compiler/
├── Makefile              # Build system
├── README.md             # User documentation
├── ARCHITECTURE.md       # This file
├── src/
│   ├── main.cc          # Compiler driver
│   ├── scan.{h,cc}      # Lexical analyzer
│   ├── parse.{h,cc}     # Parser
│   ├── type.{h,cc}      # Type checker
│   ├── gen.{h,cc}       # Code generator
│   ├── asm.{h,cc}       # Assembler
│   └── wlp4data.h       # Grammar & DFA data
└── test-c-code/         # Test programs
```

## Component Interfaces

### 1. Scan (Lexical Analysis)
**Header:** `scan.h`  
**Function:** `std::vector<Token> scanTokens(const std::string& input)`

Converts WLP4 source code into a stream of tokens.

**Input:** Raw WLP4 source code as string  
**Output:** Vector of Token objects (type + lexeme pairs)  
**Errors:** Returns empty vector on error

### 2. Parse (Syntax Analysis)
**Header:** `parse.h`  
**Function:** `ParseNode* buildParseTree(const std::vector<Token>& tokens)`

Builds a parse tree using the WLP4 grammar and LR parsing.

**Input:** Vector of tokens from scanner  
**Output:** Parse tree (ParseNode*)  
**Errors:** Returns nullptr on error

Helper function: `std::string parseTreeToString(const ParseNode* root)` converts tree to string format.

### 3. Type (Semantic Analysis)
**Header:** `type.h`  
**Function:** `std::string typeCheck(const std::string& parseTreeString)`

Performs type checking and annotates the parse tree with type information.

**Input:** Parse tree as string  
**Output:** Typed parse tree as string (with `: int` and `: int*` annotations)  
**Errors:** Returns empty string on error, prints error message to stderr

### 4. Gen (Code Generation)
**Header:** `gen.h`  
**Function:** `std::string generateCode(const std::string& typedParseTreeString)`

Generates MIPS assembly code from the typed parse tree.

**Input:** Typed parse tree as string  
**Output:** MIPS assembly code as string  
**Errors:** Returns empty string on error

### 5. Asm (Assembly Processing)
**Header:** `asm.h`  
**Function:** `std::string assemble(const std::string& assemblyCode)`

Processes MIPS assembly code (label resolution, instruction encoding).

**Input:** MIPS assembly as string  
**Output:** Processed MIPS code as string  
**Errors:** Returns empty string on error, prints error message to stderr

## Data Flow

```
main.cc orchestrates the pipeline:

1. Read WLP4 source from stdin
2. scanTokens()      → tokens
3. buildParseTree()  → parse tree
4. typeCheck()       → typed tree
5. generateCode()    → assembly
6. assemble()        → final MIPS
7. Write to stdout
```

Each stage can fail independently, with error messages printed to stderr.

## Building and Testing

**Build:**
```bash
make            # Compile all components into 'wlp4' executable
make clean      # Remove build artifacts
```

**Test:**
```bash
./wlp4 < test-c-code/simple.wlp4
```

## Key Design Decisions

1. **String-based interfaces:** Each component communicates via strings for simplicity
2. **Error handling:** Empty string/nullptr return values indicate errors
3. **Memory management:** ParseNode uses manual memory management (destructor cleans up children)
4. **Global state:** Some components (type, gen) use global state for symbol tables - cleared at function start
5. **Parse tree format:** Text-based tree representation for easy debugging and intermediate inspection

## Future Improvements

- Replace string-based communication with structured AST
- Improve error messages with line numbers and context
- Add optimization passes
- Better memory management (smart pointers)
- Parallelize independent compilation units

