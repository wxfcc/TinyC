#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "JITEngine.h"

JITEngine::JITEngine(int arch): m_arch(arch), m_textSectionSize(0) {
    m_textSection = os_mallocExecutable(MAX_TEXT_SECTION_SIZE);
    funcs = new char*[256];
}
JITEngine::~JITEngine() { 
	os_freeExecutable(m_textSection); 
}
unsigned int JITEngine::getCodeSize() {
    return m_textSectionSize;
}
unsigned char* JITEngine::getCode() {
    return (unsigned char*)m_textSection;
}

unsigned char* JITEngine::getFunction(const string &name) {
	return (unsigned char*)*_getFunctionEntry(name); 
}
int JITEngine::setExecutable(){
#ifdef __MACH__
#if 0
    unsigned char shellcode[] = {
#if 0
        0xB8, 0x00, 0x00, 0x00, 0x00,                       // mov eax, 42
        0x90, 0x90, 0x90,                                   // NOP * 3
        0x68, 0x01, 0x00, 0x00, 0x00,                       // push 0x00000001
        0xc7, 0x44, 0x24, 0x04, 0x01, 0x00, 0x00, 0x00,     // mov DWORD PTR [rsp+4],0x0
        0x58,                                               // pop rax
        0xc3                                                //ret
#elif 1
        0x55,   //push   rbp
        0x48, 0x89, 0xe5,//                mov    rbp,rsp
        0x48, 0x89, 0xec,//                mov    rsp,rbp
        0x5d,             //         pop    rbp
        0xc3                                                //ret
        //    0:  52                      push   rdx
        //    1:  55                      push   rbp
        //    2:  48 89 e5                mov    rbp,rsp
        //    5:  48 81 ec f8 01 00 00    sub    rsp,0x1f8
        //    c:  48 bf e1 07 0a 03 00    movabs rdi,0x6000030a07e1
        //    13: 60 00 00
        //    16: 48 be 7b 00 00 00 00    movabs rsi,0x7b
        //    1d: 00 00 00
        //    20: ff 15 12 b7 fb 02       call   QWORD PTR [rip+0x2fbb712]        # 0x2fbb738
        //    26: 48 81 c4 18 00 00 00    add    rsp,0x18
        //    2d: 50                      push   rax
        //    2e: 48 81 c4 08 00 00 00    add    rsp,0x8
        //    35: 68 0c 00 00 00          push   0xc
        //    3a: 48 8b 04 24             mov    rax,QWORD PTR [rsp]
        //    3e: 48 83 c4 08             add    rsp,0x8
        //    42: e9 00 00 00 00          jmp    0x47
        //    47: 48 89 ec                mov    rsp,rbp
        //    4a: 5d                      pop    rbp
        //    4b: 5a                      pop    rdx
        //    4c: c3                      ret
#endif
    };
    memcpy(m_textSection, shellcode, sizeof(shellcode));
    typedef int64_t (*ShellcodeFunction)();
    ShellcodeFunction function = (ShellcodeFunction)m_textSection;
    dumpCode();
    int64_t ret = function();
    printf("ret: 0x%llx\n", ret);
#endif
    
    int erro = mprotect(m_textSection, MAX_TEXT_SECTION_SIZE, PROT_READ | PROT_EXEC);
    ASSERT(erro == 0);

#endif
    return 0;
}

FunctionBuilder* JITEngine::beginBuildFunction() {
    FunctionBuilder* builder = NULL;
    if (m_arch == JIT_X86)
        builder = new x86FunctionBuilder(this, m_textSection + m_textSectionSize);
    else if (m_arch == JIT_X64)
        builder = new x64FunctionBuilder(this, m_textSection + m_textSectionSize);

    builder->beginBuild();

    return builder;
}
void JITEngine::endBuildFunction(FunctionBuilder* builder) {
    builder->endBuild();
    *_getFunctionEntry(builder->getFuncName()) = m_textSection + m_textSectionSize;
    m_textSectionSize += builder->getCodeSize();
    ASSERT(m_textSectionSize <= MAX_TEXT_SECTION_SIZE);
    delete builder;
}


void JITEngine::addFunctionEntry(const char* funcName, char* entry) {
    m_funcEntries[funcName] = entry;
}

char** JITEngine::_getFunctionEntry(const string &name) {
	return &m_funcEntries[name]; 
}
const char* JITEngine::_getLiteralStringLoc(const string &literalStr) {
    return m_literalStrs.insert(literalStr).first->c_str();
}

void JITEngine::beginBuild() { 
}
void JITEngine::endBuild() {
    for (map<string, char*>::iterator iter = m_funcEntries.begin(); iter != m_funcEntries.end(); ++iter) {
        if (iter->second == NULL) {
            char *f = os_findSymbol(iter->first.c_str());
            ASSERT(f != NULL);
            iter->second = f;
        }
    }
}
void JITEngine::dumpCode() { 
    unsigned char* p = (unsigned char*)m_textSection;
    for (int i = 0; i < m_textSectionSize; i++) {
        printf("%02x ", p[i]);
    }
    printf("\n");
}


//============================== syntax analysis
