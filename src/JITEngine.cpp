#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/mman.h>
#include <dlfcn.h>
#include <unistd.h>
#endif
#include "JITEngine.h"
#include "FunctionX86.h"
#include "FunctionX64.h"
#include "FunctionX64Windows.h"
#include "FunctionX64Linux.h"

JITEngine::JITEngine(int arch, int os){
    m_arch = arch;
    m_os = os;
    m_process = NULL;
    m_codeSize = 0;
    m_codeMax = MAX_TEXT_SECTION_SIZE;
    m_code = mallocAlign(m_codeMax);
    memset(m_code, 0, m_codeMax);
//    m_funcPtr = new char*[MAX_FUNC_PTR_SIZE];
    m_funcPtr = (char **)mallocAlign(MAX_FUNC_PTR_SIZE);
    memset(m_funcPtr, 0, MAX_FUNC_PTR_SIZE);
    
    printf("JITEngine for %s/%s, ", getOs(), getArch());
    printf("m_code: %p, m_funcPtr: %p\n", m_code, m_funcPtr);
}

JITEngine::~JITEngine() {
    freeAlign(m_code, m_codeMax);
	delete(m_funcPtr);
}

char* JITEngine::mallocAlign(int size){
    char* buf = NULL;
#if 0 //ndef _WIN32
    buf = new char[MAX_TEXT_SECTION_SIZE];
    m_code = buf;
    //align m_code with pagesize
    int ps = getpagesize();
    int m = m_buf & (ps-1);
    if(m){
        int r = ps - m;
        m_codeMax -= r;
        m_code += r;
    }
#endif

#ifdef _WIN32
    buf = (char*)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#else
    buf = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
#endif
    ASSERT(buf != NULL);

    return buf;
}

void JITEngine::freeAlign(char*p, int size){
#ifdef _WIN32
    VirtualFree(p, size, MEM_COMMIT);
#else
    munmap(p, size);
#endif
}

int JITEngine::getCodeSize() {
    return m_codeSize;
}
char* JITEngine::getCode() {
    return m_code;
}

char* JITEngine::getFunction(const string &name) {
	return *_getFunctionEntry(name);
}
int JITEngine::setExecutable(){
#ifdef _WIN32
    DWORD oldProtect;
    bool ret = VirtualProtect(m_code, m_codeMax, PAGE_EXECUTE_READWRITE, &oldProtect);
    ASSERT(ret == true);

#elif defined(__linux__) || defined(__MACH__)
    int err = mprotect(m_code, m_codeMax, PROT_READ | PROT_EXEC);
    printf("setExecutable err=%d\n", err);
    if(err){
        printf("error=%d, %s\n", errno, strerror(errno) );
    }
    ASSERT(err == 0);

#endif
    return 0;
}

Function* JITEngine::beginBuildFunction() {
    Function* function = NULL;
    if (m_arch == JIT_X86)
        function = new FunctionX86(this, m_code + m_codeSize);
    else if (m_arch == JIT_X64) {
        if(m_os == JIT_OS_WINDOWS)
            function = new FunctionX64Windows(this, m_code + m_codeSize);
        else if(m_os == JIT_OS_LINUX || m_os == JIT_OS_MACOS)
//            function = new FunctionX64Linux(this, m_code + m_codeSize);
            function = new FunctionX64(this, m_code + m_codeSize);
    }

    function->beginBuild();

    return function;
}
void JITEngine::endBuildFunction(Function* function) {
    function->endBuild();
    *_getFunctionEntry(function->getFuncName()) = m_code + m_codeSize;
    m_codeSize += function->getCodeSize();
    ASSERT(m_codeSize <= m_codeMax);
    delete function;
}


void JITEngine::addFunctionEntry(const char* funcName, char* entry) {
    if (m_funcEntries.count(funcName) == 0) {
        int n = (int)m_funcEntries.size();
        m_funcEntries[funcName] = &m_funcPtr[n];
        m_funcPtr[n] = entry;
    }
    //m_funcEntries[funcName] = entry;
}

