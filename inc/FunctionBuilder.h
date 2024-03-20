#ifndef __FUNCTION_BUILDER__
#define __FUNCTION_BUILDER__
#include "x86Label.h"
#include "x86jit.h"
#include "common.h"

//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_LOCAL_COUNT 64

class FunctionBuilder {
public:
    FunctionBuilder(x86JITEngine *parent, char *codeBuf);
    string& getFuncName();
    int getCodeSize() const;

    void beginBuild();
    void endBuild();

    void loadImm(int imm);
    
    void loadLiteralStr(const string &literalStr);
    void loadLocal(int idx);
    void storeLocal(int idx);
    void incLocal(int idx);
    void decLocal(int idx);
    void pop(int n);
    void dup();
    void doArithmeticOp(TokenID opType); 
    void cmp(TokenID cmpType);
    void markLabel(x86Label *label);
    void jmp(x86Label *label);
    void trueJmp(x86Label *label);
    void falseJmp(x86Label *label);
    void ret();
    void retExpr();
    int beginCall();
    void endCall(const string &funcName, int callID, int paramCount);
    
protected:
    void emit(int n, ...);
    template<typename T> void emitValue(T val);

    void condJmp(TokenID tid, x86Label *label);
    
    int localIdx2EbpOff(int idx);

	//
    x86JITEngine *m_parent;
    char *m_codeBuf;
    int m_codeSize;
    string m_funcName;
    x86Label m_retLabel;
};

class x86FunctionBuilder: public FunctionBuilder {
public:
    x86FunctionBuilder(x86JITEngine* parent, char* codeBuf);
    string& getFuncName();
    int getCodeSize() const;

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
    void markLabel(x86Label* label);
    void jmp(x86Label* label);
    void trueJmp(x86Label* label);
    void falseJmp(x86Label* label);
    void ret();
    void retExpr();
    int beginCall();
    void endCall(const string& funcName, int callID, int paramCount);

private:
    void emit(int n, ...);
    template<typename T> void emitValue(T val);

    void condJmp(TokenID tid, x86Label* label);

    int localIdx2EbpOff(int idx);

    //
};
class x64JITEngine;
class x64FunctionBuilder :public FunctionBuilder {
public:
    x64FunctionBuilder(x64JITEngine* parent, char* codeBuf);
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
    void markLabel(x86Label* label);
    void jmp(x86Label* label);
    void trueJmp(x86Label* label);
    void falseJmp(x86Label* label);
    void ret();
    void retExpr();
    int beginCall();
    void endCall(const string& funcName, int callID, int paramCount);

protected:
    void emit(int n, ...);
    template<typename T> void emitValue(T val);

    void condJmp(TokenID tid, x86Label* label);

    int localIdx2EbpOff(int idx);

    //
    //x64JITEngine* m_parent;
};

#endif