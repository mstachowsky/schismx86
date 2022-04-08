#include "_stdio.h"

FILE* fopen(const char* fname, const char* mode)
{
	//create the fopen struct
	FOPEN_STRUCT fop;
	
	//populate it
	fop.fName = fname;
	if(mode[0] == 'r') //yeah yeah, I know, use strcmp
	{
		if(mode[1] != 'w')
			fop.mode = READ;
		else
			fop.mode = READ_WRITE;
	}
	else if(mode[0] == 'w')
		fop.mode = WRITE;
		
	//now we need to do a system call
	systemCall(OPEN_CALL,&fop);
	
	return fop.file;
}

//write a single character to a file stream
int fputc ( int character, FILE * stream )
{
	//check to make sure that the buffer is not full
	if(stream->filePos == (long unsigned int)stream->bufferSize-1)
	{
		//fail. Set EOF and Ferr as per the spec
		stream->eof = EOF;
		stream->ferr = FERR;
		return EOF;//TODO: there is no way to fix this at the moment!
	}
	
	//otherwise we are rockin
	stream->buf[stream->filePos] = character;
	stream->filePos += 1; //increment by 1
	return 1; //anything but EOF
}

//write a single character to stdout
int putchar(int character) //yes, it's putchar,not putc, which is a different function
{
	//try to fputc first
	if(fputc(character, kstdout)==EOF)
		return EOF; //It didn't work
		
	//otherwise, kstdout has a new character, so system call it
	systemCall(PUTC_CALL,kstdout); //this sys call just needs the output stream
	return 1; //anything but EOF
}