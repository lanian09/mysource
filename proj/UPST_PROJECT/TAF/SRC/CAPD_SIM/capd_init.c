/*******************************************************************************
                DQMS Project

   Author   : Lee Dong-Hwan
   Section  : CAPD 
   SCCS ID  : @(#)capd_init.c (V1.0)
   Date     : 07/02/09
   Revision History :
        '09.    07. 02. initial

   Description :

   Copyright (c) uPRESTO 2005
*******************************************************************************/

/** A. FILE INCLUSION *********************************************************/
#include "capd_init.h"

/** B. DEFINITION OF NEW CONSTANTS ********************************************/
/** C. DEFINITION OF NEW TYPES ************************************************/
/** D. DECLARATION OF VARIABLES ***********************************************/
/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
/** E.2 DEFINITION OF FUNCTIONS ***********************************************/
/*******************************************************************************

*******************************************************************************/
S32 dInitCapd(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO)
{
	int	dRet;			

	// Initiate log
	log_init(S_SSHM_LOG_LEVEL, getpid(), guiSeqProcID, LOG_PATH"/%s", gszMyProc);

	// Set signal
	SetUpSignal();

	// NIFO ZONE
	if((*pMEMSINFO = nifo_init_zone((U8*)gszMyProc, guiSeqProcID, FILE_NIFO_ZONE)) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
		return -1;
	}

	// NIFO GROUP
	if((*pCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL )
	{
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
			LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -2;
	}

	/* FIDB 초기화 부분 삭제
	// Initiate FIDB
	dRet = dInit_FIDB();
	if(dRet < 0) 
	{
		log_print(LOGN_INFO, LH"ERROR IN dInit_FIDB dRet:%d", LT, dRet);
		exit(0);
	}
	*/

	// Initiate port status
	memset( &stPortStatus[0], 0x00, DEF_PORTSTATUS_SIZE*2 );
	fidb->mirrorsts[0] = CRITICAL;
	fidb->mirrorsts[1] = CRITICAL;
	fidb->mirrorActsts[0] = DEF_ACTIVE;
	fidb->mirrorActsts[1] = DEF_STANDBY;

	// Set version
    //if((dRet = set_version(SEQ_PROC_CAPD, vERSION)) < 0 )
	if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, gszVersion)) < 0)
	{
        log_print(LOGN_CRI, LH"SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", LT, dRet,
					SEQ_PROC_CAPD, gszVersion);
    }

	log_print(LOGN_CRI, "MIRR STS:%d,%d ACT STS:%d,%d DAGREC_LEN[%d]",
				fidb->mirrorsts[0], fidb->mirrorsts[1], fidb->mirrorActsts[0],
				fidb->mirrorActsts[1], sizeof(dag_record_t));

	return 0;
}

int dInit_FIDB()
{
	int			dShm;

	if( (dShm = shmget( S_SSHM_FIDB, sizeof(st_NTAF), 0666|IPC_CREAT|IPC_EXCL )) < 0 ) {
		if( errno == EEXIST ) {
			if( (dShm = shmget( S_SSHM_FIDB, sizeof(st_NTAF), 0666|IPC_CREAT )) < 0 ) {
				log_print( LOGN_CRI, "ERROR SHMGET %s.%d dShm:%d", __FUNCTION__, __LINE__, dShm );
				return -1;
			}

			if( (fidb = (st_NTAF *)shmat( dShm, (char *)0, 0 )) == (st_NTAF *)-1 ) {
				log_print( LOGN_CRI, "ERROR SHMAT %s.%d", __FUNCTION__, __LINE__ );
				return -2;
			}
		}
		else
			return -3;
	}
	else {
		if( (fidb = (st_NTAF *)shmat( dShm, (char *)0, 0 )) == (st_NTAF *)-1 ) {
			log_print( LOGN_CRI, "ERROR SHMAT %s.%d", __FUNCTION__, __LINE__ );
            return -4;
		}
	}

	return 0;
} 
	

/*******************************************************************************
 USERCONTROLLED SIGNAL
*******************************************************************************/
void UserControlledSignal(int sign)
{
    loop_continue = 0;
    gdFinishFlag = sign;
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
    loop_continue = 1;

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
 FinishProgram
*******************************************************************************/
void FinishProgram()
{
    log_print( LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", gdFinishFlag);
    exit(0);
}
