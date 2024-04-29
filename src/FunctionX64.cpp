#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

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
    sub_rsp((MAX_LOCAL_COUNT - 1) * 8); // emit(0x48, 0x81, 0xec); emitValue((MAX_LOCAL_COUNT -1)* 8); 
                                        // sub rsp, MAX_LOCAL_COUNT * 8, there should keep rsp align 16 bytes for some instructions, 
                                        // like: movaps XMMWORD PTR [rsp+0x10], xmm1
    lea_rbp_rsp8(0x40);
}
//call from JITEngine
//generate epilogue code
void FunctionX64::endBuild() {
    markLabel(&m_retLabel);
    //mov_rsp_rbp();  // emit(0x48, 0x89, 0xec);  // mov rsp,rbp 
    add_rsp((MAX_LOCAL_COUNT - 1) * 8);
    pop_rbp();  // emit(0x5d); // pop rbp  
    //pop_rdx();  // emit(0x5a); // pop rdx  
    retn(); // emit(0xc3); // ret
}
/*
 in windows x64 mode, the order of parameter: rcx rdx r8 r9 [rsp] ...
*/
void FunctionX64::prepareParam(int64 paraVal, int size) {
    //printf("FunctionX64::prepareParam\n");

    if (m_callParamIndex == 0) { //rcx
        mov_rcx_imm64(paraVal); //emit(0x48, 0xb9); emitValue((int64)paraVal); // mov rcx, #imm64
    }
    else if (m_callParamIndex == 1) { //rdx
        mov_rdx_imm64(paraVal); //emit(0x48, 0xba); emitValue((int64)paraVal); // mov rdx, #imm64
    }
    else if (m_callParamIndex == 2) { //r8
        mov_r8_imm64(paraVal); //emit(0x49, 0xb8); emitValue((int64)paraVal); // mov r8, #imm64
    }
    else if (m_callParamIndex == 3) { //r9
        mov_r9_imm64(paraVal); //emit(0x49, 0xb9); emitValue((int64)paraVal); // mov r9, #imm64
    }
    else {
        if (size <= 4) {
            push_32(paraVal);//emit(0x68); emitValue((int)paraVal); // push #imm32
        }
        else {
            mov_rax_imm64(paraVal);//emit(0x48, 0xb8); emitValue((int64)paraVal); // mov rax, #imm64
            push_rax(); // emit(0x50); // push rax
        }
    }
}

void FunctionX64::loadImm(int imm) {
    if(m_beginCall)
        prepareParam(imm, sizeof(imm));
    else {
        push_32(imm);   // emit(0x68); emitValue(imm); // push imm
    }
}
void FunctionX64::loadImm64(int64 imm) {
    prepareParam(imm, sizeof(imm));
}
void FunctionX64::loadLiteralStr(const string& literalStr) {
    const char* loc = m_parent->_getLiteralStringLoc(literalStr);
    prepareParam((int64)loc, sizeof(loc));
}

