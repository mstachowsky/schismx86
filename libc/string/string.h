#ifndef STRING_H
#define STRING_H

/*
	OK, I know that we aren't SUPPOSED to include other stdlib stuff but stddef and stdint are
	supplied by the compiler
*/
#include <stddef.h>
#include <stdint.h>

//===============================================
// MEM FUNCTIONS
//===============================================

/*
	The memcpy function copies n characters from the object pointed to by s2 into
	the object pointed to by s1. If copying takes place between objects that overlap, the behaviour
	is undefined.
	
	Returns: the memcpy function returns the value of s1
*/
void* memcpy(void* s1, const void* s2, size_t n);

/*
	The memmove function copies n characters from the object pointed to by s2 into the objecte
	pointed to by s1. Copying takes place as if the n characters from the object pointed to by
	s2 are first copied into a temporary array of n characters that does not overlap the objects pointed 
	to by s1 and s2, and then the n characters from the temporray array are copied into the object
	pointed to by s1
	
	Returns: The memmove function returns the value of s1
*/
void* memmove(void* s1, const void* s2, size_t n);

//=============================================
// STR FUNCTIONS
//=============================================

/*
	The strcpy function copies the string pointed to by s2 (including the terminating null character) into
	the array pointed to by s1. If copying takes place between objects that overlap, the behaviour is undefined

	Returns: The strcpy function returns the value of s1
*/
char* strcpy(char* s1, const char*s2);
//=============================================
// STRN FUNCTIONS
//=============================================

/*
	The strncpy function copies not more than n characters (characters that follow a null character are not
	copied) from the array pointed to by s1 to the array pointed to by s1. If copying takes place between
	objects that overlap, the behaviour is undefined.
	
	If the array pointed to by s2 is a string that is shorter than n characters, null characters are appended to the copy in the array pointed
	to by s1,, until n characters in all have been written
	
	Returns: the strncpy function returns the value of s1
*/
char* strncpy(char* s1, const char* s2, size_t n);
#endif
