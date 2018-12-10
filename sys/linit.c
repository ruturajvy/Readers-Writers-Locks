#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <stdio.h>
#include <lock.h>

struct lentry locktab[NLOCK];

void linit()
{
	int i=0;
	int pid = 0;
	while(i<NLOCK){
		locktab[i].lstate = LFREE;
		locktab[i].ltype = UNUSED;
		//kprintf("Lockstate is %d \n", locktab[i].lstate);
		//kprintf("Locktype is %d \n", locktab[i].ltype);
		locktab[i].readercount = 0;
		locktab[i].lockhead= newqueue();
		locktab[i].locktail= locktab[i].lockhead + 1;
		locktab[i].lmax_prio = 0;
		pid = 0;
		while(pid<NPROC)
		{
		locktab[i].proc_list[pid] = UNUSED;
		pid++;
		}
		i++;
	}
	
}
