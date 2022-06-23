#include "string.h"

/*
	The memmove function copies n characters from the object pointed to by s2 into the objecte
	pointed to by s1. Copying takes place as if the n characters from the object pointed to by
	s2 are first copied into a temporary array of n characters that does not overlap the objects pointed 
	to by s1 and s2, and then the n characters from the temporray array are copied into the object
	pointed to by s1
	
	Returns: The memmove function returns the value of s1
*/
void* memmove(void* s1, const void* s2, size_t n)
{
	char* sc1;
	const char* sc2;
	
	sc1 = s1;
	sc2 = s2;
	
	//check for overlap. If so, copy backwards
	if(sc2<sc1 && sc1<sc2+n)
	{
		for(sc1+=n,sc2+=n;0<n;--n)
		{
			*--sc1 = *--sc2;
		}
	}
	else
	{
		for(;0<n;--n)
		{
			*sc1++ = *sc2++;
		}
	}
	return s1;
}