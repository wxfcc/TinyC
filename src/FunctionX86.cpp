#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "JITEngine.h"

FunctionX86::FunctionX86(JITEngine* parent, char* codeBuf) : Function(parent, codeBuf) {
}
FunctionX86::~FunctionX86() {
}

void FunctionX86::beginBuild() {
    push_edx(); // emit(0x52); // push edx
    push_ebp(); // emit(0x55); // push ebp
    mov_ebp_esp();  //emit(0x8b, 0xec); // mov ebp, esp
    sub_esp(MAX_LOCAL_COUNT * 4);  //emit(0x81, 0xec); emitValue(MAX_LOCAL_COUNT * 4); // sub esp, MAX_LOCAL_COUNT * 4
}
void FunctionX86::endBuild() {
    markLabel(&m_retLabel);
    mov_esp_ebp();//emit(0x8b, 0xe5);  // mov esp,ebp 
    pop_ebp();  //emit(0x5d); // pop ebp  
    pop_edx(); //emit(0x5a); // pop edx  
    retn(); //emit(0xc3); // ret
}
void FunctionX86::prepareParam(int64 paraVal, int size) {

}
void FunctionX86::loadImm(int imm) {
    push_32(imm);//emit(0x68); emitValue(imm); // push imm
}
void FunctionX86::loadLiteralStr(const string& literalStr) {
    const char* loc = m_parent->_getLiteralStringLoc(literalStr);
    push_32(loc);// emit(0x68); emitValue(loc); // push loc
}
void FunctionX86::loadLocal(int idx) {
    push_dword_ptr_ebp(localIdx2EbpOff(idx)); // emit(0xff, 0xb5); emitValue(localIdx2EbpOff(idx)); // push dword ptr [ebp + idxOff]
}
void FunctionX86::storeLocal(int idx) {
    mov_eax_dword_ptr_esp0();// emit(0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
    mov_dword_ptr_ebp_eax(localIdx2EbpOff(idx)); // emit(0x89, 0x85); emitValue(localIdx2EbpOff(idx)); // mov dword ptr [ebp + idxOff], eax
    add_esp8(4); // emit(0x83, 0xc4); emitValue((char)4); // add esp, 4
}
void FunctionX86::incLocal(int idx) {
    inc_dword_ptr_ebp(localIdx2EbpOff(idx));//emit(0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc dword ptr [ebp + idxOff]
}
void FunctionX86::decLocal(int idx) {
    dec_dword_ptr_ebp(localIdx2EbpOff(idx));//emit(0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec dword ptr [ebp + idxOff]
}
void FunctionX86::pop(int n) {
    add_esp(n * 4);//emit(0x81, 0xc4); emitValue(n * 4); // add esp, n * 4
}
void FunctionX86::dup() {
    push_dword_ptr_esp0();//emit(0xff, 0x34, 0x24); // push dword ptr [esp]
}

