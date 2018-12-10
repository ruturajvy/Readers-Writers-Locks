#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

extern unsigned long ctr1000;

int lock(int ldes1, int type, int priority)
{
	//kprintf("LOCK: Lid passed was %d\n",ldes1);
	struct lentry *lptr;
	lptr = &locktab[ldes1];
	STATWORD ps;
	struct pentry *pptr;
	int a;
	disable(ps);
	//kprintf("LOCK: State %d \n",lptr->lstate);
	//kprintf("LOCK: Type %d \n", lptr->ltype);
	//kprintf("LOCK: Lock_list entry %d\n", proctab[currpid].lock_list[ldes1]);
	//kprintf("LOCK: Proc list entry %d\n", locktab[ldes1].proc_list[currpid]);

	if (isbadlock(ldes1) || lptr->lstate == LFREE || lptr->lstate == DELETED || proctab[currpid].lock_list[ldes1] != UNUSED)
	{
		//kprintf("LOCKING ERROR! : Lid = %d\n",ldes1);
		restore(ps);
		return(SYSERR);
	}

	pptr = &proctab[currpid];
	pptr->lock_return = OK;

	switch (lptr->ltype) {

	case UNUSED:
		pptr->lock_list[ldes1] = type;
		lptr->proc_list[currpid] = type;
		lptr->ltype = type;
		pptr->acquired[ldes1] = 1;
	//	kprintf("LOCK Descriptor is : %d \n", ldes1);
	//	kprintf("LOCK CASE UNUSED: Lock acquired type %d\n", type);
	//	kprintf("LOCK: Type became %d \n", lptr->ltype);
		//kprintf("LOCK: \n");
		if (type == READ)
			lptr->readercount++;
		break;
	case READ:
		if (type == WRITE)
		{	//kprintf("LOCK CASE READ: Writer asking\n");
			pptr->lock_list[ldes1] = type;
			//kprintf("LOCK: Writer waits! And makes lock_list type  %d \n",pptr->lock_list[ldes1]);
			// kprintf("LOCK: Type became %d \n", lptr->ltype);
			pptr->pstate = PRWAIT;
			pptr->lock_time = ctr1000;
			pptr->lock_q = ldes1;
			insert(currpid, lptr->lockhead, priority);
			rampUpPriority(ldes1);
			resched();
		}
		else if (type == READ)
		{	//kprintf("LOCK CASE READ: Reader asking for lock \n");
			int flag = 1;
			int index = q[lptr->locktail].qprev;
			while ((index < NPROC) && (priority<q[index].qkey)) {
				if (proctab[index].lock_list[ldes1] == WRITE) {
			//	kprintf("LOCK: Writer found with more priority! \n");
					flag = 2;
					break;
				}
				index = q[index].qprev;
			}
			if (flag == 2)
			{//	kprintf("So now reader waits behind the writer in queue!\n");
				pptr->pstate = PRWAIT;
				pptr->lock_time = ctr1000;
				pptr->lock_q = ldes1;
				pptr->lock_list[ldes1] = type;
				insert(currpid, locktab[ldes1].lockhead, priority);
				rampUpPriority(ldes1);
				resched();
			}
			else {//  kprintf("No higher priority writer in queue, so give lock to the reader! \n");
				locktab[ldes1].ltype = type;
				locktab[ldes1].readercount++;
				pptr->lock_list[ldes1] = type;
				pptr->acquired[ldes1] = 1;
				locktab[ldes1].proc_list[currpid] = type;
			}
		}
		break;

	case WRITE:
		//kprintf("LOCK CASE WRITE:\n");
		proctab[currpid].pstate = PRWAIT;
		proctab[currpid].lock_list[ldes1] = type;
		proctab[currpid].lock_time = ctr1000;
		pptr->lock_q = ldes1;
		//kprintf("LOCK: Lock_list type %d  \n", proctab[currpid].lock_list[ldes1]);
		//kprintf("LOCK: Time it started waiting %lu \n", proctab[currpid].lock_time);
		insert(currpid, lptr->lockhead, priority);
//		kprintf("CALLING RAMPUP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		rampUpPriority(ldes1);
		resched();
		break;
	default:
		a = 10;
		while (a<0)
			a--;
	}
	restore(ps);
	return(pptr->lock_return);
}
