#ifndef __X86LABEL__H__
#define __X86LABEL__H__
#include <vector>
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
