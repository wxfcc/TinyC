#ifndef __FILE_PARSER__H__
#define __FILE_PARSER__H__
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <algorithm>
#include <functional>

#include <iostream>
#include "x86jit.h"
#include "x64jit.h"
#include "Scanner.h"
#include "FunctionParser.h"
using namespace std;

class FileParser {
public:
    FileParser(x86JITEngine *engine, Scanner *scanner);
    void parse();
private:
    bool parseFunction();
private:
    x86JITEngine *m_engine;
    Scanner *m_scanner;
};
#endif
