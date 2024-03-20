
#include "common.h"
#include "Label.h"

Label::Label(): m_address(NULL){
}
Label::~Label(){
	ASSERT(m_address != NULL); }
void Label::mark(char *address) {
    ASSERT(m_address == NULL);
    m_address = address;
    bindRefs();
}
void Label::addRef(char *ref) {
    m_refs.push_back(ref);
    bindRefs();
}
void Label::bindRefs() {
    if (m_address == NULL) return;
    for (int i = 0; i < (int)m_refs.size(); ++i) {
        *(int*)m_refs[i] = int(m_address - (m_refs[i] + 4));
    }
    m_refs.clear();
}

