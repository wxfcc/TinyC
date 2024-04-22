#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#include "JITEngine.h"

FunctionX64::FunctionX64(JITEngine* parent, char* codeBuf) : Function(parent, codeBuf) {
}
FunctionX64::~FunctionX64() {
    printf("FunctionX64: %s, param count=%d\n", m_funcName.c_str(), m_paramCount);
    //printf("FunctionX64: %s, local variable count=%d\n", m_funcName.c_str(), m_localVarCount);
}
//call from JITEngine
//generate prologue code
void FunctionX64::beginBuild() {
    emit(1, 0x52); // push rdx
    emit(1, 0x55); // push rbp
    emit(3, 0x48, 0x89, 0xe5); // mov rbp, rsp
    emit(3, 0x48, 0x81, 0xec); emitValue((MAX_LOCAL_COUNT -1)* 8); // sub rsp, MAX_LOCAL_COUNT * 8, there should keep rsp align 16 bytes for some instructions, like: movaps XMMWORD PTR [rsp+0x10], xmm1
}
//call from JITEngine
//generate epilogue code
void FunctionX64::endBuild() {
    markLabel(&m_retLabel);
    emit(3, 0x48, 0x89, 0xec);  // mov rsp,rbp 
    emit(1, 0x5d); // pop rbp  
    emit(1, 0x5a); // pop rdx  
    emit(1, 0xc3); // ret
}
/*
 in windows x64 mode, the order of parameter: rcx rdx r8 r9 [rsp] ...
*/
void FunctionX64::prepareParam(int64 paraVal, int size) {
    printf("FunctionX64Linux::prepareParam\n");

    if (m_paramIndex == 0) { //rcx
        emit(2, 0x48, 0xb9); emitValue((int64)paraVal); // mov rcx, #imm64
    }
    else if (m_paramIndex == 1) { //rdx
        emit(2, 0x48, 0xba); emitValue((int64)paraVal); // mov rdx, #imm64
    }
    else if (m_paramIndex == 2) { //r8
        emit(2, 0x49, 0xb8); emitValue((int64)paraVal); // mov r8, #imm64
    }
    else if (m_paramIndex == 3) { //r9
        emit(2, 0x49, 0xb9); emitValue((int64)paraVal); // mov r9, #imm64
    }
    else {
        if (size <= 4) {
            emit(1, 0x68); emitValue((int)paraVal); // push #imm32, in fact it'll push #imm64 with the hi 32 bits as 0 
        }
        else {
            emit(2, 0x48, 0xb8); emitValue((int64)paraVal); // mov #rax, #imm64
            emit(1, 0x50); // push rax
            // 48 C7 44 24 20 04 00 00 00             mov    qword ptr [rsp+20h], 4 
            // 48 c7 84 24 20 01 00 00 00 00 00 00    mov    QWORD PTR [rsp + 0x120], 0x0

        }
    }
    m_paramIndex++;
}

void FunctionX64::loadImm(int imm) {
    if(m_beginCall)
        prepareParam(imm, sizeof(imm));
    else {
        emit(1, 0x68); emitValue(imm); // push imm

    }
}
void FunctionX64::loadImm64(int64 imm) {
    //emit(1, 0x68); emitValue(imm); // push imm
    prepareParam(imm, sizeof(imm));
}
void FunctionX64::loadLiteralStr(const string& literalStr) {
    const char* loc = m_parent->_getLiteralStringLoc(literalStr);
    //emit(1, 0x68); emitValue(loc); // push loc, wxf, how to fix?
    prepareParam((int64)loc, sizeof(loc));
}

void FunctionX64::loadLocal(int idx) {
    int offset = localIdx2EbpOff(idx);
    if (m_beginCall) {
        //for windows
        if (m_paramIndex == 0) { //rcx
            emit(3, 0x48, 0x8b, 0x8d); emitValue(offset); // mov rcx, [rbp + offset32]
        }
        else if (m_paramIndex == 1) { //rdx
            emit(3, 0x48, 0x8b, 0x95); emitValue(offset); // mov rdx, [rbp + offset32]
        }
        else if (m_paramIndex == 2) { //r8
            emit(3, 0x4c, 0x8b, 0x85); emitValue(offset); // mov r8, [rbp + offset32]
        }
        else if (m_paramIndex == 3) { //r9
            emit(3, 0x4c, 0x8b, 0x8d); emitValue(offset); // mov r9, [rbp + offset32]
        }
        else {
            emit(2, 0xff, 0xb5); emitValue(offset); // push qword ptr [rbp + idxOff]
        }
    }
    else {
        if (m_paramIndex == 0) { //rcx
            emit(1, 0x51); // push rcx
        }
        else if (m_paramIndex == 1) { //rdx
            emit(1, 0x52); // push rdx
        }
        else if (m_paramIndex == 2) { //r8
            emit(2, 0x41, 0x50); // push r8
        }
        else if (m_paramIndex == 3) { //r9
            emit(2, 0x41, 0x51); // push r9
        }
        else {
            emit(2, 0xff, 0xb5); emitValue(offset); // push qword ptr [rbp + idxOff]
        }
    }
    m_paramIndex++; //??
}

