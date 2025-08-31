# Recursive Descent Parsing

## 1. What is Recursive Descent?

When writing a parser, there are two main approaches:

- **Top-Down Parsing:** Start from the initial non-terminal symbol and repeatedly expand non-terminals until the input terminals are matched.
- **Bottom-Up Parsing:** Start from the input terminals and repeatedly combine them into non-terminals, eventually reducing everything to the start symbol.

The top-down approach most commonly uses **recursive descent**.


## 2. Terminals and Non-terminals

If you haven't studied compiler theory, you might not be familiar with the terms "terminal" and "non-terminal." Here is a brief explanation:

- **Terminal:** A terminal is a basic symbol in the grammar that cannot be broken down further. In the context of our compiler, terminals are the tokens produced by the lexer (such as identifiers, numbers, operators, and keywords). 
    - The null character (`'\0'`), which marks the end of the input, is sometimes treated as a special terminal in parsing to signal the end of the file or input stream. However, it is not a terminal in the grammar of the language itself, but rather an implementation detail to indicate the end of input.
- **Non-terminal:** A symbol that can be expanded into one or more terminals and/or non-terminals according to the grammar rules. Non-terminals represent syntactic structures.


## 3. Introduction to BNF

BNF (Backus-Naur Form) is a formal notation for describing the grammar of a language. For example, the rules for arithmetic expressions can be written in BNF as follows:

In the context of parsing arithmetic expressions, the terms "expression," "term," and "factor" have specific meanings:

- **Expression (`<expr>`)**: An expression is the highest-level construct in arithmetic parsing. It typically represents a value that may involve addition or subtraction of terms. For example, `3 + 4 * 5` is an expression.

- **Term (`<term>`)**: A term is a component of an expression that may involve multiplication or division of factors. In the example `3 + 4 * 5`, `3` is a term, and `4 * 5` is another term.

- **Factor (`<factor>`)**: A factor is the most basic unit in this hierarchy. It can be a number (like `3` or `4`), an identifier, or a parenthesized expression (like `(2 + 5)`). Factors are combined using multiplication or division to form terms.

In short:
- An **expression** is made up of one or more **terms** combined with `+` or `-`.
- A **term** is made up of one or more **factors** combined with `*` or `/`.
- A **factor** is a number, an identifier, or a parenthesized expression.
The arthimetic operation can be expressed: 
```text
<expr> ::= <expr> + <term>
         | <expr> - <term>
         | <term>

<term> ::= <term> * <factor>
         | <term> / <factor>
         | <factor>

<factor> ::= ( <expr> )
         | Num
```
In BNF notation:

- Symbols enclosed in angle brackets `<...>` are called **non-terminals**. These represent syntactic categories that can be further expanded or replaced by the expressions on the right side of the `::=` symbol.
- The vertical bar `|` denotes a choice or alternative. For example, in the rule `<expr> ::= <expr> + <term> | <expr> - <term> | <term>`, the non-terminal `<expr>` can be replaced by any one of the alternatives: `<expr> + <term>`, `<expr> - <term>`, or `<term>`.
- Any symbol that does not appear on the left side of a `::=` rule is called a **terminal**. Terminals are the basic symbols of the language and typically correspond to the tokens produced by the lexical analyzer (such as numbers, operators, or identifiers).




### Recursive Descent Parsing for Arithmetic Expressions
Let's walk through how recursive descent parsing works using the example expression `3 * (4 + 2)`. We assume that the lexer has already tokenized the numbers as `Num`.

#### 1. Parsing Process Overview

- **Starting Point:**  
  Recursive descent parsing begins from a designated non-terminal symbol, typically the start symbol of the grammar. In our arithmetic grammar, this is `<expr>`. If not specified, the first non-terminal in the grammar is usually chosen.

- **Step-by-Step Expansion:**  
  The parsing process involves repeatedly expanding non-terminals according to the grammar rules until only terminals (tokens) remain. Here is how the parser would process `3 * (4 + 2)`:

```text
1. <expr> => <expr>
2.           => <term>        * <factor>
3.              => <factor>     |
4.                 => Num (3)   |
5.                              => ( <expr> )
6.                                   => <expr>           + <term>
7.                                      => <term>          |
8.                                         => <factor>     |
9.                                            => Num (4)   |
10.                                                        => <factor>
11.                                                           => Num (2)
```

- **Observation:**  
  The parser recursively replaces non-terminals with their definitions, moving "downward" in the parse tree until only terminals are left. Notice that non-terminals like `<expr>` are used recursively.

#### 2. Why Use Recursive Descent?

- **Direct Mapping from BNF:**  
  The recursive descent approach closely mirrors the BNF grammar. Each non-terminal in the grammar can be implemented as a function in code. For example, the rule `<expr> ::= <term> + <expr> | <term>` would correspond to a function `expr()` that calls `term()` and handles the `+` operator.

- **Simplicity and Clarity:**  
  The structure of the code directly reflects the grammar, making it easy to understand and maintain.

#### 3. Handling Multiple Choices (Lookahead)

- Recursive descent parsing starts from a designated non-terminal symbol, usually called the "start symbol." In our arithmetic example, this is `<expr>`. 
- In practice, you can specify the start symbol, but if not specified, it's typically the first non-terminal that appears in the grammar.

The parsing process works by repeatedly expanding non-terminals (moving "down" the parse tree) until only terminals (tokens) remain at the "bottom." As you can see from the parsing steps, some non-terminals like `<expr>` are used recursively.

