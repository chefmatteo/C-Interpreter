# Lexical Analysis: 
## Waht is it? 
- In short, a lexical analyzer (lexer) preprocesses the source code string to simplify the job of the parser.

- The lexer takes the source code as input and produces a stream of tokens as output. Each token typically consists of a type and a value, for example: (token, token value). For instance, if the source code contains the number '998', the lexer will output (Number, 998). Here are more examples:
```c
2 + 3 * (4 - 5)
=>
(Number, 2) Add (Number, 3) Multiply Left-Bracket (Number, 4) Subtract (Number, 5) Right-Bracket
```
- The idea here is to provide a hint for the future work to notice that the string passed is of a certain type (number / operation etc). 
- In COMP2011 of HKUST, we have accomplished similar task of using recursion to fininsh the parsing of the code, yet it is highly troublesome and chaotic when it comes to edge cases or case like multiple `((()()()))`. 
- By preprocessing with the lexer, the complexity of the parser is greatly reduced. You will see this clearly when we discuss the parser later.

## Lexical Analyzer and Compiler
- If you look deeper into the role of a lexical analyzer (lexer), you'll notice that it essentially acts as a simple compiler. While a typical compiler takes a stream of tokens as input and produces assembly code as output, the lexer takes the raw source code string as input and produces a stream of tokens.
```text

                   +-------+                      +--------+
   source code --> | lexer | --> token stream --> | parser | --> assembly
                   +-------+                      +--------+
```

- With this in mind, it's clear why we don't compile directly from source code to assembly: processing raw strings is complex and error-prone.
- By first building a simpler "compiler" (the lexer) to convert the source code into a token stream, we make the parser's job much easier. The token stream is far more manageable for the parser than the original source code.

## Implementation of a Lexical Analyzer
- Lexical analysis is a common but tedious and error-prone task, so there are many tools available to help generate lexical analyzers, such as `lex` and `flex`. These tools allow us to define patterns for tokens using regular expressions.

It's important to note that in practice, we do not convert the entire source code into a complete token stream all at once. There are two main reasons for this:

1. Tokenization can be stateful; the way a string is tokenized may depend on the surrounding context in the code.
    - This means that the process of breaking the source code into tokens may depend on what has already been seen or what is expected next. 
    - For example, in some programming languages, the meaning of a sequence of characters can change depending on whether you are inside a string, a comment, or regular code.
    - The lexer may need to keep track of its current state (such as "inside a string" or "outside a string") to correctly identify tokens. Therefore, tokenization is not always a simple, context-free process; it can require remembering previous input or the current parsing mode.
2. Storing all tokens at once is unnecessary and wastes memory.

Instead, the typical approach is to provide a function (Called `next()` in our case, as mentioned earlier) that returns the next token each time it is called. This way, tokens are generated on demand as the parser needs them.

### Supported notation: 
- Global definition: 
```c
// tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
```
- The above enum objects are the operation that the compiler will support. For instance, we will assign `=` as `Assign`; `==` as `Equivalnce`; `!=` as `Not Eq`. 
- It's important to recognize that a token can often consist of multiple characters (for example, `==`, `!=`, etc.). The main reason a lexical analyzer simplifies parsing is that it encodes the raw source string into a more meaningful sequence of tokens, effectively "compressing" the code into a more manageable form for the parser.
- The order of these tokens is not arbitrary; it is determined by their precedence in the C language. For instance, the multiplication operator `*` (represented as `Mul`) has higher precedence than the addition operator `+` (represented as `Add`). The significance of this ordering will become clearer when we discuss parsing in more detail later.

- Additionally, there are some characters that form tokens by themselves, such as the right bracket `]` or the tilde `~`. We do not need to handle these separately for two main reasons:
    1. They are single-character tokens, unlike multi-character tokens such as `==`.
    2. They do not participate in operator precedence relationships.
    
> The lexical analyzer identifies both multi-character and single-character tokens, and by respecting operator precedence, it lays the groundwork for a much simpler and more robust parsing process.


## Architecture of the lexical analyzer
aka the main body of the next() function:
```c
void next(){
    char * last_pos; 
    int hash; 

    while(token = *src){
        ++src; //move to the next character; 
        //parse token here; 
    }
    return; 
}
```
### **An interesting question raised here, why do we need to use a while loop here?**
- At first glance, it seems quite reasonable to use something like: 
```c

while (token = *src) {
    // process token
}

// vs

for (; (token = *src); ) {
    // process token
}

```
- They are functionally equivalent in this context.
- The while loop is preferred because:
    - It clearly expresses that we are looping as long as there are characters to process.
    - There is no need for explicit initialization or increment expressions in the loop header.
    - All logic for advancing the pointer and handling tokens is inside the loop body.
    - It is simpler and more idiomatic in C for this pattern.
