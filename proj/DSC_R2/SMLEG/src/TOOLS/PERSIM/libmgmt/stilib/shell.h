#ifndef SHELL_H
#define SHELL_H

#pragma pack(1)

#include <time.h>
#include <pthread.h>

#define MAXPROCESS	32

typedef struct proc {
    int     pid;
    int     run;
    time_t  stime;
    time_t  etime;
    char    path[64];       /* Fullpath */
    char    name[32];       /* Startup  */
    char    tag[32];        /* Tag      */

	char 	state;          /* for cpu load */
	unsigned long cputime;
	unsigned long otime;
	double 	ccpu;
	unsigned long dcpu;	

    int     crz_cnt;
} proc_t;

extern void* 	pstattask(void* arg);

extern proc_t 	proctab[MAXPROCESS];
extern int 		nproc;

extern int 	proc_quit();
extern void proc_show();
extern int 	proc_up(char* name);
extern int 	proc_dn(char* name);
extern int 	proc_conf(char* f,char* tag);
extern int 	proc_start(int);
extern int 	proc_stop(int);

extern void    (*proc_cb_up)(char*);
extern void    (*proc_cb_dn)(char*);

extern pthread_mutex_t	proc_mutex;

#define PROC_MUTEX_INIT()	do { pthread_mutex_init(&proc_mutex, NULL);	} while(0)
#define PROC_LOCK()			do { pthread_mutex_lock(&proc_mutex);		} while(0)
#define PROC_UNLOCK()		do { pthread_mutex_unlock(&proc_mutex);		} while(0)

#pragma pack()

#endif
