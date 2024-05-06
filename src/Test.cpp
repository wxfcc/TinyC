#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

//#include <algorithm>
#include <functional>

#include <iostream>
#include "JITEngine.h"
#include "Scanner.h"
#include "FileParser.h"

using namespace std;
typedef int(*FP_Main)();

// just for set debug breakpoint
void block() {
}

// direct call printf will cause segmentation fault  ??
void myprintf(const char* s, ...) {
    va_list args;
    va_start(args, s);
    int n = (int)va_arg(args, int);

    printf("s=%s,n=%d",s, n);
    //if (n == 0)puts("n=0"); 
    //else if(n==1)puts("n=1");
    //puts(s);
#ifdef    __va_copy
    va_list ap_save;
    __va_copy(ap_save, args); // crashed!!
    //printf(s,n);
#endif
    va_end(args);

    //printf("myprintf enter, %d\n",n);
    //printf("myprintf:s=%p\n",p);
    //printf("myprintf:%s\n", s);
    //printf(s);
}
extern "C"
void func1(long long a) {
    printf("func1, a=%llx\n", a);
//    if(a > 0x10000000)
//        printf("%s\n",(char*)a);
}

void test2(const char* f, int n) {

}
int test(JITEngine* jit) {
    //myprintf("helo", 0x100,1,2,3,4,5,6);
    unsigned char* p = (unsigned char*)jit->getCode();
    char** p2 = (char**)p;
    *p2 = (char*)myprintf;
    p += 8;
    FP_Main mainFunc = (FP_Main)p;
#if 0
    unsigned char moveax[] = { 0xB4, 0x01, 0xb8, 0x01, 0x00, 0x00, 0x00, 0xc3 }; // for test mov eax, 1
    memcpy(p, moveax, sizeof(moveax));
#else 
    unsigned char code[] = {
#ifdef _WIN64
        0x48, 0x8d, 0x0d, 0xff, 0xff, 0xff, 0xff,   //lea rcx, rip + offset32
        0x48, 0xc7, 0xc2, 0x08, 0x00, 0x00, 0x00,   //mov rdx, #imm32
#else
        0x48, 0x8d, 0x3d, 0xff, 0xff, 0xff, 0xff,   //lea rdi, rip + offset32
        0x48, 0xc7, 0xc6, 0x08, 0x00, 0x00, 0x00,   //mov rsi, 8
#endif
        0x48, 0x81, 0xec, 0x80, 0x00, 0x00, 0x00,   //sub rsp,0x20
        0xb8, 0x00, 0x00, 0x00, 0x00,               //mov eax,0
        0xff, 0x15, 0xff, 0xff, 0xff, 0xff,         //call [rip + offset32]
        0x48, 0x81, 0xc4, 0x80, 0x00, 0x00, 0x00,   //add rsp,0x20
        0xc3,                                       //ret
        'a', 'b', 'c', 0
    };
    int offset;
    memcpy(p, code, sizeof(code));
    p += 3 ;
    offset = sizeof(code) - 4-7;
    *(int*)p = offset;

    p += 21 + 5 - 3;
    offset = (int)((unsigned char*)p2 - p)-6;
    p += 2;
    *(int*)p = offset;

#endif
    jit->setExecutable();
    mainFunc();
    exit(0);
    return 0;
}

