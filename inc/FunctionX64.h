#ifndef __FUNCTION_X64_H__
#define __FUNCTION_X64_H__
#include "Label.h"
#include "Function.h"
#include "JITEngine.h"

class FunctionX64 :public Function {
public:
    FunctionX64(JITEngine* parent, char* codeBuf);
    ~FunctionX64();
    
    void beginBuild();
    void endBuild();

    void prepareParam(int64 paraVal, int size);
    //void prepareParamForWindows(int64 paraVal, int size);
    //void prepareParamForLinux(int64 paraVal, int size);
    void loadImm(int imm);
    void loadImm64(int64 imm);
    void loadLiteralStr(const string& literalStr);
    void loadLocal(int idx);
    void storeLocal(int idx);
    void incLocal(int idx);
    void decLocal(int idx);
    void pop(int n);
    void dup();
    void doArithmeticOp(TokenID opType);
    void cmp(TokenID cmpType);
    void markLabel(Label* label);
    void jmp(Label* label);
    void trueJmp(Label* label);
    void falseJmp(Label* label);
    void ret();
    void retExpr();
    int beginCall();
    void endCall(const string& funcName, int callID, int paramCount);

public:
protected:
    void emitRelativeAddr32(char* absPos, int prefixLen);

    void condJmp(TokenID tid, Label* label);

    int localIdx2EbpOff(int idx);
};

