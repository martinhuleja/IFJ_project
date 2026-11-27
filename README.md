# IFJ26 Scala Compiler

C11 compiler translating IFJ26 (a subset of Scala) into IFJcode26, developed for the [IFJ](https://www.fit.vut.cz/study/course/IFJ/) and [IAL](https://www.fit.vut.cz/study/course/IAL/) courses at FIT BUT (2026/2027).

Uses a finite-state lexer, a recursive-descent parser with bottom-up operator-precedence expression analysis, and an AST for semantic analysis and code generation. The symbol table uses a **hash table with open addressing (TRP-izp)**.

## Build and usage

Requires GCC and Make. Reads source code from standard input and writes IFJcode26 to standard output.

```sh
make
./translator < program.scala > program.ifjcode26
```

Details in [documentation](doc/dokumentace.tex).

## Team members and Division of labor

- Enzo Štětina - Lexical analysis + Unit tests + Documentation
- Martin Huleja - Syntactic analysis + Precedence syntactic analysis
- Richard Komora - Semantic analysis + Hash table
- Adam Havier - Codegen + End-to-end tests