int test2(JITEngine* jit) {
    unsigned char* p = (unsigned char*)jit->getCode();
    FP_Main mainFunc = (FP_Main)p;
    unsigned char code[] = {
        //mov rax, #imm64
        0x48, 0xB8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,				// mov rax, #imm64 
        0x48, 0xB9, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov rcx, #imm64 
        0x48, 0xBA, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov rdx, #imm64 
        0x48, 0xBB, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov rbx, #imm64 
        //0x48, 0xBC, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov rsp, #imm64 
        0x48, 0xBD, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov rbp, #imm64 
        0x48, 0xBE, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov rsi, #imm64 
        0x48, 0xBF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov rdi, #imm64 
        0x49, 0xB8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov r8,	 #imm64 
        0x49, 0xB9, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov r9,  #imm64 
        0x49, 0xBA, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov r10, #imm64 
        0x49, 0xBB, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov r11, #imm64 
        0x49, 0xBC, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov r12, #imm64 
        0x49, 0xBD, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov r13, #imm64 
        0x49, 0xBE, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov r14, #imm64 
        0x49, 0xBF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov r15, #imm64 
        // mov rax/r8, #imm32
        0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,								// mov    rax, 0x1   
        0x48, 0xc7, 0xc1, 0x01, 0x00, 0x00, 0x00,                               // mov    rcx, 0x1   
        0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,                               // mov    rdx, 0x1   
        0x48, 0xc7, 0xc3, 0x01, 0x00, 0x00, 0x00,                               // mov    rbx, 0x1   
        //0x48, 0xc7, 0xc4, 0x01, 0x00, 0x00, 0x00,                               // mov    rsp,0x1    
        0x48, 0xc7, 0xc5, 0x01, 0x00, 0x00, 0x00,                               // mov    rbp,0x1    
        0x48, 0xc7, 0xc6, 0x01, 0x00, 0x00, 0x00,                               // mov    rsi, 0x1   
        0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,                               // mov    rdi, 0x1   
        0x49, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,                               // mov    r8, 0x1    
        0x49, 0xc7, 0xc1, 0x01, 0x00, 0x00, 0x00,                               // mov    r9, 0x1    
        0x49, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,                               // mov    r10, 0x1   
        0x49, 0xc7, 0xc3, 0x01, 0x00, 0x00, 0x00,                               // mov    r11, 0x1   
        0x49, 0xc7, 0xc4, 0x01, 0x00, 0x00, 0x00,                               // mov    r12, 0x1   
        0x49, 0xc7, 0xc5, 0x01, 0x00, 0x00, 0x00,                               // mov    r13, 0x1   
        0x49, 0xc7, 0xc6, 0x01, 0x00, 0x00, 0x00,                               // mov    r14, 0x1   
        0x49, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,                               // mov    r15, 0x1   
        // mov eax/r8d, #imm64
        0xb8, 0x02, 0x00, 0x00, 0x00,											// mov    eax, 0x1 
        0xb9, 0x02, 0x00, 0x00, 0x00,                                           // mov    ecx, 0x1 
        0xba, 0x02, 0x00, 0x00, 0x00,                                           // mov    edx, 0x1 
        0xbb, 0x02, 0x00, 0x00, 0x00,                                           // mov    ebx, 0x1 
        //0xbc, 0x02, 0x00, 0x00, 0x00,                                           // mov    esp, 0x1 
        0xbd, 0x02, 0x00, 0x00, 0x00,                                           // mov    ebp, 0x1 
        0xbe, 0x02, 0x00, 0x00, 0x00,                                           // mov    esi, 0x1 
        0xbf, 0x02, 0x00, 0x00, 0x00,                                           // mov    edi, 0x1 
        0x41, 0xb8, 0x02, 0x00, 0x00, 0x00,                                     // mov    r8d, 0x1 
        0x41, 0xb9, 0x02, 0x00, 0x00, 0x00,                                     // mov    r9d, 0x1 
        0x41, 0xba, 0x02, 0x00, 0x00, 0x00,                                     // mov    r10d, 0x1
        0x41, 0xbb, 0x02, 0x00, 0x00, 0x00,                                     // mov    r11d, 0x1
        0x41, 0xbc, 0x02, 0x00, 0x00, 0x00,                                     // mov    r12d, 0x1
        0x41, 0xbd, 0x02, 0x00, 0x00, 0x00,                                     // mov    r13d, 0x1
        0x41, 0xbe, 0x02, 0x00, 0x00, 0x00,                                     // mov    r14d, 0x1
        0x41, 0xbf, 0x02, 0x00, 0x00, 0x00,                                     // mov    r15d, 0x1
        0xc3,                                                                   // ret
    };

    memcpy(p, code, sizeof(code));
    jit->setExecutable();
    mainFunc();
    exit(0);
    return 0;
}

// __cdecl __fastcall __stdcall __pascal,  '__fastcall/__stdcall/__pascal' calling convention is not supported for this target
static int t(int a, int b, int c, int d, int e, int f, int g) {
    return a;
}
//int     printf(const char * __restrict, ...) __printflike(1, 2);
void func4(float a, ...){
    long long k=0xff,s=0xfe;
    //asm{push rbx}   // cause EXC_BAD_ACCESS (code=EXC_I386_GPFLT)
    printf("hello %f\n", a);
//    asm{pop rbx}
}
void func3(){
    func4(0);
    printf("hello\n",1);
}

void test_parameters() {
    func4(1.0, 9);
    func3();
    t(1,2,3,4,5,6,7);
    t(1,2,3,4,5,6,7);
    myprintf("helo", 1, 2, 3, 4); // , (long long)0x123456789, (long long)0x123456789a);
    exit(0);
}

