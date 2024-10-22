// Sleeping locks

#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "inc/string.h"
#include "sleeplock.h"
#include "channel.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"

void init_sleeplock(struct sleeplock *lk, char *name)
{
	init_channel(&(lk->chan), "sleep lock channel");
	init_spinlock(&(lk->lk), "lock of sleep lock");
	strcpy(lk->name, name);
	lk->locked = 0;
	lk->pid = 0;
}
int holding_sleeplock(struct sleeplock *lk)
{
	int r;
	acquire_spinlock(&(lk->lk));
	r = lk->locked && (lk->pid == get_cpu_proc()->env_id);
	release_spinlock(&(lk->lk));
	return r;
}
//==========================================================================
#define Busy 1
#define Free 0
void acquire_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'24.MS1 - #13] [4] LOCKS - acquire_sleeplock
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("acquire_sleeplock is not implemented yet");
	//Your Code is Here...

	//place a spinLock gaurd on the sleepLock because its a critical section
	acquire_spinlock(&(lk->lk));

	//if the door is locked go to sleep (call the sleep function)
	while(lk->locked==Busy){
		//the spinLock is released in the sleep function
		sleep(&(lk->chan),&(lk->lk));
	}
	//if the door is open go in and lock it so others cant go in
	lk->locked=Busy;
	lk->pid=get_cpu_proc()->env_id;
//	cprintf("%d\n",holding_sleeplock(lk));
	release_spinlock(&(lk->lk));
}

void release_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'24.MS1 - #14] [4] LOCKS - release_sleeplock
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("release_sleeplock is not implemented yet");
	//Your Code is Here...

	//place a spinLock gaurd on the sleepLock because its a critical section
	acquire_spinlock(&(lk->lk));

	//wake up all the processes waiting in the queue
	if(queue_size(&(lk->chan.queue))){
		wakeup_all(&(lk->chan));
	}
	//release the locks
	lk->locked=Free;
	release_spinlock(&(lk->lk));

}










