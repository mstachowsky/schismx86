#include "string.h"
/*
	The strncpy function copies not more than n characters (characters that follow a null character are not
	copied) from the array pointed to by s1 to the array pointed to by s1. If copying takes place between
	objects that overlap, the behaviour is undefined.
	
	If the array pointed to by s2 is a string that is shorter than n characters, null characters are appended to the copy in the array pointed
	to by s1,, until n characters in all have been written
	
	Returns: the strncpy function returns the value of s1
	
	MIKE'S NOTE: I chose to copy this from Plauger's book, because I didn't want to figure out
	the special cases
*/
char* strncpy(char* s1, const char* s2, size_t n)
{
	char* s;
	
	for(s = s1; 0<n && *s2 != 0; --n)
		*s++ = *s2++;
	for(;0<n;--n)
		*s++ = 0;
	return s1;
}