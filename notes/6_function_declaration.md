# Function declaration: 
- In the previous chapter, we have already discussed about EBNF expression. Which is a way used in recursion descent to allow the comiler to prase arithemetic expression. 
- And we have already talked about when to start parsing a function definition. The relevant code is as follows:

```c
...
if (token == '(') {
    current_id[Class] = Fun;
    current_id[Value] = (int)(text + 1); // the memory address of function
    function_declaration();
} else {
...
```

Before this code snippet, we have already set the correct type for the current identifier. The code above sets the identifier's class to `Fun` (function) and assigns its position in the text segment (code segment).
> Logic: if the function is of certain datatype then with `(` then it is a function, otherwise, is just a variable declaration.

Next, we begin parsing the function definition itself, which includes the parameter declaration (`parameter_decl`) and the function body (`body_decl`).

#### Function Parameters and Assembly Code
Now, let's recall how a "function" is translated into the corresponding assembly code, because this determines what information we need to collect during parsing. Consider the following function:
```c
int demo(int param_a, int *param_b) {
    int local_1;
    char local_2;

    ...
}
```
It leads to a certain question: What kind of logic does it implies behind this code snippet?

When the demo function is being called, it will do the following task in the CPU: 

|
|    ....       |  <-- higher memory addresses (older stack data)
+---------------+
| arg: param_a  |    new_bp + 3   first argument to the function
+---------------+
| arg: param_b  |    new_bp + 2   second argument to the function
+---------------+
|return address |    new_bp + 1   address to return to after function finishes
+---------------+
| old BP        | <- new BP       previous base pointer (BP), now the current BP
+---------------+
| local_1       |    new_bp - 1   first local variable (allocated below BP)
+---------------+
| local_2       |    new_bp - 2   second local variable
+---------------+
|    ....       |  <-- lower memory addresses (more local variables, etc.)


- Function arguments are located at positive offsets from the base pointer (BP).
- The return address and old BP are also stored above the current BP.
- Local variables are stored at negative offsets from BP.
- This organization allows the function to access its arguments and local variables using fixed offsets from BP.

- The most important point here is that both function parameters (such as param_a) and local variables (such as local_1) are stored on the stack. This is different from global variables, which are stored in the data segment.

- The **data segment** is a portion of a program's memory where global variables and static variables are stored. These variables have a fixed size and lifetime: they exist for the entire duration of the program. The data segment is typically divided into two parts: the initialized data segment (for variables with an initial value) and the uninitialized data segment (also called the BSS segment, for variables without an explicit initial value).

- The **heap** is a separate region of memory used for dynamic memory allocation. Memory on the heap is managed at runtime using functions like `malloc`, `calloc`, `realloc`, and `free` in C. Variables allocated on the heap do not have fixed names in the source code; instead, they are accessed via pointers. The lifetime of heap-allocated memory is controlled by the programmer: it persists until it is explicitly freed.

- Inside a function, parameters and local variables are accessed via the new_bp pointer (the base pointer) and their respective offsets. Therefore, during parsing, we need to keep track of the number of parameters and the offset of each parameter. (`OLD_BP + 1/ -1 and so on`)

#### Parsing a Function Definition
```c
void function_declaration() {
    // type func_name (...) {...}
    //               | this part

    match('(');
    function_parameter();// Parse the function's parameter list and add parameter information to the symbol table.
    match(')');
    match('{');
    function_body();
    //match('}');                 //  ①

    // ②
    // unwind local variable declarations for all local variables.

   

    current_id = symbols;
    while (current_id[Token]) {
        // This loop restores the symbol table entries for all local variables after a function definition is parsed.
        if (current_id[Class] == Loc) {
            // Restore the original (outer/global) symbol table information for this identifier.
            current_id[Class] = current_id[BClass];
            current_id[Type]  = current_id[BType];
            current_id[Value] = current_id[BValue];
        }
        current_id = current_id + IdSize;
    }
}
```
- Imagine the symbol table as a row of lockers in a school hallway. Each locker (identifier) can hold different items (information about variables).
- When a function is called, some students (local variables or parameters) temporarily put their own items in certain lockers, covering up whatever was there before (possibly global variables with the same name).
- At the end of the function, we need to restore the original contents of those lockers so the hallway returns to its previous state.

