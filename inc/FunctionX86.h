#ifndef __FUNCTION_X86_H__
#define __FUNCTION_X86_H__
#include "Label.h"
#include "Function.h"
#include "JITEngine.h"

class FunctionX86: public Function {
public:
    FunctionX86(JITEngine* parent, char* codeBuf);
    ~FunctionX86();

    //string& getFuncName();
    //int getCodeSize() const;

    void beginBuild();
    void endBuild();
    void prepareParam(int64 paraVal, int size);

    void loadImm(int imm);

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
    
private:

    void condJmp(TokenID tid, Label* label);

    int localIdx2EbpOff(int idx);

};

#define add_eax_dword_ptr_esp0()            {emit(0x03, 0x04, 0x24);}                        // add eax, dword ptr [esp]
#define add_esp(n)                          {emit(0x81, 0xc4); emitValue(n);}                // add esp, n 
#define add_esp8(n)                         {emit(0x83, 0xc4); emitValue((char)n);}          // add esp, 8
#define call_dword_ptr_eip(offset32)        {emit(0xff, 0x15); emitValue((int)offset32);}    // call dword ptr[eip+offset]
#define call_eip_offset32(offset32)         {emit(0xe8); emitValue(offset32);}               // call eip+offset
#define cmp_eax_edx()                       {emit(0x3b, 0xc2);} //0x39, 0xd0);               // cmp eax, edx
#define dec_dword_ptr_ebp(offxet32)         {emit(0xff, 0x8d); emitValue(offxet32);}         // dec dword ptr [ebp + idxOff]
#define idiv_dword_ptr_esp0()               {emit(0xf7, 0x3c, 0x24);}                        // idiv dword ptr [esp]
#define imul_eax_dword_ptr_esp0()           {emit(0x0f, 0xaf, 0x04, 0x24);}                  // imul eax, dword ptr [esp]
#define inc_dword_ptr_ebp(offset32)         {emit(0xff, 0x85); emitValue(offset32);}         // inc dword ptr [ebp + idxOff]
#define je(offset32)                        {emit(0x0f, 0x84); emitValue(offset32);}         // je
#define jg(offset32)                        {emit(0x0f, 0x8f); emitValue(offset32);}         // jg
#define jl(offset32)                        {emit(0x0f, 0x8c); emitValue(offset32);}         // jl
#define jge(offset32)                       {emit(0x0f, 0x8d); emitValue(offset32);}         // jge
#define jle(offset32)                       {emit(0x0f, 0x8e); emitValue(offset32);}         // jle
#define jmp_eip_offset32(offset32)          {emit(0xe9); emitValue(offset32);}               // jmp eip+offset32
#define jne(offset32)                       {emit(0x0f, 0x85); emitValue(offset32);}         // jne
#define mov_dword_ptr_esp_eax(offset8)      {emit(0x89, 0x44, 0x24, offset8);}               // mov dword ptr [esp+4], eax
#define mov_dword_ptr_ebp_eax(offset32)     {emit(0x89, 0x85); emitValue(offset32);}         // mov dword ptr [ebp + idxOff], eax
#define mov_dword_ptr_esp_ecx(offset8)      {emit(0x89, 0x4d, offset8);}                     // mov    dword PTR[esp + 0x04], ecx
#define mov_dword_ptr_esp_edx(offset8)      {emit(0x89, 0x55, offset8);}                     // mov    dword PTR[esp + 0x08], edx
#define mov_eax_dword_ptr_esp0()            {emit(0x8b, 0x04, 0x24);}                        // mov eax, dword ptr [esp]
#define mov_eax_dword_ptr_esp8(offset8)     {emit(0x8b, 0x44, 0x24, offset8);}               // mov eax, dword ptr [esp+8]
#define mov_eax_edx()                       {emit(0x8b, 0xc2);}	//0x89, 0xd0);               // mov eax, edx
#define mov_ebp_esp()                       {emit(0x8b, 0xec);} //0x89, 0xe5);               // mov ebp, esp
#define mov_ecx_ebp_offset32(offset32)      {emit(0x8b, 0x8d); emitValue(offset32);}         // mov ecx, [ebp + offset32]
#define mov_edx_dword_ptr_esp0()            {emit(0x8b, 0x14, 0x24);}                        // mov edx, dword ptr[esp]
#define mov_edx_ebp_offset32(offset32)      {emit(0x8b, 0x95); emitValue(offset32);}         // mov edx, [ebp + offset32]
#define mov_esp_ebp()                       {emit(0x8b, 0xe5);} //0x89, 0xec);               // mov esp,ebp 
#define pop_ebp()                           {emit(0x5d);}                                    // pop ebp  
#define pop_edx()                           {emit(0x5a);}                                    // pop edx  
#define push_32(n)                          {emit(0x68); emitValue((int)n);}                 // push #imm32
#define push_8(n)                           {emit(0x6a, n);}                                 // push #imm8
#define push_dword_ptr_ebp(offset32)        {emit(0xff, 0xb5); emitValue(offset32);}         // push dword ptr [ebp + idxOff]
#define push_dword_ptr_esp(offset32)        {emit(0xff, 0xb4, 0x24); emitValue(offset32);}   // push dword ptr [esp+4*i]
#define push_dword_ptr_esp0()               {emit(0xff, 0x34, 0x24);}                        // push dword ptr [esp]
#define push_eax()                          {emit(0x50);}                                    // push eax
#define push_ebp()                          {emit(0x55);}                                    // push ebp
#define push_ecx()                          {emit(0x51);}                                    // push ecx
#define push_edx()                          {emit(0x52);}                                    // push edx
#define retn()                              {emit(0xc3);}                                    // ret
#define sub_eax_dword_ptr_esp0()            {emit(0x2b, 0x04, 0x24);}                        // sub eax, dword ptr [esp]
#define sub_esp(n)                          {emit(0x81, 0xec); emitValue(n);}                // sub esp, n
#define test_eax_eax()                      {emit(0x85, 0xc0);}                              // test eax, eax
#define xor_edx_edx()                       {emit(0x33, 0xd2);}                              // xor edx, edx

#endif
