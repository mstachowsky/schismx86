#include "string.h"

/*
	The strcpy function copies the string pointed to by s2 (including the terminating null character) into
	the array pointed to by s1. If copying takes place between objects that overlap, the behaviour is undefined

	Returns: The strcpy function returns the value of s1
*/
char* strcpy(char* s1, const char*s2)
{
	char* su1 = s1;
	const char* su2 = s2;
	
	while((*su1++ = *su2++));
	return s1;
}
