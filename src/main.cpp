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
#include "x86jit.h"
using namespace std;
//============================== 

static string unescape(const string &s) {
    string r;
    for (int i = 0; i < (int)s.size(); ++i) {
        if (s[i] == '\\') {
            switch (s[++i]) {
                case 't': r += '\t'; break;
                case 'n': r += '\n'; break;
                case 'r': r += '\r'; break;
                default: r += s[i]; break;
            }
        } else r += s[i];
    }
    return r;
}
static string format(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    static vector<char> buf(256);
    while ((int)buf.size() == vsnprintf(&buf[0], buf.size(), fmt, args)) {
        buf.resize(buf.size() * 3 / 2);
    }
    va_end(args);
    return &buf[0];
}
static string readFile(const string &fileName) {
    string r;
    FILE *f = fopen(fileName.c_str(), "rb");
    if (f == NULL) return r;
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    r.resize(size + 1, 0);
    int bytes = (int)fread(&r[0], size, 1, f);
    ASSERT(bytes == 1);
    fclose(f);
    return r;
}
//============================== lexical analysis
const char *lexemes[] = {
    "(", ")", "{", "}", "[", "]", ",", ";",
    "if", "else", "for", "while", "continue", "break", "return",
    "!", "++", "--",
    "=",
    "&&", "||",
    "+", "-", "*", "/", "%", 
    "<", "<=", ">", ">=", "==", "!=",
    "int", "string", "void",
    "true", "false",
};
struct Token {
    TokenID tid;
    string lexeme;
    Token(): tid(TID_EOF) {}
    explicit Token(TokenID _tt): tid(_tt) {}
    Token(TokenID _tt, const char *begin, const char *end): tid(_tt), lexeme(begin, end){}
};
static map<string, Token> setupBuildinTokens() {
    map<string, Token> tokens;
    for (int i = 0; i < ARRAY_SIZE(lexemes); ++i) tokens[lexemes[i]] = Token((TokenID)i);
    return tokens; 
}
static Token* getBuildinToken(const string &lexeme) {
    static map<string, Token> s_tokens(setupBuildinTokens());
    map<string, Token>::iterator iter = s_tokens.find(lexeme);
    return iter == s_tokens.end() ? NULL : &iter->second;
}
class Scanner { 
public:
    explicit Scanner(const char *src): m_src(src) { }
    Token LA(int n) { fetchN(n); return m_LAList[n - 1]; }
    Token next(int n) {
        fetchN(n);
        Token token = m_LAList[0];
        m_LAList.erase(m_LAList.begin(), m_LAList.begin() + n);
        return token;
    }
private:
    void fetchN(int n) {
        while ((int)m_LAList.size() < n) {
            for (; isspace(*m_src); ++m_src);

            if (m_src[0] == 0) {
                m_LAList.push_back(Token());
            } else if (m_src[0] == '/' && m_src[1] == '/') {
                while (*++m_src != '\n');
            } else if (m_src[0] == '/' && m_src[1] == '*') {
                ++m_src;
                do { ++m_src; } while(m_src[0] != '*' || m_src[1] != '/');
                m_src += 2;
            } else if (isdigit(m_src[0])) {
                const char *begin = m_src;
                while (isdigit(*++m_src));
                m_LAList.push_back(Token(TID_INT, begin, m_src));
            } else if (m_src[0] == '"') {
                const char *begin = m_src;
                while (*++m_src != '"') {
                    if (*m_src == '\\') ++m_src;
                }
                m_LAList.push_back(Token(TID_STRING, begin, ++m_src));
            } else if (isalpha(m_src[0]) || m_src[0] == '_') {
                const char *begin = m_src;
                do { ++m_src; } while(isalpha(m_src[0]) || m_src[0] == '_' || isdigit(m_src[0]));
                string lexeme(begin, m_src);
                if (Token *token = getBuildinToken(lexeme)) m_LAList.push_back(*token);
                else m_LAList.push_back(Token(TID_ID, begin, m_src));
            } else {
                string lexeme(m_src, m_src + 2);
                if (Token *token = getBuildinToken(lexeme)) {
                    m_LAList.push_back(*token);
                    m_src += 2;
                } else {
                    lexeme.assign(m_src, m_src + 1);
                    if (Token *token = getBuildinToken(lexeme)) {
                        m_LAList.push_back(*token);
                        ++m_src;
                    } else ASSERT(0 && "invalid token");
                }
            }
        }
    }
private:
    vector<Token> m_LAList;
    const char *m_src;
};
//============================== platform details for jit
#ifdef _MSC_VER
#pragma warning(disable : 4312)
#pragma warning(disable : 4311)
#include <Windows.h>
#include <Psapi.h>
#undef TokenID
#pragma comment(lib, "Psapi.lib")
#pragma warning(default : 4311)
#pragma warning(default : 4312)
char* os_mallocExecutable(int size) {
    char *p = (char*)::VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    ASSERT(p != NULL);
    return p;
}
void os_freeExecutable(char *p) {
    ASSERT(p != NULL);
	::VirtualFree(p, 0, MEM_RELEASE);
}
char* os_findSymbol(const char *funcName) {
    HANDLE process = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    ASSERT(process != NULL);

    vector<HMODULE> modules;
    DWORD bytes;
    EnumProcessModules(process, NULL, 0, &bytes);
    modules.resize(bytes / sizeof(modules[0]));
    EnumProcessModules(process, &modules[0], bytes, &bytes);

    ASSERT(modules.size() > 0);

    char *func = NULL;
    for (int i = 0; i < (int)modules.size(); ++i) {
        if (func = (char*)::GetProcAddress(modules[i], funcName)) {
            char moduleName[256] = { 0 };
            GetModuleBaseName(process, modules[i], moduleName, sizeof(moduleName));
            printf("%s %s\n", moduleName, funcName);
            break;
        }
    }

    ::CloseHandle(process);
    return func;
}
#endif
//==============================
#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
char* os_mallocExecutable(int size) {
    char *p = NULL;
    int erro = ::posix_memalign((void**)&p, ::getpagesize(), size);
    ASSERT(erro == 0 && p != NULL);
    erro = ::mprotect(p, size, PROT_READ | PROT_WRITE | PROT_EXEC);
    ASSERT(erro == 0);
    return p;
}
void os_freeExecutable(char *p) {
    ASSERT(p != NULL);
    ::free(p);
}
char* os_findSymbol(const char *funcName) {
    void *m = ::dlopen(NULL, RTLD_NOW);
    char *r = (char*)::dlsym(m, funcName);
    ::dlclose(m);
    return r;
}
#endif

