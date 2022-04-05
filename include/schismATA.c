#include "schismATA.h"

//This is going to be one almighty function until we get the ID command working
uint8_t* _ATA_sendID(ahcihba* HBA){
	//OK, we need to get various addresses out, so first we need to find the HDD.
	//By this point we are VERY sure that we have a HDD
	
	ahciDevice* curDev = HBA->deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		curDev = curDev->next; //find the last one
	}
	//curDev now holds all of the references to the HDD
	uint32_t prt = curDev->port;
	
	//now we need to actually access the two structures stored there
	uint32_t* cmdListAddr = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA));
	//uint32_t* PXFB = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA)+AHCI_PORT_FB);
	//uint8_t* receivedFISAddr = (uint8_t*)(*PXFB);
	
	//create the command FIS. For Identify device this is an FIS_REG_H2D
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
	
	//now create the PRDT in the command table
	cmdT.physicalRegion.dataBaseAddr = (uint8_t*)kernel_malloc_align(0x800,4);
	kernel_memclr((uint8_t*)cmdT.physicalRegion.dataBaseAddr,0x800);
	cmdT.physicalRegion.descriptionInformation = 0;
	cmdT.physicalRegion.descriptionInformation &= 1<<31; //enable interrupt on completion
	cmdT.physicalRegion.descriptionInformation += 0x800; //set the byte count to the size of the PRDT
	
	//Now set up the command header
	cmdHeader *cmdH = (cmdHeader*)(*cmdListAddr);
	kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdH->clearBusy = 1;
	cmdH->CFL = sizeof(FIS_REG_H2D)/4; //It's the length of the command header in DWords, not bytes
	cmdH->CTBA = (uint32_t)&cmdT;
	cmdH->RWBit = 1; //it's a write to the device
	cmdH->PRDTL = 1;
	
	//At this point we are ready to issue the command.
	
	uint32_t* portCmdEnable = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CI);
	
	*portCmdEnable = 1; //this sets command 1 and tells the HBA to fetch it
	
	//wait for it to complete
	while(*portCmdEnable){}
	
	return (uint8_t*)cmdT.physicalRegion.dataBaseAddr;
}

//Yet another function. Yeah, need to do this better
void _ATA_sendDataFixed(ahcihba* HBA){
	//OK, we need to get various addresses out, so first we need to find the HDD.
	//By this point we are VERY sure that we have a HDD
	
	ahciDevice* curDev = HBA->deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		curDev = curDev->next; //find the last one
	}
	//curDev now holds all of the references to the HDD
	uint32_t prt = curDev->port;
	
	//now we need to actually access the two structures stored there
	uint32_t* cmdListAddr = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA));
//	uint32_t* PXFB = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA)+AHCI_PORT_FB);
	//uint8_t* receivedFISAddr = (uint8_t*)(*PXFB);
	
	//create the command FIS. For Identify device this is an FIS_REG_H2D
	FIS_REG_H2D fis;
	kernel_memclr((uint8_t*)&fis, sizeof(FIS_REG_H2D));
	fis.fis_type = FIS_TYPE_REG_H2D;
	fis.command = ATA_CMD_WRITE_DMA;	// 0xCA
	fis.device = 0;			// Master device
	fis.c = 1;				// Write command register
	fis.lba0 = 3; //why not sector 3?
	fis.countl = 1;

	//now set it up in the command table
	cmdTable cmdT;
	kernel_memclr((uint8_t*)&cmdT,sizeof(cmdTable));

	//copy the FIS over
	kernel_memcpy((uint8_t*)&fis,(uint8_t*)&cmdT,sizeof(FIS_REG_H2D));
	
	//now create the PRDT in the command table
	cmdT.physicalRegion.dataBaseAddr = (uint8_t*)kernel_malloc_align(0x800,4);
	kernel_printf("Write PR: %u\n",cmdT.physicalRegion.dataBaseAddr);
	kernel_memclr((uint8_t*)cmdT.physicalRegion.dataBaseAddr,0x800);
	
	//make it say hello
	uint8_t* str = (uint8_t*)cmdT.physicalRegion.dataBaseAddr;
	/*str[0] = 'H';
	str[1] = 'E';
	str[2] = 'L';
	str[3] = 'L';
	str[4] = 'O';
	*/
	kernel_printf("Ready to write\n>>");
	getline((char*)str);
	
	
	cmdT.physicalRegion.descriptionInformation = 0;
	cmdT.physicalRegion.descriptionInformation &= 1<<31; //enable interrupt on completion
	cmdT.physicalRegion.descriptionInformation += 0x800; //set the byte count to the size of the PRDT
	
	//Now set up the command header
	cmdHeader *cmdH = (cmdHeader*)(*cmdListAddr);
	kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdH->clearBusy = 1;
	cmdH->CFL = sizeof(FIS_REG_H2D)/4; //It's the length of the command header in DWords, not bytes
	cmdH->CTBA = (uint32_t)&cmdT;
	cmdH->RWBit = 1; //it's a write to the device
	cmdH->PRDTL = 1;
	
	//At this point we are ready to issue the command.
	
	uint32_t* portCmdEnable = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CI);
	
	*portCmdEnable = 1; //this sets command 1 and tells the HBA to fetch it
	
	//wait for it to complete
	while(*portCmdEnable){}
	
	kernel_printf("Yo...yo...I finished what I was doing but...did I\njust fail silently? let's read the data back\n");
}