void FunctionX64::loadLocal(int idx) {
    int offset = localIdx2EbpOff(idx);
    //push_qword_ptr_rbp(offset);
    if (idx < 0 || offset < 0) {
        printf("loadLocal idx=%d, offset=%d, m_callParamIndex=%d\n", idx, offset, m_callParamIndex);
    }
#if 0
    if (m_callParamIndex == 0) { //rcx
        mov_rcx_rbp_offset32(offset); //emit(0x48, 0x8b, 0x8d); emitValue(offset); // mov rcx, [rbp + offset32]
        //push_rcx();
    }
    else if (m_callParamIndex == 1) { //rdx
        mov_rdx_rbp_offset32(offset); //emit(0x48, 0x8b, 0x95); emitValue(offset); // mov rdx, [rbp + offset32]
        //push_rdx();
    }
    else if (m_callParamIndex == 2) { //r8
        mov_r8_rbp_offset32(offset); //emit(0x4c, 0x8b, 0x85); emitValue(offset); // mov r8, [rbp + offset32]
    }
    else if (m_callParamIndex == 3) { //r9
        mov_r9_rbp_offset32(offset); //emit(0x4c, 0x8b, 0x8d); emitValue(offset); // mov r9, [rbp + offset32]
    }
    else {
        push_qword_ptr_rbp(offset); //emit(0xff, 0xb5); emitValue(offset); // push qword ptr [rbp + idxOff]
    }
#endif
#if 1
    if (m_beginCall) {
        //for windows
        if (m_callParamIndex == 0) { //rcx
            mov_rcx_rbp_offset32(offset); //emit(0x48, 0x8b, 0x8d); emitValue(offset); // mov rcx, [rbp + offset32]
        }
        else if (m_callParamIndex == 1) { //rdx
            mov_rdx_rbp_offset32(offset); //emit(0x48, 0x8b, 0x95); emitValue(offset); // mov rdx, [rbp + offset32]
        }
        else if (m_callParamIndex == 2) { //r8
            mov_r8_rbp_offset32(offset); //emit(0x4c, 0x8b, 0x85); emitValue(offset); // mov r8, [rbp + offset32]
        }
        else if (m_callParamIndex == 3) { //r9
            mov_r9_rbp_offset32(offset); //emit(0x4c, 0x8b, 0x8d); emitValue(offset); // mov r9, [rbp + offset32]
        }
        else {
            printf("loadLocal: m_callParamIndex=%d, idx=%d\n", m_callParamIndex,idx);
            push_qword_ptr_rbp(offset);//emit(0xff, 0xb5); emitValue(offset); // push qword ptr [rbp + idxOff]
        }
    }
    else {
        if (m_callParamIndex == 0) { //rcx
            push_rcx(); // emit(0x51); // push rcx
        }
        else if (m_callParamIndex == 1) { //rdx
            push_rdx(); // emit(0x52); // push rdx
        }
        else if (m_callParamIndex == 2) { //r8
            push_r8(); // emit(0x41, 0x50); // push r8
        }
        else if (m_callParamIndex == 3) { //r9
            push_r9(); // emit(0x41, 0x51); // push r9
        }
        else {
            printf("m_callParamIndex=%d\n", m_callParamIndex);
            push_qword_ptr_rbp(offset); //emit(0xff, 0xb5); emitValue(offset); // push qword ptr [rbp + idxOff]
        }
    }
#endif
}

void FunctionX64::storeLocal(int idx) {
    int offset = localIdx2EbpOff(idx);
#if 1
    pop_rax();  //test
    mov_qword_ptr_rbp_rax(offset); //emit(0x48, 0x89, 0x85); emitValue(offset); // mov qword ptr [rbp + idxOff], rax
#else
    mov_rax_qword_ptr_rsp0(); //emit(0x48, 0x8b, 0x04, 0x24); // mov rax, qword ptr [rsp]
    mov_qword_ptr_rbp_rax(offset); //emit(0x48, 0x89, 0x85); emitValue(offset); // mov qword ptr [rbp + idxOff], rax
    add_rsp(8); // emit(0x48, 0x83, 0xc4); emitValue((char)8); // add rsp, 8
#endif
}
void FunctionX64::incLocal(int idx) {
    inc_qword_ptr_rbp(localIdx2EbpOff(idx));//emit(0x48, 0xff, 0x85); emitValue(localIdx2EbpOff(idx)); // inc qword ptr [rbp + idxOff]
}
void FunctionX64::decLocal(int idx) {
    dec_qword_ptr_rbp(localIdx2EbpOff(idx));//emit(0x48, 0xff, 0x8d); emitValue(localIdx2EbpOff(idx)); // dec qword ptr [rbp + idxOff]
}
void FunctionX64::pop(int n) {
    add_rsp(n*8);//emit(0x48, 0x81, 0xc4); emitValue(n * 8); // add rsp, n * 8
}
void FunctionX64::dup() {
    push_qword_ptr_rsp0();//emit(0xff, 0x34, 0x24); // push qword ptr [rsp]
}

