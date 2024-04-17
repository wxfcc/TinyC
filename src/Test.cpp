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

void block() {} // just for set debug breakpoint
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

void test2(const char* f, int n) {

}
int test(JITEngine* g_jitEngine) {
    //myprintf("helo", 0x100,1,2,3,4,5,6);
    unsigned char* p = g_jitEngine->getCode();
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
    block();
    mainFunc();
    exit(0);
    return 0;
}


