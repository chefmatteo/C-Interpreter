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
    //call subroutine
    *--sp = (int)(pc+1); 
    pc = (int*)*pc; 
}
```