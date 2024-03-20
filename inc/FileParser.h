#ifndef __FILE_PARSER__H__
#define __FILE_PARSER__H__
#include "JITEngine.h"
#include "Scanner.h"
#include "FunctionParser.h"
using namespace std;

class FileParser {
public:
    FileParser(JITEngine*engine, Scanner *scanner);
    void parse();
private:
    bool parseFunction();
private:
    JITEngine*m_engine;
    Scanner *m_scanner;
};
#endif
