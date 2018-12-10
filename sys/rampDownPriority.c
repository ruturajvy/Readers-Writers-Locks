#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

void rampDownPriority(int lid) 
{
	int i;
	int index;
	int priority = 0;
	int orig = 0;
	index = q[locktab[lid].locktail].qprev;
	while (index < NPROC)								
	{
		if (priority < proctab[index].pprio)
			priority = proctab[index].pprio;
		index = q[index].qprev;
	}
	locktab[lid].lmax_prio = priority;

	int j;
	int p = 0;
	for (j = 0; j < NLOCK; j++)  
	{
		if (proctab[currpid].acquired[j] == 1)
		{
			if (locktab[j].lmax_prio > p)
			{
				p = locktab[j].lmax_prio;
			}
		}
	}
	orig = proctab[currpid].pprio;
	if (p == 0 || p < proctab[currpid].pinh)  
	{
		proctab[currpid].pprio = proctab[currpid].pinh;
	//	kprintf("Priority assigned was the original priority!: %d", proctab[currpid].pinh);
	}
	else 
	{
		proctab[currpid].pprio = p;
	//	kprintf("Priority assigned was the current max lprio of all locks held!\n", p);
	}
	if (proctab[currpid].pprio < orig)
	{
		if (proctab[currpid].lock_q != -1)
		{
		//	kprintf("Calling resursive ramp up\n");
			recursiveRampDown(proctab[currpid].lock_q);
		}
	}
}

void recursiveRampDown(int lid) 
{
	int i;
	int index;
	int priority= 0;
	//kprintf("Called recursive ramp down for lock id %d\n", lid);
	index = q[locktab[lid].locktail].qprev;
	while (index < NPROC)							
	{//	kprintf("LOCK QUEUE: Process: %s Priority %d\n", proctab[index].pname,proctab[index].pprio);
		if (priority < proctab[index].pprio)
			priority = proctab[index].pprio;
		index = q[index].qprev;
	}
		locktab[lid].lmax_prio = priority;
	//	kprintf("Maximum priority in the queue: %d\n",locktab[lid].lmax_prio);				

	for (i = 0; i < NPROC; i++)							
	{
		if(locktab[lid].proc_list[i] != UNUSED)
		{
		//kprintf("The process holding the lock is: %s\n", proctab[i].pname);
		//kprintf("Pinh: %d\n",proctab[i].pinh);
		//kprintf("Priority %d\n",proctab[i].pprio);
			if (locktab[lid].lmax_prio < proctab[i].pprio)
			{
			//	kprintf("Lock_q of %s is %d\n",proctab[i].pname,proctab[i].lock_q);
				if (locktab[lid].lmax_prio < proctab[i].pinh)
					{
					proctab[i].pprio = proctab[i].pinh;
					
					}
				else
				{
					proctab[i].pprio =  locktab[lid].lmax_prio;
				}
				if (proctab[i].lock_q != -1)
					{
				//	kprintf("Calling RECURSIVE ramp up RECURSIVELY for no reason!\n");
					recursiveRampDown(proctab[i].lock_q);
					}
			}
		}
	}

}
