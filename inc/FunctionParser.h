#ifndef __FUNCTION_PARSER__H__
#define __FUNCTION_PARSER__H__
#include <stdarg.h>

#include "JITEngine.h"
#include "Scanner.h"
using namespace std;

class FunctionParser {
public:
    FunctionParser(Function* function, Scanner* scanner);
    void parse();
private:
    void _function_define();
    void _block();
    void _statement();
    void _if();
    void _while();
    void _for();
    void _local_define_list();
    void _id_or_assignment(const string& type);
    void _expr(int rbp);
    void _expr_nud();    
    void _expr_led();
    void _expr_call(const Token& funcToken);

    int getOperatorLBP(TokenID tid);
    int getOperatorRBP(TokenID tid);

    void pushScope();
    void popScope();
    int declareArg(const string &name, const string &type);
    int declareLocal(const string &name, const string &type);
    int getLocal(const string &name);
    Label* pushContinueLabel();
    void popContinueLabel();
    Label* getLastContinueLabel();
    Label* pushBreakLabel();
    void popBreakLabel();
    Label* getLastBreakLabel();

private:
    Function*m_function;
    Scanner *m_scanner;
    vector<map<string, int> > m_nestedLocals;
    map<string, int> m_args;
    vector<Label*> m_continueLabels, m_breakLabels;
};
#endif
