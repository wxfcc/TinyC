#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <iostream>
#include "JITEngine.h"
#include "Scanner.h"
#include "FileParser.h"

using namespace std;
void myprintf(const char* s, ...);
int test(JITEngine* g_jitEngine);
int test2(JITEngine* g_jitEngine);
void test_parameters();
extern "C"{
void dbg_break();
void func1(int a);
void func2(int a);
void printf2(char*fmt);
}

static JITEngine* g_jitEngine;
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
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    r.resize(size + 1, 0);
    int bytes = (int)fread(&r[0], size, 1, f);
    ASSERT(bytes == 1);
    fclose(f);
    return r;
}

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

static int handleInput(){
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
                int ret = g_jitEngine->callFunction(tf);            	
                printf("ret=%d\n", ret);
            }

        } catch(const exception &e) {
            puts(e.what());
        } 
    }
    return 0;	
}

static int handleFile(char *file){
    int ret = loadFile(file);
    if(ret == 0){
        ret = g_jitEngine->executeCode();
        printf("%s return: %d\n", file, ret);
    }
	return ret;
}

JITEngine* createJitEngine(int arch, int os){
    JITEngine* engine = new JITEngine(arch, os);

    engine->addFunctionEntry("loadFile", (char*)loadFile);
    engine->addFunctionEntry("runFile", (char*)handleFile);
    engine->addFunctionEntry("printf", (char*)printf);
    engine->addFunctionEntry("printf2", (char*)printf2);
    engine->addFunctionEntry("myprintf", (char*)myprintf);
    engine->addFunctionEntry("func1", (char*)func1);
    engine->addFunctionEntry("func2", (char*)func2);
    engine->addFunctionEntry("dbg_break", (char*)dbg_break);
	return engine;
}
/*
 -a   arch: x86/x64/arm/arm64
 -os  windows/linux/macos
 */
// _start() 0x7ff8155adbd0
int main(int argc, char *argv[]) {
#if 0
    test_parameters();
#endif

    int ret = 0;
    int arch = JIT_X86, os = JIT_OS_WINDOWS;
#if defined _WIN64 || defined __x86_64__
    arch = JIT_X64;
#endif
#if defined __linux__ 
    os = JIT_OS_LINUX;
#elif defined __APPLE__
    os = JIT_OS_MACOS;
#endif
//    printf("helo %d %d\n", 1,2 ); exit(0);
    g_jitEngine = createJitEngine(arch, os);

    //test(g_jitEngine);
    //test2(g_jitEngine);

    if (argc >= 2) {
        ret = handleFile(argv[1]);
    }
    else{
    	ret = handleInput();    	
    }    
    return ret;
}

