#include "FileParser.h"
using namespace std;

FileParser::FileParser(JITEngine*engine, Scanner *scanner): m_engine(engine), m_scanner(scanner) {
}
void FileParser::parse() { 
    while (parseFunction()); 
}

bool FileParser::parseFunction() {
    if (m_scanner->LA(1).tid != TID_EOF) {
        Function *function = m_engine->beginBuildFunction();
        FunctionParser(function, m_scanner).parse();
        m_engine->endBuildFunction(function);
        return true;
    }
    return false;
}

