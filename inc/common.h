#ifndef __COMMON__H__
#define __COMMON__H__
#include <stdexcept>

#define _TO_STRING(e) #e
#define TO_STRING(e) _TO_STRING(e)
#define FILE_LINE __FILE__ "(" TO_STRING(__LINE__) ")"
#define ASSERT(b) do { if(!(b)) throw logic_error(FILE_LINE " : assert failed ! " #b); } while(0)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

enum TokenID {
    TID_LP, TID_RP, TID_LBRACE, TID_RBRACE, TID_LBRACKET, TID_RBRACKET, TID_COMMA, TID_SEMICELON,
    TID_IF, TID_ELSE, TID_FOR, TID_WHILE, TID_CONTINUE, TID_BREAK, TID_RETURN,
    TID_OP_NOT, TID_OP_INC, TID_OP_DEC,
    TID_OP_ASSIGN,
    TID_OP_AND, TID_OP_OR,
    TID_OP_ADD, TID_OP_SUB, TID_OP_MUL, TID_OP_DIV, TID_OP_MOD,   
    TID_OP_LESS, TID_OP_LESSEQ, TID_OP_GREATER, TID_OP_GREATEREQ, TID_OP_EQUAL, TID_OP_NEQUAL, 
    TID_TYPE_INT, TID_TYPE_STRING, TID_TYPE_VOID,
    TID_TRUE, TID_FALSE,
    // special 
    TID_INT, TID_ID, TID_STRING,
    TID_EOF,
};

char* os_mallocExecutable(int size);
void os_freeExecutable(char *p);
char* os_findSymbol(const char *funcName);

#endif