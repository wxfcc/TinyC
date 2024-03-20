#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "JITEngine.h"
#include "FunctionBuilder.h"

FunctionBuilder::FunctionBuilder(JITEngine*parent, char *codeBuf): m_parent(parent), m_codeBuf(codeBuf), m_codeSize(0){
}
//FunctionBuilder* FunctionBuilder::newBuilder(JITEngine* parent, char* codeBuf) {
//    return NULL;
//}

string& FunctionBuilder::getFuncName() {
	return m_funcName;
}
int FunctionBuilder::getCodeSize() const{ 
	return m_codeSize;
}

//void FunctionBuilder::beginBuild(){
//}
//void FunctionBuilder::endBuild(){
//}

//void FunctionBuilder::loadImm(int imm){
//}
//void FunctionBuilder::loadLiteralStr(const string &literalStr){
//}
//void FunctionBuilder::loadLocal(int idx){
//}
//void FunctionBuilder::storeLocal(int idx) {
//}
//void FunctionBuilder::incLocal(int idx) {
//}
//void FunctionBuilder::decLocal(int idx) {
//}
//void FunctionBuilder::pop(int n){
//}
//void FunctionBuilder::dup(){
//}
//
//void FunctionBuilder::doArithmeticOp(TokenID opType) {
//}
//void FunctionBuilder::cmp(TokenID cmpType) {
//}

void FunctionBuilder::markLabel(Label *label){ 
    label->mark(m_codeBuf + m_codeSize); 
}
//void FunctionBuilder::jmp(Label *label) { 
//}
//void FunctionBuilder::trueJmp(Label *label) {
//}
//void FunctionBuilder::falseJmp(Label *label) {
//}
//void FunctionBuilder::ret() { 
//    jmp(&m_retLabel); 
//}
//void FunctionBuilder::retExpr() {
//}
//
//int FunctionBuilder::beginCall(){
//    return 0;
//}
//void FunctionBuilder::endCall(const string &funcName, int callID, int paramCount){
//}

void FunctionBuilder::emit(int n, ...) {
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; ++i) 
        m_codeBuf[m_codeSize++] = (char)va_arg(args, int);
    va_end(args);
}

template<typename T>
void FunctionBuilder::emitValue(T val) {
    memcpy(m_codeBuf + m_codeSize, &val, sizeof(val));
    m_codeSize += sizeof(val);
}

//void FunctionBuilder::condJmp(TokenID tid, Label *label) {
//}
//
// calc ebp offset for local var
//int FunctionBuilder::localIdx2EbpOff(int idx) { 
//	return 0; 
//}

