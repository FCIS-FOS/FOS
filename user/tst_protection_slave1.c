#include <inc/lib.h>
#include <lib/uheap.c>
void
_main(void)
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE THAT SOME
	 * PAGES ARE ALLOCATED IN KERNEL DYNAMIC ALLOCATOR DUE
	 * TO sbrk()
	 *********************************************************/

	/*=================================================*/
	//Initial test to ensure it works on "PLACEMENT" not "REPLACEMENT"
#if USE_KHEAP
	{
		if (LIST_SIZE(&(myEnv->page_WS_list)) >= myEnv->page_WS_max_size)
			panic("Please increase the WS size");
	}
#else
	panic("make sure to enable the kernel heap: USE_KHEAP=1");
#endif
	/*=================================================*/

	{
		// cprintf("befoore\n");
		// for(int i=0;i<(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE;i++){
		// 	cprintf("%d ",page_alloc[i].is_marked);
		// }
		// for(int i=0;i<(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE;i++){
		// 	page_alloc[i].is_marked=gettst();
		// }
		// cprintf("after\n");
		// for(int i=0;i<(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE;i++){
		// 	cprintf("%d ",page_alloc[i].is_marked);
		// }
		char initname[10] = "x";
		char name[10] ;
#define NUM_OF_OBJS 5000
		uint32* vars[NUM_OF_OBJS];
		for (int s = 0; s < NUM_OF_OBJS; ++s)
		{
			char index[10];
			ltostr(s, index);
			strcconcat(initname, index, name);
			vars[s] = smalloc(name, PAGE_SIZE, 1);
			// cprintf("after smalloc # %d and %d\n",s,myEnv->env_id);
			*vars[s] = s;
		}
		for (int s = 0; s < NUM_OF_OBJS; ++s)
		{
			// cprintf("in second loop %d and env id %d\n",s,myEnv->env_id);
			assert(*vars[s] == s);
		}
	}

	inctst();
}
