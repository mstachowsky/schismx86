#include "schismAHCI.h"

uint32_t _AHCI_getBaseAddress(ahcihba hostBus)
{
	//this requires us to create the address, send it to PCI, then read from it
	uint32_t pciAddr = _PCI_makeBusDevFunc(hostBus.PCIBus,hostBus.PCIDevice,hostBus.PCIFunction,ABAR_offset);
	
	_PCI_writeAddr(pciAddr);
	
	return _PCI_readData();
}

uint32_t _AHCI_readControlReg(ahcihba hostBus)
{
	//this requires us to create the address, send it to PCI, then read from it
	uint32_t pciAddr = _PCI_makeBusDevFunc(hostBus.PCIBus,hostBus.PCIDevice,hostBus.PCIFunction,PCI_REGISTER_CTRL);
	_PCI_writeAddr(pciAddr);
	//now return what we read
	return _PCI_readData() & 0xFFFF; //read the lower 16 bits only
}

//configures the control register to make sure that AHCI is enabled, memory space access is enabled, and that the HBA can control access
void _AHCI_configure(ahcihba hostBus)
{
	//this requires us to create the address, send it to PCI, then read from it.  We are changing the control register
	uint32_t pciAddr = _PCI_makeBusDevFunc(hostBus.PCIBus,hostBus.PCIDevice,hostBus.PCIFunction,PCI_REGISTER_CTRL);
	
	//now create the control register data.  First, we take what is already there to avoid killing it
	uint32_t ctrlReg = _AHCI_readControlReg(hostBus);
	
	ctrlReg |= AHCI_BUS_MASTER_ENABLE;
	ctrlReg |= AHCI_MEMORY_SPACE_ENABLE;
	ctrlReg |= AHCI_IO_SPACE_ENABLE;
	
	//write the data?
	_PCI_writeDataToAddress(pciAddr,ctrlReg);	
}

//detects the number of valid ports and returns it
uint32_t _AHCI_detectPorts(ahcihba hostBus)
{
	//get the base address
	uint8_t* ahciBaseAddr = (uint8_t*)(_AHCI_getBaseAddress(hostBus));
	
	//get the ports
	uint32_t ports = *(uint32_t*)(ahciBaseAddr + 0x0C);
	
	//enumerate them
	int numPorts = 0;
	while(ports!=0)
	{
		numPorts += ports&1; //add one if something is there
		ports = ports>>1; //shift it right by one
	}
	
	return numPorts;
}
