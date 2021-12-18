#include "stdlib.h"

void initKernelMalloc(heapData* heap,ramData masterRam,uint32_t heapMaxSize)
{
	//this actually isn't SUPER difficult now that kernel_util handles all the memory lookup.
	//the heap goes right at heapBottom.  We jsut need to make sure we didn't accidentally
	//ask for more memory than we have
	
	if(heapMaxSize > masterRam.ramSize)
	{
		kernel_printf("Malloc initialization failure: Heap is too big\n");
		heap->heapStart = 0; //nullify it
		return;
	}
	
	//if not, we're rockin'.  Let's go!
	heap->heapStart = (void*)heapBottom;
	heap->heapSize = heapMaxSize;
	
	//now set up the freelist
	*(uint32_t*)(heap->heapStart) = heap->heapSize; //indicates the size of this block.  At the
	//start, the block is all of available memory
	*(uint32_t*)(heap->heapStart+sizeof(uint32_t)) = 0; //indicates no more blocks
	*(uint32_t*)(heap->heapStart+2*sizeof(uint32_t)) = FREE_MEMORY; //indicates free memory
	
	kernel_printf("Malloc allocated at %u with size %u\n",heapBottom,heapMaxSize);
}

//allocates size bytes on the heap
void* kernel_malloc(uint32_t size)
{
	//begin at the first block
	uint32_t* block = (uint32_t*)masterHeap.heapStart;

	uint8_t* byteBlock = (uint8_t*)block;
	bool memFound = false;
	while(!memFound)
	{
		uint32_t blockSize = block[0];
		if(blockSize >= size && block[2] == FREE_MEMORY) //we've found a block
		{
			//set the size of the current block and allocate it
			block[0] = size;
			//kernel_printf("Block location: %u, Real size: %u\n",block,size);
			block[2] = ALLOCATED_MEMORY; //memory is in use
			
			//Make this block smaller if we can store anything in it
			if(blockSize - size > 3)
			{
				//if this block can still be used, cut it down in size
				
				//the next block will be at block + size + 3 because 
				//size, next pointer, and allocation status live in memory as well
				
				*(uint32_t*)(byteBlock+size+3*sizeof(uint32_t)) = blockSize - size - 3; //set the new size
				*(uint32_t*)(byteBlock+size+4*sizeof(uint32_t)) = *(block + 1); //set the next address of the next block
				//to the next address of this block
				
				//we know for sure that this is free memory, so set it as such
				*(uint32_t*)(byteBlock+size+5*sizeof(uint32_t)) = FREE_MEMORY;
				
				//point this block to it
				*(block + 1) = (byteBlock+size+3*sizeof(uint32_t));
				
			}
			else
				block[1] = 0; //no more blocks!
			
			//if this block is the exact right size we won't cut it
			//return the pointer to memory
			return (void*)(block+3); //return the memory location 3 after the start of this block
		}
		else
		{
			//we need the next block
			if(*(block+1) != 0) //this indicates there is more memory
			{
				block = *(uint32_t*)(block+1); //move to the next pointer
				byteBlock = (uint8_t*)block;
			}
			else
				return (void*)0; //fail! We are out of memory. 
		}
	}
}

//allocates size bytes on the heap aligned properly
void* kernel_malloc_align(uint32_t size,uint32_t alignment)
{
	kernel_printf("Allocating aligned memory. TODO: There is no FREE for aligned memory!\n");
	//basically, we add alignment to size so ensure we can align:
	size = size+alignment;

	//begin at the first block
	uint32_t* block = (uint32_t*)masterHeap.heapStart;

	uint8_t* byteBlock = (uint8_t*)block;
	bool memFound = false;
	while(!memFound)
	{
		uint32_t blockSize = block[0];
		if(blockSize >= size && block[2] == FREE_MEMORY) //we've found a block
		{
			//set the size of the current block and allocate it
			block[0] = size;
			//kernel_printf("Block location: %u, Real size: %u\n",block,size);
			block[2] = ALLOCATED_MEMORY; //memory is in use
			
			//Make this block smaller if we can store anything in it
			if(blockSize - size > 3)
			{
				//if this block can still be used, cut it down in size
				
				//the next block will be at block + size + 3 because 
				//size, next pointer, and allocation status live in memory as well
				
				*(uint32_t*)(byteBlock+size+3*sizeof(uint32_t)) = blockSize - size - 3; //set the new size
				*(uint32_t*)(byteBlock+size+4*sizeof(uint32_t)) = *(block + 1); //set the next address of the next block
				//to the next address of this block
				
				//we know for sure that this is free memory, so set it as such
				*(uint32_t*)(byteBlock+size+5*sizeof(uint32_t)) = FREE_MEMORY;
				
				//point this block to it
				*(block + 1) = (byteBlock+size+3*sizeof(uint32_t));
				
			}
			else
				block[1] = 0; //no more blocks!
			
			//if this block is the exact right size we won't cut it
			
			//return the pointer to memory
			//determine the offset required
			int offset = 0;
			while((uint32_t)(byteBlock + 3*sizeof(uint32_t) + offset) % alignment)
			{
				offset++;
			}
			
			kernel_printf("Memory aligned with %u alignment set to %u\n",alignment,(uint32_t)(byteBlock + 3*sizeof(uint32_t) + offset));
			return (void*)(byteBlock+3*sizeof(uint32_t) + offset); //return the memory location 3 after the start of this block
		}
		else
		{
			//we need the next block
			if(*(block+1) != 0) //this indicates there is more memory
			{
				block = *(uint32_t*)(block+1); //move to the next pointer
				byteBlock = (uint8_t*)block;
			}
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

