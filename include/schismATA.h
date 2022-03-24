#ifndef ATA
#define ATA

#include "schismAHCI.h"
#include "schismKernelIO.h"

#define ATA_CMD_IDENTIFY 0xEC

//a function to send a command! Here's hoping!
void _ATA_sendID(ahcihba* HBA); //probably needs HBA

#endif
