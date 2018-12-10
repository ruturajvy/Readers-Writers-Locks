/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	int a;
	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
//	kprintf("Process %s is getting killed, it's in state %d\n", pptr->pname, pptr->pstate, pptr-> pstate);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
		//	kprintf("Process itself called release\n");
			for(a=0;a<NLOCK; a++){
			if(proctab[pid].lock_list[a]!=UNUSED)
				{
				//kprintf("Lock type is not unused!\n");
				release(a, pid);
				}
			}
			resched();
			break;

	case PRWAIT:	semaph[pptr->psem].semcnt++;
			dequeue(pid);
		//	kprintf("Recursive rampdown-------------------\n");
			recursiveRampDown(proctab[pid].lock_q);
			pptr->lock_q = -1;
			break;

	case PRREADY://	kprintf("Process was in ready queue and got released\n");
			for(a=0;a<NLOCK; a++){
			if(proctab[pid].lock_list[a]!=UNUSED)
				release(a,pid);
			}
			dequeue(pid);
			pptr->pstate = PRFREE;
			break;
		
	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
		//	kprintf("Process was not released from any of the states so got released here!\n");
			for(a=0; a<NLOCK; a++){
			if(proctab[pid].lock_list[a]!=UNUSED)
				release(a,pid);
			}
	}
	restore(ps);
	return(OK);
}
