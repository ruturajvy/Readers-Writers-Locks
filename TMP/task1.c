#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>


#define LOCK_PRIO 20	

LOCAL void writer11 (char *msg, int lck1){
	int i,j;
        kprintf("%c started\n", msg);
        int retval = lock (lck1, WRITE, LOCK_PRIO);
	
        if(retval == SYSERR){
            kprintf("%c failed acquiring lock! Exiting", msg);
            return;
        }
        if(retval == DELETED){
             kprintf("%c acquiring a deleted lock! Exiting", msg);
            return;           
        }

        kprintf("%c enters critical section\n", msg);
	sleep(3);
/*      	 for(i=0;i<100;i++)
        {
        for(j=0;j<1000000;j++)
        ;
        kprintf("W");
        }
*/
      
        kprintf("%c done, pprio=%d releasing locks\n", msg, getprio(currpid));
        
        releaseall (1, lck1);
}
LOCAL void reader11 (char *msg, int lck1){
	int i,j;
        kprintf("%c started\n", msg);
        int retval = lock (lck1, READ, 10);

        if(retval == SYSERR){
            kprintf("%c failed acquiring lock! Exiting", msg);
            return;
        }
        if(retval == DELETED){
             kprintf("%c acquiring a deleted lock! Exiting", msg);
            return;
        }
        kprintf("%c enters critical section\n", msg);
	sleep(3);
  /*       for(i=0;i<100;i++)
        {
        for(j=0;j<1000000;j++)
        ;
        kprintf("R");
        }
*/

        kprintf("%c done, pprio=%d releasing locks\n", msg, getprio(currpid));

        releaseall (1, lck1);
}


LOCAL void writer22(char* msg, int sem){
	int i,j;
        kprintf("%c started\n", msg);
        int retval = wait (sem);

        if(retval == SYSERR){
            kprintf("%c failed acquiring lock!", msg);
            return;
        }
        if(retval == DELETED){
             kprintf("%c acquiring a deleted lock! Exiting", msg);
            return;           
        }        
        kprintf("%c enters critical section\n", msg);
	sleep(3);
  /*    	 for(i=0;i<100;i++)
        {
        for(j=0;j<1000000;j++)
        ;
        kprintf("W");
        }
*/
      
        kprintf("%c done, pprio=%d releasing locks\n", msg, getprio(currpid));
        signal(sem);
}
LOCAL void reader22(char* msg, int sem){
	int i,j;
        kprintf("%c started\n", msg);
        int retval = wait (sem);

        if(retval == SYSERR){
            kprintf("%c failed acquiring lock!", msg);
            return;
        }
        if(retval == DELETED){
             kprintf("%c acquiring a deleted lock! Exiting", msg);
            return;
        }
        kprintf("%c enters critical section\n", msg);
	sleep(3);
/*	 for(i=0;i<100;i++)
        {
        for(j=0;j<1000000;j++)
        ;
        kprintf("R");
        }
*/

        kprintf("%c done, pprio=%d releasing locks\n", msg, getprio(currpid));
        signal(sem);
}

LOCAL void proc (char *msg){
	int i,j;
        kprintf("%c started\n", msg);
/*	 for(i=0;i<100;i++)
        {
        for(j=0;j<1000000;j++)
        ;
        kprintf("M");
        }
*/	sleep(2);
        kprintf("%c done, pprio=%d\n", msg, getprio(currpid));
}



void task1 (){

        int a, b, c;
        kprintf("\nTesting priority inheritance with PA2 implementation\n");

        int lck1  = lcreate ();
        if(lck1 == SYSERR || lck1 == DELETED){
            kprintf("error creating the lock!\n");
            return;
        }

        a = create(writer11, 2000, 30, "writer1", 2, 'W', lck1);
        b = create(proc, 2000, 40, "proc", 1, 'M');
        c = create(reader11, 2000, 50, "reader1", 2, 'R', lck1);

        resume(a);
        sleep(1);
        resume(b);
        sleep(1);
        resume(c);
        sleep(10);


       kprintf("\n\nTesting XINU lock implementation\n");

        int sem = screate(1);
        if(sem == SYSERR || sem == DELETED){
            kprintf("error creating the lock!\n");
            return;
        }        
        a = create(writer22, 2000, 30, "writer1", 2, 'W', sem);
        b = create(proc, 2000, 40, "proc", 1, 'M');
        c = create(writer22, 2000, 50, "reader1", 2, 'R', sem);
        
        resume(a);
        sleep(1);
        resume(b);
        sleep(1);
        resume(c);

        sleep(10);

        kprintf("Task1 done!\n");
}