#define add_rax_qword_ptr_rsp0()            {emit(0x48, 0x03, 0x04, 0x24);}                  // add rax, qword ptr [rsp]
#define add_rsp(n)                          {emit(0x48, 0x81, 0xc4); emitValue(n);}          // add rsp, n 
#define add_rsp8(n)                         {emit(0x48, 0x83, 0xc4); emitValue((char)n);}    // add rsp, 8
#define call_qword_ptr_rip(offset32)        {emit(0xff, 0x15); emitValue((int)offset32);}    // call qword ptr[rip+offset]
#define call_rip_offset32(offset32)         {emit(0xe8); emitValue(offset32);}               // call rip+offset
#define cmp_rax_rdx()                       {emit(0x48, 0x39, 0xd0);}                        // cmp rax, rdx
#define dec_qword_ptr_rbp(offxet32)         {emit(0x48, 0xff, 0x8d); emitValue(offxet32);}   // dec qword ptr [rbp + idxOff]
#define idiv_qword_ptr_rsp0()               {emit(0x48, 0xf7, 0x3c, 0x24);}                  // idiv qword ptr [rsp]
#define imul_rax_qword_ptr_rsp0()           {emit(0x48, 0x0f, 0xaf, 0x04, 0x24);}            // imul rax, qword ptr [rsp]
#define inc_qword_ptr_rbp(offset32)         {emit(0x48, 0xff, 0x85); emitValue(offset32);}   // inc qword ptr [rbp + idxOff]
#define je(offset32)                        {emit(0x0f, 0x84); emitValue(offset32);}         // je
#define jg(offset32)                        {emit(0x0f, 0x8f); emitValue(offset32);}         // jg
#define jl(offset32)                        {emit(0x0f, 0x8c); emitValue(offset32);}         // jl
#define jge(offset32)                       {emit(0x0f, 0x8d); emitValue(offset32);}         // jge
#define jle(offset32)                       {emit(0x0f, 0x8e); emitValue(offset32);}         // jle
#define jmp_rip_offset32(offset32)          {emit(0xe9); emitValue(offset32);}               // jmp rip+offset32
#define jne(offset32)                       {emit(0x0f, 0x85); emitValue(offset32);}         // jne
#define mov_qword_ptr_rsp_rax(offset8)      {emit(0x48, 0x89, 0x44, 0x24, offset8);}         // mov qword ptr [rsp+8], rax
#define mov_qword_ptr_rbp_rax(offset32)     {emit(0x48, 0x89, 0x85); emitValue(offset32);}   // mov qword ptr [rbp + idxOff], rax
#define mov_qword_ptr_rsp_rcx(offset8)      {emit(0x48, 0x89, 0x4d, offset8);}               // mov    QWORD PTR[rsp + 0x08], rcx
#define mov_qword_ptr_rsp_rdx(offset8)      {emit(0x48, 0x89, 0x55, offset8);}               // mov    QWORD PTR[rsp + 0x10], rdx
#define mov_r8_imm64(n)                     {emit(0x49, 0xb8); emitValue((int64)n);}         // mov r8, #imm64
#define mov_r8_rbp_offset32(offset32)       {emit(0x4c, 0x8b, 0x85); emitValue(offset32);}   // mov r8, [rbp + offset32]
#define mov_r9_imm64(n)                     {emit(0x49, 0xb9); emitValue((int64)n);}         // mov r9, #imm64
#define mov_r9_rbp_offset32(offset32)       {emit(0x4c, 0x8b, 0x8d); emitValue(offset32);}   // mov r9, [rbp + offset32]
#define mov_rax_imm64(n)                    {emit(0x48, 0xb8); emitValue((int64)n);}         // mov rax, #imm64
#define mov_rax_qword_ptr_rsp0()            {emit(0x48, 0x8b, 0x04, 0x24);}                  // mov rax, qword ptr [rsp]
#define mov_rax_qword_ptr_rsp8(offset8)     {emit(0x48, 0x8b, 0x44, 0x24, offset8);}         // mov rax, qword ptr [rsp+8]
#define mov_rax_rdx()                       {emit(0x48, 0x89, 0xd0);}                        // mov rax, rdx
#define mov_rbp_rsp()                       {emit(0x48, 0x89, 0xe5);}                        // mov rbp, rsp
#define mov_rcx_imm64(n)                    {emit(0x48, 0xb9); emitValue((int64)n);}         // mov rcx, #imm64
#define mov_rcx_rbp_offset32(offset32)      {emit(0x48, 0x8b, 0x8d); emitValue(offset32);}   // mov rcx, [rbp + offset32]
#define mov_rdx_imm64(n)                    {emit(0x48, 0xba); emitValue((int64)n);}         // mov rdx, #imm64
#define mov_rdx_qword_ptr_rsp0()            {emit(0x48, 0x8b, 0x14, 0x24);}                  // mov rdx, qword ptr[rsp]
#define mov_rdx_rbp_offset32(offset32)      {emit(0x48, 0x8b, 0x95); emitValue(offset32);}   // mov rdx, [rbp + offset32]
#define mov_rsp_rbp()                       {emit(0x48, 0x89, 0xec);}                        // mov rsp,rbp 
#define pop_rbp()                           {emit(0x5d);}                                    // pop rbp  
#define pop_rdx()                           {emit(0x5a);}                                    // pop rdx  
#define push_32(n)                          {emit(0x68); emitValue((int)n);}                 // push #imm32
#define push_8(n)                           {emit(0x6a, n);}                                 // push #imm8
#define push_qword_ptr_rbp(offset32)        {emit(0xff, 0xb5); emitValue(offset);}           // push qword ptr [rbp + idxOff]
#define push_qword_ptr_rsp(offset32)        {emit(0xff, 0xb4, 0x24); emitValue(offset32);}   // push qword ptr [rsp+8*i]
#define push_qword_ptr_rsp0()               {emit(0xff, 0x34, 0x24);}                        // push qword ptr [rsp]
#define push_r8()                           {emit(0x41, 0x50);}                              // push r8
#define push_r9()                           {emit(0x41, 0x51);}                              // push r9
#define push_rax()                          {emit(0x50);}                                    // push rax
#define push_rbp()                          {emit(0x55);}                                    // push rbp
#define push_rcx()                          {emit(0x51);}                                    // push rcx
#define push_rdx()                          {emit(0x52);}                                    // push rdx
#define retn()                              {emit(0xc3);}                                    // ret
#define sub_rax_qword_ptr_rsp0()            {emit(0x48, 0x2b, 0x04, 0x24);}                  // sub rax, qword ptr [rsp]
#define sub_rsp(n)                          {emit(0x48, 0x81, 0xec); emitValue(n);}          // sub rsp, n
#define test_rax_rax()                      {emit(0x48, 0x85, 0xc0);}                        // test rax, rax
#define xor_rdx_rdx()                       {emit(0x48, 0x31, 0xd2);}                        // xor rdx, rdx

#endif
