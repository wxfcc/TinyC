#ifndef __JITENGINE_H__
#define __JITENGINE_H__
#include <string>
#include <set>
#include <map>
#include "Label.h"
#include "common.h"
#include "FunctionBuilder.h"
#include "x86FunctionBuilder.h"
#include "x64FunctionBuilder.h"

//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_LOCAL_COUNT 64

class JITEngine {
public:
    JITEngine(FunctionBuilder_FP builder_fp);
    ~JITEngine();
    unsigned char* getFunction(const string &name);

    void beginBuild();
    char** _getFunctionEntry(const string &name);
    const char* _getLiteralStringLoc(const string &literalStr);
    FunctionBuilder* beginBuildFunction();
    void endBuildFunction(FunctionBuilder *builder);
    void endBuild();
    void addFunctionEntry(const char* funcName, char* entry);

protected:
    char *m_textSection;
    int m_textSectionSize;
    map<string, char*> m_funcEntries;
    set<string> m_literalStrs;
    //(FunctionBuilder*) (m_funcBuilder);
    FunctionBuilder_FP m_funcBuilderFP;
};


#endif