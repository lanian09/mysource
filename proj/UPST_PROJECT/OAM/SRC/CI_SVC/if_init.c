/** A. FILE INCLUSION *********************************************************/

/* SYS HEADER */
#include <stdlib.h>		/* EXIT(3) */
#include <unistd.h>		/* CLOSE(2) */
#include <string.h>		/* strerror() */
#include <errno.h>		/* errno */
#include <signal.h>		/* signal */
/* LIB HEADER */
#include "commdef.h"	/* FILE_* */
#include "loglib.h"
#include "clisto.h"		/* U8 */
#include "mems.h"		/* stMEMSINFO */
#include "cifo.h"		/* stCIFO */
#include "nifo.h"		/* nifo_init_zone() */
#include "gifo.h"		/* gifo_init_group() */
/* PRO HEADER */
#include "path.h"
#include "msgdef.h"		/* S_MSGQ_* */
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "if_init.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern stMEMSINFO *gpMEMSINFO;
extern stCIFO	  *gpCIFO;

extern int g_FinishFlag;
extern int g_JiSTOPFlag;

int init_ipcs()
{
	gpMEMSINFO = nifo_init_zone((U8*)"CI_SVC", SEQ_PROC_SI_SVC, FILE_NIFO_ZONE);
    if( gpMEMSINFO == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN nifo_init, NULL", LT);
        return -1;
    }
            
    //GIFO를 사용하기 위한 group 설정
    gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
    if( gpCIFO == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group. cifo=%s, gifo=%s",
                LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }
	return 0;
}

int dInitProc()
{
	int		dRet;

	SetUpSignal();


	dRet = init_ipcs();
	if(dRet < 0)
	{
		log_print(LOGN_DEBUG,"[dInit_Ipaf_T_If] [PROCESS INIT FAIL]");
		return -1;
	}

	return 0;
}

void UserControlledSignal(int sign)
{
    g_JiSTOPFlag = 0;
    g_FinishFlag = sign;
}


void FinishProgram(int dSocket)
{
	close(dSocket);
    log_print(LOGN_CRI,"PROGRAM IS NORMALLY TERMINATED, Cause = %d", g_FinishFlag);
    exit(0);
}

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_WARN,"UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

void SetUpSignal()
{
    g_JiSTOPFlag = 1;

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

    log_print(LOGN_DEBUG,"SIGNAL HANDLER WAS INSTALLED");
}
