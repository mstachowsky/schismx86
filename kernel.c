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
#include "kernel_util.h"
#include "schismGDT.h"
#include "schism_PIC.h"
#include "schism_IDT.h"
#include "ISR_Test.h"
#include "schismATA.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif


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

static inline bool are_interrupts_enabled()
{
    unsigned long flags;
    asm volatile ( "pushf\n\t"
                   "pop %0"
                   : "=g"(flags) );
    return flags & (1 << 9);
}

int test = 0;

//the master heap record.  It belongs to the kernel
heapData masterHeap;

void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();
	
	kernel_printf("Schism Initialized.  Ready to Discover \n");
	
	//obtain the multiboot header
	multiBootHeader mbh;
	_MB_setFlagsAndAddr(multibootHeaderLocation, &mbh);
	_MB_FillHeader(&mbh);
	
	//Get ready for RAM data
	initRamData(mbh,&kernelMasterRam);
	
	//did it work?
	kernel_printf("Ram found. Ram start: %u, Ram size (B): %u \n",kernelMasterRam.ramStart, kernelMasterRam.ramSize);
	
	//now initialize malloc.  It's heap time!  Let's make a 4MB kernel heap for now
	kernel_printf("Initializing Malloc\n");
	initKernelMalloc(&masterHeap,kernelMasterRam,KERNEL_HEAP_MAX);
	
	//now rock the GDT
	kernel_printf("Initializing and Loading GDT\n");
	createGDT(kernelMasterRam);
	
	//that "24" tells us that the current GDT is 24 bytes.  This should be nGDTEntries*8,
	//since each entry is 8 bytes long.  My current GDT has 3 entries.  This is 
	//kludgy...
	setGDT(GDT,24);
	reloadSegments();
	
	kernel_printf("GDT Loaded.  Initializing Kernel Master Record\n");
	
	masterRecord kernelMaster;
	kernelMaster.heapptr = (&masterHeap);
	kernelMaster.mbootheader = (&mbh);
	kernelMaster.pciptr = (pciRecord*)kernel_malloc(sizeof(pciRecord));

	(kernelMaster.pciptr)->nextRecord = 0xFFFFFFFF; //Special value to indicate it's new
	
	_PCI_enumerate(kernelMaster.pciptr);
//	_PCI_output(kernelMaster.pciptr);
//	_PS2_CheckDevice();
	

	kernel_printf("Setting up interrupts\n");
	createIDT(IDT_NUM_INTERRUPTS); //why not?
	//create for me a new IDT Entry
	IDT_entry keyboardIDT ={.offset = (uint32_t)&isr_keyboard,.selector = KERNEL_CODE_SEGMENT,.gate = INTERRUPT_GATE,.DPL = RING_0,.present=VALID_DESCRIPTOR};
	IDT_entry genericIDT ={.offset = (uint32_t)&isr_generic,.selector = KERNEL_CODE_SEGMENT,.gate = INTERRUPT_GATE,.DPL = RING_0,.present=VALID_DESCRIPTOR};
	
	kernel_printf("Filling IDT \n");
	
	for(int i = 0; i < IDT_NUM_INTERRUPTS - 0x41; i++)
		packIDTEntry(genericIDT,i);
	
	kernel_printf("Done \n");	
	
	kernel_printf("Adding keyboard interrupt\n");
	packIDTEntry(keyboardIDT,1);
	
	kernel_printf("Setting up the PIC:\n");
	PIC_standard_setup();
	
	//enable the keyboard interrupt
	IRQ_enable(1);
	
	//disable the timer interrupt
	IRQ_disable(0);
	
	kernel_printf("Enable interrupts via sti command: \n");
	asm("sti");

	kernel_printf("Are interrupts enabled? %u\n",are_interrupts_enabled());
	
	ahcihba hba;
	//get the correct BDF address
	_AHCI_getBDF(kernelMaster.pciptr,&hba);
//	hba.PCIBus = 0;
//	hba.PCIDevice = 31;
//	hba.PCIFunction = 2;
	
	//uint8_t* ahciBaseAddr = (uint8_t*)(_AHCI_getBaseAddress(hba));
	_AHCI_getBaseAddress(&hba);
	_AHCI_initDeviceList(&hba);
	_AHCI_configure(&hba);
	
//	kernel_printf("AHCI Base Addr: %u\n",hba.baseAddr);
	
	//HBA diagnostic stuff. Not required unless curious
/*	kernel_printf("HBA CAP Reg: ");
	printBytesBinary(4,hba.baseAddr);
	kernel_printf("\nEnter any key to continue: \n");
	char dfs = kernel_getch();
	dfs = kernel_getch();
	dfs = kernel_getch();
*/
//	_AHCI_printDevices(hba);
	
	//OK...now let's see if we can initiate a command to the HDD
	
	//write it
	_ATA_sendDataFixed(&hba);
	
	//read it
	_ATA_receivedDataFixed(&hba);
	
/*	uint8_t* prdtData = _ATA_sendID(&hba);
	uint16_t* identData = (uint16_t*)prdtData;
	
	kernel_printf("First 32 bytes returned, in binary:\n");
	printBytesBinaryLines(32,prdtData);
	kernel_printf("\n");
	
	int injd = 256;
	//kernel_printf("Word at index %u: %u\n",injd,(uint32_t)identData[injd]);
	kernel_printf("Next five bytes after word 256: \n");
	printBytesBinaryLines(5,identData[injd+2]);*/
//	kernel_printf("\nEntering main loop: \n");
//	memExploreLoop();
	
	/*char c = kernel_getch();
	while(c != ESCAPE_BYTE)
	{
		if(c!= 0x00)
		{
			if(c!='\n' && c!= BACKSPACE_BYTE)
				terminal_putchar(c);
			else if (c == '\n')
				terminal_handle_newline();
			else
				terminal_handle_backspace();
				
			kernel_printf("Test: %u\n",test);
		}
		c = kernel_getch();
	}*/
	
	
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
