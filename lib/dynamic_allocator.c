/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"



//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d,address: %p)\n", get_block_size(blk), is_free_block(blk), blk) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING

	// panic("initialize_dynamic_allocator is not implemented yet");

	//Your Code is Here...

	//set begin and end block
	uint32* begBlock=(uint32*)daStart;
	*begBlock=(uint32)1;
	uint32* endBlock=(uint32*)(daStart+initSizeOfAllocatedSpace-sizeof(int));
	*endBlock=1;

	//set the block headers
	uint32* blkHeader=(uint32*)(daStart+sizeof(int));
	*blkHeader=initSizeOfAllocatedSpace-2*sizeof(int);

	uint32* blkFooter = (uint32*)(daStart+initSizeOfAllocatedSpace-2*sizeof(int));
	*blkFooter=initSizeOfAllocatedSpace-2*sizeof(int);

	//initalize list with head being the block just initliazied	
	struct BlockElement* freeBlock=(struct BlockElement*)(daStart+2*sizeof(int));

	LIST_INIT(&freeBlocksList);
	LIST_INSERT_HEAD(&freeBlocksList,freeBlock);
}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...
	if(totalSize<16)
	 return;

	if(totalSize%2!=0)
	   totalSize++;

    uint32 start=(uint32)va; 
	uint32 *header=(uint32 *)(start-sizeof(uint32));
	uint32 *footer= (uint32 *)(start+totalSize-2*sizeof(uint32));
	*header=*footer=(totalSize | isAllocated); 
	
}


