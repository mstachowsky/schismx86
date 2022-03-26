#ifndef ATA
#define ATA

#include "schismAHCI.h"
#include "schismKernelIO.h"

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_READ_DMA 0xC8

//a function to send a command! Here's hoping!
uint8_t* _ATA_sendID(ahcihba* HBA); //probably needs HBA
void _ATA_sendDataFixed(ahcihba* HBA);
void _ATA_receivedDataFixed(ahcihba* HBA);
#endif
