#include <iostream>
#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(...) printf(__VA_ARGS__) 
#else 
#define DEBUG(...) 
#endif 