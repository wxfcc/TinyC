#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include "FunctionX64.h"
#include "JITEngine.h"

FunctionX64::FunctionX64(JITEngine* parent, char* codeBuf) : Function(parent, codeBuf) {
}
FunctionX64::~FunctionX64() {
    printf("FunctionX64: %s, param count=%d\n", m_funcName.c_str(), m_myParamCount);
    //printf("FunctionX64: %s, local variable count=%d\n", m_funcName.c_str(), m_localVarCount);
}
//call from JITEngine
//generate prologue code
void FunctionX64::beginBuild() {
    //push_rdx();// emit(0x52); // push rdx
    push_rbp();// emit(0x55); // push rbp
    mov_rbp_rsp(); // emit(0x48, 0x89, 0xe5); // mov rbp, rsp
    sub_rsp((MAX_LOCAL_COUNT - 0) * 8); // emit(0x48, 0x81, 0xec); emitValue((MAX_LOCAL_COUNT -1)* 8);
                                        // sub rsp, MAX_LOCAL_COUNT * 8, there should keep rsp align 16 bytes for some instructions,
                                        // like: movaps XMMWORD PTR [rsp+0x10], xmm1
    //lea_rbp_rsp_8(0x40);
}
//call from JITEngine
//generate epilogue code
void FunctionX64::endBuild() {
    if(m_retLabel.removeRef(m_codeBuf + m_codeSize - sizeof(int))){ // remove unnecessary jmp
        m_codeSize -= 5;    //sizeof jmp offset32
    }
    markLabel(&m_retLabel);
    mov_rsp_rbp();  // emit(0x48, 0x89, 0xec);  // mov rsp,rbp 
    //add_rsp((MAX_LOCAL_COUNT - 1) * 8);
    pop_rbp();  // emit(0x5d); // pop rbp  
    //pop_rdx();  // emit(0x5a); // pop rdx  
    retn(); // emit(0xc3); // ret
}
/*
 in windows x64 mode, the order of parameter: rcx rdx r8 r9 [rsp] ...
*/
void FunctionX64::prepareParam(int64 paraVal, int size) {
    //printf("FunctionX64::prepareParam\n");

}

void FunctionX64::loadImm(int imm) {
    push_32(imm);   // emit(0x68); emitValue(imm); // push imm
}
void FunctionX64::loadImm64(int64 imm) {
    //prepareParam(imm, sizeof(imm));
}
void FunctionX64::loadLiteralStr(const string& literalStr) {
    const char* loc = m_parent->_getLiteralStringLoc(literalStr);
//    push_32((long long)loc);// emit(0x68); emitValue(loc); // push loc
    mov_rax_imm64((long long)loc);
    push_rax();
//    mov [rsp-8],rax
//    mov rax, 123
//    mov [rsp],rax
//    pop rax
}

void FunctionX64::loadLocal(int idx) {
    int offset = localIdx2EbpOff(idx);
    push_qword_ptr_rbp(offset);
}

void FunctionX64::storeLocal(int idx) {
    int offset = localIdx2EbpOff(idx);
    mov_rax_qword_ptr_rsp0(); //emit(0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    mov_qword_ptr_rbp_rax(offset); //emit(0x48, 0x89, 0x85); emitValue(offset); // mov qword ptr [rbp + idxOff], rax
    add_rsp(8); // emit(0x48, 0x83, 0xc4); emitValue((char)8); // add rsp, 8
}
void FunctionX64::incLocal(int idx) {
    inc_qword_ptr_rbp(localIdx2EbpOff(idx));//emit(0x48, 0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc qword ptr [rbp + idxOff]
}
void FunctionX64::decLocal(int idx) {
    dec_qword_ptr_rbp(localIdx2EbpOff(idx));//emit(0x48, 0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec qword ptr [rbp + idxOff]
}
void FunctionX64::pop(int n) {
    if(n != 0)
    	add_rsp(n*8);//emit(0x48, 0x81, 0xc4); emitValue(n * 8); // add rsp, n * 8
}
void FunctionX64::dup() {
    push_qword_ptr_rsp0();//emit(0xff, 0x34, 0x24); // push qword ptr [rsp]
}