//Yet another function. Yeah, need to do this better
void _ATA_receivedDataFixed(ahcihba* HBA){
	//OK, we need to get various addresses out, so first we need to find the HDD.
	//By this point we are VERY sure that we have a HDD
	
	ahciDevice* curDev = HBA->deviceList;
	while(curDev != 0 && curDev->type != AHCI_DEVICE_HDD)
	{
		curDev = curDev->next; //find the last one
	}
	//curDev now holds all of the references to the HDD
	uint32_t prt = curDev->port;
	
	//now we need to actually access the two structures stored there
	uint32_t* cmdListAddr = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA));
//	uint32_t* PXFB = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA)+AHCI_PORT_FB);
//	uint8_t* receivedFISAddr = (uint8_t*)(*PXFB);
	
	//create the command FIS. For Identify device this is an FIS_REG_H2D
	FIS_REG_H2D fis;
	kernel_memclr((uint8_t*)&fis, sizeof(FIS_REG_H2D));
	fis.fis_type = FIS_TYPE_REG_H2D;
	fis.command = ATA_CMD_READ_DMA;	// 0xC8
	fis.device = 0;			// Master device
	fis.c = 1;				// Write command register
	fis.lba0 = 3; //why not sector 3?
	fis.countl = 1;

	//now set it up in the command table
	cmdTable cmdT;
	kernel_memclr((uint8_t*)&cmdT,sizeof(cmdTable));

	//copy the FIS over
	kernel_memcpy((uint8_t*)&fis,(uint8_t*)&cmdT,sizeof(FIS_REG_H2D));
	
	//now create the PRDT in the command table
	cmdT.physicalRegion.dataBaseAddr = (uint8_t*)kernel_malloc_align(0x800,4);
	kernel_printf("Read PR: %u\n",cmdT.physicalRegion.dataBaseAddr);
	kernel_memclr((uint8_t*)cmdT.physicalRegion.dataBaseAddr,0x800);
	
	
	
	cmdT.physicalRegion.descriptionInformation = 0;
	cmdT.physicalRegion.descriptionInformation &= 1<<31; //enable interrupt on completion
	cmdT.physicalRegion.descriptionInformation += 0x800; //set the byte count to the size of the PRDT
	
	//Now set up the command header
	cmdHeader *cmdH = (cmdHeader*)(*cmdListAddr);
	kernel_memclr((uint8_t*)cmdH,sizeof(cmdHeader));
	cmdH->clearBusy = 1;
	cmdH->CFL = sizeof(FIS_REG_H2D)/4; //It's the length of the command header in DWords, not bytes
	cmdH->CTBA = (uint32_t)&cmdT;
	cmdH->RWBit = 0; //This one is a ready
	cmdH->PRDTL = 1;
	
	//At this point we are ready to issue the command.
	
	uint32_t* portCmdEnable = (uint32_t*)(_AHCI_getPortBaseAddr(prt,*HBA) + AHCI_PORT_CI);
	
	*portCmdEnable = 1; //this sets command 1 and tells the HBA to fetch it
	
	//wait for it to complete
	while(*portCmdEnable){}
	
	//let's just print what we get
	kernel_printf("Read done!\n");
	terminal_writestring((char*)cmdT.physicalRegion.dataBaseAddr); 
	kernel_printf("\n");
}