void FunctionX64::storeLocal(int idx) {
    emit(4, 0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(3, 0x48, 0x89, 0x85); emitValue(localIdx2EbpOff(idx)); // mov qword ptr [rbp + idxOff], rax
    emit(3, 0x48, 0x83, 0xc4); emitValue((char)8); // add rsp, 8
}
void FunctionX64::incLocal(int idx) {
    emit(3, 0x48, 0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc qword ptr [rbp + idxOff]
}
void FunctionX64::decLocal(int idx) {
    emit(3, 0x48, 0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec qword ptr [rbp + idxOff]
}
void FunctionX64::pop(int n) {
    emit(3, 0x48, 0x81, 0xc4); emitValue(n * 8); // add rsp, n * 8
}
void FunctionX64::dup() {
    emit(3, 0xff, 0x34, 0x24); // push qword ptr [rsp]
}

void FunctionX64::doArithmeticOp(TokenID opType) {
    emit(5, 0x48, 0x8b, 0x44, 0x24, 0x08); // mov rax, qword ptr [rsp+8]
    switch (opType) {
    case TID_OP_ADD:
        emit(4, 0x48, 0x03, 0x04, 0x24); // add rax, qword ptr [rsp]
        break;
    case TID_OP_SUB:
        emit(4, 0x48, 0x2b, 0x04, 0x24); // sub rax, qword ptr [rsp]
        break;
    case TID_OP_MUL:
        emit(5, 0x48, 0x0f, 0xaf, 0x04, 0x24); // imul rax, qword ptr [rsp]
        break;
    case TID_OP_DIV:
    case TID_OP_MOD:
        emit(3, 0x48, 0x31, 0xd2); // xor rdx, rdx
        emit(4, 0x48, 0xf7, 0x3c, 0x24); // idiv qword ptr [rsp]
        if (opType == TID_OP_MOD) {
            emit(3, 0x48, 0x89, 0xd0); // mov rax, rdx
        }
        break;
    default: ASSERT(0); break;
    }
    emit(5, 0x48, 0x89, 0x44, 0x24, 0x08); // mov qword ptr [rsp+8], rax
    emit(4, 0x48, 0x83, 0xc4, 0x08); // add rsp, 8
}
void FunctionX64::cmp(TokenID cmpType) {
    Label label_1, label_0, label_end;
    emit(5, 0x48, 0x8b, 0x44, 0x24, 0x08); // mov rax, qword ptr [rsp+8] 
    emit(4, 0x48, 0x8b, 0x14, 0x24); // mov rdx, qword ptr[rsp]
    emit(4, 0x48, 0x83, 0xc4, 0x08); //emitValue((char)8);// add rsp, 8
    emit(3, 0x48, 0x39, 0xd0); // cmp rax, rdx
    condJmp(cmpType, &label_1);
    jmp(&label_0);
    markLabel(&label_1);
    emit(2, 0x6a, 0x01); // push 1
    jmp(&label_end);
    markLabel(&label_0);
    emit(2, 0x6a, 0x00); // push 0
    markLabel(&label_end);
}

void FunctionX64::markLabel(Label* label) {
    label->mark(m_codeBuf + m_codeSize); 
}
void FunctionX64::jmp(Label* label) {
    emit(1, 0xe9);                      // jmp rip+offset32
    char* ref = m_codeBuf + m_codeSize;
    emitValue((int)NULL);
    label->addRef(ref);
}
void FunctionX64::trueJmp(Label* label) {
    emit(4, 0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(4, 0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    emit(3, 0x48, 0x85, 0xc0); // test rax, rax
    condJmp(TID_OP_NEQUAL, label);
}
void FunctionX64::falseJmp(Label* label) {
    emit(4, 0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(4, 0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    emit(3, 0x48, 0x85, 0xc0); // test rax, rax
    condJmp(TID_OP_EQUAL, label);
}
void FunctionX64::ret() { 
    jmp(&m_retLabel); 
}
void FunctionX64::retExpr() {
    emit(4, 0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(4, 0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    jmp(&m_retLabel);
}
//call from FunctionParser, begin prepare parametes for call
//48 89 4d 00             mov    QWORD PTR [rbp+0x0],rcx
//48 89 4d 10             mov    QWORD PTR [rbp+0x10],rcx
//48 89 55 10             mov    QWORD PTR [rbp+0x10],rdx
//48 89 5d 10             mov    QWORD PTR [rbp+0x10],rbx
//48 89 8d 00 01 00 00    mov    QWORD PTR [rbp+0x100],rcx
//48 89 95 00 01 00 00    mov    QWORD PTR [rbp+0x100],rdx
int FunctionX64::beginCall() {
    m_beginCall = 1;
    if (m_paramCount > 0) {
        emit(4, 0x48, 0x89, 0x4d, 0x10);//          mov    QWORD PTR[rsp + 0x08], rcx
    }
    if (m_paramCount > 1) {
        emit(4, 0x48, 0x89, 0x55, 0x18);//          mov    QWORD PTR[rsp + 0x10], rdx
    }
    return 0;
}
/*
//call from FunctionParser
 1. call rip+offset32
 2. call [rip+offset32]
 3. call rax
 4. call [rax], when func ptr too far
    call [rsp+0x8]
    call [rbp+0x8]
 */
void FunctionX64::endCall(const string& funcName, int callID, int paramCount) {
    char** entry = m_parent->_getFunctionEntry(funcName);
    for (int i = 0; i < paramCount - 1; ++i) {
        //emit(3, 0xff, 0xb4, 0x24); emitValue(((i + 1) * 2 - 1) * 8); // push qword ptr [rsp+8*i]
    }
    //emit(2, 0xff, 0x15); emitValue(entry); // call [entry]  #call   QWORD PTR [rip+entry] 
    char* rip = m_codeBuf + m_codeSize ;
    char* funcPtr = *entry;
    int64 offset = (int64)(funcPtr - rip) - 5;  // 5 = sizeof(call rip+offset)
    int64 max = INT_MAX;
    int64 min = INT_MIN;
    printf("endCall, funcName: %-16s: paramCount=%d, address: %p, rip: %p\n", funcName.c_str(), paramCount, funcPtr, rip);
#if 0
    if ( offset > max || offset < min) {
    }
    else 
    {
        int val = (int)offset;
        printf("offset: %llx, rip: 0x%x\n", offset, val);
        emit(1, 0xe8); emitValue(val);  // call rip+offset
    }
#else
    offset = (int64)((char*)entry - rip)-6;
    if (offset > max || offset < min) {
        printf("offset too large, %llx\n", offset);
        int next = 8;                           // skip funcPtr
        emit(1, 0xe9); emitValue(next);         // jmp rip+offset32, this instruction should save in a special section?
        emitValue(funcPtr);                     // the func ptr to be call, should be saved in a special section?
        offset = -8 - 6;
    }
    
    emit(2, 0xff, 0x15); emitValue((int)offset); // call qword ptr[rip+offset]
#endif
    pop(paramCount + (paramCount > 0 ? paramCount - 1 : 0));
    emit(1, 0x50); // push rax

    m_paramIndex = 0;
    m_beginCall = 0;
    //m_paramCount = paramCount;
}

// relative rip
void FunctionX64::emitRelativeAddr32(char* absPos, int prefixLen) {
    char* rip = m_codeBuf + m_codeSize - prefixLen;
    int64 offset = (int64)(absPos - rip) - prefixLen - 4;  // prefixLen+4  = sizeof(call rip+offset)
    if (offset > INT_MAX) {
        //error
    }
    int val = (int)offset;
    memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
    m_codeSize += sizeof(val);
}


void FunctionX64::condJmp(TokenID tid, Label* label) {
    switch ((int)tid) {
    case TID_OP_LESS:       emit(2, 0x0f, 0x8c); break;     // jl
    case TID_OP_LESSEQ:     emit(2, 0x0f, 0x8e); break;     // jle
    case TID_OP_GREATER:    emit(2, 0x0f, 0x8f); break;     // jg
    case TID_OP_GREATEREQ:  emit(2, 0x0f, 0x8d); break;     // jge
    case TID_OP_EQUAL:      emit(2, 0x0f, 0x84); break;     // je
    case TID_OP_NEQUAL:     emit(2, 0x0f, 0x85); break;     // jne
    }
    char* ref = m_codeBuf + m_codeSize;
    emitValue(NULL);
    label->addRef(ref); // 64bit ??
}

int FunctionX64::localIdx2EbpOff(int idx) {
    return idx < 0 ? 8 - idx * 8 : -(1 + idx) * 8;
}

