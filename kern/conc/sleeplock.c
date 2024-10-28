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

// Acquires the sleeplock, making the current process sleep if the lock is already held
void acquire_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'24.MS1 - #13] [4] LOCKS - acquire_sleeplock
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("acquire_sleeplock is not implemented yet");
	//Your Code is Here...

    // Protect the critical section with a spinlock
    acquire_spinlock(&(lk->lk));

    // If the sleeplock is already locked, put the current process to sleep
    while (lk->locked == Busy)
    {
        sleep(&(lk->chan), &(lk->lk));  // Release spinlock and make the process sleep
    }

    // Once the lock is free, acquire it for the current process
    lk->locked = Busy;
    lk->pid = get_cpu_proc()->env_id;  // Set the current process as the owner of the lock

    release_spinlock(&(lk->lk));  // Release the spinlock
}

// Releases the sleeplock and wakes up any processes that are waiting on it
void release_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'24.MS1 - #14] [4] LOCKS - release_sleeplock
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("release_sleeplock is not implemented yet");
	//Your Code is Here...
	
    // Protect the critical section with a spinlock
    acquire_spinlock(&(lk->lk));

    // Wake up all processes waiting on the sleeplock's channel
    if (queue_size(&(lk->chan.queue)))
    {
        wakeup_all(&(lk->chan));  // Wake up all processes sleeping on the channel
    }

    // Free the lock and reset its state
    lk->locked = Free;

    release_spinlock(&(lk->lk));  // Release the spinlock
}










