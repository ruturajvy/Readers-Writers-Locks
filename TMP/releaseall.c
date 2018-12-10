#include<conf.h>
#include<kernel.h>
#include<proc.h>
#include<q.h>
#include<sem.h>
#include<stdio.h>
#include<sleep.h>
#include<lock.h>

extern unsigned long ctr1000;

int are_no_holders(int lock);
int release(int lid, int proc);
int releaseall(int numlocks, int ldes1, ...) {
	int value = OK;
	STATWORD ps;
	int lock_id;
	int sp = 0;
	int flag = 1;
	disable(ps);
//	kprintf("RELEASE: Lock ID %d\n", ldes1);
//	kprintf("RELEASE: Lock Type %d\n", locktab[ldes1].ltype);
//	kprintf("RELEASE: Lock ID %d\n", locktab[ldes1].proc_list[currpid]);
	while (sp < numlocks) {
		lock_id = (int)*((&ldes1) + sp);
	//	kprintf("RELEASE ARGS: Lock ID %d\n", lock_id);
	//        kprintf("RELEASE ARGS: Lock Type %d\n", locktab[lock_id].ltype);
        //	kprintf("RELEASE ARGS: Lock ID %d\n", locktab[lock_id].proc_list[currpid]);

		value = release(lock_id, currpid);
		if (value == SYSERR)
			{
		//	kprintf(" The returned value is SYSERR or %d for lock id %d\n",value, lock_id);
			flag = 2;
			}
	numlocks--;
	}
	if(flag == 2) {
		resched();
		restore(ps);
		return SYSERR;
	}
	else {
		resched();
		restore(ps);
		return value;
	}
}