Another reason is: 
- And we once said a lexical analyzer is a kind of compiler: **How should errors be handled?** and **How should a lexical analyzer handle unrecognized characters?** There are generally two approaches:

1. Report the location of the error and terminate the program.
2. Report the location of the error, skip the problematic character, and continue compiling.

- In our implementation, the purpose of the `while` loop is to skip over any characters that the analyzer does not recognize. This includes whitespace characters. 
- In C, spaces and other whitespace are used as separators and are not part of the language's syntax. Therefore, we treat them as "unrecognized" characters and use the loop to skip them during lexical analysis.

#### Change line: 
- Newlines are similar to spaces, but with one important difference: each time we encounter a newline character, we need to increment the current line number.
```c
//we initialize the line variable earlier" 
if (token == '\n') {
    ++line;
}
```

#### Global definition: 
- C language use the character `#` for gloabl definition, like `#include <stdio.h>`, we compiler doesnt support global definition, so we can skip that part directly. 


### Identifiers and the Symbol Table

> Why do we need a symbol table for identifiers?

- **Identifiers** are names for variables, functions, etc. in a program.
- For syntax analysis, we do not care about the actual name of an identifier, but rather about the unique entity it represents.  
  For example:
    ```c
    int a;
    a = 10;
    ```
    Here, both occurrences of `a` must refer to the same variable.

#### How does the lexical analyzer handle identifiers?

- The lexical analyzer maintains a **symbol table** to store all identifiers it encounters.
- When a new identifier is found:
    1. It checks the symbol table to see if the identifier already exists.
    2. If it exists, it returns its unique identifier (such as an index or pointer).
    3. If it does not exist, it adds the identifier to the table and assigns it a unique identifier.

#### How are identifiers represented?

- Identifiers are usually stored as strings in the symbol table.
- Each identifier has a unique entry in the table, which can be referenced during later stages of compilation.

> How do we recognzie the lexcial symbol?
```c
struct identifier{
    int token;
    int hash;
    char * name;
    int class;
    int type;
    int value;
    int Bclass;
    int Btype;
    int Bvalue;
}

```
#### Explanation of Each Field in the Identifier Structure
- **token**  
  The lexical token returned for this identifier. In theory, all variables return the token `Id`, but since we also add keywords (such as `if`, `while`, etc.) to the symbol table, they each have their own specific token.
- **hash**  
  The hash value of the identifier, used for fast comparison and lookup.
- **name**  
  The string representing the identifier itself.
- **class**  
  The category of the identifier, such as number, global variable, or local variable.
- **type**  
  The type of the identifier. If it is a variable, this indicates whether it is of type `int`, `char`, or a pointer.
- **value**  
  The value associated with the identifier. For example, if the identifier is a function, this stores the function's address.
- **Bclass, Btype, Bvalue**  
  In C, identifiers can be global or local. When a local identifier has the same name as a global one, these fields are used to temporarily store the global identifier's information.

> Note:  
> Our lexical analyzer differs from a traditional one. A traditional symbol table only needs to record a unique identifier for each symbol, but our implementation also stores information (such as `type`) that is typically only needed by the syntax analyzer. This is to support our goal of self-hosting and to accommodate our grammar, which does not support `struct`. Therefore, we use separate fields instead of a C `struct` for symbol table entries.

### Differences from Traditional Lexical Analyzers
- Our lexical analyzer differs from traditional ones in several key ways:
- **Traditional symbol tables** only need to record a unique identifier for each symbol (such as a variable or function name).
- **Our implementation** stores additional information—such as `type`, that is typically only required by the syntax analyzer. This is because our design aims for self-hosting and our grammar does not support C's `struct` feature.

### Symbol Table Structure

To accommodate these requirements, we use a flat array to represent each identifier entry, rather than a C `struct`. Each entry in the symbol table contains the following fields:

| token | hash | name | type | class | value | btype | bclass | bvalue | ... |
|-------|------|------|------|-------|-------|-------|--------|--------|-----|
|<------------------- one single identifier entry -------------------------->|

- Each row represents a single identifier and its associated information.
- This structure allows us to store all necessary data for both lexical and syntactic analysis, even without `struct` support in our language.

- We use an integer array to store information about each identifier. Each identifier uses 9 consecutive slots in the array. The following code shows how identifiers are processed:

