#include "schismAHCI.h"

void _AHCI_getBaseAddress(ahcihba* hostBus)
{
	//this requires us to create the address, send it to PCI, then read from it
	uint32_t pciAddr = _PCI_makeBusDevFunc(hostBus->PCIBus,hostBus->PCIDevice,hostBus->PCIFunction,ABAR_offset);
	
	_PCI_writeAddr(pciAddr);
	
	hostBus->baseAddr = _PCI_readData();
}

uint32_t _AHCI_readControlReg(ahcihba hostBus)
{
	//this requires us to create the address, send it to PCI, then read from it
	uint32_t pciAddr = _PCI_makeBusDevFunc(hostBus.PCIBus,hostBus.PCIDevice,hostBus.PCIFunction,PCI_REGISTER_CTRL);
	_PCI_writeAddr(pciAddr);
	//now return what we read
	return _PCI_readData() & 0xFFFF; //read the lower 16 bits only
}


//puts all implemented ports into an idle state before continuing AHCI configuration
void _AHCI_resetPorts(ahcihba hostBus)
{
	//get the ports
	uint32_t numPorts = _AHCI_detectPorts(hostBus);
	
	//now look through them and reset them if needed
	for(int i = 0; i < numPorts; i++)
	{
		uint32_t* pxcmd = (uint32_t *)((hostBus.baseAddr + AHCI_PORT_REGS_OFFSET + AHCI_PORT_REGS_SIZE*i+ AHCI_PORT_CMD));
		
		//read the individual things. If they are nonzero we need to reset them
		uint32_t portST = (*pxcmd) & AHCI_PORT_CMD_ST;
		uint32_t portFRE = (*pxcmd) & AHCI_PORT_CMD_FRE;
		uint32_t portFR = (*pxcmd) & AHCI_PORT_CMD_FR;
		uint32_t portCR = (*pxcmd) & AHCI_PORT_CMD_CR;
		
		if(portST | portFRE | portFR | portCR)
		{
			//this means that the port is not idle
			(*pxcmd) &= 0b11111111111111111111111111111110; //clear  the last bit
			kernel_printf("Clearing port %d. Waiting on CR bit",i);
			while((*pxcmd)&AHCI_PORT_CMD_CR){}
			kernel_printf(".......port cleared\n");
			
			if((*pxcmd)&AHCI_PORT_CMD_FRE)
			{
				kernel_printf("Clearing port %d FRE. Waiting on FR bit.",i);
			//	kernel_printf("P%dCMD = %u",i,(*pxcmd));
				(*pxcmd) &= 0b11111111111111111111111111101111; //clear 
				while((*pxcmd)&AHCI_PORT_CMD_FR){
				kernel_printf(" P%dCMD = %u\n",i,(*pxcmd));
				//char c = kernel_getch();
				}
				kernel_printf("......FRE cleared\n");
			}
		}
	}
	
}

//configures the control register to make sure that AHCI is enabled, memory space access is enabled, and that the HBA can control access
void _AHCI_configure(ahcihba* hba)
{
	
	ahcihba hostBus = *hba; //this is for convenience. We'll need the pointer, too
	
	//this requires us to create the address, send it to PCI, then read from it.  We are changing the control register
	uint32_t pciAddr = _PCI_makeBusDevFunc(hostBus.PCIBus,hostBus.PCIDevice,hostBus.PCIFunction,PCI_REGISTER_CTRL);
	
	//now create the control register data.  First, we take what is already there to avoid killing it
	uint32_t ctrlReg = _AHCI_readControlReg(hostBus);
	
	ctrlReg |= AHCI_BUS_MASTER_ENABLE;
	ctrlReg |= AHCI_MEMORY_SPACE_ENABLE;
	ctrlReg |= AHCI_IO_SPACE_ENABLE;
	
	//write the data?
	_PCI_writeDataToAddress(pciAddr,ctrlReg);
	
	//OK, so now the HBA is ready on PCI, we need to do stuff to its memory mapped stuff as well
	
	uint32_t* ghcReg = (uint32_t*)(hostBus.baseAddr + AHCI_GHC); //the address of the GHC
	
	//set its 32nd bit to 1 - we are doing this even if it's already set by BIOS
	//this enables the HBA in AHCI mode
	*ghcReg |= 1<<32; 	
	
	//reset the ports so we can do things to the HBA without breaking stuff
	_AHCI_resetPorts(hostBus);
	
	//OK, now we determine how many command slots per port exist by reading the CAP.NCS register
	//CAP.NCS is bits 8-12 of CAP, which is at offset 0. Add 1 since a value of 
	//0 means "one is available" and we count up from there.
	hba->NCS = ((*(uint32_t*)(hostBus.baseAddr)) >> 8) & 0b11111 + 1;
	
	//init the harddrive. Note this is a HUGE hack
	kernel_printf("Initializing Received FIS For HDD. FIX THIS LATER \n");
	ahciDevice* curDev = hostBus.deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		kernel_printf("Type: %u\n",curDev->type);
		curDev = curDev->next; //find the last one
	}
	if(curDev!=0) //we've found the HDD
	{
		//allocate it
		curDev->received_FIS = kernel_malloc_align(AHCI_RECEIVEDFIS_SIZE,AHCI_RECEIVEDFIS_ALIGNMENT);
		//Zero out the memory, which is "recommended"
		for(int i = 0; i < AHCI_RECEIVEDFIS_SIZE; i++)
			*(curDev->received_FIS+i) = 0;
		
		//set the pointer in the port CMD register
		uint8_t* PXFB = (uint8_t *)(hostBus.baseAddr + AHCI_PORT_REGS_OFFSET + AHCI_PORT_REGS_SIZE*curDev->port+ AHCI_PORT_FB);
		*PXFB = curDev->received_FIS;
	}
	else
	{
		kernel_printf("Uh...HDD not found...wtf do we do now?\n");
	}
	
}

