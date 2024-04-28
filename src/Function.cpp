#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "JITEngine.h"
#include "Function.h"

Function::Function(JITEngine*parent, char *codeBuf){
    m_parent = parent;
    m_codeBuf = codeBuf;
    m_codeSize = 0;
    m_paramCount = 0;
    m_paramIndex = -1;
    m_beginCall = 0;
    m_localVarCount = 0;
}

Function::~Function(){
}

void Function::setFuncName(string& name, int argsCount) {
    m_funcName = name;
    m_paramCount = argsCount;
    saveParameters();
}

const string& Function::getFuncName() {
	return m_funcName;
}
int Function::getCodeSize() const{ 
	return m_codeSize;
}

void Function::markLabel(Label *label){ 
    label->mark(m_codeBuf + m_codeSize); 
}

void Function::emitCode(int n, ...) {
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; ++i) 
        m_codeBuf[m_codeSize++] = (char)va_arg(args, int);
    va_end(args);
}
void Function::setParamIndex(int paramIndex) {
    m_paramIndex = paramIndex;
}
