#ifndef __X86_FUNCTION_BUILDER_H__
#define __X86_FUNCTION_BUILDER_H__
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
    void emit(int n, ...);
    template<typename T> void emitValue(T val);

    void condJmp(TokenID tid, Label* label);

    int localIdx2EbpOff(int idx);

    //
};

#endif
