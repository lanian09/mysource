#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/statvfs.h>
#include <sys/procfs.h>
#include <errno.h>

#include <sys/file.h>
#include <sys/swap.h>
#include <sys/sysinfo.h>


#include "sysconf.h"

#define QUE_PUTTER 1
#define QUE_GETTER 2

#define MAXQUEUE 100
#define MAX_NUM_MSG 4096
#define MAX_MSG_SIZE 6000

typedef struct queue {
	int head;
	int tail;
	char  slot[MAX_NUM_MSG][MAX_MSG_SIZE];
} Queue;

typedef struct shmQueue {
	int   key;
	Queue *que;
} ShmQueue;

static ShmQueue shmQue[MAXQUEUE];
static	int	nextAllocPtr=0;

int timeQuantum;
int msgAvailable;
int idx;

int GetOption ();
void SigHandler ();
void PrintMsgHead ();
void PrintMsgNum ();

int qd[MAXQUEUE];


int get_msgQ (char *procname) 
{
	char        *env;
	char    	iv_home[64], l_sysconf[256], tmp[32];
	int			key, Qid;
 

    if( (env = getenv(IV_HOME)) == NULL)                                      
    {                                                                         
        fprintf(stderr, "[%s:%s:%d] not found %s environment name\n", __FILE__, __FUNCTION__, __LINE__, IV_HOME);
        return -1;                                                            
    }                                                                         
    strcpy(iv_home, env); 


	sprintf(l_sysconf, "%s/%s", iv_home, SYSCONF_FILE); 

    if(conflib_getNthTokenInFileSection(l_sysconf, "APPLICATIONS", procname, 1, tmp) < 0)
        exit(0);

    key = strtol(tmp,0,0);
    if( (Qid = msgget(key, IPC_CREAT | 0666)) < 0)
    {
        fprintf(stderr, "[%s:%s:%d] msgget fail; key=%x, err=%d(%s)\n", __FILE__, __FUNCTION__,__LINE__, key, errno, strerror(errno));
        return -1;
    }	

	return Qid;
}



main(argc, argv)
int argc;
char *argv[];
{
	int idx=0;
	int	tmr=1;

	if (argc ==2 ) tmr = atoi(argv[1]);

	for (idx=0; idx<MAXQUEUE; idx++)
		qd[idx] = -1;
	idx=0;
	
	qd[idx++] = get_msgQ ("CAPD");
	qd[idx++] = get_msgQ ("PANA");
	qd[idx++] = get_msgQ ("RANA");
	qd[idx++] = get_msgQ ("RLEG");
	qd[idx++] = get_msgQ ("SAMD");
	qd[idx++] = get_msgQ ("IXPC");

#if 0

    /* DSCB1 <--> SCPIF Queue */
    qd[idx++] = InitQueue (S_SHM_Q_DSCB1_SCPIF1, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_SCPIF1_DSCB1, QUE_PUTTER);

	/* CHAIF1,2,3,4 <--> DSCB1 Queue */
    qd[idx++] = InitQueue (S_SHM_Q_CHAIF1_DSCB1, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_DSCB1_CHAIF1, QUE_PUTTER);

    qd[idx++] = InitQueue (S_SHM_Q_CHAIF2_DSCB1, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_DSCB1_CHAIF2, QUE_PUTTER);

    qd[idx++] = InitQueue (S_SHM_Q_CHAIF3_DSCB1, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_DSCB1_CHAIF3, QUE_PUTTER);

    qd[idx++] = InitQueue (S_SHM_Q_CHAIF4_DSCB1, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_DSCB1_CHAIF4, QUE_PUTTER);

    qd[idx++] = InitQueue (S_SHM_Q_CSRB_DSCB, QUE_PUTTER);

    /* DSCB2 <--> SCPIF Queue */
    qd[idx++] = InitQueue (S_SHM_Q_DSCB2_SCPIF1, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_SCPIF1_DSCB2, QUE_PUTTER);

	/* CHAIF1,2,3,4 <--> DSCB2 Queue */
    qd[idx++] = InitQueue (S_SHM_Q_CHAIF1_DSCB2, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_DSCB2_CHAIF1, QUE_PUTTER);

    qd[idx++] = InitQueue (S_SHM_Q_CHAIF2_DSCB2, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_DSCB2_CHAIF2, QUE_PUTTER);

    qd[idx++] = InitQueue (S_SHM_Q_CHAIF3_DSCB2, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_DSCB2_CHAIF3, QUE_PUTTER);

    qd[idx++] = InitQueue (S_SHM_Q_CHAIF4_DSCB2, QUE_PUTTER);
    qd[idx++] = InitQueue (S_SHM_Q_DSCB2_CHAIF4, QUE_PUTTER);

    qd[idx++] = InitQueue (S_SHM_Q_CSRB_DSCB2, QUE_PUTTER);
#endif

	while (1)
    {
       PrintMsgHead ();
       PrintMsgNum ();
       sleep (tmr);
	}
}


