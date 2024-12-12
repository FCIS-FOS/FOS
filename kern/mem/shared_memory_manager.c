#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...
		acquire_spinlock(&AllShares.shareslock);

    struct FrameInfo **framestorge=(struct FrameInfo **)kmalloc(numOfFrames*sizeof(struct FrameInfo*));
		release_spinlock(&AllShares.shareslock);

	if(framestorge==NULL)
	    return NULL;
    for(int i=0;i<numOfFrames;i++)
	    framestorge[i]=NULL;
	
	return framestorge;
}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...
	acquire_spinlock(&AllShares.shareslock);
    struct Share * share = (struct Share *)kmalloc(sizeof(struct Share));
	release_spinlock(&AllShares.shareslock);

	if(share == NULL)
	 return NULL;
	share->references=1;
	share->ID=(((uint32)share)<<1)>>1;
	share->ownerID=ownerID;
	share->isWritable=isWritable;
	share->size=size; 
	int len=strlen(shareName);
	for(int i=0;i<len;i++)
	 share->name[i]=shareName[i];
	
	int numOfFrames =(ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE);
	share->framesStorage=create_frames_storage(numOfFrames);
    if(share->framesStorage==NULL){
		kfree(share);
		return NULL;
	}
    return share;
}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	
    struct Share* current=NULL;
    acquire_spinlock(&AllShares.shareslock);
	LIST_FOREACH(current, &AllShares.shares_list) {
        if (current->ownerID == ownerID && strlen(name)==strlen(current->name) && strcmp(current->name,name)==0) { 
			return current;
		}

    }
	release_spinlock(&AllShares.shareslock);

	return NULL;

}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment

	/*
	## OUTLINE
		0. Handling if the shared object exists already.
			|
			-> Check if the shared object exists using get share before pushing it, if existed return E_SHARE_MEM_EXISTS.

		1. make a new shared object using the create_share function.
			|
			-> return E_NO_SHARE  if the return of the function is NULL.

		2. push the new shared object in the share list
			|
			-> Acquire the list's spinlock before pushing and releasing after allocation.

		3. Allocate and map the required space in physical memory
			|
			-> Check if we have the required frames in the free frames list before allocate_frame.
			|
			-> push every successfully allocated frame in the frames storage of the shared objecT.
	*/


	// # step 0
	void* shared_exist = (void*)get_share(ownerID, shareName);

	if(shared_exist != NULL)
	{
		return E_SHARED_MEM_EXISTS;
	} 

	// # step 1
	struct Share* new_shared_obj = create_share(ownerID, shareName, size, isWritable);

	if(new_shared_obj == NULL)
	{
		return E_NO_SHARE;
	}

	// # step 2

	acquire_spinlock(&AllShares.shareslock);

	LIST_INSERT_TAIL(&AllShares.shares_list, new_shared_obj);

	release_spinlock(&AllShares.shareslock);

	// # step 3

	uint32 req_frames = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	if(req_frames > LIST_SIZE(&MemFrameLists.free_frame_list)){
		return E_NO_SHARE;
	}

	
	uint32 mapping_virtual_address = (uint32)virtual_address;
	acquire_spinlock(&AllShares.shareslock);
	for(int i = 0; i < req_frames; i++)
	{
		struct FrameInfo* ptr_frame_info;

		allocate_frame(&ptr_frame_info);

		new_shared_obj->framesStorage[i] = ptr_frame_info;

		map_frame(myenv->env_page_directory, ptr_frame_info, mapping_virtual_address, PERM_WRITEABLE|PERM_PRESENT|PERM_USER);

		mapping_virtual_address += PAGE_SIZE;
	}
	release_spinlock(&AllShares.shareslock);
 
	//->ID = (uint32)virtual_address | 0x80000000;

	return new_shared_obj->ID;


}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("getSharedObject is not implemented yet");
	//Your Code is Here...
	struct Env* myenv = get_cpu_proc(); //The calling environment
	struct Share *current_share=NULL,*save_share=NULL;

	//protect the shared list
	acquire_spinlock(&AllShares.shareslock);
	LIST_FOREACH(current_share,&AllShares.shares_list){
		//owner id matches , names have the same length and match
		if(current_share->ownerID==ownerID && 
		strlen(shareName)==strlen(current_share->name) && 
		strcmp(shareName,current_share->name)==0)
		{
			save_share=current_share;
			break;
		}
	}
	//release the gaurd
	release_spinlock(&AllShares.shareslock);

	//didnt find the shared object
	if(save_share==NULL){
		return E_SHARED_MEM_NOT_EXISTS;
	}
	uint32 virtual_address_int=(uint32)virtual_address;
	uint32 current_page=ROUNDDOWN(virtual_address_int,PAGE_SIZE);
	uint32 num_of_frames=ROUNDUP(save_share->size,PAGE_SIZE)/PAGE_SIZE;
	//the permissions 
	uint32 perm=PERM_PRESENT | PERM_USER;
	if(save_share->isWritable){
		perm|=PERM_WRITEABLE;
	}
	//map each page to a shared frame
	for(uint32 i =0;i<num_of_frames;i++){
		map_frame(myenv->env_page_directory,save_share->framesStorage[i],current_page,perm);
		current_page+=PAGE_SIZE;
	}
	save_share->references++;
	// retrun the share id (might need to mask it if its not masked)
	uint32 mask =0x7FFFFFFF;
	return save_share->ID & mask;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("free_share is not implemented yet");
	//Your Code is Here...

}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("freeSharedObject is not implemented yet");
	//Your Code is Here...

}