//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...

	struct BlockElement* currentBlock = LIST_FIRST(&freeBlocksList);

	LIST_FOREACH(currentBlock, &freeBlocksList){
		uint32 initialBlockSize = get_block_size(currentBlock);

		//current block doesn't have sufficent size
		if(initialBlockSize<size+2*sizeof(int)){
			continue;
		}

		uint32 remaining = initialBlockSize - (size + 8);
		//current block will be split into two 
		if(remaining >= 16){
			set_block_data(currentBlock, size+8, 1);

			struct BlockElement* splitSegment = (struct BlockElement*)((uint32)currentBlock + size + 8);
			set_block_data(splitSegment, initialBlockSize-(size+8), 0);

			struct BlockElement* prev = LIST_PREV(currentBlock);
			LIST_REMOVE(&freeBlocksList, currentBlock);
			if(prev == NULL){
				LIST_INSERT_HEAD(&freeBlocksList, splitSegment);
			}else{
				LIST_INSERT_AFTER(&freeBlocksList, prev, splitSegment);
			}
			return currentBlock;
		//internal fragmanation
		}else if(remaining < 16 && remaining >= 0){
			set_block_data(currentBlock, initialBlockSize, 1);
			LIST_REMOVE(&freeBlocksList, currentBlock);
			return currentBlock;
		}
	}

	// [MS2 - KERNEL HEAP - sbrk] if loop ended without hitting a return  
	int ret = (int)sbrk(ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE);

	if(ret == -1) // sbrk failed
	{
		return NULL;	
	}

	uint32* va = (uint32*)ret;

	free_block(va);

	return alloc_block_FF(size);

}

	
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...


	struct BlockElement* currentBlock,*bestBlock;
	uint32 bestBlockSize=__INT_MAX__;
	//loop over all the blocks and find the smallest one that fits the criteria 
	LIST_FOREACH(currentBlock, &freeBlocksList){
		uint32 currentBlockSize = get_block_size(currentBlock);

		if(currentBlockSize>=size+8 && currentBlockSize<bestBlockSize){
			bestBlockSize=currentBlockSize;
			bestBlock=currentBlock;
		}
	}
	//didn't find a block with the required size
	if(bestBlockSize==__INT_MAX__){
		sbrk(size/PAGE_SIZE);
		return NULL;
	}

	uint32 remaining = bestBlockSize - (size+8);
	//if the remaining is too big  split the block into two 
	if(remaining >= 16){
			set_block_data(bestBlock, size+8, 1);

			struct BlockElement* splitSegment = (struct BlockElement*)((uint32)bestBlock + size + 8);
			
			set_block_data(splitSegment, bestBlockSize-(size+8), 0);

			struct BlockElement* prev = LIST_PREV(bestBlock);

			LIST_REMOVE(&freeBlocksList, bestBlock);

			if(prev == NULL){
				LIST_INSERT_HEAD(&freeBlocksList, splitSegment);
			}else{
				LIST_INSERT_AFTER(&freeBlocksList, prev, splitSegment);
			}


	}
	//internal fragmanation
	else if(remaining < 16 && remaining >= 0){
		set_block_data(bestBlock, bestBlockSize, 1);
		LIST_REMOVE(&freeBlocksList, bestBlock);

	}
	return bestBlock;

}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...

	/*
		if va == null -> return

		1- save left and right sizes + set left, right, current flags(free status)
			1.1 check if block is last(1) and or first(2)
				1.1.1 increment to current + current size (will be at right meta data)	
					  if size of meta data == 0 then current is last + rightFlag = 0
					  else normal block so get size and status

				1.1.2 get footer of previous data 
					  if: footer size == 0 then this footer is start block + leftFlag = 0
					  else: normal block so get size

		3- according to flags --> set block data, set anchor pointer (first ptr in coalced memory)
		4- edit linked list
		(to get prev)4.2- for each block --> save ptr if less than anchor , if > than anchor brak
			4.3 if prev null -> LIST_INSERT_HEAD else LIST_INSERT_AFTER prev
	*/
	//invalid address
	if(va == NULL){
		return;
	}

	bool isLeftFree;
	bool isRightFree;

	struct BlockElement* leftAdr;
	struct BlockElement* rightAdr;

	uint32 currentSize = get_block_size(va);
	uint32 rightSize;
	uint32 leftSize;

	//getting sizes and flags

	//for left
	uint32* leftMeta = (uint32*)va - 2;
  
	//current block is first block in heap
	if(*leftMeta == (uint32)1 || !(~(*leftMeta) & 0x1)){
		isLeftFree = 0;
		leftSize = 0;
	}else{//left block is an actual block
		leftSize = *leftMeta & (UINT_MAX-1);
		isLeftFree = (~(*leftMeta) & 0x1);
		leftAdr = (struct BlockElement*) ((uint32)va - leftSize);
	}


	//for right
	uint32* rightMeta = (uint32*) ((uint32)va + get_block_size(va));
	//current block is last block in heap
	if(*(rightMeta-1) == (uint32)1 ||  !is_free_block(rightMeta)){
		isRightFree = 0;
		rightSize = 0;
	}else{//right block is an actual block
		rightSize = get_block_size(rightMeta);
		isRightFree = is_free_block(rightMeta);
		rightAdr = (struct BlockElement*) rightMeta;
	}

	//set anchor and block data
	struct BlockElement* anchorBlock;
	if(isLeftFree && isRightFree){
		anchorBlock = leftAdr;
		LIST_REMOVE(&freeBlocksList, rightAdr);
		LIST_REMOVE(&freeBlocksList, leftAdr);

	}else if(isLeftFree){
		anchorBlock = leftAdr;
		LIST_REMOVE(&freeBlocksList, leftAdr);

	}else if(isRightFree){
		anchorBlock = (struct BlockElement*)va;
		LIST_REMOVE(&freeBlocksList, rightAdr);
	}else{
		anchorBlock = (struct BlockElement*)va;
	}

	//coalecing required block
	set_block_data(anchorBlock, currentSize+rightSize+leftSize, 0);

	//linked list manipulation
	struct BlockElement* prev = NULL;
	struct BlockElement*  currentBlock = NULL;

	LIST_FOREACH(currentBlock, &freeBlocksList){
		if((uint32)currentBlock > (uint32)anchorBlock){
			break;
		}
		prev = currentBlock;
	}

	if(prev == NULL){
		LIST_INSERT_HEAD(&freeBlocksList, anchorBlock);
	}else{
		LIST_INSERT_AFTER(&freeBlocksList, prev,anchorBlock);
	}
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================


