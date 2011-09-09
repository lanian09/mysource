#ifndef __CHSMD_HW_H__
#define __CHSMD_HW_H__

#include "time.h"

#define LSIZE 256  /* ONE LINE SIZE */

enum _en_dag_type {
	TYPE_DAG3_80S, //0
	TYPE_DAG3_70G, //1
	TYPE_DAG4_3GE, //2
	TYPE_DAG4_5GE, //3
	TYPE_DAG7_5G2
};

#define _UP		1 /* DAG STATUS : PORT ALIVE */

typedef struct _st_PthrdDirect
{
    pthread_t       PthrdDirID;
    pthread_mutex_t PthrdMutex;
    char            cStop;
    int             dArg;
    unsigned int    dRunCount;
    unsigned int    dLastCount;
    void            *pRetVal;

    time_t          tStart;
    time_t          tLast;
    time_t          tLastRenew;
    FILE            *fPipe;
} st_PthrdDirect;


extern int dCheckHW(void);
extern int dCheckDagType(void);
extern int dGetDagSts(char *flag, int cnt, int type);

extern void Send_CondMess(int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm);
extern void Send_CondDirSWMess(unsigned char cSysNo, char cInvType, unsigned char cInvNo, char cCurAlmStat, char cOldAlmStat);
extern void DecideResult(int dDirectIdx, time_t tUpdate, char *n1, char *n2, char *m1);

#endif /* __CHSMD_HW_H__ */
