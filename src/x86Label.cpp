
#include "common.h"
#include "x86Label.h"

x86Label::x86Label(): m_address(NULL){
}
x86Label::~x86Label(){
	ASSERT(m_address != NULL); }
void x86Label::mark(char *address) {
    ASSERT(m_address == NULL);
    m_address = address;
    bindRefs();
}
void x86Label::addRef(char *ref) {
    m_refs.push_back(ref);
    bindRefs();
}
void x86Label::bindRefs() {
    if (m_address == NULL) return;
    for (int i = 0; i < (int)m_refs.size(); ++i) {
        *(int*)m_refs[i] = int(m_address - (m_refs[i] + 4));
    }
    m_refs.clear();
}

