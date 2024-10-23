/*
 * channel.c
 *
 *  Created on: Sep 22, 2024
 *      Author: HP
 */
#include "channel.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <inc/string.h>
#include <inc/disk.h>

//===============================
// 1) INITIALIZE THE CHANNEL:
//===============================
// initialize its lock & queue
void init_channel(struct Channel *chan, char *name)
{
	strcpy(chan->name, name);
	init_queue(&(chan->queue));
}

//===============================
// 2) SLEEP ON A GIVEN CHANNEL:
//===============================
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// Ref: xv6-x86 OS code
void sleep(struct Channel *chan, struct spinlock *lk)
{
    struct Env *threadToSleep  = get_cpu_proc();  // Get the current process/thread
    acquire_spinlock(&ProcessQueues.qlock);  // Lock the queue to protect it
    release_spinlock(lk);  // Release the provided lock so other processes can run
    enqueue(&(chan->queue), threadToSleep);  // Add the process to the channel's waiting queue
    threadToSleep->env_status = ENV_BLOCKED;  // Mark the process as blocked
    sched();  // Switch to another process (since the current one is now blocked)
    acquire_spinlock(lk);  // Reacquire the provided lock after waking up
    release_spinlock(&ProcessQueues.qlock);  // Unlock the queue
}

//==================================================
// 3) WAKEUP ONE BLOCKED PROCESS ON A GIVEN CHANNEL:
//==================================================
// Wake up ONE process sleeping on chan.
// The qlock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_one(struct Channel* chan)
{
	//TODO: [PROJECT'24.MS1 - #11] [4] LOCKS - wakeup_one
    if (queue_size(&(chan->queue))) {  // Check if there are any processes waiting on the channel
        acquire_spinlock(&ProcessQueues.qlock);  // Lock the queue to protect it
        struct Env *temp = dequeue(&(chan->queue));  // Remove one process from the waiting queue
        sched_insert_ready0(temp);  // Add the process to the ready queue so it can run
        release_spinlock(&ProcessQueues.qlock);  // Unlock the queue
    }
}

//====================================================
// 4) WAKEUP ALL BLOCKED PROCESSES ON A GIVEN CHANNEL:
//====================================================
// Wake up all processes sleeping on chan.
// The queues lock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes

void wakeup_all(struct Channel* chan)
{
	//TODO: [PROJECT'24.MS1 - #12] [4] LOCKS - wakeup_all
    while (queue_size(&(chan->queue)))  // Keep waking up processes while the queue is not empty
    {
        wakeup_one(chan);  // Wake up one process at a time
    }
}

