#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code
	//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");
	if(daStart+initSizeToAllocate>daLimit)
	 panic("exceeds limit");
    start=daStart;
	brk=daStart+initSizeToAllocate;
	limit=daLimit;
	struct FrameInfo *frame;
	uint32 va=(uint32)daStart;
	while(va<daStart+initSizeToAllocate){
		allocate_frame(&frame);
		map_frame(ptr_page_directory,frame,va,PERM_PRESENT|PERM_WRITEABLE);
		va+=PAGE_SIZE;
	}
	initialize_dynamic_allocator(daStart,initSizeToAllocate);


	//initilize page table entires
	for(uint32 curPage = limit + PAGE_SIZE; curPage < KERNEL_HEAP_MAX; curPage+=PAGE_SIZE){
		uint32* ptr_page_table = NULL;
		uint32 ret = get_page_table(ptr_page_directory, curPage, &ptr_page_table);

		ptr_page_table[PTX(curPage)] = 1;

		// for(uint32 i=0; i<PAGE_SIZE/sizeof(uint32);i++){
		// 	ptr_page_table[i] = 1;
		// }
	}

	return 0;
}

void* sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");

	// numOfPages = Zero -> return current break
	if(!numOfPages)
		return (void*)brk;


	// numOfPages > Zero -> follow the logic discussed above

	uint32 increment = (numOfPages * PAGE_SIZE);
	uint32 old_brk = brk;
	uint32 new_brk = brk + increment; // the new break rests after the end block so we consider its size

	// new break exceed the hard limit
	if(new_brk > limit)
	{
		return (void*)-1;
	}
	
	// setting current endblock to zero
	uint32* endBlock = (uint32*)(brk - sizeof(int));
	*endBlock = 0;
	


	// we are still below the hard limit
	for(uint32 va = old_brk; va < new_brk; va += PAGE_SIZE)
	{
		struct FrameInfo* ptr_frame_info;
		int ret = allocate_frame(&ptr_frame_info);

		
		if(ret == E_NO_MEM)// we may run out of memory (free frames)
		{
			// returning the end block to its initial state
			*endBlock = 1;

			return (void*)-1;
		}

		// we still have memory so we map the frame
		int ret2 = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_PRESENT | PERM_WRITEABLE);

		if(ret2 == E_NO_MEM)// no table of a given virtual address and no frames to make one 
		{
			// returning the end block to its initial state
			*endBlock = 1;

			return (void*)-1;
		}
	}

	// setting the new end block 
	uint32* new_endBlock = (uint32*)(new_brk - sizeof(int));
	*new_endBlock = 1;

	// setting the new brk
	brk = new_brk;

	// inserting the new allocated memory in the free blocks list
	struct BlockElement* new_freeBlock = (struct BlockElement*)(old_brk);
	set_block_data( (void*)old_brk, increment, 0);
	LIST_INSERT_TAIL(&freeBlocksList, new_freeBlock);

	// the starting address we can allocate on is the old end block
	return (uint32*)old_brk;

}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator


//helper function for kmalloc return 1 if page is free
bool pageIsFree(void* va){
	uint32* ptr_pageTable = NULL;
	struct FrameInfo* frameInfo = get_frame_info(ptr_page_directory, (uint32)va, &ptr_pageTable);

	if(frameInfo == NULL){
		return 1;
	}else{
		return 0;
	}

	// uint32* pageTablePtr;
	// uint32 ret = get_page_table(ptr_page_directory, (uint32)va, &pageTablePtr);

	// if(ret == TABLE_NOT_EXIST){//page table doesn't exist
	// 	return 0;
	// }

	// uint32 tableEntry = pageTablePtr[PTX(va)];

	// if(tableEntry & PERM_PRESENT){
	// 	return 0;
	// }else{
	// 	return 1;
	// }
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	// kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	//division with rouding up



	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		return alloc_block_FF(size);
	}

	// uint32 requiredPages = (size+PAGE_SIZE-1)/PAGE_SIZE;
	uint32 requiredPages = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	const uint32 HARD_LIMIT = limit;
	uint32 pages = 0;
	uint32 allocStart = HARD_LIMIT + PAGE_SIZE;


	uint32 currentPage = HARD_LIMIT + PAGE_SIZE;
	while(currentPage < KERNEL_HEAP_MAX){
		if(pages == requiredPages){
			break;
		}

		if(pageIsFree((void*)currentPage)){
			pages++;
			currentPage += PAGE_SIZE;
		}else{
			if(pages<requiredPages){
				while(currentPage<KERNEL_HEAP_MAX && !pageIsFree((void*)currentPage)){
					currentPage+=PAGE_SIZE;
				}
				allocStart = currentPage;
				pages = 0;
			}
		}

	}

	if(pages!=requiredPages){
		return NULL;
	}else{
		uint32 addr = allocStart;
		for(int i = 0; i<requiredPages; i++){
			struct FrameInfo *frame;
			int allocRet = allocate_frame(&frame);
			if(allocRet == E_NO_MEM){
				return NULL;
			}

			int mapRet = map_frame(ptr_page_directory, frame, addr, PERM_WRITEABLE|PERM_PRESENT);
			if(mapRet == E_NO_MEM){
				return NULL;
			}

			addr += PAGE_SIZE;
		}
	}

	return (void*)allocStart;
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}
unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
	uint32* page_table=NULL;
	get_page_table(ptr_page_directory,(uint32)virtual_address,&page_table);
	if (page_table!=NULL){
		uint32 entry=page_table[PTX((uint32)virtual_address)];


		uint32 is_mapped=entry & PERM_PRESENT;
		if (is_mapped==0)return 0;
		// uint32 off=virtual_address <<20;
		// off=off>>20;
		uint32 off = virtual_address & 0xFFF;
		


		uint32 physical_add=entry>>12;//to extract the frame number
		physical_add=(physical_add<<12)+off;
		
		return physical_add;
	}
	
	return 0;


	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	struct FrameInfo* ptr_frame_info=to_frame_info((uint32)physical_address);
	if (ptr_frame_info == NULL) {
    return 0;
	}
	//
	
	if(ptr_frame_info->references==1){
		if (ptr_frame_info->bufferedVA==0)
		return 0;
		
		uint32 off = physical_address & 0xFFF;
		uint32 vir_address=ptr_frame_info->bufferedVA;
		vir_address+=off;
		return vir_address;
	}
	else return 0;



	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
