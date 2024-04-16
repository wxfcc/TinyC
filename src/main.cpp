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

static string unescape(const string &s) {
    string r;
    for (int i = 0; i < (int)s.size(); ++i) {
        if (s[i] == '\\') {
            switch (s[++i]) {
                case 't': r += '\t'; break;
                case 'n': r += '\n'; break;
                case 'r': r += '\r'; break;
                default: r += s[i]; break;
            }
        } else r += s[i];
    }
    return r;
}
static string format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    static vector<char> buf(256);
    while ((int)buf.size() == vsnprintf(&buf[0], buf.size(), fmt, args)) {
        buf.resize(buf.size() * 3 / 2);
    }
    va_end(args);
    return &buf[0];
}
static string readFile(const string &fileName) {
    string r;
    FILE *f = fopen(fileName.c_str(), "rb");
    if (f == NULL) return r;
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    r.resize(size + 1, 0);
    int bytes = (int)fread(&r[0], size, 1, f);
    ASSERT(bytes == 1);
    fclose(f);
    return r;
}
//============================== platform details for jit
#ifdef _MSC_VER
#pragma warning(disable : 4312)
#pragma warning(disable : 4311)
#include <Windows.h>
#include <Psapi.h>
#undef TokenID
#pragma comment(lib, "Psapi.lib")
#pragma warning(default : 4311)
#pragma warning(default : 4312)
char* os_mallocExecutable(int size) {
    char *p = (char*)::VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    ASSERT(p != NULL);
    return p;
}
void os_freeExecutable(char *p) {
    ASSERT(p != NULL);
	::VirtualFree(p, 0, MEM_RELEASE);
}
char* os_findSymbol(const char *funcName) {
    HANDLE process = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    ASSERT(process != NULL);

    vector<HMODULE> modules;
    DWORD bytes;
    EnumProcessModules(process, NULL, 0, &bytes);
    modules.resize(bytes / sizeof(modules[0]));
    EnumProcessModules(process, &modules[0], bytes, &bytes);

    ASSERT(modules.size() > 0);

    char *func = NULL;
    for (int i = 0; i < (int)modules.size(); ++i) {
        if (func = (char*)::GetProcAddress(modules[i], funcName)) {
            char moduleName[256] = { 0 };
            GetModuleBaseName(process, modules[i], moduleName, sizeof(moduleName));
            printf("%s %s: %p\n", moduleName, funcName, func);
            break;
        }
    }

    ::CloseHandle(process);
    return func;
}
#endif
//==============================
#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
char* os_mallocExecutable(int size) {
    void *p = NULL;
#ifdef __MACH__
    p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
    ASSERT(p != NULL);

//    erro = mprotect(p, size, PROT_READ | PROT_EXEC);
//    ASSERT(erro == 0);
#else
    int erro = ::posix_memalign(&p, ::getpagesize(), size);
    ASSERT(erro == 0 && p != NULL);
    erro = ::mprotect(p, size, PROT_READ | PROT_WRITE | PROT_EXEC);
    ASSERT(erro == 0);
#endif
    return (char*)p;
}
void os_freeExecutable(char *p) {
    ASSERT(p != NULL);
    ::free(p);
}
char* os_findSymbol(const char *funcName) {
    void *m = ::dlopen(NULL, RTLD_NOW);
    char *r = (char*)::dlsym(m, funcName);
    ::dlclose(m);
    return r;
}
#endif

//============================== syntax analysis
//============================== 
static JITEngine* g_jitEngine;
static int loadFile(const char *fileName) {
    try {
        printf("loadFile: %s\n", fileName);
        g_jitEngine->beginBuild();
        string source = readFile(fileName);
        Scanner scanner(source.c_str());
        FileParser(g_jitEngine, &scanner).parse();
        g_jitEngine->endBuild();
        return 0;
    } catch(const exception &e) {
        puts(e.what());
        return 1;
    }
}

int handleInput(){
    int lineNum = 0;
    for (string line; ; ++lineNum) {
        printf(">>> ");
        if (!getline(cin, line)) break;
		line.erase(line.begin(), std::find_if(line.begin(), line.end(), std::not1(std::ptr_fun(::isspace))));
        if (line.size() == 0)continue;
        if (line == "q")break;

        try {
            Scanner scanner(line.c_str());

            bool isFuncDefine = false;
            switch (scanner.LA(1).tid) {
                case TID_TYPE_INT: 
                case TID_TYPE_STRING: 
                case TID_TYPE_VOID: 
                    if (scanner.LA(2).tid == TID_ID && scanner.LA(3).tid == TID_LP) isFuncDefine = true;
                    break;
                default: 
                	break;
            }

            if (!isFuncDefine) {
                line = format("int temp_%d(){ return %s; }", lineNum, line.c_str());
                scanner = Scanner(line.c_str());
            }

            g_jitEngine->beginBuild();
            FileParser(g_jitEngine, &scanner).parse();
            g_jitEngine->endBuild();

            if (!isFuncDefine) {
            	string tf = format("temp_%d", lineNum);
                g_jitEngine->setExecutable();
            	unsigned char*p = g_jitEngine->getFunction(tf);

	            printf("func %s: %p, code: %02x %02x %02x %02x\n", tf.c_str(), p, p[0], p[1], p[2], p[3]);
            	int(*f)() = (int(*)())p;
            	
                printf("ret=%d\n", f());
            }

        } catch(const exception &e) {
            puts(e.what());
        } 
    }
    return 0;	
}
typedef int(*FP_Main)();
void block() {} // just for set debug breakpoint
int handleFile(char *file){
    int ret = loadFile(file);
    if(ret == 0){
        g_jitEngine->setExecutable();
        FP_Main mainFunc = (FP_Main)g_jitEngine->getFunction("_start");
        if (mainFunc) {
            ret = mainFunc();
        }
        else {
            mainFunc = (FP_Main)g_jitEngine->getFunction("main");
            if (mainFunc) {
                g_jitEngine->dumpCode();
                block();
                ret = mainFunc();
                printf("%s return: %d\n", file, ret);
            }
            else
                printf("not found main()\n");
        }
    }
	return ret;
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

JITEngine* createJitEngine(int arch){
    JITEngine* engine = new JITEngine(arch);

    engine->addFunctionEntry("loadFile", (char*)loadFile);
    engine->addFunctionEntry("runFile", (char*)handleFile);
    engine->addFunctionEntry("printf", (char*)printf);
    engine->addFunctionEntry("myprintf", (char*)myprintf);
	return engine;
}
void test2(const char* f, int n) {

}
int test() {
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

int main(int argc, char *argv[]) {
    //GetStdHandle(0);
    //myprintf("helo", 1, 2, 3, 4,(long long)0x123456789, (long long)0x123456789a);
	int ret = 0;
    int arch = JIT_X86;
    const char*archs[] = {"x86", "x64"};
#if defined _WIN64 || defined __x86_64__
    arch = JIT_X64;
#endif
	g_jitEngine = createJitEngine(arch);

    printf("arch: %s, myprintf: %p, printf: %p\n", archs[arch], myprintf, printf);

    //test();

    if (argc >= 2) {
        ret = handleFile(argv[1]);
    }
    else{
    	ret = handleInput();    	
    }    

    return ret;
}

