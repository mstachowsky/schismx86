#ifndef SCHISMAHCI
#define SCHISMAHCI

#include "schismPCI.h"
#include "schismKernelIO.h" //needed for debug and other info messages

//PCI offsets for various registers
#define ABAR_offset 0x24 //The AHCI Base Address (where in memory AHCI registers live) is on register 0x24h on the PCI bus
#define ABAR_shift 13; //we shift the ABAR register from the PCI bus right by 13 to get ABAR

//command register bits
#define AHCI_BUS_MASTER_ENABLE 4
#define AHCI_MEMORY_SPACE_ENABLE 2
#define AHCI_IO_SPACE_ENABLE 1

//store HBA stuff in a struct.  This will eventually encompass all of the relevant addresses and info for the HBA
typedef struct{
	//these three variables store the BDF address of the HBA on the PCI bus
	unsigned int PCIBus;
	unsigned int PCIDevice;
	unsigned int PCIFunction;
}ahcihba;

uint32_t _AHCI_readControlReg(ahcihba hostBus);
uint32_t _AHCI_getBaseAddress(ahcihba hostBus);
void _AHCI_configure(ahcihba hostBus);
uint32_t _AHCI_detectPorts(ahcihba hostBus);
#endif
