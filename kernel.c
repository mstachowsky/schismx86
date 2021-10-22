#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "schismIOPort.h"
#include "schismKernelIO.h"
#include "schismPCI.h"
#include "schismPS2.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();
	kernel_printf("Schism Initialized.  Ready to Discover \n");
	_PCI_enumerate();
	
	if(_PS2_selfTest())
	{
		kernel_printf("PS/2 OK\n");
		uint32_t ps2Device = _PS2_CheckDevice();
		kernel_printf("PS/2 device is: %d %d",(int)ps2Device>>8,(int)ps2Device&0xFF);
	}
	else
		kernel_printf("PS/2 Failure \n");
		
	kernel_printf("Testing newline \n");
	for(int i = 0; i < 10; i++)
		kernel_printf("I am a newline \n");
}
