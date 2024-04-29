#ifndef __FUNCTION_X64_H__
#define __FUNCTION_X64_H__
#include "Label.h"
#include "Function.h"
#include "JITEngine.h"
// in x64 mode, just only support __fastcall calling convention 
class FunctionX64 :public Function {
public:
    FunctionX64(JITEngine* parent, char* codeBuf);
    ~FunctionX64();
    
    void beginBuild();
    void endBuild();

    void prepareParam(int64 paraVal, int size);
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
    void saveParameters();

public:
protected:
    void emitRelativeAddr32(char* absPos, int prefixLen);

    void condJmp(TokenID tid, Label* label);

    int localIdx2EbpOff(int idx);
};

#define add_rax_qword_ptr_rsp0()            {emit(0x48, 0x03, 0x04, 0x24);}                     // add rax, qword ptr [rsp]
#define add_rsp(n)                          {emit(0x48, 0x81, 0xc4); emitValue((int)n);}        // add rsp, n 
#define add_rsp_8(n)                        {emit(0x48, 0x83, 0xc4, n);}                        // add rsp, n
#define call_qword_ptr_rip(offset32)        {emit(0xff, 0x15); emitValue((int)offset32);}       // call qword ptr[rip + offset]
#define call_rip_offset32(offset32)         {emit(0xe8); emitValue(offset32);}                  // call rip+offset
#define cmp_rax_rdx()                       {emit(0x48, 0x39, 0xd0);}                           // cmp rax, rdx
#define dec_qword_ptr_rbp(offset32)         {emit(0x48, 0xff, 0x8d); emitValue(offset32);}      // dec qword ptr [rbp + offset32]
#define idiv_qword_ptr_rsp0()               {emit(0x48, 0xf7, 0x3c, 0x24);}                     // idiv qword ptr [rsp]
#define imul_rax_qword_ptr_rsp0()           {emit(0x48, 0x0f, 0xaf, 0x04, 0x24);}               // imul rax, qword ptr [rsp]
#define inc_qword_ptr_rbp(offset32)         {emit(0x48, 0xff, 0x85); emitValue(offset32);}      // inc qword ptr [rbp + offset32]
#define je(offset32)                        {emit(0x0f, 0x84); emitValue(offset32);}            // je
#define jg(offset32)                        {emit(0x0f, 0x8f); emitValue(offset32);}            // jg
#define jl(offset32)                        {emit(0x0f, 0x8c); emitValue(offset32);}            // jl
#define jge(offset32)                       {emit(0x0f, 0x8d); emitValue(offset32);}            // jge
#define jle(offset32)                       {emit(0x0f, 0x8e); emitValue(offset32);}            // jle
#define jmp_rip_offset32(offset32)          {emit(0xe9); emitValue(offset32);}                  // jmp rip+offset32
#define jne(offset32)                       {emit(0x0f, 0x85); emitValue(offset32);}            // jne
#define lea_rbp_rsp_8(n)                    {emit(0x48, 0x8d, 0x6c, 0x24, n);}                  // lea    rbp, [rsp + 0x40]
#define lea_rbp_rsp32(n)                    {emit(0x48, 0x8d, 0xac, 0x24); emitValue(n);}       // lea    rbp, [rsp + 0x400]

#define mov_qword_ptr_rsp_rax_8(n)          {emit(0x48, 0x89, 0x44, 0x24, n);}                  // mov qword ptr [rsp+8], rax
#define mov_qword_ptr_rsp_rcx_8(n)          {emit(0x48, 0x89, 0x4c, 0x24, n);}                  // mov qword ptr[rsp + 0x08], rcx
#define mov_qword_ptr_rsp_rdx_8(n)          {emit(0x48, 0x89, 0x54, 0x24, n);}                  // mov qword ptr[rsp + 0x10], rdx
#define mov_qword_ptr_rbp_rax(offset32)     {emit(0x48, 0x89, 0x85); emitValue(offset32);}      // mov qword ptr [rbp + offset32], rax
#define mov_qword_ptr_rbp_rcx_8(n)          {emit(0x48, 0x89, 0x4d, n);}                        // mov qword ptr[rbp + 0x08], rcx
#define mov_qword_ptr_rbp_rdx_8(n)          {emit(0x48, 0x89, 0x55, n);}                        // mov qword ptr[rbp + 0x10], rdx
#define mov_qword_ptr_rbp_r8_8(n)           {emit(0x4c, 0x89, 0x45, n);}                        // mov qword ptr[rbp + 0x08], r8
#define mov_qword_ptr_rbp_r9_8(n)           {emit(0x4c, 0x89, 0x4d, n);}                        // mov qword ptr[rbp + 0x10], r9
#define mov_qword_ptr_rbp_rdi_8(n)          {emit(0x48, 0x89, 0x7d, n);}                        // mov qword ptr[rbp + 0x10], rdi
#define mov_qword_ptr_rbp_rsi_8(n)          {emit(0x48, 0x89, 0x75, n);}                        // mov qword ptr[rbp + 0x10], rsi
#define mov_qword_ptr_rbp_rdi(offset32)     {emit(0x48, 0x89, 0xbd); emitValue(offset32);}      // mov qword ptr[rbp + 0x100], rdi
#define mov_qword_ptr_rbp_rsi(offset32)     {emit(0x48, 0x89, 0xb5); emitValue(offset32);}      // mov qword ptr[rbp + 0x100], rsi
#define mov_qword_ptr_rbp_imm32(offset32,n) {emit(0x48, 0xc7, 0x85); emitValue(offset32);emitValue(n);}   // mov qword ptr[rbp + 0x100], #n

