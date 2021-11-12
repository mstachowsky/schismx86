#ifndef SCHISM_STDLIB
#define SCHISM_STDLIB

//standard includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

//need the multiboot stuff so that we can search the memory map for real RAM
#include "schismMultiBoot.h"
#include "schismKernelIO.h"

#define FREE_MEMORY 1
#define ALLOCATED_MEMORY 0

extern uint32_t* heapBottom;
//this is a data structure that contains the start location and the size of the heap
typedef struct{
	uint32_t heapSize;
	void* heapStart; //byte addressable for now
}heapData;

//this is the master heap record
heapData masterHeap;

//this will search for where the heapBottom is, and then figure out
//how big the heap is allowed to be
void initMalloc(heapData* heap,multiBootHeader mbh);

void* malloc(uint32_t size);
void free(void* ptr);

#endif
