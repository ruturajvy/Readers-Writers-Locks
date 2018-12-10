#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

int ldelete(int lid){

STATWORD ps;
int proc;
disable(ps);

if(locktab[lid].lstate == DELETED || isbadlock(lid)|| locktab[lid].lstate == LFREE)
{
restore(ps);
return (SYSERR);
}

locktab[lid].lstate = DELETED;
locktab[lid].ltype = UNUSED;
locktab[lid].readercount = 0;

/* Clearing the lock wating queue */

/*clear_lock_q(lid);
clear_proc_list(lid);*/



/* For PROCESSES in LOCK QUEUE */
int index = q[locktab[lid].lockhead].qnext;
if (q[locktab[lid].lockhead].qnext < NPROC)
{
while(index < NPROC)
{
proctab[index].lock_list[lid] = DELETED;
proctab[index].lock_return = DELETED;
dequeue(index);
ready(index, RESCHNO);
index = q[index].qnext;
}
resched();
}

/* For the LOCK's process list which is the list of processes holding the lock */
index = 0;
while(index < NPROC) 
{
if (locktab[lid].proc_list[index] == WRITE || locktab[lid].proc_list[index] == READ)
	locktab[lid].proc_list[index] = DELETED;
index++;
}

restore(ps);
return(OK);
}


