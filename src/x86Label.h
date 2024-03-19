#ifndef __X86LABEL__H__
#define __X86LABEL__H__

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
#include "common.h"
using namespace std;

class x86Label { 
public:
    x86Label();
    ~x86Label();
    void mark(char *address);
    void addRef(char *ref);
private:
    void bindRefs();
private:
    char *m_address;
    vector<char*> m_refs;
};

#endif
