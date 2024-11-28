#include <inc/lib.h>
struct allocations page_alloc[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE];

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	// panic("malloc() is not implemented yet...!!");
	if(size<=DYN_ALLOC_MAX_BLOCK_SIZE){
		return alloc_block_FF(size);
	}
	uint32 start_page_allocator=myEnv->uheap_limit+PAGE_SIZE;
	uint32 num_of_pages=ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	uint32 num_of_pages_unmarked=0;
	uint32 start_virtual_addr=0;

	for (uint32 addr = myEnv->uheap_limit+PAGE_SIZE; addr < USER_HEAP_MAX; addr+=PAGE_SIZE)
	{
		
		if(page_alloc[(addr-USER_HEAP_START)/PAGE_SIZE].is_marked==0){
			if(num_of_pages_unmarked==0)start_virtual_addr=addr;
			num_of_pages_unmarked++;
		}
		else {
			num_of_pages_unmarked=0;
			start_virtual_addr=0;
		}

		if(num_of_pages_unmarked==num_of_pages)break;
	}
	if(num_of_pages_unmarked==num_of_pages){
		// page_alloc[(start_virtual_addr-USER_HEAP_START)]
		
		sys_allocate_user_mem(start_virtual_addr,size);
		  for(uint32 current_page=start_virtual_addr;
        current_page<start_virtual_addr+(num_of_pages*PAGE_SIZE);
        current_page+=PAGE_SIZE)
        {
            page_alloc[(current_page-USER_HEAP_START)/PAGE_SIZE].start_va=start_virtual_addr;
            page_alloc[(current_page-USER_HEAP_START)/PAGE_SIZE].size=size;
			page_alloc[(current_page-USER_HEAP_START)/PAGE_SIZE].is_marked=1;
        }
		return (void *) start_virtual_addr;
	}
	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	 //TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
 	// Write your code here, remove the panic and write your code
 	//panic("free() is not implemented yet...!!");
	uint32 virtual_addr=(uint32)virtual_address;
	if (virtual_addr>=USER_HEAP_START&&virtual_addr<myEnv->uheap_limit){
  		free_block((void *)virtual_address);
 	}
 	else if (virtual_addr>=myEnv->uheap_limit+PAGE_SIZE&&virtual_addr<USER_HEAP_MAX){
			uint32 start=page_alloc[(ROUNDDOWN(virtual_addr,PAGE_SIZE)-USER_HEAP_START)/PAGE_SIZE].start_va;/////////////////// miss calulate from Env
			uint32 size=page_alloc[(ROUNDDOWN(virtual_addr,PAGE_SIZE)-USER_HEAP_START)/PAGE_SIZE].size;
			sys_free_user_mem(start,size);
			uint32 va_page_start=ROUNDDOWN(virtual_addr,PAGE_SIZE);
			uint32 num_of_pages= ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
			for(uint32 current_page=va_page_start;current_page<va_page_start+(num_of_pages*PAGE_SIZE);current_page+=PAGE_SIZE){
				page_alloc[(current_page-USER_HEAP_START)/PAGE_SIZE].is_marked=0;
			}
 	}
 	else panic("Invalid Address");
 


}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	// panic("sget() is not implemented yet...!!");
	

	int size= sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	size = ROUNDUP((uint32)size,PAGE_SIZE);
	if(size == E_SHARED_MEM_NOT_EXISTS||size == 0)return NULL;

	uint32 start_page_allocator=myEnv->uheap_limit+PAGE_SIZE;
	uint32 num_of_pages=ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	uint32 num_of_pages_unmarked=0;
	uint32 start_virtual_addr=0;

	for (uint32 addr = myEnv->uheap_limit+PAGE_SIZE; addr < USER_HEAP_MAX; addr+=PAGE_SIZE)
	{
			if(page_alloc[(addr-USER_HEAP_START)/PAGE_SIZE].is_marked==0){
			if(num_of_pages_unmarked==0)start_virtual_addr=addr;
			num_of_pages_unmarked++;
		}
		else {
			num_of_pages_unmarked=0;
			start_virtual_addr=0;
		}

		if(num_of_pages_unmarked==num_of_pages)break;
	}
	if(num_of_pages_unmarked==num_of_pages){
		int id = sys_getSharedObject(ownerEnvID,sharedVarName,(void *)start_virtual_addr);
		if(id == E_SHARED_MEM_NOT_EXISTS)
		{
		return NULL;
		}
		return (void *)start_virtual_addr;
	}

	return NULL;

}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
