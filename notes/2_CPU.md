# CPU commands: 
An instruction set is the collection of commands that a CPU can recognize and execute—in other words, it's the "language" that the CPU understands. Here, we will design our own instruction set for our virtual machine. These instructions are inspired by the x86 instruction set, but are much simpler.

First, add an enum type to the global variables section. This enum will define all the instructions our virtual machine will support:
```c
// instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };
```
The order of these instructions is intentional: instructions that take arguments come first, followed by those that do not. This arrangement is mainly for convenience when printing debugging information. However, the order in which we explain the instructions does not follow this sequence.

#### MOV
- The MOV instruction is the most basic command in a CPU. It is used to move data into a register or a memory address, similar to an assignment in C. 
- In x86 assembly, MOV has two arguments: a destination and a source, like MOV dest, source. This means "copy the value from source to dest." The source and destination can be a number, a register, or a memory address.

In our virtual machine, things are simpler. We only have one main register (ax), and it's hard to tell if an argument is a value or an address. So, instead of one MOV instruction, we split it into five simple instructions, each with only one argument:

- IMM <num>: Put the number <num> into the ax register.
- LC: This instruction loads a single character (1 byte) from the memory address currently stored in the ax register, and puts that character's value back into ax. In other words, after executing LC, ax will contain the character found at the address ax was pointing to.
  - LC(Char) 好似你攞住 ax 呢把鎖匙，去 ax 指住嘅抽屜（記憶體位址）拎出入面嗰粒字（1 byte），然後 ax 就變咗做你手上拎住嗰粒字。即係 ax 由指住個位，變咗攞咗入面嘅嘢返嚟。

- LI (integer): This instruction loads an integer (typically 4 bytes) from the memory address currently stored in the ax register, and puts that integer's value back into ax. So after executing LI, ax will contain the integer found at the address ax was pointing to.
  - LI 好似 ax 拎住一條抽屜鎖匙，去 ax 指住嘅抽屜（記憶體位址）打開，攞出入面嗰本 4 頁厚嘅簿仔（整數），然後 ax 就變咗攞住嗰本簿仔返嚟。即係 ax 由指住個位，變咗攞咗入面嘅整數值返嚟。

- SC: Store the value in ax as a character into the address at the top of the stack.
- SI: Store the value in ax as an integer into the address at the top of the stack.

You might think splitting MOV into several instructions makes things more complicated, but it actually makes the implementation much easier. In x86, there are already many versions of MOV for different types (like MOVB for bytes, MOVW for words). Our LC/SC and LI/SI instructions are just for handling characters and integers.

The key point is: only IMM needs a parameter, and we don't need to check types. This makes the virtual machine much simpler to write.

Add the following code to the eval() function:
```c
int eval() {
    //op means operation
    int op, *tmp;
    while (1) {
        if (op == IMM)       {ax = *pc++;}                                     // load immediate value to ax
        else if (op == LC)   {ax = *(char *)ax;}                               // load character to ax, address in ax
        else if (op == LI)   {ax = *(int *)ax;}                                // load integer to ax, address in ax
        else if (op == SC)   {ax = *(char *)*sp++ = ax;}                       // save character to address, value in ax, address on stack
        else if (op == SI)   {*(int *)*sp++ = ax;}                             // save integer to address, value in ax, address on stack
    }
    ...
    return 0;
}
```
`*sp++`: Stack pointer is pointing to the top of the stack memory, now we deref it and add 1, which is equivalent to the pop action

- For the SI/SC instructions, the address is taken from the stack, while for LI/LC, the address is taken from the ax register.
- The reason is that by default, the result of most calculations is stored in ax, and addresses are usually computed and end up in ax as well. So, when executing LI/LC, it's more efficient to fetch the value directly from the address in ax.
- On the other hand, for SI/SC, the address to store to is expected to be on the stack. Also, note that our PUSH instruction can only push the value of ax onto the stack; it cannot take an arbitrary value as a parameter.

#### `PUSH`: 
In x86 assembly, the PUSH instruction is used to place a value or register onto the stack. In our virtual machine, PUSH simply pushes the value of the ax register onto the stack. This design choice simplifies the implementation of the virtual machine, especially since we only have a single register, ax. 
```c
else if (op == PUSH){*--sp = ax;} //push the value of ax onto the stack;
``` 

- In C, the expression `*--sp = ax;` is used to push a value onto the stack when the stack grows downward (from high to low addresses).
  - Step-by-step:
    - `--sp`: Decrement the stack pointer first, so it now points to the next empty slot on the stack.
    - `*--sp = ax;`: Store the value of `ax` at the new location pointed to by `sp`.