```c
int token_val;                // value of current token (mainly for number)
int *current_id,              // current parsed ID
    *symbols;                 // symbol table

// fields of identifier
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};


void next() {
        ...

        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {

            // parse identifier
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }

            // look for existing identifier, linear search
            current_id = symbols;
            while (current_id[Token]) {
                // This line checks if the current symbol table entry matches the identifier being parsed.
                // It first compares the stored hash value (current_id[Hash]) with the computed hash for the new identifier.
                // If the hashes match, it then compares the actual identifier strings using memcmp:
                //   - (char *)current_id[Name] is the stored name pointer for the symbol table entry.
                //   - last_pos is the start of the identifier in the source code.
                //   - src - last_pos is the length of the identifier.
                // If both the hash and the string match, this means the identifier already exists in the symbol table.
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    //found one, return
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }


            // store new ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        ...
}
```
- The above code performs the following tasks:
  - Detects when the current character is the start of an identifier (a letter or underscore).
  - Collects all subsequent valid identifier characters (letters, digits, or underscores) to form the full identifier.
  - Computes a hash value for the identifier as it is parsed.
  - Searches the symbol table (an integer array) to check if the identifier already exists:
    - Compares both the hash and the actual string to find a match.
    - If found, sets the current token to the existing identifier and returns.
  - If the identifier is new:
    - Stores its name and hash in the symbol table.
    - Sets its token type to `Id`.
    - Updates the current token to this new identifier.
  - This mechanism ensures that each identifier is uniquely tracked and can be efficiently referenced during later stages of compilation.


#### Numbers
When parsing numbers, we need to support decimal, hexadecimal, and octal formats. The logic is mostly straightforward, but the hexadecimal conversion can be a bit tricky.

For hexadecimal digits, the value is calculated like this:
token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);

Here's how it works:
- (token & 15) extracts the numeric value of the character (for '0'-'9', 'A'-'F', 'a'-'f').

**Bitwise AND Operator (Binary):**
When `&` used between two operands (e.g., operand1 & operand2), it performs a bitwise AND operation on their binary representations.
For each corresponding bit position in the two operands, the resulting bit is 1 only if both bits in the operands are 1; otherwise, it's 0.

**Example: 5 & 3 (binary 0101 & 0011) results in 0001 (decimal 1).**

- (token >= 'A' ? 9 : 0) adds 9 if the character is a letter (A-F or a-f), to adjust the value to the correct range.
- This formula works because, in ASCII, the codes for 'a' and 'A' are higher than for '0', and the bitwise operation helps extract the right value for each character.

In such case the code can convert both uppercase and lowercase hexadecimal digits to their numeric values.

The ASCII value of 'a' is 0x61 (97 in decimal), and 'A' is 0x41 (65 in decimal).
For hexadecimal parsing, we want to convert characters '0'-'9', 'A'-'F', and 'a'-'f' to their numeric values (0-15).
The expression (token & 15) works as follows:

Let's break down how (token & 0x0F) works for different ASCII characters:

- For '0' (ASCII 0x30): 0x30 in binary is 0011 0000. 0x0F is 0000 1111.
  0011 0000 & 0000 1111 = 0000 0000 (decimal 0)
- For '9' (ASCII 0x39): 0x39 is 0011 1001.
  0011 1001 & 0000 1111 = 0000 1001 (decimal 9)

For letters used in hexadecimal digits:

- For 'A' (ASCII 0x41): 0x41 is 0100 0001.
  0100 0001 & 0000 1111 = 0000 0001 (decimal 1)
- For 'F' (ASCII 0x46): 0x46 is 0100 0110.
  0100 0110 & 0000 1111 = 0000 0110 (decimal 6)
- For 'a' (ASCII 0x61): 0x61 is 0110 0001.
  0110 0001 & 0000 1111 = 0000 0001 (decimal 1)
- For 'f' (ASCII 0x66): 0x66 is 0110 0110.
  0110 0110 & 0000 1111 = 0000 0110 (decimal 6)

Notice that for digits '0'-'9', this gives the correct value directly (0-9).
For letters 'A'-'F' and 'a'-'f', it gives values 1-6, but in hexadecimal,
'A'/'a' should represent 10, 'B'/'b' is 11, ..., 'F'/'f' is 15.
To correct this, the code adds 9 if the character is a letter (token >= 'A'),
so (token & 0x0F) + 9 gives the correct hex value for letters.
For example:
  'A': (0x41 & 0x0F) = 1, 1 + 9 = 10
  'F': (0x46 & 0x0F) = 6, 6 + 9 = 15
  'a': (0x61 & 0x0F) = 1, 1 + 9 = 10
  'f': (0x66 & 0x0F) = 6, 6 + 9 = 15

