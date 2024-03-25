#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#include "JITEngine.h"

x64FunctionBuilder::x64FunctionBuilder(JITEngine* parent, char* codeBuf) : FunctionBuilder(parent, codeBuf) {
    m_paramCount = 0;
    m_beginCall = 0;
}

void x64FunctionBuilder::beginBuild() {
    emit(1, 0x52); // push rdx
    emit(1, 0x55); // push rbp
    emit(3, 0x48, 0x89, 0xe5); // mov rbp, rsp
    emit(3, 0x48, 0x81, 0xec); emitValue(MAX_LOCAL_COUNT * 8); // sub rsp, MAX_LOCAL_COUNT * 8
}
void x64FunctionBuilder::endBuild() {
    markLabel(&m_retLabel);
    emit(3, 0x48, 0x89, 0xec);  // mov rsp,rbp 
    emit(1, 0x5d); // pop rbp  
    emit(1, 0x5a); // pop rdx  
    emit(1, 0xc3); // ret
}
/*
 in windows x64 mode, the order of parameter: rcx rdx r8 r9 [rsp] ...
 48 B8 #imm64	mov rax, #imm64
 48 B9 #imm64	mov rcx, #imm64
 48 BA #imm64	mov rdx, #imm64
 48 BB #imm64	mov rbx, #imm64
 48 BC #imm64	mov rsp, #imm64
 48 BD #imm64	mov rbp, #imm64
 48 BE #imm64	mov rsi, #imm64
 48 BF #imm64	mov rdi, #imm64
 49 B8 #imm64	mov r8,	 #imm64
 49 B9 #imm64	mov r9,  #imm64
 49 BA #imm64	mov r10, #imm64
 49 BB #imm64	mov r11, #imm64
 49 BC #imm64	mov r12, #imm64
 49 BD #imm64	mov r13, #imm64
 49 BE #imm64	mov r14, #imm64
 49 BF #imm64	mov r15, #imm64
*/
void x64FunctionBuilder::prepareParamForWindows(int64 paraVal, int size) {
    if (m_paramCount == 0) { //rcx
        emit(2, 0x48, 0xb9); emitValue((int64)paraVal); // mov rcx, #imm64
    }
    else if (m_paramCount == 1) { //rdx
        emit(2, 0x48, 0xba); emitValue((int64)paraVal); // mov rdx, #imm64
    }
    else if (m_paramCount == 2) { //r8
        emit(2, 0x49, 0xb8); emitValue((int64)paraVal); // mov r8, #imm64
    }
    else if (m_paramCount == 3) { //r9
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
}
/*
 in linux x64 mode, the order of parameter: rdi rsi rdx rcx r8 r9 [rsp] ...
*/
void x64FunctionBuilder::prepareParamForLinux(int64 paraVal, int size) {
    if (m_paramCount == 0) { 
        emit(2, 0x48, 0xbf); emitValue((int64)paraVal); // mov rdi, #imm64
    }
    else if (m_paramCount == 1) { 
        emit(2, 0x48, 0xbe); emitValue((int64)paraVal); // mov rsi, #imm64
    }
    else if (m_paramCount == 2) { 
        emit(2, 0x48, 0xba); emitValue((int64)paraVal); // mov rdx, #imm64
    }
    else if (m_paramCount == 3) { 
        emit(2, 0x48, 0xb9); emitValue((int64)paraVal); // mov rcx, #imm64
    }
    else if (m_paramCount == 4) { 
        emit(2, 0x49, 0xb8); emitValue((int64)paraVal); // mov r8, #imm64
    }
    else if (m_paramCount == 5) { 
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
}
void x64FunctionBuilder::prepareParam(int64 paraVal, int size) {
#ifdef _WIN32
    prepareParamForWindows(paraVal, size);
#else
    prepareParamForLinux(paraVal, size);
#endif
    m_paramCount++;
}
void x64FunctionBuilder::loadImm(int imm) {
    if(m_beginCall)
        prepareParam(imm, sizeof(imm));
    else {
        emit(1, 0x68); emitValue(imm); // push imm

    }
}
void x64FunctionBuilder::loadImm64(int64 imm) {
    //emit(1, 0x68); emitValue(imm); // push imm
    prepareParam(imm, sizeof(imm));
}
void x64FunctionBuilder::loadLiteralStr(const string& literalStr) {
    const char* loc = m_parent->_getLiteralStringLoc(literalStr);
    //emit(1, 0x68); emitValue(loc); // push loc, wxf, how to fix?
    prepareParam((int64)loc, sizeof(loc));
}
void x64FunctionBuilder::loadLocal(int idx) {
    emit(2, 0xff, 0xb5); emitValue(localIdx2EbpOff(idx)); // push qword ptr [rbp + idxOff]
}
void x64FunctionBuilder::storeLocal(int idx) {
    emit(4, 0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(3, 0x48, 0x89, 0x85); emitValue(localIdx2EbpOff(idx)); // mov qword ptr [rbp + idxOff], rax
    emit(3, 0x48, 0x83, 0xc4); emitValue((char)8); // add rsp, 8
}
void x64FunctionBuilder::incLocal(int idx) {
    emit(3, 0x48, 0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc qword ptr [rbp + idxOff]
}
void x64FunctionBuilder::decLocal(int idx) {
    emit(3, 0x48, 0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec qword ptr [rbp + idxOff]
}
void x64FunctionBuilder::pop(int n) {
    emit(3, 0x48, 0x81, 0xc4); emitValue(n * 8); // add rsp, n * 8
}
void x64FunctionBuilder::dup() {
    emit(3, 0xff, 0x34, 0x24); // push qword ptr [rsp]
}

void x64FunctionBuilder::doArithmeticOp(TokenID opType) {
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
void x64FunctionBuilder::cmp(TokenID cmpType) {
    Label label_1, label_0, label_end;
    emit(5, 0x48, 0x8b, 0x44, 0x24, 0x08); // mov rax, qword ptr [rsp+8] 
    emit(4, 0x48, 0x8b, 0x14, 0x24); // mov rdx, qword ptr[rsp]
    emit(4, 0x48, 0x83, 0xc4); emitValue((char)8);// add rsp, 8
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

void x64FunctionBuilder::markLabel(Label* label) {
    label->mark(m_codeBuf + m_codeSize); 
}
void x64FunctionBuilder::jmp(Label* label) {
    emit(1, 0xe9);
    char* ref = m_codeBuf + m_codeSize;
    emitValue(NULL);
    label->addRef(ref);
}
void x64FunctionBuilder::trueJmp(Label* label) {
    emit(4, 0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(4, 0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    emit(3, 0x48, 0x85, 0xc0); // test rax, rax
    condJmp(TID_OP_NEQUAL, label);
}
void x64FunctionBuilder::falseJmp(Label* label) {
    emit(4, 0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(4, 0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    emit(3, 0x48, 0x85, 0xc0); // test rax, rax
    condJmp(TID_OP_EQUAL, label);
}
void x64FunctionBuilder::ret() { 
    jmp(&m_retLabel); 
}
void x64FunctionBuilder::retExpr() {
    emit(4, 0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    emit(4, 0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    jmp(&m_retLabel);
}

int x64FunctionBuilder::beginCall() {
    m_paramCount = 0;
    m_beginCall = 1;
    return 0;
}

void x64FunctionBuilder::endCall(const string& funcName, int callID, int paramCount) {
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
    printf("funcPtr: %p, rip: %p\n", funcPtr, rip);
    if ( offset > max || offset < min) {
        int next = 8;       // skip funcPtr
        //emit(5, 0x48, 0x8b, 0x4c, 0x24, 0x08);    // mov rcx, [rsp+8]
        emit(1, 0xe9);      // jmp rip+offset32, this instruction should save in a special section?
        emitValue(next);
        emitValue(funcPtr);
        next = -8 - 6;
        emit(2, 0xff, 0x15); emitValue(next);
    }
    else {
        int val = (int)offset;
        printf("offset: %p, rip: 0x%x\n", offset, val);
        emit(1, 0xe8); emitValue(val);  // call rip+offset
    }
    pop(paramCount + (paramCount > 0 ? paramCount - 1 : 0));
    emit(1, 0x50); // push rax

    m_paramCount = 0;
    m_beginCall = 0;
}

void x64FunctionBuilder::emit(int n, ...) {
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; ++i) 
        m_codeBuf[m_codeSize++] = (char)va_arg(args, int);
    va_end(args);
}

template<typename T>
void x64FunctionBuilder::emitValue(T val) {
    memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
    m_codeSize += sizeof(val);
}
// relative rip
void x64FunctionBuilder::emitRelativeAddr32(char* absPos, int prefixLen) {
    char* rip = m_codeBuf + m_codeSize - prefixLen;
    int64 offset = (int64)(absPos - rip) - prefixLen - 4;  // prefixLen+4  = sizeof(call rip+offset)
    if (offset > INT_MAX) {
        //error
    }
    int val = (int)offset;
    memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
    m_codeSize += sizeof(val);
}


void x64FunctionBuilder::condJmp(TokenID tid, Label* label) {
    switch ((int)tid) {
    case TID_OP_LESS:       emit(2, 0x0f, 0x8c); break;
    case TID_OP_LESSEQ:     emit(2, 0x0f, 0x8e); break;
    case TID_OP_GREATER:    emit(2, 0x0f, 0x8f); break;
    case TID_OP_GREATEREQ:  emit(2, 0x0f, 0x8d); break;
    case TID_OP_EQUAL:      emit(2, 0x0f, 0x84); break;
    case TID_OP_NEQUAL:     emit(2, 0x0f, 0x85); break;
    }
    char* ref = m_codeBuf + m_codeSize;
    emitValue(NULL);
    label->addRef(ref);
}

int x64FunctionBuilder::localIdx2EbpOff(int idx) {
    return idx < 0 ? 8 - idx * 8 : -(1 + idx) * 8;
}

