#ifndef __COMMON__H__
#define __COMMON__H__
#include <stdexcept>

typedef long long int64;
typedef unsigned char byte;

#define _TO_STRING(e) #e
#define TO_STRING(e) _TO_STRING(e)
#define FILE_LINE __FILE__ "(" TO_STRING(__LINE__) ")"
#define ASSERT(b) do { if(!(b)) throw logic_error(FILE_LINE " : assert failed ! " #b); } while(0)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#endif