void FunctionX64::doArithmeticOp(TokenID opType) {
    mov_rax_qword_ptr_rsp_8(8);  //emit(0x48, 0x8b, 0x44, 0x24, 0x08); // mov rax, qword ptr [rsp+8]
    switch (opType) {
    case TID_OP_ADD:
        add_rax_qword_ptr_rsp0();   //emit(0x48, 0x03, 0x04, 0x24); // add rax, qword ptr [rsp]
        break;
    case TID_OP_SUB:
        sub_rax_qword_ptr_rsp0();   //emit(0x48, 0x2b, 0x04, 0x24); // sub rax, qword ptr [rsp]
        break;
    case TID_OP_MUL:
        imul_rax_qword_ptr_rsp0();// emit(0x48, 0x0f, 0xaf, 0x04, 0x24); // imul rax, qword ptr [rsp]
        break;
    case TID_OP_DIV:
    case TID_OP_MOD:
        xor_rdx_rdx();          //emit(0x48, 0x31, 0xd2); // xor rdx, rdx
        idiv_qword_ptr_rsp0();  //emit(0x48, 0xf7, 0x3c, 0x24); // idiv qword ptr [rsp]
        if (opType == TID_OP_MOD) {
            mov_rax_rdx();      //emit(0x48, 0x89, 0xd0); // mov rax, rdx
        }
        break;
    default: ASSERT(0); break;
    }
    mov_qword_ptr_rsp_rax_8(8);   //emit(0x48, 0x89, 0x44, 0x24, 0x08); // mov qword ptr [rsp+8], rax
    add_rsp(8);                 //emit(0x48, 0x83, 0xc4, 0x08); // add rsp, 8
}
void FunctionX64::cmp(TokenID cmpType) {
    Label label_1, label_0, label_end;
    mov_rax_qword_ptr_rsp_8(8);  //emit(0x48, 0x8b, 0x44, 0x24, 0x08); // mov rax, qword ptr [rsp+8] 
    mov_rdx_qword_ptr_rsp0();   //emit(0x48, 0x8b, 0x14, 0x24); // mov rdx, qword ptr[rsp]
    add_rsp(8); //emit(0x48, 0x83, 0xc4, 0x08); //emitValue((char)8);// add rsp, 8
    cmp_rax_rdx();//emit(0x48, 0x39, 0xd0); // cmp rax, rdx
    condJmp(cmpType, &label_1);
    jmp(&label_0);
    markLabel(&label_1);
    push_8(1); //emit(0x6a, 0x01); // push 1
    jmp(&label_end);
    markLabel(&label_0);
    push_8(0); //emit(0x6a, 0x00); // push 0
    markLabel(&label_end);
}

void FunctionX64::markLabel(Label* label) {
    label->mark(m_codeBuf + m_codeSize); 
}
void FunctionX64::jmp(Label* label) {
    jmp_rip_offset32(0);
    char* ref = m_codeBuf + m_codeSize - sizeof(int);
    label->addRef(ref);
}
void FunctionX64::trueJmp(Label* label) {
    mov_rax_qword_ptr_rsp0();   //emit(0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    add_rsp(8);                 // emit(0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    test_rax_rax();             //emit(0x48, 0x85, 0xc0); // test rax, rax
    condJmp(TID_OP_NEQUAL, label);
}
void FunctionX64::falseJmp(Label* label) {
    mov_rax_qword_ptr_rsp0();   //emit(0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    add_rsp(8);                 // emit(0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    test_rax_rax();             //emit(0x48, 0x85, 0xc0); // test rax, rax
    condJmp(TID_OP_EQUAL, label);
}
void FunctionX64::ret() {
    jmp(&m_retLabel); 
}
void FunctionX64::retExpr() {
    mov_rax_qword_ptr_rsp0();   //emit(0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    add_rsp(8); // emit(0x48, 0x83, 0xc4, 0x08); // add rsp, 8
    jmp(&m_retLabel);
}
//begin call a function in current function
//call from FunctionParser, begin prepare parametes for call
int FunctionX64::beginCall() {
    m_beginCall = 1;
    return 0;
}
/*
* after call compelet, we need to do something, such as process return value ...
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
    for (int i = 0; i < paramCount - 1; ++i) {  // copy parameters
//        push_qword_ptr_rsp(((i + 1) * 2 - 1) * 8); // emit(0xff, 0xb4, 0x24); emitValue(((i + 1) * 2 - 1) * 8); // push qword ptr [rsp+8*i]
//        push_qword_ptr_rsp((i + 2) * 8);
    }

    mov_rax_imm64((long long)entry);
    call_qdowrd_ptr_rax();
//    call_qword_ptr_rip((long long)entry); //emit(0xff, 0x15); emitValue((int)offset); // call qword ptr[rip+offset]
//    pop(paramCount + (paramCount > 0 ? paramCount - 1 : 0));
    pop(paramCount);
    push_rax();//emit(0x50); // push rax

    m_callParamIndex = -1;
    m_beginCall = 0;
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
    if (tid == TID_OP_LESS) { jl(0); }
    else if (tid == TID_OP_LESSEQ) { jle(0); }
    else if (tid == TID_OP_GREATER) { jg(0); }
    else if (tid == TID_OP_GREATEREQ) { jge(0); }
    else if (tid == TID_OP_EQUAL) { je(0); }
    else if (tid == TID_OP_NEQUAL) { jne(0); }

    char* ref = m_codeBuf + m_codeSize - sizeof(int);
    label->addRef(ref); 
}

int FunctionX64::localIdx2EbpOff(int idx) {
    int offset = 0;
    int push_n = 1; //2 // push count in beginBuild
    if (idx < 0)
        offset = (push_n - idx) * 8;
    else
        offset = -(1 + idx) * 8;
    return offset;
}

void FunctionX64::saveParameters() {

}

