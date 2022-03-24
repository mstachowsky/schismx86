#include "schismATA.h"

//This is going to be one almighty function until we get the ID command working
void _ATA_sendID(ahcihba* HBA){
	//OK, we need to get various addresses out, so first we need to find the HDD.
	//By this point we are VERY sure that we have a HDD
	
	ahciDevice* curDev = HBA->deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		curDev = curDev->next; //find the last one
	}
	kernel_printf("HDD found at port: %u, type: %u\n",curDev->port,curDev->type);
	//curDev now holds all of the references to the HDD
	uint32_t prt = curDev->port;
	
	//now we need to actually access the two structures stored there
	uint32_t* cmdListAddr = _AHCI_getPortBaseAddr(prt,*HBA);
			kernel_printf("And did it stay set: %u\n",*cmdListAddr);
	uint32_t* receivedFISAddr = _AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_FB;
	kernel_printf("CmdListdfs Addr: %u\n",(uint32_t)(*cmdListAddr));
	
	//create the command FIS
	FIS_REG_H2D fis;
	kernel_memclr((uint8_t*)&fis, sizeof(FIS_REG_H2D));
	fis.fis_type = FIS_TYPE_REG_H2D;
	fis.command = ATA_CMD_IDENTIFY;	// 0xEC
	fis.device = 0;			// Master device
	fis.c = 1;				// Write command register

	
	//now set it up in the command table
	cmdTable cmdT;
	kernel_memclr((uint8_t*)&cmdT,sizeof(cmdTable));
	//copy the FIS over
	kernel_memcpy((uint8_t*)&fis,(uint8_t*)&cmdT,sizeof(FIS_REG_H2D));
		
	//now set it up in the command header
	//cmdHeader *cmdH = (cmdHeader*)cmdListAddr; //This is fast and loose since we know the port currently holds only a single commandd
	
	//kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdHeader *cmdH = (cmdHeader*)(*cmdListAddr);
	kernel_printf("CmdHeader Addr: %u\n",(uint32_t)cmdH);
	kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdH->clearBusy = 1;
	cmdH->CFL = sizeof(FIS_REG_H2D)/4; //I think this is right. It's the length in DWORDS, so #bytes/4
	cmdH->CTBA = (uint32_t)&cmdT;
	cmdH->RWBit = 1; //it's a write to the device
	cmdH->PRDTL = 1;
	kernel_printf("CmdHeader Addr: %u\n",(uint32_t)cmdH);

	kernel_printf("CmdList Addr: %u\n",(uint32_t)(*cmdListAddr));
	kernel_printf("Command Header: ");
	printBytesBinaryLines(sizeof(cmdHeader),cmdListAddr);
	
//	kernel_printf("Sending the command\n");
	//I think that does it? Let's issue the command
	
	uint32_t* portCmdEnable = _AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CI;
	uint32_t* portSACT = _AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_SACT;
	uint32_t* portIS = _AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_IS;
	uint32_t* portSERR = _AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_SERR;
	uint32_t* portTFD = _AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_TFD;
	uint32_t* portCMD = _AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CMD;
		//set the appropriate bits
	//	*portCMD |= AHCI_PORT_CMD_FRE; //allow the FIS to be written
	//	*portCMD |= 1; //set the start bit, telling the HBA to go team
		
	//_AHCI_activatePorts(*HBA);

	kernel_printf("\nTFD: %u\n",*portTFD);
	*portCmdEnable = 1; //this sets command 1

	char c = 0;
	while(c != 'e')
	{
		kernel_printf("\n");
		kernel_printf("Enter 'e' to end, anything else to continue\n");

		kernel_printf("CMDEnable: %u",*portCmdEnable);
		kernel_printf("\nCMD: %u\n",*portCMD);
		kernel_printf("IS: %u\n",*portIS);
		c = kernel_getch();
				
	}
	
	printBytesBinaryLines(32,receivedFISAddr+0x40);
	
}