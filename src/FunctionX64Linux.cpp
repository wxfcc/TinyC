#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#include "JITEngine.h"

FunctionX64Linux::FunctionX64Linux(JITEngine* parent, char* codeBuf) : FunctionX64(parent, codeBuf) {
}
FunctionX64Linux::~FunctionX64Linux() {
    printf("FunctionX64Linux: %s, param count=%d\n", m_funcName.c_str(), m_paramCount);
    //printf("FunctionX64Linux: %s, local variable count=%d\n", m_funcName.c_str(), m_localVarCount);
}

/*
 in linux x64 mode, the order of parameter: rdi rsi rdx rcx r8 r9 [rsp] ...
*/
void FunctionX64Linux::prepareParam(int64 paraVal, int size) {
    printf("FunctionX64Linux::prepareParam\n");
    if (m_paramIndex == 0) {
        emit(0x48, 0xbf); emitValue((int64)paraVal); // mov rdi, #imm64
    }
    else if (m_paramIndex == 1) { 
        emit(0x48, 0xbe); emitValue((int64)paraVal); // mov rsi, #imm64
    }
    else if (m_paramIndex == 2) { 
        emit(0x48, 0xba); emitValue((int64)paraVal); // mov rdx, #imm64
    }
    else if (m_paramIndex == 3) { 
        emit(0x48, 0xb9); emitValue((int64)paraVal); // mov rcx, #imm64
    }
    else if (m_paramIndex == 4) { 
        emit(0x49, 0xb8); emitValue((int64)paraVal); // mov r8, #imm64
    }
    else if (m_paramIndex == 5) { 
        emit(0x49, 0xb9); emitValue((int64)paraVal); // mov r9, #imm64
    }
    else {
        if (size <= 4) {
            emit(0x68); emitValue((int)paraVal); // push #imm32, in fact it'll push #imm64 with the hi 32 bits as 0 
        }
        else {
            emit(0x48, 0xb8); emitValue((int64)paraVal); // mov #rax, #imm64
            emit(0x50); // push rax
            // 48 C7 44 24 20 04 00 00 00             mov    qword ptr [rsp+20h], 4 
            // 48 c7 84 24 20 01 00 00 00 00 00 00    mov    QWORD PTR [rsp + 0x120], 0x0

        }
    }
    m_paramIndex++;
}

#if 0
void FunctionX64Linux::loadImm(int imm) {
    if(m_beginCall)
        prepareParam(imm, sizeof(imm));
    else {
        emit(0x68); emitValue(imm); // push imm

    }
}
void FunctionX64Linux::loadImm64(int64 imm) {
    //emit(0x68); emitValue(imm); // push imm
    prepareParam(imm, sizeof(imm));
}
void FunctionX64Linux::loadLiteralStr(const string& literalStr) {
    const char* loc = m_parent->_getLiteralStringLoc(literalStr);
    //emit(0x68); emitValue(loc); // push loc, wxf, how to fix?
    prepareParam((int64)loc, sizeof(loc));
}

void FunctionX64Linux::loadLocal(int idx) {
    int offset = localIdx2EbpOff(idx);
    if (m_beginCall) {
        //for windows
        if (m_paramIndex == 0) { //rcx
            emit(0x48, 0x8b, 0x8d); emitValue(offset); // mov rcx, [rbp + offset32]
        }
        else if (m_paramIndex == 1) { //rdx
            emit(0x48, 0x8b, 0x95); emitValue(offset); // mov rdx, [rbp + offset32]
        }
        else if (m_paramIndex == 2) { //r8
            emit(0x4c, 0x8b, 0x85); emitValue(offset); // mov r8, [rbp + offset32]
        }
        else if (m_paramIndex == 3) { //r9
            emit(0x4c, 0x8b, 0x8d); emitValue(offset); // mov r9, [rbp + offset32]
        }
        else {
            emit(0xff, 0xb5); emitValue(offset); // push qword ptr [rbp + idxOff]
        }
    }
    else {
        if (m_paramIndex == 0) { //rcx
            emit(0x51); // push rcx
        }
        else if (m_paramIndex == 1) { //rdx
            emit(0x52); // push rdx
        }
        else if (m_paramIndex == 2) { //r8
            emit(0x41, 0x50); // push r8
        }
        else if (m_paramIndex == 3) { //r9
            emit(0x41, 0x51); // push r9
        }
        else {
            emit(0xff, 0xb5); emitValue(offset); // push qword ptr [rbp + idxOff]
        }
    }
    m_paramIndex++;

}
void FunctionX64Linux::storeLocal(int idx) {
    emit(0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(0x48, 0x89, 0x85); emitValue(localIdx2EbpOff(idx)); // mov qword ptr [rbp + idxOff], rax
    emit(0x48, 0x83, 0xc4); emitValue((char)8); // add rsp, 8
}
void FunctionX64Linux::incLocal(int idx) {
    emit(0x48, 0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc qword ptr [rbp + idxOff]
}
void FunctionX64Linux::decLocal(int idx) {
    emit(0x48, 0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec qword ptr [rbp + idxOff]
}
#endif