void FunctionX86::doArithmeticOp(TokenID opType) {
    mov_eax_dword_ptr_esp8(4);//emit(0x8b, 0x44, 0x24, 0x04); // mov eax, dword ptr [esp+4]
    switch (opType) {
    case TID_OP_ADD:
        add_eax_dword_ptr_esp0();//emit(0x03, 0x04, 0x24); // add eax, dword ptr [esp]
        break;
    case TID_OP_SUB:
        sub_eax_dword_ptr_esp0();//emit(0x2b, 0x04, 0x24); // sub eax, dword ptr [esp]
        break;
    case TID_OP_MUL:
        imul_eax_dword_ptr_esp0();//emit(0x0f, 0xaf, 0x04, 0x24); // imul eax, dword ptr [esp]
        break;
    case TID_OP_DIV:
    case TID_OP_MOD:
        xor_edx_edx();// emit(0x33, 0xd2); // xor edx, edx
        idiv_dword_ptr_esp0();//emit(0xf7, 0x3c, 0x24); // idiv dword ptr [esp]
        if (opType == TID_OP_MOD) {
            mov_eax_edx();//emit(0x8b, 0xc2); // mov eax, edx
        }
        break;
    default: ASSERT(0); break;
    }
    mov_dword_ptr_esp_eax(4);   //emit(0x89, 0x44, 0x24, 0x04); // mov dword ptr [esp+4], eax
    add_esp8(4);    // emit(0x83, 0xc4, 0x04); // add esp, 4
}
void FunctionX86::cmp(TokenID cmpType) {
    Label label_1, label_0, label_end;
    mov_eax_dword_ptr_esp8(4);//emit(0x8b, 0x44, 0x24, 0x04); // mov eax, dword ptr [esp+4] 
    mov_edx_dword_ptr_esp0();//emit(0x8b, 0x14, 0x24); // mov edx, dword ptr[esp]
    add_esp(8);//emit(0x83, 0xc4); emitValue((char)8);// add esp, 8
    cmp_eax_edx();//emit(0x3b, 0xc2); // cmp eax, edx
    condJmp(cmpType, &label_1);
    jmp(&label_0);
    markLabel(&label_1);
    push_8(1);//emit(0x6a, 0x01); // push 1
    jmp(&label_end);
    markLabel(&label_0);
    push_8(0);//emit(0x6a, 0x00); // push 0
    markLabel(&label_end);
}

void FunctionX86::markLabel(Label* label) {
    label->mark(m_codeBuf + m_codeSize);
}
void FunctionX86::jmp(Label* label) {
    emit(0xe9);
    char* ref = m_codeBuf + m_codeSize;
    emitValue(NULL);
    label->addRef(ref);
}
void FunctionX86::trueJmp(Label* label) {
    emit(0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
    emit(0x83, 0xc4, 0x04); // add esp, 4
    emit(0x85, 0xc0); // test eax, eax
    condJmp(TID_OP_NEQUAL, label);
}
void FunctionX86::falseJmp(Label* label) {
    emit(0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
    emit(0x83, 0xc4, 0x04); // add esp, 4
    emit(0x85, 0xc0); // test eax, eax
    condJmp(TID_OP_EQUAL, label);
}
void FunctionX86::ret() {
    jmp(&m_retLabel);
}
void FunctionX86::retExpr() {
    emit(0x8b, 0x04, 0x24); // mov eax, dword ptr [esp]
    emit(0x83, 0xc4, 0x04); // add esp, 4
    jmp(&m_retLabel);
}

int FunctionX86::beginCall() {
    return 0;
}
void FunctionX86::endCall(const string& funcName, int callID, int paramCount) {
    char** entry = m_parent->_getFunctionEntry(funcName);
    for (int i = 0; i < paramCount - 1; ++i) {
        emit(0xff, 0xb4, 0x24); emitValue(((i + 1) * 2 - 1) * 4); // push dword ptr [esp+4*i]
    }
    emit(0xff, 0x15); emitValue(entry); // call [entry]
    pop(paramCount + (paramCount > 0 ? paramCount - 1 : 0));
    emit(0x50); // push eax
}

void FunctionX86::condJmp(TokenID tid, Label* label) {
    switch ((int)tid) {
    case TID_OP_LESS:       emit(0x0f, 0x8c); break;
    case TID_OP_LESSEQ:     emit(0x0f, 0x8e); break;
    case TID_OP_GREATER:    emit(0x0f, 0x8f); break;
    case TID_OP_GREATEREQ:  emit(0x0f, 0x8d); break;
    case TID_OP_EQUAL:      emit(0x0f, 0x84); break;
    case TID_OP_NEQUAL:     emit(0x0f, 0x85); break;
    }
    char* ref = m_codeBuf + m_codeSize;
    emitValue(NULL);
    label->addRef(ref);
}

int FunctionX86::localIdx2EbpOff(int idx) {
    return idx < 0 ? 8 - idx * 4 : -(1 + idx) * 4;
}

