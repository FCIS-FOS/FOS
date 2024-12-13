// User-level Semaphore

#include "inc/lib.h"
struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	struct semaphore semaphores;
	if(value<0) 
	{
		semaphores.semdata=NULL;
		return semaphores;
	}
	struct __semdata* priv_data=(struct __semdata*)smalloc(semaphoreName,sizeof(struct __semdata),1);
	// if (priv_data==NULL)return ;// if(value<0) return; ??????????????????
	if(priv_data==NULL) 
	{
		semaphores.semdata=NULL;
		return semaphores;
	}
	priv_data->count=value;
	priv_data->lock=0;
	strcpy(priv_data->name,semaphoreName);
	
	sys_init_queue(&priv_data->queue);
	/*= (struct semaphore*)smalloc(semaphoreName,sizeof(struct semaphore),1)*/
	semaphores.semdata=priv_data;
	return semaphores; 
	//Your Code is Here...
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	// panic("get_semaphore is not implemented yet");
	struct __semdata* priv_data=(struct __semdata*)sget(ownerEnvID,semaphoreName);
	struct semaphore semaphores;
	if(priv_data==NULL)
	{
		semaphores.semdata=NULL;
		return semaphores;
	}
	semaphores.semdata=priv_data;
	return semaphores;
	//Your Code is Here...
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...

	cprintf("wait000\n");
    while(xchg(&(sem.semdata->lock), 1) != 0);
	cprintf("wait001\n");
    
    sem.semdata->count--;

    if(sem.semdata->count < 0)
    {
    
		cprintf("wait010\n");
       // while(xchg(&(sem.semdata->qlock), 1) != 0);
		cprintf("wait011\n");
        
        
        struct Env* current_env = sys_enqueue(&sem.semdata->queue, (struct Env*)myEnv, 0, &(sem.semdata->lock));
        
        
       // sem.semdata->qlock = 0;
        
        
        sem.semdata->lock = 0;
        
        
		cprintf("wait100\n");
       // current_env->env_status = ENV_BLOCKED;
		cprintf("wait101\n");
    }
    else
    {
        
		cprintf("wait110\n");
        sem.semdata->lock = 0;
    }
		cprintf("wait111\n");

}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...

	
    while(xchg(&(sem.semdata->lock), 1) != 0);
    
    sem.semdata->count++;
    
    if(sem.semdata->count <= 0)
    {
        
        //while(xchg(&(sem.semdata->qlock), 1) != 0);
        
        
        struct Env* e = sys_dequeue(&sem.semdata->queue);
        
        
        //sem.semdata->qlock = 0;
        
        

			e->env_status = ENV_READY;
        	sys_enqueue(&sem.semdata->queue, e, 1, &(sem.semdata->lock));
        

    }
    
    
    sem.semdata->lock = 0;
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
