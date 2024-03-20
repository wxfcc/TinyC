#include <stdio.h>
#include <stdlib.h>

#include "x64jit.h"

#ifdef _WIN64
x64FunctionBuilder* x64JITEngine::beginBuildFunction() {
    x64FunctionBuilder *r = new x64FunctionBuilder(this, m_textSection + m_textSectionSize);
    r->beginBuild();
    return r;
}
void x64JITEngine::endBuildFunction(x64FunctionBuilder *builder) {
    builder->endBuild();
    *_getFunctionEntry(builder->getFuncName()) = m_textSection + m_textSectionSize;
    m_textSectionSize += builder->getCodeSize();
    ASSERT(m_textSectionSize <= MAX_TEXT_SECTION_SIZE);
    delete builder;
}
//===============================================
//============================== syntax analysis
#endif