#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <algorithm>
#include <functional>
#include <iterator>

#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include "x86Label.h"
#include "common.h"

//============================== code generator
#define MAX_TEXT_SECTION_SIZE (4096 * 8)
#define MAX_LOCAL_COUNT 64
class x86FunctionBuilder;
class x86JITEngine {
public:
    x86JITEngine();
    ~x86JITEngine();
    unsigned char* getFunction(const string &name);

    void beginBuild();
    char** _getFunctionEntry(const string &name);
    const char* _getLiteralStringLoc(const string &literalStr);
    x86FunctionBuilder* beginBuildFunction();
    void endBuildFunction(x86FunctionBuilder *builder);
    void endBuild();
private:
    char *m_textSection;
    int m_textSectionSize;
    map<string, char*> m_funcEntries;
    set<string> m_literalStrs;
};

class x86FunctionBuilder {
public:
    x86FunctionBuilder(x86JITEngine *parent, char *codeBuf);
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
    
private:
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
