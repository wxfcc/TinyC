#ifndef __X64_FUNCTION_BUILDER_H__
#define __X64_FUNCTION_BUILDER_H__
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
    void prepareParamForWindows(int64 paraVal, int size);
    void prepareParamForLinux(int64 paraVal, int size);
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

protected:
    void emitRelativeAddr32(char* absPos, int prefixLen);

    void condJmp(TokenID tid, Label* label);

    int localIdx2EbpOff(int idx);

    //
    //x64JITEngine* m_parent;
};

#endif
