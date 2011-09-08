/**
 *	Include headers
 */
#include <stdlib.h>
#include <errno.h>
#include <sys/msg.h>
#include <signal.h>

// TOP
#include "common_stg.h"
#include "procid.h"
#include "sshmid.h"
#include "commdef.h"
#include "path.h"

// LIB
#include "loglib.h"
#include "ipclib.h"
#include "filelib.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

// TAF headers
#include "mmdblist_ftp.h"

// .
#include "ftp_init.h"


/**
 *	Declare var.
 */
extern stMEMSINFO		*pstMEMSINFO;
extern stCIFO			*gpCIFO;
extern int				gACALLCnt;
extern int				JiSTOPFlag;
extern int				FinishFlag;

/**
 *	Implement func.
 */
/*******************************************************************************
 INIT_IPCS 
*******************************************************************************/
int init_ipcs()
{
	if((pstMEMSINFO = nifo_init_zone((U8*)"A_FTP", SEQ_PROC_A_FTP, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	gACALLCnt = get_block_num(FILE_MC_INIT, "A_CALL");
    log_print(LOGN_INFO, "INIT., A_CALL ProcCount[%d]", gACALLCnt);
	
	return 0;
}


/*******************************************************************************
 USERCONTROLLED SIGNAL 
*******************************************************************************/
void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    FinishFlag = sign;
}


/*******************************************************************************
 IGNORE SIGNAL 
*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print( LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}


/*******************************************************************************
 SETUP SIGNAL 
*******************************************************************************/
void SetUpSignal()
{
    JiSTOPFlag = 1;

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
}


/*******************************************************************************
 dInitProc 
*******************************************************************************/
int dInitProc()
{
	int		dRet;
	time_t	tTime;

	time(&tTime);

	SetUpSignal();

	dRet = init_ipcs();
	if(dRet < 0)
		return -1;

	dRet = dInitSHMMMAB_FTP( S_SSHM_A_FTP );
	if( dRet < 0 ) {
		log_print( LOGN_CRI, "[ERROR] ERROR IN dInitSHMMMAB RET[%d]", dRet );
		return -1;
	}

	log_print( LOGN_CRI, "[dInitProc] [PROCESS INIT SUCCESS]");
	return 0;
}


/*******************************************************************************
 FinishProgram 
*******************************************************************************/
void FinishProgram()
{
    log_print( LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);
    exit(0);
}
