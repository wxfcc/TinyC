#ifndef __SCANNER__H__
#define __SCANNER__H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <algorithm>
#include <functional>

#include <iostream>
#include "x86jit.h"
#include "x64jit.h"
using namespace std;

struct Token {
    Token(): tid(TID_EOF) {}
    explicit Token(TokenID _tt): tid(_tt) {}
    Token(TokenID _tt, const char *begin, const char *end): tid(_tt), lexeme(begin, end){}

    TokenID tid;
    string lexeme;
};

class Scanner { 
public:
    explicit Scanner(const char *src);
    Token LA(int n);
    Token next(int n);

private:
    void fetchN(int n);

private:
    vector<Token> m_LAList;
    const char *m_src;
};

#endif
