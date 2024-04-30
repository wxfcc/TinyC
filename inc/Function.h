#ifndef __FUNCTION__H__
#define __FUNCTION__H__
#include "Label.h"
//#include "JITEngine.h"
#include "Scanner.h"
#include "common.h"

#define ARG_T(t)  t  
#define ARG_N(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31,a32,N,...)  N 
#define ARG_N_HELPER(...)  ARG_T(ARG_N(__VA_ARGS__)) 
#define COUNT_ARG(...)  ARG_N_HELPER(__VA_ARGS__,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define emit(...) emitCode(COUNT_ARG(__VA_ARGS__),__VA_ARGS__)
//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_LOCAL_COUNT 64
class JITEngine;
class Function ;
class Function {
public:
    Function(JITEngine*parent, char *codeBuf);
    virtual ~Function();

    void setFuncName(string& name, int argsCount);
    const string& getFuncName();
    int getCodeSize() const;

    virtual void beginBuild() = 0;
    virtual void endBuild() = 0;
    virtual void prepareParam(int64 paraVal, int size) = 0;

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
    virtual void saveParameters() = 0;
    void setCallParamIndex(int paramIndex);

protected:
    void emitCode(int n, ...);

    template<typename T>
    void emitValue(T val) {
        memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
        m_codeSize += sizeof(val);
    }

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
    int m_beginCall;
    int m_myParamCount;
    int m_callParamIndex;
    int m_localVarCount;
};



#endif
