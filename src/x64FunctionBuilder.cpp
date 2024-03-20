#include <stdio.h>
#include <stdarg.h>

#include "JITEngine.h"

x64FunctionBuilder::x64FunctionBuilder(JITEngine* parent, char* codeBuf) : FunctionBuilder(parent, codeBuf) {
}
FunctionBuilder* x64FunctionBuilder::newBuilder(JITEngine* parent, char* codeBuf) {
    FunctionBuilder* builder = new x64FunctionBuilder(parent, codeBuf);
    return builder;
}

void x64FunctionBuilder::beginBuild() {
    emit(3, 0x48, 0x31, 0xc0);
    emit(1, 0xc3); // ret,wxf
    emit(1, 0x52); // push edx
    emit(1, 0x55); // push ebp
    emit(2, 0x8b, 0xec); // mov ebp, esp
    emit(2, 0x81, 0xec); emitValue(MAX_LOCAL_COUNT * 4); // sub esp, MAX_LOCAL_COUNT * 4
}
void x64FunctionBuilder::endBuild() {
    markLabel(&m_retLabel);
    emit(2, 0x8b, 0xe5);  // mov esp,ebp 
    emit(1, 0x5d); // pop ebp  
    emit(1, 0x5a); // pop edx  
    emit(1, 0xc3); // ret
}

void x64FunctionBuilder::loadImm(int imm) {
    emit(1, 0x68); emitValue(imm); // push imm
}
void x64FunctionBuilder::loadLiteralStr(const string& literalStr) {
    const char* loc = m_parent->_getLiteralStringLoc(literalStr);
    emit(1, 0x68); emitValue(loc); // push loc
}
void x64FunctionBuilder::loadLocal(int idx) {
    emit(2, 0xff, 0xb5); emitValue(localIdx2EbpOff(idx)); // push dword ptr [ebp + idxOff]
}
void x64FunctionBuilder::storeLocal(int idx) {
    emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
    emit(2, 0x89, 0x85); emitValue(localIdx2EbpOff(idx)); // mov dword ptr [ebp + idxOff], eax
    emit(2, 0x83, 0xc4); emitValue((char)4); // add esp, 4
}
void x64FunctionBuilder::incLocal(int idx) {
    emit(2, 0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc dword ptr [ebp + idxOff]
}
void x64FunctionBuilder::decLocal(int idx) {
    emit(2, 0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec dword ptr [ebp + idxOff]
}
void x64FunctionBuilder::pop(int n) {
    emit(2, 0x81, 0xc4); emitValue(n * 4); // add esp, n * 4
}
void x64FunctionBuilder::dup() {
    emit(3, 0xff, 0x34, 0x24); // push dword ptr [esp]
}

void x64FunctionBuilder::doArithmeticOp(TokenID opType) {
    emit(4, 0x8b, 0x44, 0x24, 0x04); // mov eax, dword ptr [esp+4]
    switch (opType) {
    case TID_OP_ADD:
        emit(3, 0x03, 0x04, 0x24); // add eax, dword ptr [esp]
        break;
    case TID_OP_SUB:
        emit(3, 0x2b, 0x04, 0x24); // sub eax, dword ptr [esp]
        break;
    case TID_OP_MUL:
        emit(4, 0x0f, 0xaf, 0x04, 0x24); // imul eax, dword ptr [esp]
        break;
    case TID_OP_DIV:
    case TID_OP_MOD:
        emit(2, 0x33, 0xd2); // xor edx, edx
        emit(3, 0xf7, 0x3c, 0x24); // idiv dword ptr [esp]
        if (opType == TID_OP_MOD) {
            emit(2, 0x8b, 0xc2); // mov eax, edx
        }
        break;
    default: ASSERT(0); break;
    }
    emit(4, 0x89, 0x44, 0x24, 0x04); // mov dword ptr [esp+4], eax
    emit(3, 0x83, 0xc4, 0x04); // add esp, 4
}
void x64FunctionBuilder::cmp(TokenID cmpType) {
    Label label_1, label_0, label_end;
    emit(4, 0x8b, 0x44, 0x24, 0x04); // mov eax, dword ptr [esp+4] 
    emit(3, 0x8b, 0x14, 0x24); // mov edx, dword ptr[esp]
    emit(2, 0x83, 0xc4); emitValue((char)8);// add esp, 8
    emit(2, 0x3b, 0xc2); // cmp eax, edx
    condJmp(cmpType, &label_1);
    jmp(&label_0);
    markLabel(&label_1);
    emit(2, 0x6a, 0x01); // push 1
    jmp(&label_end);
    markLabel(&label_0);
    emit(2, 0x6a, 0x00); // push 0
    markLabel(&label_end);
}

void x64FunctionBuilder::markLabel(Label* label) { label->mark(m_codeBuf + m_codeSize); }
void x64FunctionBuilder::jmp(Label* label) {
    emit(1, 0xe9);
    char* ref = m_codeBuf + m_codeSize;
    emitValue(NULL);
    label->addRef(ref);
}
void x64FunctionBuilder::trueJmp(Label* label) {
    emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
    emit(3, 0x83, 0xc4, 0x04); // add esp, 4
    emit(2, 0x85, 0xc0); // test eax, eax
    condJmp(TID_OP_NEQUAL, label);
}
void x64FunctionBuilder::falseJmp(Label* label) {
    emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
    emit(3, 0x83, 0xc4, 0x04); // add esp, 4
    emit(2, 0x85, 0xc0); // test eax, eax
    condJmp(TID_OP_EQUAL, label);
}
void x64FunctionBuilder::ret() { jmp(&m_retLabel); }
void x64FunctionBuilder::retExpr() {
    emit(3, 0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
    emit(3, 0x83, 0xc4, 0x04); // add esp, 4
    jmp(&m_retLabel);
}

int x64FunctionBuilder::beginCall() {
    return 0;
}
void x64FunctionBuilder::endCall(const string& funcName, int callID, int paramCount) {
    char** entry = m_parent->_getFunctionEntry(funcName);
    for (int i = 0; i < paramCount - 1; ++i) {
        emit(3, 0xff, 0xb4, 0x24); emitValue(((i + 1) * 2 - 1) * 4); // push dword ptr [esp+4*i]
    }
    emit(2, 0xff, 0x15); emitValue(entry); // call [entry]
    pop(paramCount + (paramCount > 0 ? paramCount - 1 : 0));
    emit(1, 0x50); // push eax
}

void x64FunctionBuilder::emit(int n, ...) {
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; ++i) m_codeBuf[m_codeSize++] = (char)va_arg(args, int);
    va_end(args);
}

template<typename T>
void x64FunctionBuilder::emitValue(T val) {
    memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
    m_codeSize += sizeof(val);
}

void x64FunctionBuilder::condJmp(TokenID tid, Label* label) {
    switch ((int)tid) {
    case TID_OP_LESS: emit(2, 0x0f, 0x8c); break;
    case TID_OP_LESSEQ: emit(2, 0x0f, 0x8e); break;
    case TID_OP_GREATER: emit(2, 0x0f, 0x8f); break;
    case TID_OP_GREATEREQ: emit(2, 0x0f, 0x8d); break;
    case TID_OP_EQUAL: emit(2, 0x0f, 0x84); break;
    case TID_OP_NEQUAL: emit(2, 0x0f, 0x85); break;
    }
    char* ref = m_codeBuf + m_codeSize;
    emitValue(NULL);
    label->addRef(ref);
}

int x64FunctionBuilder::localIdx2EbpOff(int idx) {
    return idx < 0 ? 8 - idx * 4 : -(1 + idx) * 4;
}

