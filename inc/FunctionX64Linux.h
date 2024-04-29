#ifndef __FUNCTION_X64_LINUX_H__
#define __FUNCTION_X64_LINUX_H__
#include "Label.h"
#include "FunctionX64.h"
#include "JITEngine.h"

class FunctionX64Linux :public FunctionX64 {
public:
    FunctionX64Linux(JITEngine* parent, char* codeBuf);
    ~FunctionX64Linux();
    
    void prepareParam (int64 paraVal, int size);
    //void loadImm(int imm);
    //void loadImm64(int64 imm);
    //void loadLiteralStr(const string& literalStr);
    void loadLocal(int idx);
    //void storeLocal(int idx);
    //void incLocal(int idx);
    //void decLocal(int idx);
    void saveParameters();

protected:

};

#endif