//============================== syntax analysis
class FunctionParser {
public:
    FunctionParser(x86FunctionBuilder *builder, Scanner *scanner): m_builder(builder), m_scanner(scanner) {}
    void parse() { _function_define(); }
private:
    void _function_define() {
        m_scanner->next(1); // type
        m_builder->getFuncName() = m_scanner->next(1).lexeme;
        ASSERT(m_scanner->next(1).tid == TID_LP);
        while (m_scanner->LA(1).tid != TID_RP) {
            string type = m_scanner->next(1).lexeme;
            declareArg(m_scanner->next(1).lexeme, type);
            if (m_scanner->LA(1).tid == TID_COMMA) m_scanner->next(1);
        }
        ASSERT(m_scanner->next(1).tid == TID_RP);
        _block();
    }
    void _block() {
        pushScope();
        ASSERT(m_scanner->next(1).tid == TID_LBRACE);
        while (m_scanner->LA(1).tid != TID_RBRACE) _statement();
        ASSERT(m_scanner->next(1).tid == TID_RBRACE);
        popScope();
    }
    void _statement() {
        switch (m_scanner->LA(1).tid) {
            case TID_SEMICELON: m_scanner->next(1); break;
            case TID_CONTINUE: m_scanner->next(2); m_builder->jmp(getLastContinueLabel()); break;
            case TID_BREAK: m_scanner->next(2); m_builder->jmp(getLastBreakLabel()); break;
            case TID_RETURN: 
                m_scanner->next(1);
                if (m_scanner->LA(1).tid != TID_SEMICELON) {
                    _expr(0);
                    m_builder->retExpr();
                } else m_builder->ret();
                ASSERT(m_scanner->next(1).tid == TID_SEMICELON);
                break;
            case TID_LBRACE: _block(); break;
            case TID_IF: _if(); break;
            case TID_WHILE: _while(); break;
            case TID_FOR: _for(); break;
            case TID_TYPE_STRING: case TID_TYPE_INT: _local_define_list(); break;
            case TID_TYPE_VOID: ASSERT(0); break;
            default: _expr(0); m_builder->pop(1); ASSERT(m_scanner->next(1).tid == TID_SEMICELON); break;
        }
    }
    void _if() {
        x86Label label_true, label_false, label_end;

        m_scanner->next(2);
        _expr(0);
        ASSERT(m_scanner->next(1).tid == TID_RP);
        m_builder->trueJmp(&label_true);
        m_builder->jmp(&label_false);

        m_builder->markLabel(&label_true);
        _statement();
        m_builder->jmp(&label_end);

        m_builder->markLabel(&label_false);
        if (m_scanner->LA(1).tid == TID_ELSE) {
            m_scanner->next(1);
            _statement();
        }

        m_builder->markLabel(&label_end);
    }
    void _while() {
        x86Label *label_break = pushBreakLabel(), *label_continue = pushContinueLabel();

        m_builder->markLabel(label_continue);
        m_scanner->next(2);
        _expr(0);
        ASSERT(m_scanner->next(1).tid == TID_RP);
        m_builder->falseJmp(label_break);

        _statement();
        m_builder->jmp(label_continue);
        m_builder->markLabel(label_break);

        popBreakLabel(); popContinueLabel();
    }
    void _for() {
        pushScope();
        x86Label *label_continue = pushContinueLabel(), *label_break = pushBreakLabel();
        x86Label label_loop, label_body;

        m_scanner->next(2);
        switch (m_scanner->LA(1).tid) {
            case TID_SEMICELON: break;
            case TID_TYPE_INT: case TID_TYPE_STRING: _local_define_list(); break;
            default: _expr(0); m_builder->pop(1); ASSERT(m_scanner->next(1).tid == TID_SEMICELON); break;
        }

        m_builder->markLabel(&label_loop);
        if (m_scanner->LA(1).tid != TID_SEMICELON) _expr(0);
        else m_builder->loadImm(1);
        ASSERT(m_scanner->next(1).tid == TID_SEMICELON);
        m_builder->falseJmp(label_break);
        m_builder->jmp(&label_body);

        m_builder->markLabel(label_continue);
        if (m_scanner->LA(1).tid != TID_RP) {
            _expr(0); 
            m_builder->pop(1);
        }
        ASSERT(m_scanner->next(1).tid == TID_RP);
        m_builder->jmp(&label_loop);

        m_builder->markLabel(&label_body);
        _statement();
        m_builder->jmp(label_continue);

        m_builder->markLabel(label_break);
        popContinueLabel(); popBreakLabel();
        popScope();
    }
    void _local_define_list() {
        string type = m_scanner->next(1).lexeme;
        _id_or_assignment(type);
        while (m_scanner->LA(1).tid == TID_COMMA) {
            m_scanner->next(1);
            _id_or_assignment(type);
        }
        ASSERT(m_scanner->next(1).tid == TID_SEMICELON);
    }
    void _id_or_assignment(const string &type) {
        Token idToken = m_scanner->next(1);
        declareLocal(idToken.lexeme, type);
        if (m_scanner->LA(1).tid == TID_OP_ASSIGN) {
            m_scanner->next(1);
            _expr(0);
            m_builder->storeLocal(getLocal(idToken.lexeme));
        }
    }
    void _expr(int rbp) {
        if (m_scanner->LA(1).tid == TID_ID && m_scanner->LA(2).tid == TID_OP_ASSIGN) {
            Token idToken = m_scanner->next(2);
            _expr(0);
            m_builder->dup();
            m_builder->storeLocal(getLocal(idToken.lexeme));
        } else {
            _expr_nud();
            while (rbp < getOperatorLBP(m_scanner->LA(1).tid)) _expr_led();
        }
    }
    void _expr_nud() {
        Token token = m_scanner->next(1);
        switch (token.tid) {
            case TID_TRUE: m_builder->loadImm(1); break;
            case TID_FALSE: m_builder->loadImm(0); break;
            case TID_INT: m_builder->loadImm(atoi(token.lexeme.c_str())); break;
            case TID_STRING: m_builder->loadLiteralStr(unescape(token.lexeme.substr(1, token.lexeme.size() - 2))); break;
            case TID_ID: 
                if (m_scanner->LA(1).tid == TID_LP) _expr_call(token);
                else m_builder->loadLocal(getLocal(token.lexeme)); 
                break;
            case TID_LP: _expr(0); ASSERT(m_scanner->next(1).tid == TID_RP); break;
            case TID_OP_NOT: 
                 _expr(getOperatorRBP(token.tid)); 
                 m_builder->loadImm(0);
                 m_builder->cmp(TID_OP_EQUAL);
                 break;
            case TID_OP_INC: 
            case TID_OP_DEC: {
                    int localIdx = getLocal(m_scanner->next(1).lexeme);
                    if (token.tid == TID_OP_INC) m_builder->incLocal(localIdx);
                    else m_builder->decLocal(localIdx);
                    m_builder->loadLocal(localIdx);
                } break;
            default: 
                ASSERT(0); break;
        }
    }
    void _expr_led() {
        Token opToken = m_scanner->next(1);
        switch (opToken.tid) {
            case TID_OP_AND: case TID_OP_OR: {
                    x86Label label_end;
                    m_builder->dup();
                    if (opToken.tid == TID_OP_AND) m_builder->falseJmp(&label_end);
                    else m_builder->trueJmp(&label_end);
                    m_builder->pop(1);
                    _expr(getOperatorRBP(opToken.tid));
                    m_builder->markLabel(&label_end);
                } break;
            case TID_OP_LESS: case TID_OP_LESSEQ: case TID_OP_GREATER: case TID_OP_GREATEREQ: case TID_OP_EQUAL: case TID_OP_NEQUAL:
                _expr(getOperatorRBP(opToken.tid));
                m_builder->cmp(opToken.tid);
                break;
            case TID_OP_ADD: case TID_OP_SUB: case TID_OP_MUL: case TID_OP_DIV: case TID_OP_MOD:
                _expr(getOperatorRBP(opToken.tid));
                m_builder->doArithmeticOp(opToken.tid);
                break;
            default: ASSERT(0); break;
        }
    }
    void _expr_call(const Token &funcToken) {
        ASSERT(m_scanner->next(1).tid == TID_LP);
        int callID = m_builder->beginCall();
        int paramCount = 0;
        while (m_scanner->LA(1).tid != TID_RP) {
            ++paramCount;
            _expr(0);
            if (m_scanner->LA(1).tid == TID_COMMA) m_scanner->next(1);
        }
        ASSERT(m_scanner->next(1).tid == TID_RP);
        m_builder->endCall(funcToken.lexeme, callID, paramCount);
    }
private:
    static int getOperatorLBP(TokenID tid) {
        switch (tid) {
            case TID_RP: 
            case TID_COMMA: 
            case TID_SEMICELON: 
            	return 0;
            case TID_OP_AND: 
            case TID_OP_OR: 
            	return 10;
            case TID_OP_LESS: 
            case TID_OP_LESSEQ: 
            case TID_OP_GREATER: 
        	case TID_OP_GREATEREQ: 
    		case TID_OP_EQUAL: 
        	case TID_OP_NEQUAL:
                return 20;
            case TID_OP_ADD: 
            case TID_OP_SUB: 
            	return 30;
            case TID_OP_MUL: 
        	case TID_OP_DIV: 
        	case TID_OP_MOD: 
        		return 40;
            default: 
            	ASSERT(0); 
            	return 0;
        }
    }
    static int getOperatorRBP(TokenID tid) {
        switch (tid) {
            case TID_OP_NOT: 
            	return 100;
            default: 
            	return getOperatorLBP(tid);
        }
    }
private:
    void pushScope() { m_nestedLocals.resize(m_nestedLocals.size() + 1); }
    void popScope() { m_nestedLocals.pop_back(); }
    int declareArg(const string &name, const string &type) {
        ASSERT(m_args.count(name) == 0);
        int idx = -((int)m_args.size() + 1);
        return m_args[name] = idx;
    }
    int declareLocal(const string &name, const string &type) {
        ASSERT(m_nestedLocals.back().count(name) == 0);
        int idx = 0;
        for (int i = 0; i < (int)m_nestedLocals.size(); ++i) idx += (int)m_nestedLocals[i].size();
        ASSERT(idx + 1 <= MAX_LOCAL_COUNT);
        return m_nestedLocals.back()[name] = idx;
    }
    int getLocal(const string &name) {
        map<string, int>::iterator iter;
        for (int i = 0; i < (int)m_nestedLocals.size(); ++i) {
            iter = m_nestedLocals[i].find(name);
            if (iter != m_nestedLocals[i].end()) return iter->second;
        }
        iter = m_args.find(name);
        ASSERT(iter != m_args.end());
        return iter->second;
    }
    x86Label* pushContinueLabel() { m_continueLabels.push_back(new x86Label()); return m_continueLabels.back(); }
    void popContinueLabel() { delete m_continueLabels.back(); m_continueLabels.pop_back();}
    x86Label* getLastContinueLabel() { return m_continueLabels.back(); }
    x86Label* pushBreakLabel(){ m_breakLabels.push_back(new x86Label()); return m_breakLabels.back(); }
    void popBreakLabel(){ delete m_breakLabels.back(); m_breakLabels.pop_back();}
    x86Label* getLastBreakLabel() { return m_breakLabels.back(); }
private:
    x86FunctionBuilder *m_builder;
    Scanner *m_scanner;
    vector<map<string, int> > m_nestedLocals;
    map<string, int> m_args;
    vector<x86Label*> m_continueLabels, m_breakLabels;
};
class FileParser {
public:
    FileParser(x86JITEngine *engine, Scanner *scanner): m_engine(engine), m_scanner(scanner) {}
    void parse() { while (parseFunction()); }
private:
    bool parseFunction() {
        if (m_scanner->LA(1).tid != TID_EOF) {
            x86FunctionBuilder *builder = m_engine->beginBuildFunction();
            FunctionParser(builder, m_scanner).parse();
            m_engine->endBuildFunction(builder);
            return true;
        }
        return false;
    }
private:
    x86JITEngine *m_engine;
    Scanner *m_scanner;
};
//============================== 
static x86JITEngine* g_jitEngine;
static int loadFile(const char *fileName) {
    try {
        printf("loadFile: %s\n", fileName);
        g_jitEngine->beginBuild();
        string source = readFile(fileName);
        Scanner scanner(source.c_str());
        FileParser(g_jitEngine, &scanner).parse();
        g_jitEngine->endBuild();
        return 0;
    } catch(const exception &e) {
        puts(e.what());
        return 1;
    }
}

