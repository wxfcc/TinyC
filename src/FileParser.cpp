#include <stdio.h>

#include "FileParser.h"
using namespace std;

FileParser::FileParser(x86JITEngine *engine, Scanner *scanner): m_engine(engine), m_scanner(scanner) {
}
void FileParser::parse() { 
    while (parseFunction()); 
}

bool FileParser::parseFunction() {
    if (m_scanner->LA(1).tid != TID_EOF) {
        x86FunctionBuilder *builder = m_engine->beginBuildFunction();
        FunctionParser(builder, m_scanner).parse();
        m_engine->endBuildFunction(builder);
        return true;
    }
    return false;
}