- The last while loop starts at the first locker (current_id = symbols) and checks every locker (while current_id[Token] is nonzero).
- If a locker is currently holding a local variable (Class == Loc), we put back the original items from the backup compartments (BClass, BType, BValue).
- After checking each locker, we move to the next one (current_id += IdSize), ensuring the hallway is ready for the next class (scope).

- In part ①, we do not consume the final `}` character. The reason for this is that variable declarations and function declarations are parsed together. 
- Variable declarations end with a `;`, while function declarations end with a `}`.
- If we were to consume the ending character here using match, the outer while loop would not be able to accurately determine when a function definition has ended. 
- Say like we want to define a local variable in the function, the above appraoch would have mistakenly consider `;` as the end of the function. 
- Therefore, we leave the parsing of the ending character to the outer while loop.

- The code in part ② is used to restore the symbol table information back to the global state. This is necessary because local variables can have the same names as global variables. 
- When this happens, the local variable will shadow the global variable within the function body. Once we exit the function body, the global variable should regain its original effect. This code linearly traverses all identifiers and restores the information saved in the BXXX backup fields.

> Using the BNF grammar: 
```c
parameter_decl ::= type {'*'} id {',' type {'*'} id}
```

- Parsing function parameters involves processing a comma-separated list of identifiers, while recording their positions and types.

```c
int index_of_bp; // index of bp pointer on stack

void function_parameter() {
    int type;
    int params;
    params = 0;
    while (token != ')') {
        // ①

        // int name, ...
        type = INT;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHAR;
            match(Char);
        }

        // pointer type
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        // parameter name
        if (token != Id) {
            printf("%d: bad parameter declaration\n", line);
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            printf("%d: duplicate parameter declaration\n", line);
            exit(-1);
        }

        match(Id);

        //②
        // store the local variable
        current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
        current_id[BType]  = current_id[Type];  current_id[Type]   = type;
        current_id[BValue] = current_id[Value]; current_id[Value]  = params++;   // index of current parameter

        if (token == ',') {
            match(',');
        }
    }

    // ③
    index_of_bp = params+1;
}
```
- ① is similar to global variable parsing; it determines the parameter type.
- ② Here, we save the old (global) variable info in BXXX, then set the parameter info (like its position) for the local variable. This lets local variables override globals.
- ③ index_of_bp stores the number of parameters plus one, which helps generate assembly code for the function.

#### For parsing the function body:
In our C implementation, all variable declarations must come before any statements inside a function. It doesnt support declaration between operating statments: 

```c
void function_body() {
    // type func_name (...) {...}
    //                   -->|   |<--

    // ... {
    // 1. local declarations
    // 2. statements
    // }

    int pos_local; // position of local variables on the stack.
    int type;
    pos_local = index_of_bp;

    // ①
    while (token == Int || token == Char) {
        // local variable declaration, just like global ones.
        basetype = (token == Int) ? INT : CHAR;
        match(token);

        while (token != ';') {
            type = basetype;
            // token == Mul checks if the current token is a '*' (pointer declarator).
            // Each '*' increases the pointer level (type + PTR).
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }

            if (token != Id) {
                // invalid declaration
                printf("%d: bad local declaration\n", line);
                exit(-1);
            }
            if (current_id[Class] == Loc) {
                // identifier exists
                printf("%d: duplicate local declaration\n", line);
                exit(-1);
            }
            match(Id);

            // store the local variable
            current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
            current_id[BType]  = current_id[Type];  current_id[Type]   = type;
            current_id[BValue] = current_id[Value]; current_id[Value]  = ++pos_local;   // index of current parameter

            if (token == ',') {
                match(',');
            }
        }
        match(';');
    }

    // ②
    // save the stack size for local variables
    *++text = ENT;
    *++text = pos_local - index_of_bp;

    // statements
    while (token != '}') {
        statement();
    }

    // emit code for leaving the sub function
    *++text = LEV;
}
```
Part ① is responsible for parsing the definitions of local variables inside the function body. The logic here is almost the same as for global variable declarations.

Part ② is responsible for generating the assembly code. As mentioned in Chapter 3 (the virtual machine), we need to reserve space on the stack for local variables, and these two lines of code accomplish that task.
