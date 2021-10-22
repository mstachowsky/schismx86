#include "schismPCI.h"


//this function writes to the PCI port
void _PCI_writeAddr(uint32_t address)
{
	_IOPORT_writeDWord((uint16_t)PCI_ADDR,address);
}

//this function reads data from the PCI port
uint32_t _PCI_readData()
{
	return _IOPORT_readDWord((uint16_t)(PCI_DATA));
}

//gets a PCI bus, device, function string ready to query
uint32_t _PCI_makeBusDevFunc(uint8_t bus, uint8_t device, uint8_t funct,uint8_t regOffset)
{
	uint32_t addr = 0;
	addr |= bus<<PCI_BUS_NUM_SHIFT;
	addr |= device << PCI_DEVICE_NUM_SHIFT;
	addr |= funct << PCI_FUNCTION_NUM_SHIFT;
	addr |= 1 << PCI_ENABLE_SHIFT;
	addr |= regOffset;
	return addr;
}

/*
	Enumerates PCI stuff.  This is SUPER brute force: I go through every
	bus, every device on each bus, and every function
*/
void _PCI_enumerate()
{
	kernel_printf("Enumerating PCI Devices \n");
	uint8_t bus = 0;
	uint8_t dev = 0;
	uint8_t function = 0;
	for(bus=0; bus < 255; bus++)
	{
		for(dev=0; dev < 32; dev++)
		{
			for(function = 0; function < 8; function++)
			{
				//Set up the read by reading the vendor and device ID
				uint32_t pciAddr = _PCI_makeBusDevFunc(bus,dev,function,PCI_REGISTER_IDS);
				_PCI_writeAddr((uint32_t)pciAddr);
				uint32_t outData = _PCI_readData();
				uint32_t vendorID = outData & PCI_LOW_WORD; //only want lower bytes
				uint32_t deviceID = (outData >> PCI_HIGH_WORD);
				if(vendorID != PCI_NO_DEVICE)
				{
					//we found one! So now we want its data.
					
					//get vendor and device ID first
					kernel_printf("Valid device. Bus: %d Device: %d Function: %d\n",bus,dev,function);
					
					//now get the class and subclass
					uint32_t pciAddr = _PCI_makeBusDevFunc(bus,dev,function,PCI_REGISTER_CLASS);
					_PCI_writeAddr((uint32_t)pciAddr);
					outData = _PCI_readData();
					uint32_t classData = outData >> PCI_HIGH_WORD; //high byte is class code, low byte is subclass
					uint32_t extraData = outData & PCI_LOW_WORD; //high byte is Prog IF, low byte is revision ID
					kernel_printf("\t Class Code: %d Subclass: %d Prog IF: %d \n",classData>>8,classData&0xFF,extraData>>8);	
				}
			}
		}
	}
	kernel_printf("Done\n");
}
