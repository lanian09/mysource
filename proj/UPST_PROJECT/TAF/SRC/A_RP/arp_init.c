/**
 * Include headers
 */
#include <signal.h>
#include <unistd.h>

// TOP
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "filter.h"

// LIB headers
#include "common_stg.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "ipclib.h"
#include "typedef.h"

// TAF headers
#include "debug.h"
#include "arp_head.h"
#include "mmdb_psess.h"
#include "mmdb_greentry.h"

// .
#include "arp_init.h"

/**
 * Delcare variables
 */
stMEMSINFO      		*pMEMSINFO;
stCIFO					*gpCIFO;
stHASHOINFO				*pCALLHASH;
DEBUG_INFO      		*pDEBUGINFO;
st_TraceList    		*pstTRACE;

extern st_Flt_Info		*flt_info;

PSESS_DATA              g_FirstPSessData, g_LastPSessData;

extern S32				giStopFlag;         /**< main loop Flag 0: Stop, 1: Loop */
extern S32				FinishFlag;

extern int				gAGRECnt;
extern char				gszMyProc[32];
extern int				PROCNO;
extern UINT				guiSeqProcID;

extern S32				gSemID;

/**
 *	Implement func.
 */
int dInit_Proc()
{
	int		i, dRet;
	int		dAGRECnt;
	U32     uiMsgQKey;

	char	szProcName[32];


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

	
    if( (dRet = shm_init(S_SSHM_UTIL, sizeof(DEBUG_INFO), (void**)&pDEBUGINFO)) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(DEBUG_INFO=%d)", S_SSHM_UTIL);
		return -3;
    }

    if( (dRet = Init_A11_PSESS()) < 0 ) {
        log_print( LOGN_INFO, "[%s.%d] ERROR IN Init_A11_PSESS dRet:%d", __FUNCTION__, __LINE__, dRet );
        return -4;
    }
    else if( dRet == 1 ) {
        /* CREATE */
        Init_PSESS();

		/* ADD INIT */
        if( (dRet = Insert_PSESS( &g_FirstPSessData )) < 0 ) {
            log_print( LOGN_CRI, "[%s.%d] ERROR FIRST SESS Insert_PSESS dRet:%d",
                              __FUNCTION__, __LINE__, dRet );
			return -5;
		}

        if( (dRet = Insert_PSESS( &g_LastPSessData )) < 0 ) {
			log_print( LOGN_CRI, "[%s.%d] ERROR LAST SESS Insert_PSESS dRet:%d",
                              __FUNCTION__, __LINE__, dRet );
            return -6;
        }
		
    }
	
	/* SEMA FOR CALL */
	gSemID = mems_sem_init(S_SEMA_CALL, MEMS_SEMA_ON);

	if( (pCALLHASH = hasho_init( S_SSHM_A11CALL, DEF_CALLHASHKEY_SIZE, DEF_CALLHASHKEY_SIZE, DEF_CALLHASHDATA_SIZE, DEF_CALLHASH_COUNT, 0 )) == NULL ) {
		log_print( LOGN_CRI, "[%s.%d] ERROR IN hasho_init", __FUNCTION__, __LINE__ );
		return -7;
	}

    if( (dRet = Init_TraceShm()) < 0) {
        log_print( LOGN_INFO, "[%s.%d] ERROR IN Init_TraceShm dRet:%d", __FUNCTION__, __LINE__, dRet );
        return -8;
    }

    if( (dRet = Init_GREEntry_Shm()) < 0) {
        log_print( LOGN_INFO, "[%s.%d] ERROR IN Init_GREEntry_Shm dRet:%d", __FUNCTION__, __LINE__, dRet );
        return -9;
    }

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
        return -10;
    }

	return 1;
}

/*******************************************************************************

*******************************************************************************/
void UserControlledSignal(int sign)
{
    giStopFlag = 0;
    FinishFlag = sign;
}


/*******************************************************************************

*******************************************************************************/
void FinishProgram()
{
    log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);
    exit(0);
}


/*******************************************************************************

*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}


/*******************************************************************************

*******************************************************************************/
void SetUpSignal()
{
    giStopFlag = 1;

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

    log_print(LOGN_CRI, "SIGNAL HANDLER WAS INSTALLED");
}


/*******************************************************************************

*******************************************************************************/
int Init_A11_PSESS( )
{
	UINT	uiShmSESSKey;

	uiShmSESSKey = S_SSHM_A11_PSESS0 + PROCNO;

	log_print( LOGN_CRI, "INIT A11 SHM KEY:%u PROCNO:%u", uiShmSESSKey, PROCNO );

	if( shm_init(uiShmSESSKey, sizeof(SESS_TABLE), (void**)&psess_tbl) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(A11 SHM Key=%d)", uiShmSESSKey);
        return -1;
    }

    return 1;
}

/*******************************************************************************

*******************************************************************************/
int Init_TraceShm( )
{
	if( shm_init(S_SSHM_TRACE_INFO, st_TraceList_SIZE, (void**)&pstTRACE) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(TRACE_INFO=%d)", S_SSHM_TRACE_INFO);
        return -1;
    }

    return 0;
}

int Init_GREEntry_Shm(void)
{
	UINT    uiShmSESSKey;

	uiShmSESSKey = S_SSHM_A11_GREENTRY0 + PROCNO;

	log_print( LOGN_CRI, "INIT A11_GREENTRY SHM KEY:%u PROCNO:%u", uiShmSESSKey, PROCNO );

	if( shm_init(uiShmSESSKey, sizeof(GREENTRY_TABLE), (void**)&greentry_tbl) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(A11 SHM Key=%d)", uiShmSESSKey);
        return -1;
    }
	return 1;
}