int release(int lid, int proc) {
	struct pentry *ptr = &proctab[proc];
	struct lentry *ltr = &locktab[lid];
//	kprintf("Process calling RELEASE is %s \n",ptr->pname);
	int index = 0;
	int pid1;
	int writer_pid;
	int pid2;
	unsigned long pid2_time;
	unsigned long writer_time;

	int return_value;

	if(isbadlock(lid)|| (ltr->lstate== DELETED)||(ltr->ltype==UNUSED)||(ltr->lstate==LFREE)||(ltr->proc_list[proc] == UNUSED))
	{
	//	kprintf("Lid %d\n",lid); 
	//	kprintf("Lock type was %d\n", ltr->ltype);
	//	kprintf("Lock state was %d\n", ltr->lstate);
		//kprintf("Lock_list[lid] was %d\n", ptr->lock_list[lid]);
	//	kprintf("Proc_list[currpid] was %d\n", ltr->proc_list[proc]);		
	//	kprintf("ERROR!\n");
		return SYSERR;
	}

	ltr->proc_list[proc] = UNUSED;
	ptr->lock_list[lid] = UNUSED;
	ptr->acquired[lid] = 0;

	if(are_no_holders(lid)) {
	//	kprintf("RELEASE: No holders! Have to give lock to one of the waiters!\n");
		if (ltr->ltype == READ) {
		//kprintf("RELEASE: Current lock type is read!So this was the last reader!\n");
			if (isempty(ltr->lockhead)) {
		//	kprintf("RELEASE: No one is waiting in the queue! Make the locktype UNUSED and return!\n");
				ltr->ltype = UNUSED;
				return OK;
			}	
			else
			{
				//kprintf("RELEASE: Some one is waiting in the queue and that means it's a writer! Giving lock to it!\n");
				index = q[ltr->locktail].qprev;
				ltr->proc_list[index] = WRITE;
				proctab[index].lock_list[lid] = WRITE;
				ltr->ltype = WRITE;
				dequeue(index);
				ptr->lock_q = -1;
				rampDownPriority(lid);
				ready(index, RESCHNO);
				return OK;
			}
			}
		else{	//kprintf("RELEASE: Writer is holding the lock!\n");
			if (isempty(ltr->lockhead)) {
				//kprintf("RELEASE: No one is waiting for this lock!");
				ltr->ltype = UNUSED;
				return OK;
			}
			else {//kprintf("RELEASE: Someone is waiting!\n ");
				pid1 = q[locktab[lid].locktail].qprev;
				while ((pid1 < NPROC)&&(pid1!=0))								
				{	//kprintf("TRAVERSING the lock queue!\n");
//					kprintf("The type of the lock requested by %s is %d\n",proctab[pid1].pname,  proctab[pid1].lock_list[lid]);
					if (proctab[pid1].lock_list[lid] == READ) {
						//kprintf("TRAVERSE: Reader in the queue! Put it in ready immediately!\n");		
						dequeue(pid1);
						ptr->lock_q = -1;
						ltr->proc_list[pid1] = READ;
						proctab[pid1].lock_list[lid] = READ;
						ltr->ltype = READ;
						(ltr->readercount) += 1;
						rampDownPriority(lid);
						ready(pid1, RESCHNO);
					}
					else if(proctab[pid1].lock_list[lid] == WRITE){//kprintf("TRAVERSE: Writer in the queue!\n");											
						writer_pid = pid1;
						pid2 = q[writer_pid].qprev;
						if ((pid2 >= NPROC) && (ltr->ltype == WRITE)) 
						{
						//	kprintf("TRAVERSE: No one is after the writer AND it is the first writer in the queue! Give it the lock! Put it in ready! \n");
						//	kprintf("TRAVERSE: Breaking out of the traverse queue as writer with highest priority found!\n");											
							ltr->proc_list[writer_pid] = WRITE;
							proctab[writer_pid].lock_list[lid] = WRITE;
							ltr->ltype = WRITE;
							dequeue(writer_pid);
							ptr->lock_q = -1;
							rampDownPriority(lid);
							ready(writer_pid, RESCHNO);
							break;
						}
						while ((q[pid2].qkey == q[writer_pid].qkey) && (pid2<NPROC)&&(pid2!=0))
						{
							if (proctab[pid2].lock_list[lid] == READ)
							{
								writer_time = ctr1000 - proctab[writer_pid].lock_time;
								pid2_time = ctr1000 - proctab[pid2].lock_time;

								if ((writer_time - pid2_time) <= 500)
								{
									dequeue(pid2);
									ptr->lock_q = -1;
									ltr->proc_list[pid2] = READ;
									proctab[pid2].lock_list[lid] = READ;
									ltr->ltype = READ;
									(ltr->readercount) += 1;
									rampDownPriority(lid);
									ready(pid2, RESCHNO);
								}
							}
							pid2 = q[pid2].qprev;
						}
					//	kprintf("TRAVERSE: Confirmed that no reader after this writer that caan be given lock! Now check if it's the first writer!\n");
						if (ltr->ltype == WRITE)
						{//	kprintf("TRAVERSE: It is the first writer and no reader after it, give it the lock\n");
							ltr->proc_list[writer_pid] = WRITE;
							proctab[writer_pid].lock_list[lid] = WRITE;
							ltr->ltype = WRITE;
							dequeue(writer_pid);
							ptr->lock_q = -1;
							rampDownPriority(lid);
							ready(writer_pid, RESCHNO);
							break;
						}
					//	kprintf("ERROR: Should not have reached here! IMPOSSIBLE!\n");
						}
					else{
					dequeue(proc);
					ptr->lock_q = -1;
					rampDownPriority(lid);
					return OK;
					}
					pid1 = q[pid1].qprev;
				}	
				return OK;
			}
		}
	}
	else
		{
		//kprintf("Some readers are still holding the lock so do nothing for the time being!\n");
		return OK;
		}
}

int are_no_holders(int ldesc)
{//	kprintf("Checking holder......\n");
	if (locktab[ldesc].ltype == READ)
	{       
		(locktab[ldesc].readercount)--;
		if ((locktab[ldesc].readercount) > 0)
			return FALSE;
		else
			return TRUE;
	}
	if (locktab[ldesc].ltype == WRITE)
		return TRUE;
}
