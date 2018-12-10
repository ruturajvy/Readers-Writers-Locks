#ifndef H_LOCK_H_
#define H_LOCK_H_

/*#ifndef PROC
#define NPROC 50
#endif */
#include "proc.h"  

#ifndef	NLOCK
#define NLOCK 50
#endif

/* LOCK TYPE */
#define READ	11		/* locked by a reader */
#define WRITE	22		/* locked by a writer */
#define UNUSED	33		/* locked by no process */


/* LOCK STATE */

#define LFREE	44
#define LUSED 	55

struct lentry{
	int lstate;		/* LFREE or LUSED*/
	int ltype;		/* UNUSED READ or WRITE */
	int readercount;	/* Number of readers using the lock if type is READ */
	int lockhead;		/* Waiting queue head id*/
	int locktail;		/* Waiting queue tail id*/
	int proc_list[NPROC];	/* Bitmask storing the the processes holding the lock along with the lock type READ, WRITE */
	int lmax_prio;
};

extern struct lentry locktab[NLOCK];
extern int nextlock;
extern unsigned long ctr1000;

SYSCALL lcreate();
int ldelete (int lockdescriptor);
void linit();
int lock( int ldes1, int type, int priority);
int releaseall( int numlocks, int ldes1, ...);
int release( int lid, int proc);
void rampUpPriority(int lid);
void rampDownPriority(int lock);
void recursiveRampDown(int lock_id);
#define isbadlock(l)    (l<0 || l>=NLOCK)
#endif
