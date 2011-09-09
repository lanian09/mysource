/**
	@file		mmcd_init.c
	@author
	@version
	@date		2011-07-14
	@brief		mmcd_init.c
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

/* LIB HEADER */
#include "commdef.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "config.h"
#include "loglib.h"
/* PRO HEADER */
#include "path.h"
#include "msgdef.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "mmcd_init.h"

/**
	Declare variables
*/
extern stMEMSINFO	*gpRECVMEMS;		/*< mems.h */
extern stCIFO		*pCIFO;				/*< mems.h */
extern int			gdSvrSfd;			/*< mmcd_main.h */
extern int			gdStopFlag;			/*< mmcd_main.h */

/**
 *	Declare func.
 */
extern void SetUpSignal();
extern void UserControlledSignal(int sign);
extern void FinishProgram();
extern void IgnoreSignal(int sign);

/**
 *	Implement func.
 */
int dInitIPCs()
{
#if 0
	gdMyQid = msgget( S_MSGQ_MMCD, 0666|IPC_CREAT );
	if( gdMyQid < 0 ) {
		log_print(LOGN_CRI, "MSGGET S_MSGQ_MMCD %s", strerror(errno) );
		return -1;
	}

	gdMngQid = msgget( S_MSGQ_S_MNG, 0666|IPC_CREAT );
	if( gdMngQid < 0 ) {
		log_print(LOGN_CRI, "MSGGET S_MSGQ_S_MNG %s", strerror(errno) );
		return -1;
	}
#endif

	// GIFO를 사용하기 위한 nifo_zone 설정
	gpRECVMEMS = nifo_init_zone((U8*)"MMCD", SEQ_PROC_MMCD, FILE_NIFO_ZONE);
	if( gpRECVMEMS == NULL )
	{
		log_print(LOGN_CRI, "FAILED IN nifo_init_zone, NULL");
		return -1;
	}

	// GIFO를 사용 하기 위한 group 설정
	pCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if( pCIFO == NULL )
	{
		log_print(LOGN_CRI, "FAILED IN gifo_init_group, RET=NULL, cifo=%s, gifo=%s",
					FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -2;
	}

	return 0;
}

void SetUpSignal()
{
    /*
	* WANTED SIGNALS
	*/
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /*
	* UNWANTED SIGNALS
	*/
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);

    return;
}

void UserControlledSignal(int sign)
{
	if( gdSvrSfd > 0 )
		close(gdSvrSfd);

	log_print(LOGN_CRI, "USER CONTROLLED SIGNAL CAPTURED SIGNAL = %d", sign );

	gdStopFlag = 0;
}


void FinishProgram(void)
{
	if(gdSvrSfd > 0)
		close(gdSvrSfd);

	gdStopFlag = 0;
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", gdStopFlag);
	log_print(LOGN_CRI, "NORMALLY TERMINATED\n");
}

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI,"UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);

    signal(sign, IgnoreSignal);
}