int handleInput(){
    int lineNum = 0;
    for (string line; ; ++lineNum) {
        printf(">>> ");
        if (!getline(cin, line)) break;
		line.erase(line.begin(), std::find_if(line.begin(), line.end(), std::not1(std::ptr_fun(::isspace))));
        if (line.size() == 0)continue;
        if (line == "q")break;

        try {
            Scanner scanner(line.c_str());

            bool isFuncDefine = false;
            switch (scanner.LA(1).tid) {
                case TID_TYPE_INT: 
                case TID_TYPE_STRING: 
                case TID_TYPE_VOID: 
                    if (scanner.LA(2).tid == TID_ID && scanner.LA(3).tid == TID_LP) isFuncDefine = true;
                    break;
                default: 
                	break;
            }

            if (!isFuncDefine) {
                line = format("int temp_%d(){ return %s; }", lineNum, line.c_str());
                scanner = Scanner(line.c_str());
            }

            g_jitEngine->beginBuild();
            FileParser(g_jitEngine, &scanner).parse();
            g_jitEngine->endBuild();

            if (!isFuncDefine) {
            	string tf = format("temp_%d", lineNum);
            	unsigned char*p = g_jitEngine->getFunction(tf);

	            printf("func %s: %p, code: %02x %02x %02x %02x\n", tf.c_str(), p, p[0], p[1], p[2], p[3]);
            	int(*f)() = (int(*)())p;
            	
                printf("ret=%d\n", f());
            }

        } catch(const exception &e) {
            puts(e.what());
        } 
    }
    return 0;	
}

int handleFile(char *file){
    int ret = loadFile(file);
    if(ret == 0){
    	int(*mainFunc)() = (int(*)())g_jitEngine->getFunction("main");
    	ret = mainFunc();
    }
	return ret;
}

x86JITEngine* createX86JitEngine(){
    x86JITEngine *engine = new x86JITEngine();
    *engine->_getFunctionEntry("loadFile") = (char*)loadFile;
    *engine->_getFunctionEntry("runFile") = (char*)handleFile;
    *engine->_getFunctionEntry("printf") = (char*)printf;
	return engine;
}

int main(int argc, char *argv[]) {
	int ret = 0;
	g_jitEngine = createX86JitEngine();

    if (argc >= 2) {
        ret = handleFile(argv[1]);
    }
    else{
    	ret = handleInput();    	
    }    

    return ret;
}