#define mov_r8_imm64(n)                     {emit(0x49, 0xb8); emitValue((int64)n);}            // mov r8, #imm64
#define mov_r8_rbp_offset32(n)              {emit(0x4c, 0x8b, 0x85); emitValue(n);}             // mov r8, [rbp + offset32]
#define mov_r9_imm64(n)                     {emit(0x49, 0xb9); emitValue((int64)n);}            // mov r9, #imm64
#define mov_r9_rbp_offset32(n)              {emit(0x4c, 0x8b, 0x8d); emitValue(n);}             // mov r9, [rbp + offset32]
#define mov_rax_imm64(n)                    {emit(0x48, 0xb8); emitValue((int64)n);}            // mov rax, #imm64
#define mov_rax_qword_ptr_rsp0()            {emit(0x48, 0x8b, 0x04, 0x24);}                     // mov rax, qword ptr [rsp]
#define mov_rax_qword_ptr_rsp_8(n)          {emit(0x48, 0x8b, 0x44, 0x24, n);}                  // mov rax, qword ptr [rsp+8]
#define mov_rax_rdx()                       {emit(0x48, 0x89, 0xd0);}                           // mov rax, rdx
#define mov_rbp_rsp()                       {emit(0x48, 0x89, 0xe5);}                           // mov rbp, rsp
#define mov_rcx_imm64(n)                    {emit(0x48, 0xb9); emitValue((int64)n);}            // mov rcx, #imm64
#define mov_rcx_rbp_offset32(n)             {emit(0x48, 0x8b, 0x8d); emitValue(n);}             // mov rcx, [rbp + offset32]
#define mov_rcx_rsp_offset32(n)             {emit(0x48, 0x8b, 0x8c, 0x24); emitValue(n);}       // mov rcx, [rsp + offset32]
#define mov_rdx_rsp_8(n)                    {emit(0x48, 0x8b, 0x54, 0x24, n);}                  // mov rdx, [rsp + offset8]
#define mov_rdx_rsp_offset32(n)             {emit(0x48, 0x8b, 0x94, 0x24); emitValue(n);}       // mov rdx, [rsp + offset32]
#define mov_r8_rsp_8(n)                     {emit(0x4C, 0x8B, 0x44, 0x24, n);}                  // mov r8, [rsp + offset8]
#define mov_r8_rsp_offset32(n)              {emit(0x4C, 0x8B, 0x84, 0x24); emitValue(n);}       // mov r8, [rsp + offset32]
#define mov_r9_rsp_8(n)                     {emit(0x4C, 0x8B, 0x4C, 0x24, n);}                  // mov r9, [rsp + offset8]
#define mov_r9_rsp_offset32(n)              {emit(0x4C, 0x8B, 0x8C, 0x24); emitValue(n);}       // mov r9, [rsp + offset32]

#define mov_rdi_rbp_offset32(offset32)      {emit(0x48, 0x8b, 0xbd); emitValue(offset32);}      // mov rdi, [rbp + offset32]
#define mov_rsi_rbp_offset32(offset32)      {emit(0x48, 0x8b, 0xb5); emitValue(offset32);}      // mov rsi, [rbp + offset32]

#define mov_rdx_imm64(n)                    {emit(0x48, 0xba); emitValue((int64)n);}            // mov rdx, #imm64
#define mov_rdx_qword_ptr_rsp0()            {emit(0x48, 0x8b, 0x14, 0x24);}                     // mov rdx, qword ptr[rsp]
#define mov_rdx_rbp_offset32(offset32)      {emit(0x48, 0x8b, 0x95); emitValue(offset32);}      // mov rdx, [rbp + offset32]
#define mov_rsp_rbp()                       {emit(0x48, 0x89, 0xec);}                           // mov rsp,rbp 
#define pop_rax()                           {emit(0x58);}                                       // pop rax
#define pop_rbp()                           {emit(0x5d);}                                       // pop rbp  
#define pop_rdx()                           {emit(0x5a);}                                       // pop rdx  
#define push_32(n)                          {emit(0x68); emitValue((int)n);}                    // push #imm32
#define push_8(n)                           {emit(0x6a, n);}                                    // push #imm8
#define push_qword_ptr_rbp(offset32)        {emit(0xff, 0xb5); emitValue(offset32);}            // push qword ptr [rbp + offset32]
#define push_qword_ptr_rsp(offset32)        {emit(0xff, 0xb4, 0x24); emitValue(offset32);}      // push qword ptr [rsp+8*i]
#define push_qword_ptr_rsp0()               {emit(0xff, 0x34, 0x24);}                           // push qword ptr [rsp]
#define push_r8()                           {emit(0x41, 0x50);}                                 // push r8
#define push_r9()                           {emit(0x41, 0x51);}                                 // push r9
#define push_rax()                          {emit(0x50);}                                       // push rax
#define push_rcx()                          {emit(0x51);}                                       // push rcx
#define push_rdx()                          {emit(0x52);}                                       // push rdx
#define push_rbp()                          {emit(0x55);}                                       // push rbp
#define push_rsi()                          {emit(0x56);}                                       // push rsi
#define push_rdi()                          {emit(0x57);}                                       // push rdi
#define retn()                              {emit(0xc3);}                                       // ret
#define sub_rax_qword_ptr_rsp0()            {emit(0x48, 0x2b, 0x04, 0x24);}                     // sub rax, qword ptr [rsp]
#define sub_rsp(n)                          {emit(0x48, 0x81, 0xec); emitValue((int)n);}        // sub rsp, n
#define test_rax_rax()                      {emit(0x48, 0x85, 0xc0);}                           // test rax, rax
#define xor_rdx_rdx()                       {emit(0x48, 0x31, 0xd2);}                           // xor rdx, rdx

#endif
