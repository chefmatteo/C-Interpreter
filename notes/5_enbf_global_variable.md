# Global Variables:
EBNF Representation

EBNF is an extension of the BNF introduced in the previous chapter. Its syntax is easier to understand and more intuitive to implement. However, it can still look a bit messy—feel free to skip this section if you find it tedious.
```text
program ::= {global_declaration}+

global_declaration ::= enum_decl | variable_decl | function_decl

enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'

variable_decl ::= type {'*'} id { ',' {'*'} id } ';'

function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'

parameter_decl ::= type {'*'} id {',' type {'*'} id}

body_decl ::= {variable_decl}, {statement}

statement ::= non_empty_statement | empty_statement

non_empty_statement ::= if_statement | while_statement | '{' statement '}'
                     | 'return' expression | expression ';'

if_statement ::= 'if' '(' expression ')' statement ['else' non_empty_statement]

while_statement ::= 'while' '(' expression ')' non_empty_statement
```
Our language does not support cross-function recursion, and to make the compiler self-hosting, we actually can't use recursion at all (which is ironic, given we've just spent a whole chapter on recursive descent parsing).

## Analysis of definition for variable:
- `num_declar` and `variable_declar`
```c
void program(){
    next(); 
    while(token > 0){
        global_declaration(); 
    }
}
```
#### global_declaration()
This represents a global declaration statement, which includes variable definitions, type definitions (only enums are supported), and function definitions. The code is as follows:

```c
int basetype;    // the type of a declaration, make it global for convenience
int expr_type;   // the type of an expression

void global_declaration() {
    // global_declaration ::= enum_decl | variable_decl | function_decl
    //
    // enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'} '}'
    //
    // variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
    //
    // function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'


    int type; // tmp, actual type for variable
    int i; // tmp

    basetype = INT; 
    

    // parse enum, this should be treated alone.
    if (token == Enum) {
        // enum [id] { a = 10, b = 20, ... }
        match(Enum);
        if (token != '{') {
            match(Id); // skip the [id] part coz this is invalid 
        }
        if (token == '{') {
            // parse the assign part
            match('{');
            enum_declaration();
            match('}');
        }

        match(';');
        return;
    }

    // parse type information
    if (token == Int) {
        match(Int);
    }
    else if (token == Char) {
        match(Char);
        basetype = CHAR;
    }

    // parse the comma seperated variable declaration.
    while (token != ';' && token != '}') {
        type = basetype;
        // parse pointer type, note that there may exist `int ****x;`
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        if (token != Id) {
            // invalid declaration
            printf("%d: bad global declaration\n", line);
            exit(-1);
        }
        if (current_id[Class]) {
            // identifier exists
            printf("%d: duplicate global declaration\n", line);
            exit(-1);
        }
        match(Id);
        current_id[Type] = type;

        if (token == '(') {
            current_id[Class] = Fun;
            current_id[Value] = (int)(text + 1); // the memory address of function
            function_declaration();
        } else {
            // variable declaration
            current_id[Class] = Glo; // global variable
            current_id[Value] = (int)data; // assign memory address
            data = data + sizeof(int);
        }

        if (token == ',') {
            match(',');
        }
    }
    next();
}
```
#### Walkthrough: 
This code segment is responsible for parsing global variable and function declarations in a C-like language. Here’s how you would implement it, step by step:

> Note
The `match` function is used to verify that the current token matches the expected token in the parsing process. 
If the current token is as expected, `match` advances to the next token (typically by calling the lexer or tokenizer).
If the current token does not match the expected token, `match` usually reports a syntax error and may terminate parsing or handle the error appropriately.

> For example, `match(Int);` checks that the current token is the 'int' keyword, and then advances to the next token.


1. **Parse Type Information**
   - Start by checking if the current token is a type keyword (`int` or `char`).
   - Set the `basetype` accordingly. This will be used for all variables/functions in this declaration statement.


For the part: 
```c
    if (token == Enum) {
        // enum [id] { a = 10, b = 20, ... }
        match(Enum);
        if (token != '{') {
            match(Id); // skip the [id] part
        }
        if (token == '{') {
            // parse the assign part
            match('{');
            enum_declaration();
            match('}');
        }

        match(';');
        return;
    }
```

