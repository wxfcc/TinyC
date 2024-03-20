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
typedef FunctionBuilder* (*FunctionBuilder_FP)(JITEngine* parent, char* codeBuf) ;   // function pointer
class FunctionBuilder {
public:
    FunctionBuilder(JITEngine*parent, char *codeBuf);
    
    string& getFuncName();
    int getCodeSize() const;

    virtual void beginBuild() = NULL;
    virtual void endBuild() = NULL;

    virtual void loadImm(int imm) = NULL;
    
    virtual void loadLiteralStr(const string &literalStr) = NULL;
    virtual void loadLocal(int idx) = NULL;
    virtual void storeLocal(int idx) = NULL;
    virtual void incLocal(int idx) = NULL;
    virtual void decLocal(int idx) = NULL;
    virtual void pop(int n) = NULL;
    virtual void dup() = NULL;
    virtual void doArithmeticOp(TokenID opType) = NULL;
    virtual void cmp(TokenID cmpType) = NULL;
    virtual void markLabel(Label *label);
    virtual void jmp(Label *label) = NULL;
    virtual void trueJmp(Label *label) = NULL;
    virtual void falseJmp(Label *label) = NULL;
    virtual void ret() = NULL;
    virtual void retExpr() = NULL;
    virtual int beginCall() = NULL;
    virtual void endCall(const string &funcName, int callID, int paramCount) = NULL;
    virtual void condJmp(TokenID tid, Label *label) = NULL;
    virtual int localIdx2EbpOff(int idx) = NULL;
    
    //static FunctionBuilder* newBuilder(JITEngine* parent, char* codeBuf);
protected:
    void emit(int n, ...);
    template<typename T> void emitValue(T val);

	//
    JITEngine*m_parent;
    char *m_codeBuf;
    int m_codeSize;
    string m_funcName;
    Label m_retLabel;
};



#endif