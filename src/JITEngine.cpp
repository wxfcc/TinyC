#include <stdio.h>
#include <stdlib.h>
#include "JITEngine.h"


JITEngine::JITEngine(int arch): m_arch(arch), m_textSectionSize(0) {
    m_textSection = os_mallocExecutable(MAX_TEXT_SECTION_SIZE);
}
JITEngine::~JITEngine() { 
	os_freeExecutable(m_textSection); 
}
FunctionBuilder* JITEngine::beginBuildFunction() {
    FunctionBuilder* builder = NULL;
    if (m_arch == JIT_X86)
        builder = new x86FunctionBuilder(this, m_textSection + m_textSectionSize);
    else if (m_arch == JIT_X64)
        builder = new x64FunctionBuilder(this, m_textSection + m_textSectionSize);

    builder->beginBuild();

    return builder;
}
void JITEngine::endBuildFunction(FunctionBuilder* builder) {
    builder->endBuild();
    *_getFunctionEntry(builder->getFuncName()) = m_textSection + m_textSectionSize;
    m_textSectionSize += builder->getCodeSize();
    ASSERT(m_textSectionSize <= MAX_TEXT_SECTION_SIZE);
    delete builder;
}

unsigned char* JITEngine::getFunction(const string &name) {
	return (unsigned char*)*_getFunctionEntry(name); 
}

void JITEngine::addFunctionEntry(const char* funcName, char* entry) {
    m_funcEntries[funcName] = entry;
}

char** JITEngine::_getFunctionEntry(const string &name) {
	return &m_funcEntries[name]; 
}
const char* JITEngine::_getLiteralStringLoc(const string &literalStr) {
    return m_literalStrs.insert(literalStr).first->c_str();
}

void JITEngine::beginBuild() { 
}
void JITEngine::endBuild() {
    for (map<string, char*>::iterator iter = m_funcEntries.begin(); iter != m_funcEntries.end(); ++iter) {
        if (iter->second == NULL) {
            char *f = os_findSymbol(iter->first.c_str());
            ASSERT(f != NULL);
            iter->second = f;
        }
    }
}
void JITEngine::dumpCode() { 
    unsigned char* p = (unsigned char*)m_textSection;
    for (int i = 0; i < m_textSectionSize; i++) {
        printf("%02x ", p[i]);
    }
    printf("\n");
}


//============================== syntax analysis
