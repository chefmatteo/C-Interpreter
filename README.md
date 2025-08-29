# C-Interpreter

> Project: Rewrite the project `C4` with a easier to understand approach;
> [c4](https://github.com/rswier/c4), created by the author rswier;

The purpose of this project is to provide a clear and understandable rewrite of the original `c4` project—a minimal C compiler that became popular for its ability to self-host (compile itself) using only four core functions. While the original `c4` is impressive for its compactness and completeness, its code is intentionally obfuscated and difficult to follow, as it was designed to minimize code size above all else. This project aims to retain the core functionality and educational value of `c4`, but with clean, well-structured, and well-documented code that is accessible to learners and developers interested in compiler construction.

## There are mainly four aspects we gonna focus on in this project: 
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
