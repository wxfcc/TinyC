#ifndef __JITENGINE_H__
#define __JITENGINE_H__
#include <string>
#include <set>
#include <map>
#include "Label.h"
#include "common.h"
#include "Scanner.h"
#include "FunctionX86.h"
#include "FunctionX64.h"
#include "FunctionX64Linux.h"

//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_FUNC_COUNT  1024
#define MAX_FUNC_PTR_SIZE (MAX_FUNC_COUNT * 8)
#define MAX_LOCAL_COUNT 64
#define JIT_X86     0
#define JIT_X64     1
#define JIT_ARM     2
#define JIT_ARM64   3
#define JIT_OS_WINDOWS   0
#define JIT_OS_LINUX   1
#define JIT_OS_MACOS   2

typedef int(*FP_MAIN)();

class JITEngine {
public:
    JITEngine(int arch, int os);
    ~JITEngine();
    char* mallocAlign(int size);
    void freeAlign(char*p, int size);
    int getCodeSize();
    char* getCode();
    char* getFunction(const string& name);
    int setExecutable();

    void beginBuild();
    char** _getFunctionEntry(const string &name);
    const char* _getLiteralStringLoc(const string &literalStr);
    Function* beginBuildFunction();
    void endBuildFunction(Function *builder);
    void endBuild();
    void addFunctionEntry(const char* funcName, char* entry);
    void dumpCode();
    char* findSystemSymbol(const string& name);
    int callFunction(const string& name);
    int executeCode();
    const char *getArch();
    const char *getOs();


protected:
    char *m_code;
    int m_codeMax;
    int m_codeSize;
    map<string, char**> m_funcEntries;
    set<string> m_literalStrs;

    int m_arch;
    int m_os;
    char ** m_funcPtr;
    void* m_process;
};


#endif
