#ifndef CRT0
#define CRT0

#include "stdio.h"

FILE* stdout;
FILE* stdin;

void _start(void* args);
void exit(int retVal);
int main();

#endif
