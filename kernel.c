
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

//cpuid testing
#include <cpuid.h>

#include "schismIOPort.h"
#include "schismKernelIO.h"
#include "schismPCI.h"
#include "schismPS2.h"
#include "schismAHCI.h"
#include "schismMultiBoot.h"
#include "stdlib.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

//uint32_t* heapBottom __attribute__((section("heapLoc")));


extern uintptr_t heapLoc;
uint32_t* heapBottom = &heapLoc; //we can sort of put this here

extern volatile uint32_t* st;
extern volatile uint32_t* start_start;
extern volatile uint32_t* multibootHeaderLocation; 

//declare the memory detector (eventually move this to a library
//extern do_e820();
//#define MEM_MAP_LOC 0x8000
 
/* Example: Get CPU's model number.  I have ZERO clue why, but
	the input arguments make zero sense, but such is the magic of 
	inline assembly.
 */
void get_cpuData(unsigned int inputCode, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
	__get_cpuid(inputCode, eax, ebx, ecx, edx);
}


void createCharArrayFromUInt(unsigned int a,char* outChar,int startIndex)
{
	char ls = a >> 24;
	char mb1 = (a>>16)&0xFF;
	char mb2 = (a>>8)&0xFF;
	char msb = a&0xFF;
	
	outChar[startIndex] = msb;
	outChar[startIndex+1] = mb2;
	outChar[startIndex+2] = mb1;
	outChar[startIndex+3] = ls;
}

/*
	Provides the vendor string in outChar.
	
	The return value is the highest function callable with cpuID for this chip
*/
int get_cpuModel(unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx,char* outChar)
{
	get_cpuData(0,eax,ebx, ecx, edx);
	
	createCharArrayFromUInt(*ebx,outChar,0);
	createCharArrayFromUInt(*edx,outChar,4);
	createCharArrayFromUInt(*ecx,outChar,8);
	
	return *eax;
}


void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();
	//heapBottom = &heapBottom; //can you do this?
	kernel_printf("Schism Initialized.  Ready to Discover \n");
	multiBootHeader mbh;
	_MB_setFlagsAndAddr(multibootHeaderLocation, &mbh);
	_MB_FillHeader(&mbh);
	
	//_MB_printMMap(mbh);
	
	//now initialize malloc.  It's heap time!
	initMalloc(&masterHeap,mbh);
	
	//uhoh...it's malloc time
	
	kernel_printf("Block size: %u\n",*(uint32_t*)(masterHeap.heapStart));
	kernel_printf("Block Next: %u\n",*(uint32_t*)(masterHeap.heapStart+sizeof(uint32_t)));
	kernel_printf("Block Allocated: %u\n",*(uint32_t*)(masterHeap.heapStart + 2*sizeof(uint32_t)));
	char* newChar1 = (char*)malloc(15);
	char* newChar2 = (char*)malloc(51);
	char* newChar3 = (char*)malloc(523);
	free(newChar2);
	char* newChar4 = (char*)malloc(544);
	char* newChar5 = (char*)malloc(511111);
	char* newChar6 = (char*)malloc(50);
	free(newChar5);
	char* newChar = (char*)malloc(5);
	kernel_printf("Block location: %u\n",newChar);
	newChar[0] = 'H';
	newChar[1] = 'I';
	newChar[2] = '!';
	newChar[3] = '\n';
	newChar[4] = 0;
	terminal_writestring(newChar);
	
	//print out some diagnostics
	kernel_printf("Block size: %u\n",*(uint32_t*)(masterHeap.heapStart));
	
	free(newChar);
	kernel_printf("Block size: %u\n",*(uint32_t*)(masterHeap.heapStart));
	kernel_printf("Block Next: %u\n",*(uint32_t*)(masterHeap.heapStart+sizeof(uint32_t)));
	kernel_printf("Block Allocated: %u\n",*(uint32_t*)(masterHeap.heapStart + 2*sizeof(uint32_t)));
	
	//make sure it worked
//	kernel_printf("Heap start: %u, Heap size: %u \n",masterHeap.heapStart,masterHeap.heapSize);
	
	//_PCI_enumerate();
	/*
	kernel_printf("Done.  Detecting PS/2 Hardware \n");
	if(_PS2_selfTest())
	{
		kernel_printf("PS/2 OK\n");
		uint32_t ps2Device = _PS2_CheckDevice();
		kernel_printf("PS/2 device is: %d %d \n",(int)ps2Device>>8,(int)ps2Device&0xFF);
	}
	else
		kernel_printf("PS/2 Failure \n");
		
	kernel_printf("Done.  Detecting CPU Features \n");
	
	/*
		CPU Identification stuff
	*/
	/*	
	unsigned int a; 
	unsigned int b; 
	unsigned int c;
	unsigned int d;


	//First get the CPU model and highest function
	char cpuString[16] = {0};
	int highestFunction = get_cpuModel(&a,&b,&c,&d,cpuString);
	kernel_printf("Highest function: %d \n",highestFunction);
	terminal_write(cpuString,12);
	kernel_printf("\n");
	
	//reset the CPU stuff and figure out if it has certain features
	a = 0;
	b = 0; 
	c = 0;
	d = 0;
	
	//this gets the data
	get_cpuData(1,&a,&b,&c,&d);
	
	//now that we have the data, check for various features
	//check for FPU
	int hasFPU = c&0x1;
	//kernel_printf("Does it have FPU?: %d\n",hasFPU);
	
	//check for MSRs
	int hasMSRs = (d>>5)&1;
	//kernel_printf("Does it have MSRs?: %d\n",hasMSRs);
	
	//now play with AHCI
	ahcihba hba;
	hba.PCIBus = 0;
	hba.PCIDevice = 31;
	hba.PCIFunction = 2;
	
	uint8_t* ahciBaseAddr = (uint8_t*)(_AHCI_getBaseAddress(hba));
	//kernel_printf("AHCI Number of ports: %u \n",_AHCI_detectPorts(hba));
	
	int numPorts = _AHCI_detectPorts(hba);
	
	//check AHCI stuff - I need to think about this a LOT more, but:
	/*
		ahciBaseAddr is the base address of the AHCI registers.  Now, I personally have this stored
		as a pointer to a uint32_t, which means that if I add 1 to it I actually add
		4 more bytes.  The port initialized register is an offset of 12 bytes, therefore
		I need to add 3.
		
		When I do this, I get 0b11111111.  This means...I have 8 ports enabled? How do I know
		how many have stuff attached to them? Perhaps by calling identify?  This does seem wrong
		though since I shouldn't have 8 things.  The VM only has 2...
	
	kernel_printf("Enumerating AHCI\n");
	for(int i = 0; i < numPorts; i++)
	{
		uint32_t sig = *(uint32_t *)((ahciBaseAddr + 0x100 + 0x80*i+ 0x24));
		if(sig != 0xFFFFFFFF)
		{
			kernel_printf("AHCI Device found.   Signature: %u\n",sig);
			if(sig == 0x101)
				kernel_printf("    AHCI Device: SATA HDD\n");
			else if (sig == 0xEB140101)
				kernel_printf("    AHCI Device: SATA ODD\n");
		}
	}
*/	
	kernel_printf("Done.  Schism Ended.");	

}
