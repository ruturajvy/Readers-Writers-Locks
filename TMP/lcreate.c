#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlid();

SYSCALL lcreate()
{
	int lid;
	STATWORD ps;
	disable(ps);
	lid = newlid();
	//kprintf("LCREATE: Got the lock id as %d\n", lid);
	if(lid == SYSERR)
	{
		restore(ps);
		return (SYSERR);
	}

	restore(ps);
	//kprintf("LCREATE returned the lock id: %d\n",lid);
	return(lid);
}

LOCAL int newlid()
{
	int i;
	int lock;
	//kprintf("NEWLID thinks nextlock is %d\n",nextlock);
	//kprintf("NEWLID thinks NLOCK is %d\n", NLOCK);
	for(i=0; i< NLOCK; i++)
	{
		lock = nextlock--;
		if(nextlock <0)
			nextlock = NLOCK - 1;
		if (locktab[lock].lstate==LFREE || locktab[lock].lstate==DELETED){
			locktab[lock].lstate = LUSED;
			locktab[lock].ltype = UNUSED;
		//	kprintf("NEWLID return the lock id as %d\n",lock);
			return(lock);
		}
	}
	return(SYSERR);
}