This bitwise trick works because the lower 4 bits of the ASCII codes for
'A'-'F' and 'a'-'f' are the same as their hex values minus 9.
So, the formula (token & 0x0F) + (token >= 'A' ? 9 : 0) efficiently
converts any valid hex digit character to its numeric value.

This gives us the correct value for digits, but for letters ('A'-'F', 'a'-'f'), we need to add 9 to get the correct hex value:
  'A'/'a' should be 10, 'B'/'b' should be 11, ..., 'F'/'f' should be 15.
That's why the code uses: (token & 15) + (token >= 'A' ? 9 : 0)
- For 'A' (0x41): (0x41 & 0x0F) = 1, plus 9 = 10
- For 'B' (0x42): (0x42 & 0x0F) = 2, plus 9 = 11
- For 'F' (0x46): (0x46 & 0x0F) = 6, plus 9 = 15
- For 'a' (0x61): (0x61 & 0x0F) = 1, plus 9 = 10
- For 'f' (0x66): (0x66 & 0x0F) = 6, plus 9 = 15

This trick works because the lower 4 bits of the ASCII codes for 'A'-'F' and 'a'-'f' are the same as their hex values minus 9.
So, (token & 15) gives the base value, and adding 9 for letters gives the correct hex digit value.

## String and Character Literal Lexical Analysis
### Handling Strings
- When the lexer encounters a string (e.g., `"hello"`), it stores the string content in the `data` segment (as described in previous sections).
- The lexer then returns the address of the string in the `data` segment as the token value.
- This allows later stages (parser/codegen) to refer to string constants by their address.

### Escape Sequence Support

- Strings and character literals often contain escape sequences, such as `\n` for newline.
- In this compiler, only `\n` (newline) and `\"` (double quote) are supported as escape sequences, which is sufficient for self-hosting.
- Other escape sequences (like `\t`, `\r`, etc.) are not implemented, but the syntax allows for things like `\a` (which just means the character `a`) and `\"` (which means a literal `"` in the string).

### Character vs. String Literals

- The lexer handles both character literals (e.g., `'a'`) and string literals (e.g., `"a string"`).
- For a character literal, the lexer returns a token of type `Num` with the character's numeric value.
- For a string literal, the lexer returns the address in the `data` segment where the string is stored.

### Implementation: 
```c
void next() {
        ...

        else if (token == '"' || token == '\'') {
            // parse string literal, currently, the only supported escape
            // character is '\n', store the string literal into data.
            last_pos = data;
            
            // The second condition (*src != token) is necessary so that we stop reading characters when we reach the
            // matching closing quote for the string or character literal. Without this, the loop would keep reading
            // past the end of the literal, potentially consuming code that is not part of the string/char.
            while (*src != 0 && *src != token) {
                token_val = *src++;

                // If the character is a backslash, handle escape sequences.
                if (token_val == '\\') {
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                    // Other escape sequences (like \") are not specially handled here,
                    // so \" will just become a literal double quote.
                }

                // If we are parsing a string literal (i.e., token == '"'),
                // store the character (possibly after escape processing) in the data segment.
                if (token == '"') {
                    *data++ = token_val;
                }
            }

            src++;
            // if it is a single character, return Num token
            if (token == '"') {
                token_val = (int)last_pos;
            } else {
                token = Num;
            }

            return;
        }
}
```
> Notice: We do not support paragraph comment, we only support one line comment: 
- That is: `//` is acceptable but not `/*   Commetns  */`: 
```c
void next() {
        ...

        else if (token == '/') {
            if (*src == '/') {
                // skip comments
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                // divide operator
                token = Div;
                return;
            }
        }

        ...
}
```

### The Concept of Lookahead
Sometimes, a single character is not enough to determine which token should be produced, because multiple tokens may start with the same character. For example, in our code, the `/` character could indicate either the division operator (`Div`) or the start of a comment (`//`). In such cases, the lexer needs to "look ahead" by examining one or more additional characters to make the correct decision.

#### How Lookahead Works

- **Lookahead** means peeking ahead at upcoming characters (or tokens) before deciding how to tokenize the current input.
- In the example above, after encountering a `/`, the lexer checks the next character:
    - If it is another `/`, it recognizes the start of a comment and skips the rest of the line.
    - Otherwise, it treats the `/` as the division operator.

