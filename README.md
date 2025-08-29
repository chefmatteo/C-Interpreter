# C-Interpreter

> Project: Rewrite the project `C4` with a easier to understand approach;
> [c4](https://github.com/rswier/c4), created by the author rswier;

The purpose of this project is to provide a clear and understandable rewrite of the original `c4` project—a minimal C compiler that became popular for its ability to self-host (compile itself) using only four core functions. While the original `c4` is impressive for its compactness and completeness, its code is intentionally obfuscated and difficult to follow, as it was designed to minimize code size above all else. This project aims to retain the core functionality and educational value of `c4`, but with clean, well-structured, and well-documented code that is accessible to learners and developers interested in compiler construction.

This repository contains two main components:

- Use `fclang` to learn the framework and concepts.
- Use `xclang` to see the working compiler/interpreter in action.

#### There are mainly four aspects we gonna focus on in this project:

#### How to Represent Syntax (BNF, etc.)

- Syntax of programming languages is often described using formal grammars.
- Backus-Naur Form (BNF) is a common notation for expressing context-free grammars.
- Extended BNF (EBNF) adds more expressive power and convenience.

#### Lexical Analysis: Finite and Infinite Automata

- Lexical analysis breaks source code into tokens (identifiers, keywords, symbols, etc.).
- Finite Automata (FA) are used to recognize regular languages, which are suitable for token patterns.
  - Deterministic Finite Automata (DFA) and Nondeterministic Finite Automata (NFA) are two types.
- Infinite automata are not typically used in lexical analysis; regular languages are always recognized by finite automata.

#### Syntax Analysis: Recursive Descent, LL(k), LALR Parsing

- Syntax analysis (parsing) checks if token sequences conform to the grammar.
- Recursive Descent Parsing is a top-down parsing technique, easy to implement for LL grammars.
- LL(k) parsers read input Left-to-right, produce a Leftmost derivation, and use k tokens of lookahead.
- LALR (Look-Ahead LR) parsers are a type of bottom-up parser, commonly used in parser generators (like Yacc/Bison), and can handle a wider range of grammars than LL parsers.
- 語法分析（parsing）就係檢查一串 token 係咪跟 grammar 規則。
- Recursive Descent Parsing 係一種 top-down 解析方法，好易 implement，啱用 LL grammar。
- LL(k) parser 係由左至右讀 input，做左推導（Leftmost derivation），可以睇前面 k 個 token 幫手決定點 parse。
- LALR（Look-Ahead LR）parser 係 bottom-up 解析，通常用 parser generator（好似 Yacc/Bison）整，處理 grammar 範圍比 LL parser 更加廣。

#### Intermediate Code Representation

- Intermediate code is an abstraction between source code and machine code.
- Common forms include three-address code, quadruples, and abstract syntax trees (AST).
- Intermediate code simplifies optimization and code generation.
- 中間碼（Intermediate code）係 source code 同機器碼之間嘅一層抽象。
- 常見有三地址碼（three-address code）、四元式（quadruples）、抽象語法樹（AST）。
- 用中間碼可以方便做優化同生成機器碼。

## Code Generation

- Code generation translates intermediate code into target machine code or assembly.
- Involves instruction selection, register allocation, and handling calling conventions.
- 代碼生成（Code generation）就係將中間碼轉做目標機器碼或者組合語言。
- 包括指令選擇、暫存器分配、同埋處理呼叫規則（calling convention）。

## Code Optimization

- Code optimization improves the efficiency of the generated code.
- Can be performed at various stages: source-level, intermediate code, or machine code.
- Techniques include dead code elimination, constant folding, loop optimization, and inlining.
- 代碼優化（Code optimization）係令生成出嚟嘅代碼跑得快啲或者慳資源。
- 可以喺唔同階段做：source code、intermediate code、machine code。
- 技巧有：死碼消除（dead code elimination）、常數摺疊（constant folding）、迴圈優化（loop optimization）、inline 展開（inlining）。

## A question beforehand: Interpreter vs Compiler:

- **Interpreter:**

  - Executes source code directly, line by line or statement by statement.
  - No separate output file; runs the program immediately.
  - Easier to debug and test code interactively.
  - Slower execution speed compared to compiled code.
  - Examples: Python, Ruby, JavaScript.
- **Compiler:**

  - Translates source code into machine code or intermediate code before execution.
  - Produces an output file (e.g., executable or bytecode) that can be run later.
  - Program runs faster after compilation.
  - Errors are detected before execution, during the compilation step.
  - Examples: C, C++, Rust, Go.

In this project, when we say is a "C language compiler," what we're actually building is a C language interpreter. This means we can run C source code files directly, just like running a script. There are two main reasons for doing this:

1. The only real difference between an interpreter and a compiler is in the code generation stage; other parts like lexical analysis and parsing are the same.
2. Building an interpreter requires us to implement our own virtual machine and instruction set, which helps us better understand how computers work under the hood.

### Practial steps to construct a compiler:

Generally, writing a compiler involves three main steps:

1. Lexical analysis(詞法分析器): converting the input string into an internal representation (tokens).

   - A **token** is a basic unit of meaning identified during lexical analysis. In the context of a compiler, tokens are categories such as keywords (e.g., `int`, `if`), identifiers (variable names), operators (`+`, `-`), literals (numbers, strings), and punctuation (semicolons, parentheses). The lexer scans the input source code and groups characters into these tokens, which are then used by the parser to understand the structure of the code.
2. Syntax analysis(語法分析器): using the stream of tokens from lexical analysis to build a syntax tree.
3. Code generation: transforming the syntax tree into target code.

There are many tools available to help with steps 1 and 2, such as flex for lexical analysis and bison for syntax analysis. However, these tools are very powerful and hide many implementation details, which is not ideal for learning how to build a compiler from scratch. Therefore, we will implement these components by hand.

In this project, we will follow these steps to build our compiler:

1. Build our own virtual machine and instruction set. The target code we generate will be for this instruction set.
2. Implement our own lexical analyzer.
3. Implement our own syntax analyzer.

The framework: 
```c
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;            // current token
char *src, *old_src;  // pointer to source code string;
int poolsize;         // default size of text/data/stack
int line;             // line number

void next() {
    token = *src++;
    return;
}

void expression(int level) {
    // do nothing
}

void program() {
    next();                  // get next token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}


int eval() { // do nothing yet
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);

    program();
    return eval();
}
```
#### Future improvements and references:

- [Let's Build a Compiler](http://compilers.iecc.com/crenshaw/)
- [Lemon Parser Generator](http://www.hwaci.com/sw/lemon/)