void FunctionX64::doArithmeticOp(TokenID opType) {
    mov_rax_qword_ptr_rsp8(8);  //emit(0x48, 0x8b, 0x44, 0x24, 0x08); // mov rax, qword ptr [rsp+8]
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
    mov_qword_ptr_rsp_rax(8);   //emit(0x48, 0x89, 0x44, 0x24, 0x08); // mov qword ptr [rsp+8], rax
    add_rsp(8);                 //emit(0x48, 0x83, 0xc4, 0x08); // add rsp, 8
}
void FunctionX64::cmp(TokenID cmpType) {
    Label label_1, label_0, label_end;
    mov_rax_qword_ptr_rsp8(8);  //emit(0x48, 0x8b, 0x44, 0x24, 0x08); // mov rax, qword ptr [rsp+8] 
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
    //emit(0xe9);                      // jmp rip+offset32
    //emitValue((int)NULL);
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
    //for (int i = 0; i < paramCount - 1; ++i) {
        //push_qword_ptr_rsp(((i + 1) * 2 - 1) * 8); // emit(0xff, 0xb4, 0x24); emitValue(((i + 1) * 2 - 1) * 8); // push qword ptr [rsp+8*i]
    //}
    //emit(0xff, 0x15); emitValue(entry); // call [entry]  #call   QWORD PTR [rip+entry] 
    char* rip = m_codeBuf + m_codeSize;
    char* funcPtr = *entry;
    int64 offset = (int64)(funcPtr - rip) - 5;  // 5 = sizeof(call rip+offset)
    int64 max = INT_MAX;
    int64 min = INT_MIN;
    printf("endCall, funcName: %-16s: paramCount=%d, address: %p, rip: %p\n", funcName.c_str(), paramCount, funcPtr, rip);
#if 0
    if (offset > max || offset < min) {
    }
    else
    {
        int val = (int)offset;
        printf("offset: %llx, rip: 0x%x\n", offset, val);
        //emit(0xe8); emitValue(val);  // call rip+offset
        call_rip_offset32(val);
    }
#else
    offset = (int64)((char*)entry - rip) - 6;
    if (offset > max || offset < min) {
        printf("offset too large, %llx\n", offset);
        int next = 8;                           // skip funcPtr
        jmp_rip_offset32(next);        //emit(0xe9); emitValue(next);         // jmp rip+offset32, this instruction should save in a special section?
        emitValue(funcPtr);                     // the func ptr to be call, should be saved in a special section?
        offset = -8 - 6;
    }

    call_qword_ptr_rip(offset); //emit(0xff, 0x15); emitValue((int)offset); // call qword ptr[rip+offset]
#endif
    //pop(paramCount + (paramCount > 0 ? paramCount - 1 : 0));
    if (paramCount > 4) {   // for fastcall
        pop(paramCount - 4);
    }
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
    if (idx < 0)
        offset = 16 - idx * 8;
    else
        offset = -(1 + idx) * 8;
    return offset;
}

void FunctionX64::saveParameters() {
    if (m_myParamCount > 0) {
        mov_qword_ptr_rbp_rcx(0x18); //emit(0x48, 0x89, 0x4d, 0x10);// mov    QWORD PTR[rbp + 0x18], rcx
    }
    if (m_myParamCount > 1) {
        mov_qword_ptr_rbp_rdx(0x20); //emit(0x48, 0x89, 0x55, 0x18);// mov    QWORD PTR[rbp + 0x20], rdx
    }
    if (m_myParamCount > 2) {
        mov_qword_ptr_rbp_r8(0x28); //emit(0x48, 0x89, 0x55, 0x18);// mov    QWORD PTR[rbp + 0x20], rdx
    }
    if (m_myParamCount > 3) {
        mov_qword_ptr_rbp_r9(0x30); //emit(0x48, 0x89, 0x55, 0x18);// mov    QWORD PTR[rbp + 0x20], rdx
    }
}

#if 0
void call_rip_offset32(int offset){
    //emit(0xe8); emitValue(offset);  // call rip+offset
}
void push_qword_ptr_rbp(int offset) {
    //emit(0xff, 0xb5); emitValue(offset); // push qword ptr [rbp + offset]
}
#endif
