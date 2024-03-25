#ifndef __FUNCTION_BUILDER__H__
#define __FUNCTION_BUILDER__H__
#include "Label.h"
#include "JITEngine.h"
#include "common.h"

//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_LOCAL_COUNT 64
class JITEngine;
class FunctionBuilder;
typedef FunctionBuilder* (*FP_FunctionBuilder)(JITEngine* parent, char* codeBuf) ;   // function pointer
class FunctionBuilder {
public:
    FunctionBuilder(JITEngine*parent, char *codeBuf);
    
    string& getFuncName();
    int getCodeSize() const;

    virtual void beginBuild() = 0;
    virtual void endBuild() = 0;

    virtual void loadImm(int imm) = 0;
    
    virtual void loadLiteralStr(const string &literalStr) = 0;
    virtual void loadLocal(int idx) = 0;
    virtual void storeLocal(int idx) = 0;
    virtual void incLocal(int idx) = 0;
    virtual void decLocal(int idx) = 0;
    virtual void pop(int n) = 0;
    virtual void dup() = 0;
    virtual void doArithmeticOp(TokenID opType) = 0;
    virtual void cmp(TokenID cmpType) = 0;
    virtual void markLabel(Label *label);
    virtual void jmp(Label *label) = 0;
    virtual void trueJmp(Label *label) = 0;
    virtual void falseJmp(Label *label) = 0;
    virtual void ret() = 0;
    virtual void retExpr() = 0;
    virtual int beginCall() = 0;
    virtual void endCall(const string &funcName, int callID, int paramCount) = 0;
    virtual void condJmp(TokenID tid, Label *label) = 0;
    virtual int localIdx2EbpOff(int idx) = 0;
    
    //static FunctionBuilder* newBuilder(JITEngine* parent, char* codeBuf);
protected:
    void emit(int n, ...);
    template<typename T> void emitValue(T val);
    //void emit8(char val);
    //void emit16(short val);
    //void emit32(int val);
    //void emit64(long long val);

	//
    JITEngine*m_parent;
    char *m_codeBuf;
    int m_codeSize;
    string m_funcName;
    Label m_retLabel;
    int m_paramCount;
    int m_beginCall;
};



#endif