- In contrast, `*sp-- = ax;` would:
  - Store `ax` at the current location pointed to by `sp`.
  - Then decrement `sp`, so `sp` points to the previous slot.
- For a downward-growing stack (as in our VM), `*--sp = ax;` is the correct way to push a value, because it ensures `sp` always points to the top of the stack (the most recently pushed value).
- Example usage in the `eval()` function:
  - `else if (op == PUSH) { *--sp = ax; } // push ax onto the stack`

#### `JMP`: 
- JMP <addr> is an unconditional jump instruction. It sets the program counter (PC) register to the specified <addr> without any condition. The implementation is as follows:
```c
else if (op == JMP)  {pc = (int *)*pc;}                                // jump to the address
```
- pc: points to the next instruction (array of ints).
- *pc: gets the jump target address.
- (int *)*pc: cast the address to a pointer.
- pc = (int *)*pc: set pc to the new instruction location. 
- lets the VM jump to a new place in the instruction stream.
- The cast is needed because the instruction array is an array of int pointers, but the address stored might be an integer value; casting it to (int *) tells the VM, "Treat this number as a pointer to the next instruction."

- The Program Counter (PC) register always points to the next instruction. At this point, it holds the parameter for the JMP instruction, which is the value of <addr>.

#### `JZ/JNZ`:
- Jump if zero and Jump if not zero
-  To implement if statements, we need conditional jump instructions. Here, we only implement the two simplest conditional jumps: jumping when the result (ax) is zero or not zero.
```c
else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}                   // jump if ax is zero
else if (op == JNZ)  {pc = ax ? (int *)*pc : pc + 1;}                   // jump if ax is not zero

//with a better readability: 
else if (op == JZ) {
    if (ax == 0) {
        pc = (int *)*pc;
    } else {
        pc = pc + 1;
    }
} 
else if(op == JNZ){
    if (ax != 0) {
        pc = (int *)*pc;
    } else {
        pc = pc + 1;
    }
}
```
#### `Function being called`: 
- Involving the instructions: `CALL`, `ENT`, `ADJ`, and `LEV`.
- Focus on CALL <addr> and `RET` instructions:
  - `CALL` <addr>: jumps to the subroutine at address <addr>.
  - `RET` (return): returns from the subroutine to the point where it was called.
- Why not just use JMP for function calls?
  - JMP only jumps unconditionally and does not remember where it came from.
  - After a function call, the program needs to return to the original location to continue execution, so the return address must be saved.
- Practical approach:
  - When executing `CALL`, save the current PC (program counter) onto the stack, then jump to the subroutine.
  - When the subroutine finishes, `RET` pops the return address from the stack and restores the PC, so execution continues after the call.

```c
else if (op == CALL){
    // Call subroutine
    // Save the return address (the instruction after CALL) onto the stack.
    // --sp moves the stack pointer down (since the stack grows downward), and then we store (pc+1) as an integer.
    // (pc+1) points to the next instruction after CALL, so when the function returns, execution can continue there.
    *--sp = (int)(pc + 1);

    // Update the program counter (pc) to the address of the function to call.
    // *pc gets the target address stored after the CALL instruction, and we cast it to (int *) to use as a pointer.
    pc = (int *)*pc;
}
    //return from subroutine: 
else if (op == CALL){
        pc = (int*)*sp++; //sp hasnt been changed so it is safe to use that directly
}
```
Note that we can replace the `GET` with `LEV` to replace it.

When calling a function in practice, we need to consider not only the address of the function but also how to pass arguments and how to return results.

**Return Value Convention:**
- If a function returns a value, we agree that the result will be stored in the `ax` register upon return. This value can be either a direct value or an address, depending on the function's purpose.

**Parameter Passing Convention:**
- Different programming languages have different conventions for function calls. In C, the standard calling convention is as follows:

1. **Argument Passing:**
   - The caller is responsible for pushing the function arguments onto the stack before making the call.
   - Arguments are pushed onto the stack in reverse order (right to left), so that the first argument is closest to the top of the stack when the function begins execution.

2. **Return Value:**
   - The callee (the called function) places the return value in the `ax` register before returning.

3. **Stack Cleanup:**
   - After the function call completes, the caller is responsible for removing (popping) the arguments from the stack. i.e. the return of the result from the `ax`.

This convention ensures that both the caller and callee know where to find arguments and return values, and it allows for consistent function calls and returns within the virtual machine or compiled code.

