#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
#define DYNAMIC_ALLOCATOR_DS 0 //ROUNDUP(NUM_OF_KHEAP_PAGES * sizeof(struct MemBlock), PAGE_SIZE)
#define INITIAL_KHEAP_ALLOCATIONS (DYNAMIC_ALLOCATOR_DS) //( + KERNEL_SHARES_ARR_INIT_SIZE + KERNEL_SEMAPHORES_ARR_INIT_SIZE) //
#define INITIAL_BLOCK_ALLOCATIONS ((2*sizeof(int) + MAX(num_of_ready_queues * sizeof(uint8), DYN_ALLOC_MIN_BLOCK_SIZE)) + (2*sizeof(int) + MAX(num_of_ready_queues * sizeof(struct Env_Queue), DYN_ALLOC_MIN_BLOCK_SIZE)))
#define ACTUAL_START ((KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE) + INITIAL_KHEAP_ALLOCATIONS)
struct Page allPages[(KERNEL_HEAP_MAX-ACTUAL_START)/PAGE_SIZE];
uint32 start_ind;
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
	brk=ROUNDUP(daStart+initSizeToAllocate,PAGE_SIZE);
	limit=daLimit;
	struct FrameInfo *frame;
	uint32 va=(uint32)daStart;
	while(va<daStart+initSizeToAllocate){
		allocate_frame(&frame);
		map_frame(ptr_page_directory,frame,va,PERM_PRESENT|PERM_WRITEABLE);
		
			uint32* page_table=NULL;
			// to store virtual address to frame info
			frame->mappedVA=va;
		

		va+=PAGE_SIZE;
	}
	initialize_dynamic_allocator(daStart,initSizeToAllocate);
	

	//initilize page table entires
	for(uint32 curPage = limit + PAGE_SIZE; curPage < KERNEL_HEAP_MAX; curPage+=PAGE_SIZE){
		uint32* ptr_page_table = NULL;
		uint32 ret = get_page_table(ptr_page_directory, curPage, &ptr_page_table);

		ptr_page_table[PTX(curPage)] = 0;
	}
	// initialize the allpages array and the start index
	allPages[0].num_of_pages=(KERNEL_HEAP_MAX-ACTUAL_START)/PAGE_SIZE;
	allPages[0].next_index=-1;
	start_ind=0;
	init_spinlock(&k_lock,"kernel_lock");
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
		//acquire_spinlock(&k_lock);
		int ret = allocate_frame(&ptr_frame_info);

		
		if(ret == E_NO_MEM)// we may run out of memory (free frames)
		{
			// returning the end block to its initial state
			*endBlock = 1;

			return (void*)-1;
		}

		// we still have memory so we map the frame
		int ret2 = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_PRESENT | PERM_WRITEABLE);
			// to store virtual address to frame info
			ptr_frame_info->mappedVA=va;
		
		

		if(ret2 == E_NO_MEM)// no table of a given virtual address and no frames to make one 
		{
			// returning the end block to its initial state
			*endBlock = 1;

			return (void*)-1;
		}
		//release_spinlock(&k_lock);
	}

	// setting the new end block 
	uint32* new_endBlock = (uint32*)(new_brk - sizeof(int));
	*new_endBlock = 1;

	// setting the new brk
	brk = new_brk;

	// creating the new free block
	set_block_data( (void*)old_brk, increment, 0);
	
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
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	// kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	//division with rouding up



	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		acquire_spinlock(&k_lock);
		void *ret=alloc_block_FF(size);
		release_spinlock(&k_lock);
		return ret;	
	}

	// uint32 requiredPages = (size+PAGE_SIZE-1)/PAGE_SIZE;
	uint32 requiredPages = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	const uint32 HARD_LIMIT = limit;
	uint32 pages = 0;
	uint32 allocStart = HARD_LIMIT + PAGE_SIZE;
	uint32 prev=0;
	acquire_spinlock(&k_lock);
	for(uint32 i=start_ind; i!=-1;i=allPages[i].next_index)
	{
		
		//num>required ---> split
		if(allPages[i].num_of_pages>requiredPages){
			//move start index
			if(i==start_ind){
				start_ind+=requiredPages;
				allPages[start_ind].next_index=allPages[i].next_index;
				allPages[start_ind].num_of_pages=allPages[i].num_of_pages-requiredPages;
			}
			// normal allocation
			else{
				allPages[prev].next_index=i+requiredPages;
				allPages[i+requiredPages].next_index=allPages[i].next_index;
				allPages[i+requiredPages].num_of_pages=allPages[i].num_of_pages-requiredPages;
			}
			pages=requiredPages;
			//the start address of allocation (hard limit + page size + (page#i * Page size))
			allocStart+=i*PAGE_SIZE;
			break;
		}
		// num == required ------> take all
		else if (allPages[i].num_of_pages==requiredPages){
			//move start index
			if(i==start_ind){
				start_ind=allPages[i].next_index;
			}
			//normal allocation
			else{
				allPages[prev].next_index=allPages[i].next_index;
			}
			pages=requiredPages;
			//the start address of allocation (hard limit + page size + (page#i * Page size))
			allocStart+=i*PAGE_SIZE;
			break;
		}
		prev=i;
		
	}
	release_spinlock(&k_lock);
	if(pages!=requiredPages){
		return NULL;
	}else{
		uint32 addr = allocStart;
		
		for(int i = 0; i<requiredPages; i++){
			acquire_spinlock(&k_lock);
			struct FrameInfo *frame;
			int allocRet = allocate_frame(&frame);
			if(allocRet == E_NO_MEM){
				return NULL;
			}

			int mapRet = map_frame(ptr_page_directory, frame, addr, PERM_WRITEABLE|PERM_PRESENT);
			if(mapRet == E_NO_MEM){
				return NULL;
			}
			//store the info that is needed for Kfree and kheap_virtual_address
			frame->allocStart=allocStart;
			frame->allocSize=requiredPages;
      
			//to store virtual address to the frame info
			frame->mappedVA=addr;
      
			addr += PAGE_SIZE;
			release_spinlock(&k_lock);
		}
	
	}

	return (void*)allocStart;
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	// panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details
	uint32 virtual_address_int=(uint32)virtual_address;
	//Virtual Address is in Block Allocator Range
    if(virtual_address_int>=start && virtual_address_int<limit){
        free_block(virtual_address);
    }
	//Virtual Address is in Page Allocator Range
    else if (virtual_address_int>=limit+PAGE_SIZE && virtual_address_int<KERNEL_HEAP_MAX){
		//acquire_spinlock(k_lock);
        uint32 * ptr_page_table=NULL;

        struct FrameInfo *frame_info = get_frame_info(ptr_page_directory,virtual_address_int,&ptr_page_table);
		//cprintf("free alloc start = %x\n",frame_info->allocStart);
		//cprintf("free alloc size = %d\n",frame_info->allocSize);
		    // get the starting point of the page allocation and the number of pages to iterate over
        uint32 allocStart=frame_info->allocStart;
        uint32 allocSize=frame_info->allocSize;
        // looping over the pages and unmapping the frames they are refrencing 
        for(uint32 current=allocStart;current<allocStart+(allocSize*PAGE_SIZE);current+=PAGE_SIZE){
            struct FrameInfo *frame = get_frame_info(ptr_page_directory,current,&ptr_page_table);
            frame->mappedVA=0;
            unmap_frame(ptr_page_directory,current);
        }

		uint32 as_ind=(allocStart-ACTUAL_START)/PAGE_SIZE;
		//free address<start_ind
		if(as_ind<start_ind){
			allPages[as_ind].next_index=start_ind;
			allPages[as_ind].num_of_pages=allocSize;
			// merge with old first block
			if(as_ind+allocSize==start_ind){
				//cprintf("nooooooooooo\n");
				allPages[as_ind].num_of_pages+=allPages[start_ind].num_of_pages;
				allPages[as_ind].next_index=allPages[start_ind].next_index;
			}
			start_ind=as_ind;
		//release_spinlock(k_lock);
		}
		else{
			// loop until free address >current block and next of current block is after free address
			//acquire_spinlock(k_lock);
			for(uint32 i = start_ind;i!=-1;i=allPages[i].next_index){
				//free address >current block and next of current block is after free address
				if(as_ind>i && as_ind<allPages[i].next_index){
					//merge with the current block
					if(allPages[i].num_of_pages+i==as_ind){
						allPages[i].num_of_pages+=allocSize;
						//merge with the next block
						if(as_ind+allocSize==allPages[i].next_index){
							allPages[i].num_of_pages+=allPages[allPages[i].next_index].num_of_pages;
							allPages[i].next_index=allPages[allPages[i].next_index].next_index;
						}
					}
					//merge with the next block
					else if(as_ind+allocSize==allPages[i].next_index){
						allPages[as_ind].num_of_pages=allPages[allPages[i].next_index].num_of_pages+allocSize;
						allPages[as_ind].next_index=allPages[allPages[i].next_index].next_index;
						allPages[i].next_index=as_ind;
					}
					//normal free
					else{
						allPages[as_ind].next_index=allPages[i].next_index;
						allPages[i].next_index=as_ind;
						allPages[as_ind].num_of_pages=allocSize;
					}
					break;
				}
				//free address is the last block in the chain
				else if (as_ind>i && allPages[i].next_index==-1){
					//merge with current block
					if(allPages[i].num_of_pages+i==as_ind){
						allPages[i].num_of_pages+=allocSize;
					}
					//normal free
					else{
						allPages[i].next_index=as_ind;
						allPages[as_ind].next_index=-1;
						allPages[as_ind].num_of_pages=allocSize;
					}
					break;
				}
			}
			//release_spinlock(k_lock);
		}
    }
  
	//Virtual Address is invalid
    else{
        panic("invalid address");
    }
	//cprintf("hello!!!!\n");
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
		if (ptr_frame_info->mappedVA==0)
		return 0;
		
		uint32 off = physical_address & 0xFFF;
		uint32 vir_address=ptr_frame_info->mappedVA;
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
	//PAGE AREA
	uint32 va=(uint32)virtual_address;
	uint32 * ptr_page_table=NULL;
	void *new_address;
	
    if (virtual_address==NULL){
		if(new_size==0)
			return NULL;
		//cprintf("here 2\n");
		//cprintf("new size =%d\n",ROUNDUP(new_size,PAGE_SIZE));
		return kmalloc(new_size);
	}
	else if(new_size> (KERNEL_HEAP_MAX - ACTUAL_START))
		return NULL;
	
	else if(va>=limit+PAGE_SIZE){
		
		struct FrameInfo *frame_info = get_frame_info(ptr_page_directory,va,&ptr_page_table);		
		//cprintf("ONEFORALL=%x\n",frame_info);
		 if (frame_info==NULL){
			//cprintf("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk");
		 	return NULL;
			//cprintf("here 2\n");
		}
    	uint32 allocSize=frame_info->allocSize;
		//cprintf("allocsize =%d\n",allocSize*PAGE_SIZE);
		
		if(new_size==0){
			kfree(virtual_address);
			return NULL;
     	}
		//cprintf("new size =%d\n",ROUNDUP(new_size,PAGE_SIZE));
		

		if(new_size>allocSize*PAGE_SIZE){
			//cprintf("uuuuuuuuuuuu\n");
			bool moving=0;
			//cprintf("new size =%d\n",ROUNDUP(new_size,PAGE_SIZE));
			//cprintf("alloc size =%d\n",allocSize);
			//cprintf("total =%d",ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE-allocSize);
			for(int i=0;i<ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE-allocSize;i++){
				if(!pageIsFree((void *)(va+PAGE_SIZE*(i+1))))
					{cprintf("i=%d",i);moving=1;break;}
			}
			//cprintf("siuuuuuuuuuuuuuuu");
			if(moving){
				new_address=kmalloc(new_size);
				memcpy(new_address,virtual_address,allocSize*PAGE_SIZE);
				kfree(virtual_address);
				return new_address;
			}
			else{
				//cprintf("siuuuuuuuuuuuuuuu\n");
				uint32 requiredPages = ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE-allocSize;
				//cprintf("req pages =%d\n",requiredPages);
				const uint32 HARD_LIMIT = limit;
				uint32 pages = 0;
				uint32 allocStart = va;
				uint32 prev=0;
				//cprintf("siuuuuuuuuuuuuuuu22222\n");
				for(uint32 i=va;i<va+(PAGE_SIZE*allocSize);i+=PAGE_SIZE){
                    struct FrameInfo *frame = get_frame_info(ptr_page_directory,i,&ptr_page_table);
					frame->allocSize=ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
				}
				for(uint32 i=start_ind; i!=-1;i=allPages[i].next_index)
				{
					//cprintf("i=%d\n",i);
					//num>required ---> split
					if(allPages[i].num_of_pages>requiredPages){
						//move start index
						
						if(i==start_ind){
							//cprintf("you 2\n");
							start_ind+=requiredPages;
							allPages[start_ind].next_index=allPages[i].next_index;
							allPages[start_ind].num_of_pages=allPages[i].num_of_pages-requiredPages;
						}
						// normal allocation
						else{
							allPages[prev].next_index=i+requiredPages;
							allPages[i+requiredPages].next_index=allPages[i].next_index;
							allPages[i+requiredPages].num_of_pages=allPages[i].num_of_pages-requiredPages;
						}
						pages=requiredPages;
						//the start address of allocation (hard limit + page size + (page#i * Page size))
						//allocStart+=i*PAGE_SIZE;
						break;
					}
					// num == required ------> take all
					else if (allPages[i].num_of_pages==requiredPages){
						//cprintf("siuuuuuuuuu3333\n");
						//move start index
						if(i==start_ind){
							start_ind=allPages[i].next_index;
						}
						//normal allocation
						else{
							allPages[prev].next_index=allPages[i].next_index;
						}
						pages=requiredPages;
						//the start address of allocation (hard limit + page size + (page#i * Page size))
						//allocStart+=i*PAGE_SIZE;
						break;
					}
					//cprintf("hererrrrrr!!!\n");
					prev=i;
				}
                //cprintf("you123!!!!!!\n");
				
				if(pages!=requiredPages){
					return NULL;
				}else{
					
					uint32 addr = allocStart+allocSize*PAGE_SIZE;
					for(int i = 0; i<requiredPages; i++){
						//cprintf("pages=%d",pages);
						struct FrameInfo *frame;
						int allocRet = allocate_frame(&frame);
						if(allocRet == E_NO_MEM){
							return NULL;
						}

						int mapRet = map_frame(ptr_page_directory, frame, addr, PERM_WRITEABLE|PERM_PRESENT);
						if(mapRet == E_NO_MEM){
							return NULL;
						}
						//store the info that is needed for Kfree and kheap_virtual_address
						frame->allocStart=allocStart;
						frame->allocSize=ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
						//to store virtual address to the frame info
						frame->mappedVA=addr;
						addr += PAGE_SIZE;
					}
					//cprintf("huuuuuuuuuuuuuuuuuuuuuu\n");
					return (void *)allocStart;
				}
				
			}
		}
		else if(ROUNDUP(new_size,PAGE_SIZE)<allocSize*PAGE_SIZE&&new_size>DYN_ALLOC_MAX_BLOCK_SIZE){
        // looping over the pages and unmapping the frames they are refrencing 
			for(uint32 current=va+ROUNDUP(new_size,PAGE_SIZE);current<va+(allocSize*PAGE_SIZE);current+=PAGE_SIZE){
				struct FrameInfo *frame = get_frame_info(ptr_page_directory,current,&ptr_page_table);
				frame->mappedVA=0;
				unmap_frame(ptr_page_directory,current);
			}
            for(uint32 current=va;current<va+ROUNDUP(new_size,PAGE_SIZE);current+=PAGE_SIZE){
				struct FrameInfo *frame = get_frame_info(ptr_page_directory,current,&ptr_page_table);
				frame->allocSize=ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
			}
			//cprintf("hey2\n");
			uint32 as_ind=(va+ROUNDUP(new_size,PAGE_SIZE)-ACTUAL_START)/PAGE_SIZE;
			if(as_ind<start_ind){
				//cprintf("hey3\n");
				allPages[as_ind].next_index=start_ind;
				allPages[as_ind].num_of_pages=allocSize-ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
				// merge with old first block
				if(as_ind+allocSize-ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE==start_ind){
					//cprintf("tst\n");
					allPages[as_ind].num_of_pages+=allPages[start_ind].num_of_pages;
					allPages[as_ind].next_index=allPages[start_ind].next_index;
				}
				start_ind=as_ind;
				return virtual_address;
			}
			else{
				//cprintf("hey3\n");
				// loop until free address >current block and next of current block is after free address
				for(uint32 i = start_ind;i!=-1;i=allPages[i].next_index){
					//free address >current block and next of current block is after free address
					if(as_ind>i && as_ind<allPages[i].next_index){
						//merge with the current block
					    //cprintf("hey\n");
						//merge with the next block
						if(as_ind+allocSize-new_size==allPages[i].next_index){
							//cprintf("herreee!!!\n");
							allPages[as_ind].num_of_pages=allPages[allPages[i].next_index].num_of_pages+allocSize-ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
							allPages[as_ind].next_index=allPages[allPages[i].next_index].next_index;
							allPages[i].next_index=as_ind;
						}
						//normal free
						else{
							allPages[as_ind].next_index=allPages[i].next_index;
							allPages[i].next_index=as_ind;
							allPages[as_ind].num_of_pages=allocSize-ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
						}
						break;
					}
					//free address is the last block in the chain
					else if (as_ind>i && allPages[i].next_index==-1){
						//merge with current block
						if(allPages[i].num_of_pages+i==as_ind){
							allPages[i].num_of_pages+=allocSize-ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
						}
						//normal free
						else{
							allPages[i].next_index=as_ind;
							allPages[as_ind].next_index=-1;
							allPages[as_ind].num_of_pages=allocSize-ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
						}
						break;
					}
				}
				frame_info->allocSize=ROUNDUP(new_size,PAGE_SIZE);
				return virtual_address;
			}
		}

		else if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE){
			
			new_address=kmalloc(new_size);
			memcpy(new_address,virtual_address,new_size);//cprintf("here!!!\n");
			kfree(virtual_address);
			return new_address;
		}
		return NULL;
	}
	else if(va<limit){
		//cprintf("testetestets\n");
		if(new_size==0){
			kfree(virtual_address);
			return NULL;
     	}

		//cprintf("get size =%d",get_block_size(virtual_address));
		if(is_free_block(virtual_address))
		 return NULL;
		if(new_size>get_block_size(virtual_address)&&new_size<=DYN_ALLOC_MAX_BLOCK_SIZE)
			return realloc_block_FF(virtual_address,new_size);
		else if(new_size<get_block_size(virtual_address))
			return realloc_block_FF(virtual_address,new_size);
		else if(new_size>DYN_ALLOC_MAX_BLOCK_SIZE){
			new_address= kmalloc(new_size);
			memcpy(new_address,virtual_address,get_block_size(virtual_address));
			kfree(virtual_address);
			return new_address;
		}
		//cprintf("testetestets\n");
		return NULL;
	}
	return NULL;
}