void PrintMsgHead ()
{
   printf ( "\n %6s %6s %6s %6s %6s %6s %6s %6s %6s %6s %6s",
            "CAPD", "PANA", "RANA", "RLEG", "SAMD", "IXPC",
			"CH3->D", "CH3<-D", "CH4->D", "CH4<-D", "CSR->D");
   printf ("\n ----------------------------------------------------------------------------\n");
}

void PrintMsgNum ()
{
	int msgNum[MAXQUEUE];
	int msgByte[MAXQUEUE];
	int i=0, j=0;

	for (i=0,j=0; i<22; i++,j++) {
		msgNum[j] = 0;
		msgByte[j] = 0;
		if (qd[i] < 0) msgNum[j] = 0;
		else           msgNum[j] = GetMsgNum(qd[i], &msgByte[i]);
	}

	for (j=0; j<11; j++) {
		printf (" %6d", msgNum[j]);
		if (j == 10) printf ("\n");
	}

	for (j=0; j<11; j++) {
		printf (" %6d", msgByte[j]);
		if (j == 10) printf ("\n");
	}


	printf ("\n");
}


int
GetMsgNum (qd, byte)
int qd;
int *byte;
{
	static struct   msqid_ds queCntrlBuf;

	/*  get MSGQ control data   */
	if(msgctl(qd, IPC_STAT, &queCntrlBuf) < 0)
	{                                                                     
		return -1;
	}   

	*byte = (int)queCntrlBuf.msg_cbytes;

	return ((int)queCntrlBuf.msg_qnum);

#if 0
	if (shmQue[qd].que->head >= shmQue[qd].que->tail)
		return (shmQue[qd].que->head - shmQue[qd].que->tail);
	return (MAX_NUM_MSG-(shmQue[qd].que->tail - shmQue[qd].que->head));
#endif
}


#if 0

int InitQueue (key, who)
int key;
int who;
{
   int i;

   for (i=0; i<MAXQUEUE; i++)
      if (shmQue[i].key == key)
      {
         /* if QUE_GETTER is initialized then Clear Queue */
         if (who == QUE_GETTER)
         {
            shmQue[i].que->tail = 0;
            shmQue[i].que->head = 0;
         }
         return i;
      }
   
   /* if all queue is used then file freed queue */
   if (nextAllocPtr >= MAXQUEUE)
   {
      /* find empty queue */
      for (i=0; i<MAXQUEUE; i++)
         if (shmQue[i].key == -1)
            break;
      /* no empty queue */
      if (i >= MAXQUEUE)
         return -1;
   } else
      i = nextAllocPtr++;

   shmQue[i].key = key;
   shmQue[i].que = (Queue *)InitShmBuff (key, sizeof (Queue));
   if (shmQue[i].que == (Queue *)0)
      return -1;

   /* if QUE_GETTER is initialized then Clear Queue */
   if (who == QUE_GETTER)
   {
      shmQue[i].que->tail = 0;
      shmQue[i].que->head = 0;
   }

   /* return Queue Descriptor */
   return i;
}

#endif