void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
    //va=The virtual address of the memory block positioned directly after the header
   /*
   
    if current address == null
		check if required size == 8 return --> null     realloc(null,0)
        	else return --> alloc(required size)        realloc(null,size)
   
 	1- if required size > current size
		store first address of current block after header and first address of next block after header and its size

	 	1.1 if next block free and total size >= required size && next block is free

            1.1.1 if total size - required size >= 16
		   		remove block from list then split the block to new block and reminder then add remaining part to free to list
            1.1.2 if total size - required size < 16 internal fragmentation
				remove block from list and set new header and footer
	      	return current address

  		1.2 else
	     	use allocate to find new block 
            1.2.1 if allocate didn't find new block --> return null
              	transfer data to new block then add free block to the list 
            return new address

        2- if required size < current size
		2.1  if required size == 8 
			add block to free list return --> null
        	2.2 if current size - required size >= 16
			split the block to new block and reminder then add remaining part to free list
            return current address        
    
	3- if required size == current size
		return current address
*/ 

    new_size+=8;//Each block must include an additional 4 bytes for the header and 4 bytes for the footer
	if (new_size%2!=0)new_size+=1;
	if (new_size<16&&new_size!=8)new_size=16;
	if (va == NULL)//In case the address is null.
	{
		if(new_size==8)
		 return NULL;
		else //must be greater than 0
		 return alloc_block_FF(new_size-8);
	}
	else //In case the address is not null.
	{
	  uint32 curr_size=get_block_size(va);
		//If the block size needs to be allocated larger than the current size.
      if(new_size>curr_size)
	  {
	   uint32 start=(uint32) va;

       uint32* next_block=(uint32*)(start+curr_size);
       uint32 next_block_size=get_block_size(next_block);

	   uint32 *reminder=(uint32 *)(start+new_size);

       uint32* rightMeta = (uint32*) ((uint32)va + get_block_size(va));
		
		//If the next block after the current one is free, and the total size of both blocks is greater than or equal to the new size.
		//and the current block is not the last block in the heap.	   
	   if(is_free_block(next_block)&&next_block_size+curr_size>=new_size&&*(rightMeta-1) != (uint32)1 )  
	   {

		//If the remaining space in the next block is 16 or greater, it can be allocated as a separate free block.
		if(next_block_size+curr_size-new_size>=16)
		{
		
		struct BlockElement *next_address;
		 next_address = (struct BlockElement*) next_block;
		 LIST_REMOVE(&freeBlocksList,next_address);
		 set_block_data(reminder,next_block_size+curr_size-new_size,0);
		 set_block_data(va,new_size,1);
         free_block(reminder);	

		 
		}
        else //If the remaining space in the next block is less than 16, so the entire next block must be allocated completely.
		{
			 struct BlockElement *next_address;
		 next_address = (struct BlockElement*) next_block;
		 LIST_REMOVE(&freeBlocksList,next_address);
		 set_block_data(va,curr_size+next_block_size,1);
		
		}
         return va;
	   }
	   else //If the next block is not free, the current block must be reallocated to a new address.
	   {
         uint32* new_block=(uint32*)alloc_block_FF(new_size-8);

		//If no free block has a size greater than or equal to the new size, or if there are no free blocks available.
		 if(new_block==NULL) return NULL;
		
		
		//transfer data from old address to new one
		uint32 start=(uint32) va;
		void *old_address=va;
		void *new_address=(void*)new_block;
		memcpy(new_address,va,curr_size-8);
		


		//If the current block is reallocated to a new address,the original address must be marked as a free block.
        free_block(va);
	  	return (void *)new_block;
	   }

	  }
	  //If the block does not need the current size, the remaining size must be marked as a free block.
	  else if(new_size<curr_size)  
	  {
        if(new_size==8)
		{
		 free_block(va);
		 return NULL;
		}
		uint32 start=(uint32) va;
		// uint32* next_block=(uint32*)(start+curr_size);
		uint32 *reminder=(uint32 *)(start+new_size);

			//If the remaining space in the next block is 16 or greater, it also can be allocated as a separate free block.
			if(curr_size-new_size>=16)
			{
			 set_block_data(va,new_size,1);
			 set_block_data(reminder,curr_size-new_size,1);/////////////////////////
			 free_block(reminder);
			
			}
			//If the remaining size is less than 16, then no action is required.
			
		
		return va;
	  }
	  else //cur size == new size
	   return va;
	}
	
}

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
