#ifndef KERNELUTIL
#define KERNELUTIL

#include "schismMultiBoot.h"
#include "kernel_util_defs.h"

extern uint32_t* heapBottom;

void kernel_initRamData(multiBootHeader* mbh);

ramData kernelMasterRam; //this will store the master information about RAM

extern void isr_keyboard();
extern void isr_generic();
#endif
