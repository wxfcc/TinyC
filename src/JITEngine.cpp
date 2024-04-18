#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/mman.h>
#endif
#include "JITEngine.h"

JITEngine::JITEngine(int arch): m_arch(arch), m_textSectionSize(0) {
    m_process = NULL;
    m_textSection = new char[MAX_TEXT_SECTION_SIZE];    // os_mallocExecutable(MAX_TEXT_SECTION_SIZE);
    memset(m_textSection, 0, MAX_TEXT_SECTION_SIZE);
    m_funcPtr = new char*[MAX_FUNC_PTR_SIZE];
    memset(m_funcPtr, 0, MAX_FUNC_PTR_SIZE);
}
JITEngine::~JITEngine() { 
	//os_freeExecutable(m_textSection); 
    delete m_textSection;
	delete(m_funcPtr);
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
#ifdef _WIN32
    DWORD oldProtect;
    bool ret = VirtualProtect(m_textSection, MAX_TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);
    ASSERT(ret == true);

#elif defined(__linux__) || defined(__MACH__)
#ifdef __MACH__
    //p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
    //ASSERT(p != NULL);
#else
    int erro = posix_memalign(&m_textSection, getpagesize(), MAX_TEXT_SECTION_SIZE);
    ASSERT(erro == 0 && p != NULL);
#endif
    erro = mprotect(m_textSection, MAX_TEXT_SECTION_SIZE, PROT_READ | PROT_EXEC);
    ASSERT(erro == 0);

#endif
    return 0;
}

Function* JITEngine::beginBuildFunction() {
    Function* builder = NULL;
    if (m_arch == JIT_X86)
        builder = new FunctionX86(this, m_textSection + m_textSectionSize);
    else if (m_arch == JIT_X64)
        builder = new FunctionX64(this, m_textSection + m_textSectionSize);

    builder->beginBuild();

    return builder;
}
void JITEngine::endBuildFunction(Function* builder) {
    builder->endBuild();
    *_getFunctionEntry(builder->getFuncName()) = m_textSection + m_textSectionSize;
    m_textSectionSize += builder->getCodeSize();
    ASSERT(m_textSectionSize <= MAX_TEXT_SECTION_SIZE);
    delete builder;
}


void JITEngine::addFunctionEntry(const char* funcName, char* entry) {
    if (m_funcEntries.count(funcName) == 0) {
        size_t n = m_funcEntries.size();
        m_funcEntries[funcName] = &m_funcPtr[n];
        m_funcPtr[n] = entry;
    }
    //m_funcEntries[funcName] = entry;
}

char** JITEngine::_getFunctionEntry(const string &name) {
    if (m_funcEntries.count(name) == 0) {
        size_t n = m_funcEntries.size();
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
            ASSERT(f != NULL);
            *iter->second = f;
        }
        printf("%-16s: %p, %p\n", iter->first.c_str(), iter->second, *iter->second);
    }
}
void JITEngine::dumpCode() { 
    unsigned char* p = (unsigned char*)m_textSection;
    for (size_t i = 0; i < m_textSectionSize; i++) {
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
    unsigned char* p = getFunction(name);

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

int JITEngine::executeCode() {
    int ret = 0;

    setExecutable();
    FP_MAIN mainFunc = (FP_MAIN)getFunction("_start");
    if (mainFunc) {
        ret = mainFunc();
    }
    else {
        mainFunc = (FP_MAIN)getFunction("main");
        if (mainFunc) {
            dumpCode();
            ret = mainFunc();
        }
        else
            printf("not found main()\n");
    }
    return ret;
}