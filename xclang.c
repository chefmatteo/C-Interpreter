#include <stdio.h>
#include <stdlib.h>
#include <memory.h> //this is used for malloc and free memory 
#include <string.h> //string operations 
#define int long long //define int as long long for the virtual machine; this is to avoid segmentation fault;


/*
Please note that the following framework DO NOTHING. 
*/

//Initialization and setup: 
int token;           // current token，token 即係「詞法分析」時搵出嚟嘅基本單位，好似關鍵字、符號、數字、變數名等等（廣東話：token 就係程式碼入面一粒粒有意思嘅字，例如 int、if、+、123、a 咁樣）

char *src, *old_src; // pointer to the source code string; 

int poolsize;        // deafult size of the text/data/stack segments; 
int line;            // current line number; 

int *text;           // text segment
int *old_text;       // for dump text segment
int *stack;          // stack
char *data;          // data segment

//virtual machine registers: 
int *pc, *bp, *sp, ax, cycle; 

//instructions that the virtual machine will support: 
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
    OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
    OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };

void next(){
    token = *src++;
    //dereference the pointer, then get the next character; 
    return; 
}

void expression(int level){
    //level: determien the precedence of the operator, we will setup the hericharcy of the operators later; 
}

void program(){
    //this is the entry point of the program; 
    next(); 
    //get the first token; 
    while(token > 0){
        //=0 means the end of the program 
        //null character is defined as 0 or ((void *)0); 
        //if the last character is not a null character, that means is not the end of the program; 
        printf("token: %c\n", token); 
        //%c is used to print the character from its ASCII value; 
        next(); 
    }
}


int eval(){
    //op means operation and *tmp is a temporary pointer; 
     int op, *tmp;
     while (1) {
         if (op == IMM)       {ax = *pc++;}                                     // load immediate value to ax
         else if (op == LC)   {ax = *(char *)ax;}                               // load character to ax, address in ax
         else if (op == LI)   {ax = *(int *)ax;}                                // load integer to ax, address in ax
         else if (op == SC)   {ax = *(char *)*sp++ = ax;}                       // save character to address, value in ax, address on stack
         else if (op == SI)   {*(int *)*sp++ = ax;}                             // save integer to address, value in ax, address on stack
     }
     
     return 0;
}

int main(int argc, char **argv){
    //argc: number of arguments; 
    //argv: array of arguments: of double layer pointers; 
    int i, fd; 
    //i for loop counter; 
    //fd for file descriptr; 

    argc--; //skip the program name; 
    argv++; //skip the program name; 

    poolsize = 256 * 1024; //256KB or arbitrary size;
    //poolsize 係用嚟設定 text/data/stack segments 嘅預設大細（即係程式碼區、數據區、同埋堆疊區嘅容量），而家設咗做 256KB。

    //format of "open" is: int open(const char *pathname, int flags);
    //int flags means the mode of the file: aka: read-only, write-on ly, read-write, etc.
    //argv initially points to the program name, add a * before it to get the actual string; 
    //After incrementing argv (argv++), *argv points to the first user-supplied argument (the filename, e.g., "test.c").

    if ((fd = open(*argv, 0)) < 0){
        printf("Can't open the file %s\n", *argv); 
        return -1;  
    }

    // Allocate memory for the source code buffer.
    // malloc(poolsize) allocates a block of memory of size 'poolsize' bytes.
    // Both 'src' and 'old_src' pointers are set to the start of this memory block.
    // If allocation fails (returns NULL), print an error and exit.

    //the order: src = malloc(poolsize); old_src = src; 
    if (!(src = old_src = malloc(poolsize))){
        printf("Can't allocate memory for the source code\n"); 
        return -1; 
    }

    // Read the contents of the opened file (file descriptor 'fd') into the buffer 'src'.
    // "read" 嘅用法係咁：ssize_t read(int fd, void *buf, size_t count);
    // 即係：
    //   - fd：檔案描述符（file descriptor），用嚟代表你開咗邊個檔案（例如之前 open(*argv, 0) 嘅 fd）。
    //   - buf：一個指向記憶體嘅指標，read 會將檔案入面讀到嘅資料放入去呢個 buffer（例如 src）。
    //   - count：你想最多讀幾多 bytes（例如 poolsize）。
    // read 會嘗試由 fd 指定嘅檔案讀最多 count bytes 資料，放入 buf。
    // 成功會 return 真正讀到嘅 bytes 數目（可能少過 count），失敗就會 return -1。

    // The number of bytes actually read is stored in 'i'.
    // If 'read' fails (returns 0 or negative), print an error and exit.
    if ((i = read(fd, src, poolsize)) <= 0){
        printf("Can't read the file %s\n", *argv); 
        return -1; 
    }

    //allocate memory for different type of segments: 
    
    memset(text, 0, poolsize);
    memset(stack, 0, poolsize);
    memset(data, 0, poolsize);


  

    src[i] = 0; //end of the string, add a null terminator; 
    close(fd); 
    program(); 

    return eval; 
}


