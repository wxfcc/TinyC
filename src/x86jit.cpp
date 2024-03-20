#include <stdio.h>
#include <stdlib.h>
#include "x86jit.h"


x86JITEngine::x86JITEngine(): m_textSectionSize(0) {
    m_textSection = os_mallocExecutable(MAX_TEXT_SECTION_SIZE);
}
x86JITEngine::~x86JITEngine() { 
	os_freeExecutable(m_textSection); 
}
unsigned char* x86JITEngine::getFunction(const string &name) {
	return (unsigned char*)*_getFunctionEntry(name); 
}

char** x86JITEngine::_getFunctionEntry(const string &name) {
	return &m_funcEntries[name]; 
}
const char* x86JITEngine::_getLiteralStringLoc(const string &literalStr) { return m_literalStrs.insert(literalStr).first->c_str();}

void x86JITEngine::beginBuild() { 
}
void x86JITEngine::endBuild() {
    for (map<string, char*>::iterator iter = m_funcEntries.begin(); iter != m_funcEntries.end(); ++iter) {
        if (iter->second == NULL) {
            char *f = os_findSymbol(iter->first.c_str());
            ASSERT(f != NULL);
            iter->second = f;
        }
    }
}


//============================== syntax analysis
