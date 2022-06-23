#include "string.h"

/*
	The memcpy function copies n characters from the object pointed to by s2 into
	the object pointed to by s1. If copying takes place between objects that overlap, the behaviour
	is undefined.
	
	Mike's note: The way this function is written is not exactly how I would ever do it, but it works. I am following a lot of the
	implementations from Plauger's "The Standard C Library".
*/
void* memcpy(void* s1, const void* s2, size_t n)
{
	uint8_t* su1;
	const uint8_t* su2;
	
	for(su1=s1, su2=s2; 0 < n; ++su1, ++su2, --n)
	{
		*su1 = *su2;
	}
	return s1;
}
