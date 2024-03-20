#ifndef __X86_JITENGINE__
#define __X86_JITENGINE__
#include <string>
#include <set>
#include <map>
#include "x86Label.h"
#include "common.h"

//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_LOCAL_COUNT 64
class x86FunctionBuilder;
class x86JITEngine {
public:
    x86JITEngine();
    ~x86JITEngine();
    unsigned char* getFunction(const string &name);

    void beginBuild();
    char** _getFunctionEntry(const string &name);
    const char* _getLiteralStringLoc(const string &literalStr);
    x86FunctionBuilder* beginBuildFunction();
    void endBuildFunction(x86FunctionBuilder *builder);
    void endBuild();
protected:
    char *m_textSection;
    int m_textSectionSize;
    map<string, char*> m_funcEntries;
    set<string> m_literalStrs;
};


#endif