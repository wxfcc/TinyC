#ifndef __JITENGINE_H__
#define __JITENGINE_H__
#include <string>
#include <set>
#include <map>
#include "Label.h"
#include "common.h"
#include "FunctionBuilder.h"
#include "FunctionBuilderX86.h"
#include "FunctionBuilderX64.h"

//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_FUNC_COUNT  1024
#define MAX_FUNC_PTR_SIZE (MAX_FUNC_COUNT * 8)
#define MAX_LOCAL_COUNT 64
#define JIT_X86     0
#define JIT_X64     1
#define JIT_ARM     2
#define JIT_ARM64   3
typedef int(*FP_MAIN)();

class JITEngine {
public:
    JITEngine(int arch);
    ~JITEngine();
    unsigned int getCodeSize();
    unsigned char* getCode();
    unsigned char* getFunction(const string& name);
    int setExecutable();

    void beginBuild();
    char** _getFunctionEntry(const string &name);
    const char* _getLiteralStringLoc(const string &literalStr);
    FunctionBuilder* beginBuildFunction();
    void endBuildFunction(FunctionBuilder *builder);
    void endBuild();
    void addFunctionEntry(const char* funcName, char* entry);
    void dumpCode();
    char* findSystemSymbol(const string& name);
    int callFunction(const string& name);
    int executeCode();

protected:
    char *m_textSection;
    unsigned int m_textSectionSize;
    map<string, char**> m_funcEntries;
    set<string> m_literalStrs;

    int m_arch;
    char ** m_funcPtr;
    void* m_process;
};


#endif