**Why choose recursive descent?**

- As shown in the previous section, the recursive descent parsing process closely matches the BNF grammar.
- More importantly, it allows us to easily convert BNF rules into actual code: for each production (i.e., each rule of the form `NonTerminal ::= ...`), we can write a function with the same name.

- A common question arises: when a non-terminal has multiple choices (alternatives), how does the parser decide which one to use? 
- For example, why choose `<expr> ::= <term> * <factor>` instead of `<expr> ::= <term> / <factor>`? This is where the concept of "lookahead k" (looking ahead k tokens) comes in, as discussed in the previous chapter. By looking ahead at the next token (for example, seeing a `*`), the parser can determine which production to apply.

#### Left Recursion and Recursive Descent

- Although we don't usually go into this much detail, it's important to note that the arithmetic grammar we discussed above is actually **left-recursive**. Left-recursive grammars cannot be directly implemented using recursive descent parsing. Therefore, before we can write a recursive descent parser, we need to **eliminate left recursion**.

After eliminating left recursion, the grammar for arithmetic expressions becomes:

```text
<expr> ::= <term> <expr_tail>
<expr_tail> ::= + <term> <expr_tail>
              | - <term> <expr_tail>
              | <empty>

<term> ::= <factor> <term_tail>
<term_tail> ::= * <factor> <term_tail>
              | / <factor> <term_tail>
              | <empty>

<factor> ::= ( <expr> )
              | Num
```
#### Implmentation of recursive descent: 
```c
int expr();



int factor() {
    int value = 0;
    if (token == '(') {
         match('(');    // This line checks that the current token is a left parenthesis '(' and advances to the next token. 
         value = expr(); // This line recursively parses the expression inside the parentheses and stores its value.
         match(')');    // This line checks that the next token is a right parenthesis ')' and advances to the next token.
    } else {
        value = token_val;
        match(Num);
    }
    return value;
}

int term_tail(int lvalue) {
    if (token == '*') {
        match('*');
        int value = lvalue * factor();
        return term_tail(value);
    } else if (token == '/') {
        match('/');
        int value = lvalue / factor();
        return term_tail(value);
    } else {
        return lvalue;
    }
}

int term() {
    int lvalue = factor();
    return term_tail(lvalue);
}

int expr_tail(int lvalue) {
    if (token == '+') {
        match('+');
        int value = lvalue + term();
        return expr_tail(value);
    } else if (token == '-') {
        match('-');
        int value = lvalue - term();
        return expr_tail(value);
    } else {
        return lvalue;
    }
}

int expr() {
    int lvalue = term();
    return expr_tail(lvalue);
}
```
Here's what each function does:

- `factor()`: Parses and evaluates a "factor", which is either a parenthesized expression (e.g., `(2+3)`) or a number. If the current token is `'('`, it recursively parses the expression inside the parentheses. Otherwise, it expects a number token.

- `term_tail(int lvalue)`: Handles the tail of a "term", recursively processing any sequence of `*` or `/` operations. It takes the left value (`lvalue`), and if the next token is `*` or `/`, it matches the operator, parses the next factor, applies the operation, and recurses. If there are no more `*` or `/`, it returns the accumulated value.

- `term()`: Parses and evaluates a "term", which is a factor possibly followed by a sequence of `*` or `/` operations. It starts by parsing a factor, then processes any following `*` or `/` using `term_tail`.

- `expr_tail(int lvalue)`: Handles the tail of an "expression", recursively processing any sequence of `+` or `-` operations. It works similarly to `term_tail`, but for addition and subtraction.

- `expr()`: Parses and evaluates an "expression", which is a term possibly followed by a sequence of `+` or `-` operations. It starts by parsing a term, then processes any following `+` or `-` using `expr_tail`.

Overall, this code parses and evaluates arithmetic expressions according to the following grammar:
```text
    expr   ::= term { ('+' | '-') term }
    term   ::= factor { ('*' | '/') factor }
    factor ::= '(' expr ')' | Num
```
It uses recursive descent parsing, where each non-terminal in the grammar corresponds to a function. The parser also evaluates the expression as it parses, returning the computed value.

#### Integration with what we have: 
```c
##include <stdio.h>
##include <stdlib.h>

enum {Num};
int token;
int token_val;
char *line = NULL;
char *src = NULL;

void next() {
    // skip white space
    while (*src == ' ' || *src == '\t') {
        src ++;
    }

    token = *src++;

    if (token >= '0' && token <= '9' ) {
        token_val = token - '0';
        token = Num;

        while (*src >= '0' && *src <= '9') {
            token_val = token_val*10 + *src - '0';
            src ++;
        }
        return;
    }
}

void match(int tk) {
    if (token != tk) {
        printf("expected token: %d(%c), got: %d(%c)\n", tk, tk, token, token);
        exit(-1);
    }
    next();
}


int main(int argc, char *argv[])
{


    size_t linecap = 0;
    ssize_t linelen;
    //These variables are used for reading lines from standard input using getline.
    // 'linecap' holds the current allocated size of the buffer 'line'.
    // 'linelen' will store the length of each line read.
    while ((linelen = getline(&line, &linecap, stdin)) > 0) {
        src = line;
     // - 'getline' reads a line from stdin, stores it in 'line', and returns the number of characters read.
    // - If a line is read (linelen > 0), the code sets 'src' to point to the start of the line.
        next();
        printf("%d\n", expr());
    }
    return 0;
}
```