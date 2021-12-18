#ifndef SCHISMAHCI
#define SCHISMAHCI

#include "schismPCI.h"
#include "schismKernelIO.h" //needed for debug and other info messages

//PCI offsets for various registers
#define ABAR_offset 0x24 //The AHCI Base Address (where in memory AHCI registers live) is on register 0x24h on the PCI bus
#define ABAR_shift 13; //we shift the ABAR register from the PCI bus right by 13 to get ABAR

//AHCI offsets for various registers
#define AHCI_GHC 0x04 //the Global Host Control register, must be set properly during
//minimal initialization
#define AHCI_PI 0x0C //the Ports Implemented register

//port specific registers
#define AHCI_PORT_REGS_OFFSET 0x100
#define AHCI_PORT_REGS_SIZE 0x80
#define AHCI_PORT_IDENT 0x24
#define AHCI_PORT_CMD 0x18 //the port command register offset
#define AHCI_PORT_FB 0x04

//port CMD bits
#define AHCI_PORT_CMD_ST 1
#define AHCI_PORT_CMD_FRE 1<<4
#define AHCI_PORT_CMD_FR 1<<14
#define AHCI_PORT_CMD_CR 1<<15

//PCI class and subclass
#define AHCI_PCI_CLASS 0x01
#define AHCI_PCI_SUBCLASS 0x06

//hardware device signatures
#define AHCI_DEVICE_HDD 0x101
#define AHCI_DEVICE_ODD 0xEB140101
#define AHCI_DEVICE_UNKNOWN 0

//command register bits
#define AHCI_BUS_MASTER_ENABLE 4
#define AHCI_MEMORY_SPACE_ENABLE 2
#define AHCI_IO_SPACE_ENABLE 1

//Now to define stuff related to the received FIS
#define AHCI_RECEIVEDFIS_SIZE 0xFF
#define AHCI_RECEIVEDFIS_ALIGNMENT 256 //the recieved FIS must be aligned on a 256 byte boundary
#define AHCI_RECEIVEDFIS_DSFIS_OFFSET 0x00 //DMA Setup FIS
#define AHCI_RECIEVEDFIS_PSFIS_OFFSET 0x20 //PIO Setup FIS
#define AHCI_RECEIVEDFIS_RFIS_OFFSET 0x40 //this is where data gets put when we read
#define AHCI_RECEIVEDFIS_SDBFIS_OFFSET 0x58 //not a clue what this does
#define AHCI_RECEIVEDFIS_UNKNOWNFIS_OFFSET 0x60 //If the device does something weird FIS

//store HBA stuff in a struct.  This will eventually encompass all of the relevant addresses and info for the HBA
typedef struct{
	//these three variables store the BDF address of the HBA on the PCI bus
	unsigned int PCIBus;
	unsigned int PCIDevice;
	unsigned int PCIFunction;
	uint8_t* baseAddr; //the base address of the HBA's control registers
	uint32_t NCS; //number of command slots per port
	struct ahciDevice* deviceList;
}ahcihba;

//stores information about a specific device attached to the HBA
typedef struct ahciDevice{
	unsigned int port;
	unsigned int type;
	struct ahciDevice* next;
	uint8_t* cmdList; //these two pointers allow us to create the memory we need to send and receive data
	uint8_t* received_FIS; 
}ahciDevice;

ahciDevice* ahciDeviceList;

uint32_t _AHCI_readControlReg(ahcihba hostBus);
void _AHCI_getBaseAddress(ahcihba* hostBus);
void _AHCI_configure(ahcihba* hba);
uint32_t _AHCI_detectPorts(ahcihba hostBus);//gets the number of ports IMPLEMENTED, not the number of ports USED
uint32_t _AHCI_initDeviceList(ahcihba* hostBus);
bool _AHCI_getBDF(pciRecord* pciBus,ahcihba* hostBus);

#endif
