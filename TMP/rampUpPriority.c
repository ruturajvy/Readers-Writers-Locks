#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

void rampUpPriority(int lid)
{

        int i;
        int index;
        int priority = 0;
       // kprintf("Process %s was inserted in the queue and called RAMPUP!\n",proctab[currpid].pname);
       // kprintf("Max Prio of this lock is %d\n",locktab[lid].lmax_prio);
        index = q[locktab[lid].locktail].qprev;
        while (index < NPROC)
        {
       // kprintf("LOCK QUEUE: Process %s \n",proctab[index].pname);
       // kprintf("LOCK QUEUE: Priority %d \n",proctab[index].pprio);
                if (proctab[index].pprio > priority)
                {
                        priority = proctab[index].pprio;
                }
                index = q[index].qprev;
        }
        locktab[lid].lmax_prio = priority;
     //   kprintf("Newly calculated max prio is %d \n",locktab[lid].lmax_prio);

        for (i = 0; i < NPROC; i++)
        {
                if (locktab[lid].proc_list[i] != UNUSED)
                {
               // kprintf("This process holds the lock! %s\n",proctab[i].pname);
               // kprintf("Its priority is %d\n",proctab[i].pprio);
                        if (locktab[lid].lmax_prio > proctab[i].pprio)
                        { //      kprintf("The priority of one of the waiters is more than the holder!\n");
                               //proctab[i].pinh = proctab[i].pprio;
                                proctab[i].pprio = locktab[lid].lmax_prio;
			//	kprintf("Pinh while ramping up %d\n",proctab[i].pinh);
                                if (proctab[i].lock_q != -1){
                                        rampUpPriority(proctab[i].lock_q);
					//kprintf("Holder waiting for lock id %d . So calling RECURSIVELY!\n",proctab[i].lock_q);
				}
			//	kprintf("Holder not waiting for anything! No recursion required.\n");
                        }
                }
        }
}