For instance, from wikipedia: 
```c
int callee(int, int, int); 

int caller(void)
{
	int i, ret;

	ret = callee(1, 2, 3);
	ret += 5;
	return ret;
}
```
will create the following x86 code: 
```text
caller:
	; make new call frame
	push    ebp
	mov     ebp, esp
        sub     1, esp       ; save stack for variable: i
	; push call arguments
	push    3
	push    2
	push    1
	; call subroutine 'callee'
	call    callee
	; remove arguments from frame
	add     esp, 12
	; use subroutine result
	add     eax, 5
	; restore old call frame
        mov     esp, ebp
	pop     ebp
	; return
	ret
```

The above assembly code presents several challenges for our own virtual machine:

- The `push ebp` instruction cannot be directly implemented, since our `PUSH` instruction does not support pushing registers other than `ax`.
- The `mov ebp, esp` instruction is also problematic, as our instruction set lacks a general-purpose `MOV` between arbitrary registers.
- Similarly, `add esp, 12` is not directly supported (and we haven't even defined such an instruction yet).

Because our instruction set is intentionally simple (for example, most instructions only operate on the `ax` register), we cannot directly implement function calls as in the above example. 
- Rather than making our existing instructions more complex (which would complicate the implementation), our solution is to expand the instruction set with a few new, higher-level instructions specifically for function call management. 
- Since we're building a virtual machine and not a real CPU, adding new instructions is a practical way to support features like function calls without overcomplicating the core instruction logic.


#### `ENT`: 
- The `ENT <size>` instruction stands for "enter" and is used to set up a new function call frame on the stack. 
- When a function is called, it needs its own stack frame to store local variables and to keep track of the previous function's state. 
- The `ENT` instruction accomplishes this by first saving the current base pointer (which marks the start of the previous stack frame), then updating the base pointer to the current stack pointer position, and finally reserving a specified amount of space on the stack for the local variables of the new function. This is similar to the following sequence in x86 assembly:

    push    ebp        ; Save the old base pointer
    mov     ebp, esp   ; Set the new base pointer to the current stack pointer
    sub     esp, <size>; Allocate space for local variables

 `ENT <size>` is a single instruction in our virtual machine that encapsulates all these steps, making it easier to manage function call frames and local variable storage.

```c
else if (op == ENT){
    *--sp = (int)bp; 
    bp = sp; 
    sp = sp - *pc++; 
     // By using *pc++, we read the value and then advance pc to the next instruction.
     // Reserve space for local variables; pc++ moves to the next instruction after reading the size.
}
```
#### `ADJ`:

- The `ADJ <size>` instruction is used to "remove arguments from frame" after a function call. When a function is called, its arguments are pushed onto the stack. After the function returns, these arguments need to be removed from the stack to restore the previous state.
- Initially our `ADD` function has limited ability, so the corresponding code is, in x86 assembly, this is typically done with:
    ```
    add     esp, 12    ; remove 3 arguments (each 4 bytes)
    ```
- In our virtual machine, since the `ADD` instruction is limited and cannot directly add an immediate value to the stack pointer, we introduce the `ADJ` instruction to perform this operation in one step.
```c
else if (op == ADJ){sp = sp + *pc++;} //add esp, <size>

/*It does the following:
 - `*pc++` reads the immediate value (e.g., the number of bytes to adjust the stack pointer by) from the instruction stream and then advances the program counter to the next instruction.
 - `sp = sp + *pc++;` increases the stack pointer (sp) by that immediate value,effectively removing arguments or cleaning up the stack after a function call.
 The program counter (pc) is only incremented by one (to skip over the immediate value), not by the memory size of sp.
*/ 
//Example: if *pc is 12, then sp = sp + 12; pc moves to the next instruction.
```

#### `LEV`:

- The `LEV` instruction stands for "leave" and is used to restore the previous function call frame and return from a function. 
- In traditional x86 assembly, this process involves restoring the stack pointer and base pointer, and then returning to the caller. The equivalent x86 instructions are:

    ```
    mov     esp, ebp   ; Restore the stack pointer to the base pointer (removes local variables)
    pop     ebp        ; Restore the previous base pointer
    ret                ; Return to the caller (pop return address into PC)
    ```

- In our virtual machine, since we do not have a general-purpose `POP` instruction and want to avoid using multiple instructions for this common pattern, we introduce the `LEV` instruction to encapsulate all these steps in one operation.

- The implementation in C is as follows:

    ```c
    else if (op == LEV)  {
        sp = bp;                // Restore stack pointer to base pointer (removes local variables)
        bp = (int *)*sp++;      // Restore previous base pointer
        pc = (int *)*sp++;      // Restore return address (program counter)
    }  // restore call frame and PC
    ```

- Note: The `LEV` instruction already includes the functionality of a `RET` (return) instruction, so we do not need a separate `RET` in our instruction set.


#### LEA

- The previous instructions solve the problem of managing call frames, but there is still the question of how a sub-function can access its passed-in arguments. - To understand this, we need to look at the layout of the call frame on the stack when arguments are passed. Let's use the following example (with arguments pushed in order):

```text
sub_function(arg1, arg2, arg3);
```
The stack frame after the function call looks like this:

|    ....       | high address
+---------------+
| arg: 1        |    new_bp + 4
+---------------+
| arg: 2        |    new_bp + 3
+---------------+
| arg: 3        |    new_bp + 2
+---------------+
|return address |    new_bp + 1
+---------------+
| old BP        | <- new BP
+---------------+
| local var 1   |    new_bp - 1
+---------------+
| local var 2   |    new_bp - 2
+---------------+
|    ....       |  low address

To access the first argument, we need to get the address at new_bp + 4. However, as mentioned earlier, our ADD instruction cannot operate on registers other than ax. Therefore, we introduce a new instruction: LEA <offset>.

```c
else if (op == LEA)  {ax = (int)(bp + *pc++);}   // load address for arguments.
```
With this, we have all the instructions needed to implement function calls in our virtual machine.

Operator Instructions

- For each operator supported in C, we provide a corresponding assembly instruction in the virtual machine.
- All operators are binary (take two operands):
    - The first operand is placed on the top of the stack.
    - The second operand is stored in the `ax` register.
- The order of operands is important, especially for non-commutative operators like `-` and `/`.
- After the operation:
    - The top value on the stack is popped (removed).
    - The result of the operation is stored in the `ax` register.
    - Both original operands are no longer accessible (from the perspective of assembly; they may still exist in memory elsewhere).

Implementation: 
```c
else if (op == OR)  ax = *sp++ | ax;
else if (op == XOR) ax = *sp++ ^ ax;
else if (op == AND) ax = *sp++ & ax;
else if (op == EQ)  ax = *sp++ == ax;
else if (op == NE)  ax = *sp++ != ax;
else if (op == LT)  ax = *sp++ < ax;
else if (op == LE)  ax = *sp++ <= ax;
else if (op == GT)  ax = *sp++ >  ax;
else if (op == GE)  ax = *sp++ >= ax;
else if (op == SHL) ax = *sp++ << ax;
else if (op == SHR) ax = *sp++ >> ax;
else if (op == ADD) ax = *sp++ + ax;
else if (op == SUB) ax = *sp++ - ax;
else if (op == MUL) ax = *sp++ * ax;
else if (op == DIV) ax = *sp++ / ax;
else if (op == MOD) ax = *sp++ % ax;
```
> Built-in Functions

To make our programs actually "useful," we need more than just core logic—we also need input and output capabilities. For example, in C, we often use the `printf` function for output. However, implementing `printf` from scratch is quite complex. If we want our compiler to be self-hosting, we would need to implement functions like `printf`, but these are not directly related to the core of the compiler itself. Therefore, we add new instructions to our virtual machine to support these functions directly.

The functions we need to support in the compiler are: `exit`, `open`, `close`, `read`, `printf`, `malloc`, `memset`, and `memcmp`. The code is as follows:

```cpp
else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
else if (op == CLOS) { ax = close(*sp);}
else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
else if (op == MALC) { ax = (int)malloc(*sp);}
else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
```

The reason this works is that these functions (such as printf, open, malloc, etc.) are already implemented on our computer as part of the standard C library. When we compile our compiler, the binary code for these functions is linked into our compiler executable. As a result, when our virtual machine executes these special instructions, it can directly call the corresponding system functions without us having to implement them ourselves.

Finally, we implement a raise error function:
```c
else {
    printf("unknown instruction:%d\n", op);
    return -1;
}
```


#### Testing: 
```c
int main(int argc, char *argv[])
{
    ax = 0;
    ...

    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;

    ...
    program();
}
```
To compile the program, use: `gcc xc-tutor.c`, and then run it with: `./a.out hello.c`. The output will be:

exit(30)

Additionally, our code contains some pointer casts that assume a 32-bit environment. On a 64-bit machine, this can cause segmentation faults. There are two ways to resolve this:

1. Compile with the `-m32` flag to force 32-bit mode: `gcc -m32 xc-tutor.c`
2. Add `#define int long long` at the beginning of your code. This makes `int` 64 bits, which avoids issues with pointer casting on 64-bit systems.

Note: Our previous program requires a source file as input, although at this stage it is not strictly necessary. As you can see from the result, our virtual machine is working correctly.