//detects the number of valid ports and returns it
uint32_t _AHCI_detectPorts(ahcihba hostBus)
{
	//get the base address
	uint8_t* ahciBaseAddr = hostBus.baseAddr;//(uint8_t*)(_AHCI_getBaseAddress(hostBus));
	
	//get the ports
	uint32_t ports = *(uint32_t*)(ahciBaseAddr + AHCI_PI);
	
	//enumerate them
	int numPorts = 0;
	while(ports!=0)
	{
		numPorts += ports&1; //add one if something is there
		ports = ports>>1; //shift it right by one
	}
	
	return numPorts;
}

//Initializes devices in the device list
uint32_t _AHCI_initDeviceList(ahcihba* hostBus)
{
	//get the base address
	uint8_t* ahciBaseAddr = hostBus->baseAddr;//(uint8_t*)(_AHCI_getBaseAddress(hostBus));
	
	//initialize the device list
	hostBus->deviceList = 0; //we initialize it to null.  Trust.
	
	int numPorts = _AHCI_detectPorts(*hostBus);
	int numDevices = 0;
	
	//ok, now that we know how many ports there are, we can read those ports for valid data
	for(int i = 0; i < numPorts; i++)
	{
		uint32_t sig = *(uint32_t *)((ahciBaseAddr + AHCI_PORT_REGS_OFFSET + AHCI_PORT_REGS_SIZE*i+ AHCI_PORT_IDENT));
		if(sig != 0xFFFFFFFF) //this is the invalid device code
		{
			numDevices++;
			
			ahciDevice* curDev = 0;
			//create the device
			if(hostBus->deviceList == 0)
			{
				hostBus->deviceList = (ahciDevice*)kernel_malloc(sizeof(ahciDevice));
				hostBus->deviceList->port = i;
				hostBus->deviceList->next = 0;
				curDev = hostBus->deviceList;
			}
			else
			{	
				curDev = hostBus->deviceList;
				while(curDev->next != 0)
					curDev = curDev->next; //find the last one
				
				curDev->next = (ahciDevice*)kernel_malloc(sizeof(ahciDevice)); //now it's not null
				curDev = curDev->next;
				curDev->next = 0;
				curDev->port = i;
			}
			
			
			if(sig == AHCI_DEVICE_HDD)
			{
				curDev->type = AHCI_DEVICE_HDD;
			}
			else if (sig == AHCI_DEVICE_ODD)
			{
				curDev->type = AHCI_DEVICE_ODD;
			}
			else
				curDev->type = AHCI_DEVICE_UNKNOWN;
		}
	}
	

	
	return numDevices;
}

//gets the BDF address from the PCI bus.  PCI must be enumerated before this function works!
bool _AHCI_getBDF(pciRecord* pciBus,ahcihba* hostBus)
{
	pciRecord* pciDevice = pciBus;
	if(pciDevice->nextRecord == UNINITIALIZED_RECORD)
	{
		kernel_printf("PCI bus not initialized!\n");
		return false;
	}
	else
	{
		bool AHCI_Found = false;
		while(pciDevice->nextRecord != 0 && AHCI_Found == false)
		{
			//check for class 1 subclass 6
			if(pciDevice->deviceClass == AHCI_PCI_CLASS && pciDevice->subclass == AHCI_PCI_SUBCLASS) //this is AHCI
			{
				hostBus->PCIBus = pciDevice->bus;
				hostBus->PCIDevice = pciDevice->device;
				hostBus->PCIFunction = pciDevice->function;
				AHCI_Found = true;
			}
			else
				pciDevice = pciDevice->nextRecord;
		}
		//now we do the last one, since the while loop ends one early
		//kernel_printf("BDF: %u, %u, %u, Class: %u, Subclass: %u, IF: %u \n",pciDevice->bus,pciDevice->device,pciDevice->function,pciDevice->deviceClass,pciDevice->subclass,pciDevice->progIF);
		if(!AHCI_Found)
		{
			if(pciDevice->deviceClass == AHCI_PCI_CLASS && pciDevice->subclass == AHCI_PCI_SUBCLASS) //this is AHCI
			{
				hostBus->PCIBus = pciDevice->bus;
				hostBus->PCIDevice = pciDevice->device;
				hostBus->PCIFunction = pciDevice->function;
				AHCI_Found = true;
			}
		}
		
		if(!AHCI_Found)
		{
			kernel_printf("Problem! AHCI Not Found!\n");
			return false;
		}
		else
		{
			kernel_printf("AHCI Initialized at BDF %u, %u, %u\n",hostBus->PCIBus,hostBus->PCIDevice,hostBus->PCIFunction);
			return true;
		}
	}
}

//print devices in the device list
void _AHCI_printDevices(ahcihba hostBus)
{
	ahciDevice* curDev = hostBus.deviceList;
	while(curDev != 0)
	{
		kernel_printf("AHCI Device: Port %u, type = ",curDev->port);
		if(curDev->type == AHCI_DEVICE_HDD)
		{
			kernel_printf("HDD\n");
		}
		else if(curDev->type == AHCI_DEVICE_ODD)
		{
			kernel_printf("ODD\n");
		}
		else
			kernel_printf("Device unknown\n");
			
		curDev = curDev->next;
	}
}