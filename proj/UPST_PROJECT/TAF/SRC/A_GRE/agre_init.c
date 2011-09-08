/**
 * Include headers
 */
#include <signal.h>

// DQMS headers
#include "commdef.h"
#include "procid.h"
#include "path.h"

// LIB headers
#include "common_stg.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"

// TAF headers
#include "mmdb_psess.h"		/* PSESS_TABLE */

// .
#include "agre_init.h"

/**
 * Declare variables
 */
extern int              JiSTOPFlag;
extern int              FinishFlag;
extern int				gAGRECnt;

extern char             gszMyProc[32];
extern UINT             guiSeqProcID;

extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*gpCIFO;

/**
 *	Implement func.
 */
int dInit_Proc()
{
	int		dRet;

	if((pMEMSINFO = nifo_init_zone((U8*)gszMyProc, guiSeqProcID, FILE_NIFO_ZONE)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	gAGRECnt = get_block_num(FILE_MC_INIT, "A_GRE" );
	log_print(LOGN_INFO, "INIT., A_GRE ProcCount[%d]", gAGRECnt);

	dRet = Init_A11_PSESS();
	if( dRet < 0 ) {
		log_print( LOGN_INFO, "[%s.%d] Init_A11_PSESS dRet:%d", __FUNCTION__, __LINE__, dRet );
			return -5;
	}

	dRet = Init_TraceShm();
	if( dRet < 0 ) {
        log_print( LOGN_INFO, "[%s.%d] Init_TraceShm dRet:%d", __FUNCTION__, __LINE__, dRet );
            return -6;
    }

	dRet = Init_GREEntry_Shm();
	if( dRet < 0 ) {
		log_print( LOGN_INFO, "[%s.%d] Init_GREEntry_Sess dRet:%d", __FUNCTION__, __LINE__, dRet );
			return -5;
	}


	return 1;
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
