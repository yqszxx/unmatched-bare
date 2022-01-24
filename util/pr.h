#ifndef _UTIL_PR_
#define _UTIL_PR_

#include <stdint.h>

extern volatile uint32_t UART[];

void pc(char c);
void ps(char* s);

#endif //_UTIL_PR_