handles the parsing of enum declarations in the global scope. 
- When the current token is Enum (i.e., the parser sees the 'enum' keyword), it enters this block.
- It optionally matches an identifier after 'enum' (the enum's name).
- If a '{' follows, it parses the enum body (the list of named constants) by calling enum_declaration().
- After the enum body, it expects a closing '}' and a terminating ';'.
- The return ensures that after handling an enum, the parser does not continue with the rest of the global declaration logic.


In summary: 
- `token == Enum` checks if the current token is the 'enum' keyword, indicating the start of an enum declaration.
- The block then parses the enum definition and skips the rest of the global declaration logic for this statement.
*/

2. **Parse Comma-Separated Declarations**
   - Use a loop to handle multiple declarations in a single statement (e.g., `int a, *b, c;`).
   - For each identifier:
     - **Parse Pointer Type:**  
       - If there are one or more `*` tokens, increment the type to indicate pointer level (e.g., `int **x;`).
     - **Check for Identifier:**  
       - Ensure the next token is an identifier. If not, report an error.
     - **Check for Duplicates:**  
       - If the identifier is already declared (i.e., `current_id[Class]` is set), report a duplicate declaration error.
     - **Match Identifier:**  
       - Consume the identifier token and set its type.


```c
 if (token == '(') {
            // Mark the current identifier as a function
            current_id[Class] = Fun;
            // Store the address of the function's code (text + 1) in the Value field
            current_id[Value] = (int)(text + 1); 
            // the memory address of function
            // Parse the function's parameters and body
            function_declaration();
        } else {
            // Otherwise, this is a variable declaration
            // Mark the current identifier as a global variable
            current_id[Class] = Glo; // global variable
            // Assign the current data segment address to the variable
            current_id[Value] = (int)data; // assign memory address
            // Increment the data pointer to reserve space for the variable
            data = data + sizeof(int);
        }
```
3. **Function or Variable?**
   - If the next token is `(`, this is a function declaration:
     - Set the class to `Fun` and store the function’s address.
     - Call `function_declaration()` to parse the function’s parameters and body.
   - Otherwise, it’s a global variable:
     - Set the class to `Glo` (global variable).
     - Assign a memory address from the `data` segment.
     - Increment the `data` pointer to reserve space for the variable.

4. **Handle Multiple Declarations**
   - If the next token is a comma, consume it and continue parsing the next declaration in the same statement.

**Function of Each Segment:**
- Type parsing: Determines the base type for the declaration.
- Pointer parsing: Handles pointer levels for each variable.
- Identifier and duplicate check: Ensures valid and unique names.
- Function/variable distinction: Differentiates between function and variable declarations.
- Memory assignment: Allocates space for global variables.
- Loop and comma handling: Supports multiple declarations in one statement.
- Token advancement: Prepares for the next parsing step.

> Lookahead Tokens:  
The use of `if (token == xxx)` statements is to look ahead at the next token in order to decide which grammar rule (production) to apply. For example, if we encounter the token `enum`, we know we need to parse an enumeration type. However, if we only see a type like `int identifier`, we cannot yet determine whether `identifier` is a variable or a function. Therefore, we need to continue checking the following tokens. If the next token is `(`, we can conclude it is a function declaration; otherwise, it is a variable declaration.

> Representation of Variable Types:  
Our compiler supports pointer types, which means it also supports multiple levels of pointers, such as `int **data;`. How do we represent pointer types? Previously, we defined the supported types as:

```c
//types of variable/ function: 
enum{CHAR, INT, PTR}
```
A type in this compiler always starts with a base type, such as CHAR or INT. When the type is a pointer to a base type (for example, `int *data`), we represent this by adding PTR to the type: `type = type + PTR;`. Similarly, for a pointer to a pointer, we add PTR again.

#### enum_declaration
- When the function receive the keyword in the token `ENUM`, it observes the string literals behind. 
- Responsible for parsing enum type definitions. Its main logic is to handle variables separated by commas (`,`). 
- An important detail is how the compiler stores information about enum variables.
- For enum variables, we set their class to Num, which means they are treated as global constants. In contrast, regular global variables have their class set to Glo. This class information is important and will be used later when parsing expressions.

```c
void enum_declaration() {
    // parse enum [id] { a = 1, b = 3, ...}
    int i;
    i = 0;
    while (token != '}') {

        // Check if the current token is an identifier (Id).
        // In an enum declaration, each member must have a valid name.
        // If the token is not an identifier, print an error message and exit.
        if (token != Id) {
            printf("%d: bad enum identifier %d\n", line, token);
            exit(-1);
        }
        next();
        if (token == Assign) {
            // assign: `==`
            // like {a=10}
            next(); //which means is valid
            if (token != Num) {
                printf("%d: bad enum initializer\n", line);
                //next the token is assigned with a certain number;
                exit(-1);
            }
            i = token_val; //we manually assign the value with the value editor want; 
            next();
        }

        // Imagine you have an enum like:
        //   enum Color { RED = 0, GREEN = 1, BLUE = 2 };
        //
        // For each member, we store its information in the symbol table:
        //   current_id[Class] = Num;   // This marks 'RED', 'GREEN', 'BLUE' as constants.
        //   current_id[Type]  = INT;   // All enum values are of integer type.
        //   current_id[Value] = i++;   // Assigns 0 to RED, 1 to GREEN, 2 to BLUE.
        //
        // For example, after parsing 'RED', the symbol table entry looks like:
        //   current_id[Class] = Num;   // constant
        //   current_id[Type]  = INT;   // int
        //   current_id[Value] = 0;     // value assigned to RED
        //
        // After 'GREEN':
        //   current_id[Class] = Num;
        //   current_id[Type]  = INT;
        //   current_id[Value] = 1;
        //
        
        current_id[Class] = Num;
        current_id[Type] = INT;
        current_id[Value] = i++;

        if (token == ',') {
            next();
        }
    }
}
```
#### Miscellaneous: 
```c
void match(int tk) {
    if (token == tk) {
        next();
    } else {
        printf("%d: expected token: %d\n", line, tk);
        exit(-1);
    }
}
```
- It wraps the 'next' function to advance the token stream, but also checks if the current token matches the expected one. 
- If the token does not match the expected value, it prints an error message with the line number and exits the program.
- In other words, 'match' ensures that the parser is seeing the correct token at this point in the grammar, and fails fast if not.