#include "crt0.h"

void _start(void* args)
{
	FILE** files = (FILE**)args;
	//instantiate
	stdin = files[0];
	stdout = files[1];

	//call exit(main). For now we are leaving this without arguments
	exit(main());
}

void exit(int retVal)
{
	//um...this actually does nothing right now, so we'll just return
	return;
}
