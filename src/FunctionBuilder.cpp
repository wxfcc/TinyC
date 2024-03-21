#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "JITEngine.h"
#include "FunctionBuilder.h"

FunctionBuilder::FunctionBuilder(JITEngine*parent, char *codeBuf): 
    m_parent(parent), m_codeBuf(codeBuf), m_codeSize(0), m_paramCount(0){
}

string& FunctionBuilder::getFuncName() {
	return m_funcName;
}
int FunctionBuilder::getCodeSize() const{ 
	return m_codeSize;
}

void FunctionBuilder::markLabel(Label *label){ 
    label->mark(m_codeBuf + m_codeSize); 
}

void FunctionBuilder::emit(int n, ...) {
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; ++i) 
        m_codeBuf[m_codeSize++] = (char)va_arg(args, int);
    va_end(args);
}

template<typename T>
void FunctionBuilder::emitValue(T val) {
    memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
    m_codeSize += sizeof(val);
}

