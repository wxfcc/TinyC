#include <stdio.h>
#include <stdlib.h>

#include "FunctionParser.h"
using namespace std;

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
//============================== platform details for jit
//============================== syntax analysis
FunctionParser::FunctionParser(FunctionBuilder *builder, Scanner *scanner): m_builder(builder), m_scanner(scanner) {
}

void FunctionParser::parse() { 
    _function_define(); 
}

void FunctionParser::_function_define() {
    m_scanner->next(1); // type
    string lexeme = m_scanner->next(1).lexeme;
    m_builder->getFuncName() = lexeme;
    ASSERT(m_scanner->next(1).tid == TID_LP);
    while (m_scanner->LA(1).tid != TID_RP) {
        string type = m_scanner->next(1).lexeme;
        declareArg(m_scanner->next(1).lexeme, type);
        if (m_scanner->LA(1).tid == TID_COMMA) m_scanner->next(1);
    }
    ASSERT(m_scanner->next(1).tid == TID_RP);
    _block();
}
void FunctionParser::_block() {
    pushScope();
    ASSERT(m_scanner->next(1).tid == TID_LBRACE);
    while (m_scanner->LA(1).tid != TID_RBRACE) _statement();
    ASSERT(m_scanner->next(1).tid == TID_RBRACE);
    popScope();
}
void FunctionParser::_statement() {
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
void FunctionParser::_if() {
    Label label_true, label_false, label_end;

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
void FunctionParser::_while() {
    Label *label_break = pushBreakLabel(), *label_continue = pushContinueLabel();

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
void FunctionParser::_for() {
    pushScope();
    Label *label_continue = pushContinueLabel(), *label_break = pushBreakLabel();
    Label label_loop, label_body;

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
void FunctionParser::_local_define_list() {
    string type = m_scanner->next(1).lexeme;
    _id_or_assignment(type);
    while (m_scanner->LA(1).tid == TID_COMMA) {
        m_scanner->next(1);
        _id_or_assignment(type);
    }
    ASSERT(m_scanner->next(1).tid == TID_SEMICELON);
}
void FunctionParser::_id_or_assignment(const string &type) {
    Token idToken = m_scanner->next(1);
    declareLocal(idToken.lexeme, type);
    if (m_scanner->LA(1).tid == TID_OP_ASSIGN) {
        m_scanner->next(1);
        _expr(0);
        m_builder->storeLocal(getLocal(idToken.lexeme));
    }
}
void FunctionParser::_expr(int rbp) {
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
void FunctionParser::_expr_nud() {
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
void FunctionParser::_expr_led() {
    Token opToken = m_scanner->next(1);
    switch (opToken.tid) {
        case TID_OP_AND: case TID_OP_OR: {
                Label label_end;
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
void FunctionParser::_expr_call(const Token &funcToken) {
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

int FunctionParser::getOperatorLBP(TokenID tid) {
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
int FunctionParser::getOperatorRBP(TokenID tid) {
    switch (tid) {
        case TID_OP_NOT: 
            return 100;
        default: 
            return getOperatorLBP(tid);
    }
}

void FunctionParser::pushScope() { m_nestedLocals.resize(m_nestedLocals.size() + 1); }
void FunctionParser::popScope() { m_nestedLocals.pop_back(); }
int FunctionParser::declareArg(const string &name, const string &type) {
    ASSERT(m_args.count(name) == 0);
    int idx = -((int)m_args.size() + 1);
    return m_args[name] = idx;
}
int FunctionParser::declareLocal(const string &name, const string &type) {
    ASSERT(m_nestedLocals.back().count(name) == 0);
    int idx = 0;
    for (int i = 0; i < (int)m_nestedLocals.size(); ++i) idx += (int)m_nestedLocals[i].size();
    ASSERT(idx + 1 <= MAX_LOCAL_COUNT);
    return m_nestedLocals.back()[name] = idx;
}
int FunctionParser::getLocal(const string &name) {
    map<string, int>::iterator iter;
    for (int i = 0; i < (int)m_nestedLocals.size(); ++i) {
        iter = m_nestedLocals[i].find(name);
        if (iter != m_nestedLocals[i].end()) return iter->second;
    }
    iter = m_args.find(name);
    ASSERT(iter != m_args.end());
    return iter->second;
}
Label* FunctionParser::pushContinueLabel() { m_continueLabels.push_back(new Label()); return m_continueLabels.back(); }
void FunctionParser::popContinueLabel() { delete m_continueLabels.back(); m_continueLabels.pop_back();}
Label* FunctionParser::getLastContinueLabel() { return m_continueLabels.back(); }
Label* FunctionParser::pushBreakLabel(){ m_breakLabels.push_back(new Label()); return m_breakLabels.back(); }
void FunctionParser::popBreakLabel(){ delete m_breakLabels.back(); m_breakLabels.pop_back();}
Label* FunctionParser::getLastBreakLabel() { return m_breakLabels.back(); }

    