char** JITEngine::_getFunctionEntry(const string &name) {
    if (m_funcEntries.count(name) == 0) {
        int n = (int)m_funcEntries.size();
        m_funcEntries[name] = &m_funcPtr[n];
        return &m_funcPtr[n];
    }
	return m_funcEntries[name]; 
}
const char* JITEngine::_getLiteralStringLoc(const string &literalStr) {
    return m_literalStrs.insert(literalStr).first->c_str();
}

void JITEngine::beginBuild() { 
}
void JITEngine::endBuild() {
    for (map<string, char**>::iterator iter = m_funcEntries.begin(); iter != m_funcEntries.end(); ++iter) {
        if (*iter->second == NULL) {
            char *f = findSystemSymbol(iter->first);
            if (f == NULL) {
                printf("Can't find %s\n", iter->first.c_str());
            }
            ASSERT(f != NULL);
            *iter->second = f;
        }
        printf("%-16s: %p, %p\n", iter->first.c_str(), iter->second, *iter->second);
    }
}
//disassemble:
//  x64: objdump -b binary -m i386:x86-64 -M intel -D <code.bin>
//  x86: objdump -b binary -m i386 -M intel -D <code.bin>
void JITEngine::dumpCode() { 
    unsigned char* p = (unsigned char*)m_code;
    for (int i = 0; i < m_codeSize; i++) {
        printf("%02x ", p[i]);
    }
    printf("\n");
}

char* JITEngine::findSystemSymbol(const string& name) {
    char* func = NULL;
    const char* funcName = name.c_str();
    if (m_process == NULL) {
#ifdef _WIN32
        m_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
#else
        m_process = dlopen(NULL, RTLD_NOW);
#endif
        ASSERT(m_process != NULL);
    }

#ifdef _WIN32
    vector<HMODULE> modules;
    DWORD bytes;
    EnumProcessModules(m_process, NULL, 0, &bytes);
    modules.resize(bytes / sizeof(modules[0]));
    EnumProcessModules(m_process, &modules[0], bytes, &bytes);

    ASSERT(modules.size() > 0);

    for (int i = 0; i < (int)modules.size(); ++i) {
        if (func = (char*)GetProcAddress(modules[i], funcName)) {
            char moduleName[256] = { 0 };
            GetModuleBaseName(m_process, modules[i], moduleName, sizeof(moduleName));
            printf("%s %s: %p\n", moduleName, funcName, func);
            break;
        }
    }

//    CloseHandle(m_process);
#else
    func = (char*)dlsym(m_process, funcName);
//    dlclose(m);
#endif
    return func;
}

int JITEngine::callFunction(const string& name) {
    int ret = 0;
    char* p = getFunction(name);

    if (p) {
        printf("func %s: %p, code: %02x %02x %02x %02x\n", name.c_str(), p, p[0], p[1], p[2], p[3]);
        FP_MAIN mainFunc = (FP_MAIN)p;
        ret = mainFunc();
    }
    else {
        printf("not found %s()\n", name.c_str());
    }
    return ret;
}
//SIGSEGV
#ifndef SIGBUS
#define SIGBUS 10
#endif
static void signal_handler(int signo){
    const char *name = "";
    if(signo == SIGBUS) name="SIGBUS";
    if(signo == SIGSEGV) name="SIGSEGV";
    
    printf("signo=%d, %s\n", signo, name);
    exit(0);
}
static void signal_init(){
    for(int i=1;i<32;i++){
        signal(i, signal_handler);
    }
}

int JITEngine::executeCode() {
    int ret = 0;
    signal_init();

    setExecutable();
    FP_MAIN mainFunc = (FP_MAIN)getFunction("_start");
    if(mainFunc == NULL)
        mainFunc = (FP_MAIN)getFunction("main");
    if (mainFunc) {
        dumpCode();
        printf("mainFunc=%p\n", mainFunc);
        ret = mainFunc();
    }
    else
        printf("not found main()\n");
    return ret;
}

const char *JITEngine::getArch() {
    static const char*archs[] = {"x86", "x64", "arm", "arm64"};
    if(m_arch < sizeof(archs)/sizeof(char*))
        return archs[m_arch];
    return "unkown";
}
const char *JITEngine::getOs() {
    static const char*oss[] = {"windows", "linux", "macos"};
    if(m_os < sizeof(oss)/sizeof(char*))
        return oss[m_os];
    return "unkown";
}
