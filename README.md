# C_Compiler
Designing a basic C Compiler from scratch

**Order of Compiler:**
- Lexing: Convert input string of C Code into tokens
- Parsing: Convert tokens into an abstract syntax tree, which represents the specific actions that the code will take. Type checking and syntax errors are detected at this step.
- IR Code Generation: Traversing the AST and generating hardware-agnostic Intermediate Representation Code
- x64 Code Generation: Convert IR code to x64 Code. Register allocation is done at this step.

**Optimizations:**
- Constant Folding
- Global Register Allocation