#### Lookahead in Parsing
- The concept of lookahead is not limited to lexical analysis. In parsing, lookahead refers to peeking ahead at upcoming tokens (not just characters) to decide how to parse the input.
- For example, in LL(k) parsers, the `k` indicates how many tokens the parser looks ahead to make parsing decisions.
- "LL" refers to a class of top-down parsers used in compiler design. The first "L" stands for scanning the input from **Left** to right, and the second "L" stands for producing a **Leftmost** derivation of the sentence. In an LL(k) parser, the `k` indicates how many tokens the parser looks ahead to make parsing decisions. For example, an LL(1) parser looks ahead one token at a time.

#### Why Lexical Analysis Reduces Lookahead Complexity

- By converting the raw source code into a stream of tokens, the lexer reduces the complexity of the parser.
- One of the main benefits is that the parser usually needs to look ahead fewer tokens (compared to characters) to make correct decisions, making the parsing process simpler and more robust.

Thats all for the more complicated systems and maping, the rest is just ordinary context: 

```c
void next() {
        ...

        else if (token == '=') {
            // parse '==' and '='
            if (*src == '=') {
                src ++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if (token == '+') {
            // parse '+' and '++'
            if (*src == '+') {
                src ++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if (token == '-') {
            // parse '-' and '--'
            if (*src == '-') {
                src ++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        }
        else if (token == '!') {
            // parse '!='
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') {
            // parse '<=', '<<' or '<'
            if (*src == '=') {
                src ++;
                token = Le;
            } else if (*src == '<') {
                src ++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        }
        else if (token == '>') {
            // parse '>=', '>>' or '>'
            if (*src == '=') {
                src ++;
                token = Ge;
            } else if (*src == '>') {
                src ++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        }
        else if (token == '|') {
            // parse '|' or '||'
            if (*src == '|') {
                src ++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if (token == '&') {
            // parse '&' and '&&'
            if (*src == '&') {
                src ++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        }
        else if (token == '^') {
            token = Xor;
            return;
        }
        else if (token == '%') {
            token = Mod;
            return;
        }
        else if (token == '*') {
            token = Mul;
            return;
        }
        else if (token == '[') {
            token = Brak;
            return;
        }
        else if (token == '?') {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            // directly return the character as token;
            return;
        }

        ...
}
```


### Keywords and Built-in Functions
After implementing the lexical analyzer, there is an important issue to address: **keywords** (such as `if`, `while`, `return`, etc.). These cannot be treated as ordinary identifiers because they have special meanings in the language.

There are generally two approaches to handling keywords:

1. **Recognize keywords directly in the lexical analyzer:**  
   The lexer checks if an identifier matches any reserved keyword and assigns it a special token.

2. **Pre-populate the symbol table with keywords before parsing:**  
   The symbol table is initialized with all keywords, each assigned the appropriate token and metadata. When the lexer encounters a keyword in the source code, it is recognized as an identifier, but the symbol table entry reveals that it is actually a keyword.

In our implementation, we use the **second approach**. We add all keywords to the symbol table before parsing begins, assigning them the correct token values (recall the `Token` field in the identifier structure). This way, when the lexer encounters a keyword, it is processed as an identifier, but the symbol table tells us it is a special keyword.

**Built-in functions** (such as `open`, `read`, `printf`, etc.) are handled similarly to keywords. The only difference is the specific information assigned to their symbol table entries. Both keywords and built-in functions are initialized in the `main` function before parsing starts.

```c
// types of variable/function
enum { CHAR, INT, PTR };
int *idmain;                  // the `main` function

void main() {
    ...

    src = "char else enum if int return sizeof while "
          "open read close printf malloc memset memcmp exit void main";

     // add keywords to symbol table
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // add library to symbol table

    // Here, i is assigned the value OPEN, which is an integer constant (likely an enum value for a built-in function token).
    i = OPEN;
    
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next(); current_id[Token] = Char; // handle void type
    next(); idmain = current_id; // keep track of main

    ...
    program();
}
```
In this chapter, we built a lexical analyzer for our compiler. Here are some key points to highlight:

- The main purpose of the lexical analyzer is to preprocess the source code string, making the job of the parser much simpler.
- The lexical analyzer itself can be considered a kind of compiler: it takes source code as input and produces a stream of tokens as output.
- The concept of `lookahead(k)` is important—it refers to looking ahead k characters or tokens to make decisions during analysis.
- We also discussed how to handle identifiers and the symbol table during lexical analysis.

