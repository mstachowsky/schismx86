#include "stdlib.h"

//this will search for where the heapBottom is, and then figure out
//how big the heap is allowed to be
void initMalloc(heapData* heap,multiBootHeader mbh)
{
	kernel_printf("Warning: Malloc currently only allocates uint32_ts.\n");
	kernel_printf("TODO: Fix this to allocate bytes\n");
	//OK, this is fairly "simple" - find the memory region that contains the
	//heap bottom, which is a global, and then return it
	
	uint32_t byteCount = 0; //gets us out of the while loop
	int i = 0; //keeps track of where in the structure we are
	while(byteCount < mbh.mmap_length)
	{
		//need to know how big this region is to increment byteCount
		uint32_t memregionSize = mbh.mmap_addr[i];
		i+=1; //get ready to read the next uint32_t
		
		uint32_t baseAddr = mbh.mmap_addr[i] + mbh.mmap_addr[i+1];
		i+=2;
		uint32_t size = mbh.mmap_addr[i] + mbh.mmap_addr[i+1];
		i+=2;
		
		//the last part is the type of memory
		uint32_t type = mbh.mmap_addr[i];
		i++;
		
		//increment byteCount: add the toal bytes of the memory region, plus
		//a sizeof(uint32_t) to account for the size variable of the region
		byteCount += memregionSize + sizeof(uint32_t);
		
		//figure out the final address of the region
		uint32_t endAddr = baseAddr + size;
		
		//now check to see if our pointer is in the right range and the type is available RAM
		if((heapBottom > (uint32_t*)baseAddr) && (heapBottom < (uint32_t*)endAddr) && type == 1)
		{
			//we've found our region
			heap->heapStart = (void*)heapBottom;
			heap->heapSize = size;
			//write the data to it
			*(uint32_t*)(heap->heapStart) = heap->heapSize; //indicates the size of this block.  At the
			//start, the block is all of available memory
			*(uint32_t*)(heap->heapStart+sizeof(uint32_t)) = 0; //indicates no more blocks
			*(uint32_t*)(heap->heapStart+2*sizeof(uint32_t)) = FREE_MEMORY; //indicates free memory
			return;
		}
	}
}

//TODO: Fix this to use uint8_ts and cast UP to uint32_ts
//so that malloc actually allocates bytes
void* malloc(uint32_t size)
{
	//begin at the first block
	uint32_t* block = (uint32_t*)masterHeap.heapStart;
	bool memFound = false;
	while(!memFound)
	{
		uint32_t blockSize = *(block);
		if(blockSize >= size && *(block+2) == FREE_MEMORY) //we've found a block
		{
			//set the size of the current block and allocate it
			*(block) = size;
			//kernel_printf("Block location: %u, Real size: %u\n",block,size);
			*(block + 2) = ALLOCATED_MEMORY; //memory is in use
			
			//Make this block smaller if we can store anything in it
			if(blockSize - size > 3)
			{
				//if this block can still be used, cut it down in size
				
				//the next block will be at block + size + 3 because 
				//size, next pointer, and allocation status live in memory as well
				
				*(block+size+3) = blockSize - size - 3; //set the new size
				*(block+size+4) = *(block + 1); //set the next address of the next block
				//to the next address of this block
				
				//we know for sure that this is free memory, so set it as such
				*(block+size+5) = FREE_MEMORY;
				
				//point this block to it
				*(block + 1) = (block+size+3);
				
			}
			
			//if this block is the exact right size we won't cut it
			
			//return the pointer to memory
			return (void*)(block+3); //return the memory location 3 after the start of this block
		}
		else
		{
			//we need the next block
			if(*(block+1) != 0) //this indicates there is no more memory
				block = *(uint32_t*)(block+1); //move to the next pointer
			else
				return (void*)0; //fail! We are out of memory. 
		}
	}
}

void free(void* ptr)
{
	//we just need to set this block to free
	uint32_t* block = ((uint32_t*)ptr);
	*(block-1) = FREE_MEMORY; //set to free
}

