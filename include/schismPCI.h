//PCI Bus stuff
#ifndef SCHISM_PCI_INCLUDE
#define SCHISM_PCI_INCLUDE

#include "schismKernelIO.h"
#include "schismIOPort.h"

//PCI bus discovery stuff
#define PCI_ADDR 0xCF8 //this is where we write to make the PCI bus do stuff
#define PCI_DATA 0xCFC //this is where the data goes to/from

//offset stuff
#define PCI_DEVICE_OFFSET 16 //shift the output of a PCI bus read to get the device ID
#define PCI_ENABLE_SHIFT 31 //the enable bit is bit 32, so shift 1 by 31 to put it in the right place
#define PCI_BUS_NUM_SHIFT 16 //addresses are from 16 to 23 for bus select
#define PCI_DEVICE_NUM_SHIFT 11 //choose which device on the bus (bits 11-15)
#define PCI_FUNCTION_NUM_SHIFT 8 //choose the function number (bits 8-10)
//Note that PCI bits 0-7 are the register selection stuff, so we don't shift it

//shift definitions for PCI data
#define PCI_LOW_WORD 0xFFFF
#define PCI_HIGH_WORD 16

//PCI register definitions
#define PCI_REGISTER_IDS 0
#define PCI_REGISTER_CLASS 8

//PCI specific definitions
#define PCI_NO_DEVICE 0xFFFF //this is what you get when enumerating and nothing is there

//this function writes to the PCI port
void _PCI_writeAddr(uint32_t address);

//this function reads data from the PCI port
uint32_t _PCI_readData();

//gets a PCI bus, device, function string ready to query
uint32_t _PCI_makeBusDevFunc(uint8_t bus, uint8_t device, uint8_t funct,uint8_t regOffset);

//Enumerates PCI stuff for the first few addresses
void _PCI_enumerate();
#endif
