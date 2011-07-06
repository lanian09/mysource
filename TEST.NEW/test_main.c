/*******************************************************************************

	Author   : Hyungsek Son
	Section  : TEST Module
	SCCS ID  : @(#)test_main.c	1.1
	Date     : 09/30/2010
	Revision History :
		'2010.	09. 30. initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <almstat.h>

#include <tam_error.h>
#include <tam_svc.h>
#include <tam_shm.h>
#include <tam_define.h>
#include <tam_names.h>
#include <utillib.h>
#include "PLAT/rsc_list.h"

#ifdef _ENABLE_NIFO_
#include <nifo.h>		/*	stMEMSINFO	*/
#endif

#include <sys/msg.h>

/** B. DEFINITION OF NEW CONSTANTS ********************************************/
#define MSG_MAX_FILE 		"/proc/sys/kernel/msgmax"
#define MSG_MAX_SIZE_FILE	"/proc/sys/kernel/msgmnb"

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/

extern int 			errno;

int 	dStopFlag = 1;
int 	Finishflag = 0;

#ifdef _ENABLE_NIFO_
stMEMSINFO	*pMEMSINFO;
#endif /* _ENABLE_NIFO */


/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
void FinishProgram(void);
int dSignalHandling(void);

#ifdef _ENABLE_DAPPLOG_
extern int Init_shm_common(void);
extern int dAppLog(int dIndex, char *fmt, ...);
extern int dAppWrite(int dLevel, char *szMsg);

extern int set_version(int prc_idx, char *ver); // 040127,poopee
#endif /* _ENABLE_DAPPLOG_ */

#ifdef _ENABLE_NIFO_
void print_func(stMEMSINFO *pMEMSINFO, stMEMSNODEHDR *pMEMSNODEHDR);
#endif /* _ENABLE_NIFO */

/** E.2 DEFINITION OF FUNCTIONS ***********************************************/
extern int dInit_CTNInfo(void);

/*******************************************************************************
 * MAIN FUNCTION
*******************************************************************************/
int main(void)
{
	int		dRet;
	time_t	tCurTime, tCmpTime;

#ifdef _ENABLE_DAPPLOG_
	AppLogInit(getpid(), SEQ_PROC_QMON, QMON_LOG, "QMON");

	if( (dRet = dInitLogShm()) < 0)
	{
		dAppWrite(LOG_CRI,"FAILED IN dInitLogShm()");
		exit(0);
	}

	if( (dRet = Init_shm_common()) < 0)
	{
		dAppWrite(LOG_CRI,"FAILED IN Init_shm_common()");
		exit(0);
	}


	if( (dRet = set_version(SEQ_PROC_QMON, vERSION)) < 0)
		dAppLog( LOG_CRI, "set_version error(ret=%d,idx=%d,ver=%s)", dRet,SEQ_PROC_QMON,vERSION);

#endif /* _ENABLE_DAPPLOG_ */

#ifdef _ENABLE_NIFO_

	if((pMEMSINFO = nifo_init_tam(S_SSHM_NIFO, S_SEMA_NIFO, "QMON", SEQ_PROC_QMON)) == NULL) {
		dAppLog(LOG_CRI, "F=%s:%s.%d nifo_init_tam NULL", __FILE__, __FUNCTION__, __LINE__);
		exit(1);
	}
#endif /* _ENABLE_NIFO_ */

	if( (dRet = dSignalHandling()) < 0)
	{
		dAppLog( LOG_CRI, "[ERROR] SignalHandling");
		exit(0);
	}

#ifdef _ENABLE_DAPPLOG_
	dAppLog(LOG_CRI, "TEST v1.0 Started");
#else
	printf("test module start\n");
#endif /* _ENABLE_DAPPLOG_ */

	tCurTime = time(NULL);
	printf("starttime=>%ld\n", tCurTime);
	dInit_CTNInfo();
	dStopFlag = 0;
	while(dStopFlag)
	{
		tCmpTime = time(NULL);
		printf("currTime=%ld\n",(long) tCmpTime);
		if( (tCmpTime - tCurTime) > 3 ){
			printf("pass 3 second\n");
			break;
		}
		dInit_CTNInfo();
		usleep(300000);
	}
	FinishProgram();

	return 0;
}

/*******************************************************************************
 * SIGNAL FUNCTION
*******************************************************************************/
void UserControlledSignal(int sign)
{
    Finishflag = sign;
    dStopFlag = 0;
}

/*******************************************************************************
 * SIGNAL FUNCTION
*******************************************************************************/
void IgnoreSignal(int sign)
{
    if(sign != SIGALRM) {
#ifdef _ENABLE_DAPPLOG_
    	dAppLog( LOG_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign );
#else
    	printf("UNWANTED SIGNAL IS RECEIVED, signal = %d\n", sign );
#endif /* _ENABLE_DAPPLOG_ */
    }

    signal(sign, IgnoreSignal);
}

/*******************************************************************************
 * FINISH PROGRAM
*******************************************************************************/
void FinishProgram()
{
#ifdef _ENABLE_DAPPLOG_
    dAppLog( LOG_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", Finishflag);
#else
    printf("PROGRAM IS NORMALLY TERMINATED, Cause = %d\n", Finishflag);
#endif /* _ENABLE_DAPPLOG_ */

    exit(0);
}

/*******************************************************************************
 * INITIALIZATION
*******************************************************************************/
int dSignalHandling(void)
{
	/* WANTED SIGNALS   */
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT,  UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP,  IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, SIG_IGN);

	return 1;
}


/*******************************************************************************
 *
*******************************************************************************/

/*******************************************************************************
 *
*******************************************************************************/
#ifdef _ENABLE_NIFO_
void print_func(stMEMSINFO *pMEMSINFO, stMEMSNODEHDR *pMEMSNODEHDR)
{
    U8          *pNode;
    S32         dRet;
    U8          *pNextNode;
    U8          *p, *data;
    S32         type, len, ismalloc;
	LOG_RPPI	*pLOGRPPI;

    pNode = (U8 *)pMEMSNODEHDR + stMEMSNODEHDR_SIZE;

    pNextNode = pNode;

    do {
        p = pNextNode;

        while(p != NULL) {
            if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, &type, &len, &data, &ismalloc, &p)) < 0)
                break;

            dAppLog(LOG_CRI, "####################################################################");
            dAppLog(LOG_CRI, "TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", type,
                ((type==START_CALL_NUM || type==STOP_CALL_NUM || type==RADIUS_START_NUM ||
                type==LOG_PISIGNAL_DEF_NUM) ? PRINT_TAG_DEF_ALL_CALL_INPUT(type) :
                PRINT_DEF_NUM_table_log(type)), len, (ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");
            switch(type)
            {
			case LOG_RPPI_DEF_NUM:
				pLOGRPPI = (LOG_RPPI *)data;
				dAppLog(LOG_CRI, "IMSI=%s CallTime=%u.%u", pLOGRPPI->szIMSI, pLOGRPPI->uiCallTime, pLOGRPPI->uiCallMTime);
				break;
            default:
                break;
            }
        }

        pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

    } while(pNode != pNextNode);

}
#endif /* _ENABLE_NIFO_ */
