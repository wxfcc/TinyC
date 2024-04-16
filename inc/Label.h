#ifndef __LABEL__H__
#define __LABEL__H__
#include <vector>
using namespace std;

class Label { 
public:
    Label();
    ~Label();
    void mark(char *address);
    void addRef(char *ref);
private:
    void bindRefs();
private:
    char *m_address;
    vector<char*> m_refs;
    int type;   // 32/64bit
};

#endif
