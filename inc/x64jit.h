#ifndef __X64_JITENGINE__
#define __X64_JITENGINE__
#include "x86Label.h"
#include "x86jit.h"
#include "common.h"
#include "FunctionBuilder.h"

//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_LOCAL_COUNT 64
class x64FunctionBuilder;
class x64JITEngine :public x86JITEngine{
public:
    //x64JITEngine();
    //~x64JITEngine();
    //unsigned char* getFunction(const string &name);

    //void beginBuild();
    //char** _getFunctionEntry(const string &name);
    //const char* _getLiteralStringLoc(const string &literalStr);
    x64FunctionBuilder* beginBuildFunction();
    void endBuildFunction(x64FunctionBuilder *builder);
    //void endBuild();
private:
    //char *m_textSection;
    //int m_textSectionSize;
    //map<string, char*> m_funcEntries;
    //set<string> m_literalStrs;
};

#endif