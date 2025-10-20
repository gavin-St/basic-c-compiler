# Basic C Compiler

A compiler for WLP4 (subset of C) that generates MIPS assembly code.

## Architecture

The compiler is organized into modular components with clean interfaces:

```
WLP4 Source Code
      ↓
   [Scan] → Tokens
      ↓
   [Parse] → Parse Tree
      ↓
   [Type] → Typed Parse Tree
      ↓
   [Gen] → MIPS Assembly
      ↓
   [Asm] → Final MIPS Code
```

## Components

- **scan.cc/h**: Lexical analysis (tokenization)
- **parse.cc/h**: Syntax analysis (parsing)
- **type.cc/h**: Semantic analysis (type checking)
- **gen.cc/h**: Code generation (MIPS assembly)
- **asm.cc/h**: Assembler (MIPS code processing)
- **main.cc**: Main compiler driver

## Building

```bash
make clean
make
```

This produces a single `wlp4` executable.

## Usage

```bash
./wlp4 < input.wlp4 > output.mips
```

Or compile from source:

```bash
cat my_program.wlp4 | ./wlp4 > my_program.mips
```

## Example WLP4 Program

```c
int wain(int a, int b) {
    return a + b;
}
```

## Development

Each module exports clean functions:
- `scanTokens(string)` → vector<Token>
- `buildParseTree(tokens)` → ParseNode*
- `typeCheck(parseTree)` → string
- `generateCode(typedTree)` → string  
- `assemble(assembly)` → string

This modular design makes it easy to test individual stages and extend the compiler.

## Clean Up

```bash
make clean
```

Removes all build artifacts.
