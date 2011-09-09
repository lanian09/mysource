/*******************************************************************************
            DQMS Project

    Author   : Jae Seung Lee
    Section  : SI_SVC
    SCCS ID  : @(#)si_svc_main.c    1.1
    Date     : 01/21/05
    Revision History :
        '05. 01. 21     Initial
        '08. 01. 07     Update By LSH for review
        '08. 01. 14     Add By LSH for IUPS NTAM        

    Description :   

    Copyright (c) uPRESTO 2005
*******************************************************************************/
    
/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

// DQMS
#include "capdef.h"
#include "procid.h"
#include "path.h"
#include "sshmid.h"

// LIB
#include "loglib.h"
#include "ipclib.h"
#include "mems.h"
#include "gifo.h"
#include "cifo.h"

// TOOLS
// TODO st_TraceList 가 common_stg.h 에 있음, 원래는 filter.h 에 있었음.
#include "common_stg.h"
#include "tools.h"

// .
#include "trace_comm.h"
#include "trace_init.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern int					JiSTOPFlag;
extern int					FinishFlag;

extern st_TraceList			*trace_tbl;
extern pst_TraceFileList 	g_pstTraceFileList;

extern st_TraceList			*ServerTrace_tbl;
extern pst_TraceFileList 	g_pstServerTraceFileList;

extern stMEMSINFO			*pMEMSINFO;
extern stCIFO				*pCIFO;
extern stHASHOINFO			*pIRMINFO;
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/
extern int Check_TraceInfo();
extern int Check_ServTraceInfo();


int dInit_Proc(void)
{
	int			i, dRet;

	SetUpSignal();

	// NIFO ZONE 설정
	pMEMSINFO = nifo_init_zone((U8*)"M_TRACE", SEQ_PROC_M_TRACE, FILE_NIFO_ZONE);
	if(pMEMSINFO == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone, NULL", LT);
		return -1;
	}
	
	// CIFO 그룹 설정
	pCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if(pCIFO == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, RET=NULL, cifo=%s, gifo=%s", LT,
					FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -2;
	}


	// 공유메모리 초기화 (S_SSHM_TRACE_INFO)
	dRet = shm_init(S_SSHM_TRACE_INFO, sizeof(st_TraceList), (void **)&trace_tbl);
	if(dRet < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN shm_init(TRACE_INFO=%d)", LT, S_SSHM_TRACE_INFO);
		return -3;
	}
	else if(dRet == SHM_CREATE)
	{
		memset(trace_tbl, 0x00, sizeof(st_TraceList));
		if( (dRet = dGetTraceTblList(FILE_TRACE_TBL, trace_tbl)) < 0)
		{
			log_print(LOGN_CRI, LH"FAILED IN dGetTraceTblList", LT);
		}

		if(dRet == 1)
		{
			log_print(LOGN_CRI, LH"FILE NOT FOUND, file=[%s]", LT, FILE_TRACE_TBL);
		}
		else
		{
			log_print(LOGN_DEBUG, LH" * SUCCESS LOADING FROM FILE(%s)[dInit_IPC]", LT, FILE_TRACE_TBL);
		}
	}
	
	// 공유메모리 초기화 (S_SSHM_SERVTRACE_INFO)
	dRet = shm_init(S_SSHM_SERVTRACE_INFO, sizeof(st_TraceList), (void **)&ServerTrace_tbl);
	if(dRet < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN shm_init(TRACE_INFO=%d)", LT, S_SSHM_SERVTRACE_INFO);
		return -5;
	}
	else if(dRet == SHM_CREATE)
	{
		memset(trace_tbl, 0x00, sizeof(st_TraceList));
		if( (dRet = dGetTraceTblList(FILE_SVRTRACE_TBL, ServerTrace_tbl)) < 0)
		{
			log_print(LOGN_CRI, LH"FAILED IN dGetTraceTblList", LT);
		}

		if(dRet == 1)
		{
			log_print(LOGN_CRI, LH"FILE NOT FOUND, file=[%s]", LT, FILE_SVRTRACE_TBL);
		}
		else
		{
			log_print(LOGN_DEBUG, LH" * SUCCESS LOADING FROM FILE(%s)[dInit_IPC]", LT, FILE_SVRTRACE_TBL);
		}
	}

	// 공유메모리 초기화 (S_SSHM_TRACE_FILE)
	dRet = shm_init(S_SSHM_TRACE_FILE, sizeof(st_TraceFileList), (void **)&g_pstTraceFileList);
	if(dRet < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN shm_init(TRACE_INFO=%d)", LT, S_SSHM_TRACE_FILE);
        return -7;
	}
	else if(dRet == SHM_CREATE)
	{
		InitTraceFile();
	}

	// 공유메모리 초기화 (S_SSHM_SERVTRACE_FILE)
	dRet = shm_init(S_SSHM_SERVTRACE_FILE, sizeof(st_TraceFileList), (void **)&g_pstServerTraceFileList);
	if(dRet < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN shm_init(TRACE_INFO=%d)", LT, S_SSHM_SERVTRACE_FILE);
        return -8;
	}
	else if(dRet == SHM_CREATE)
	{
		InitServTraceFile();
	}

	//
	for ( i=0; i<MAX_TRACE_CNT; i++ )
	{
		if ( g_pstTraceFileList->stTraceFile[i].fp!=NULL )
		{
			g_pstTraceFileList->stTraceFile[i].fp = NULL;
		}

		if ( g_pstServerTraceFileList->stTraceFile[i].fp!=NULL )
		{
			g_pstServerTraceFileList->stTraceFile[i].fp = NULL;
		}
	}

	if((pIRMINFO = hasho_init(0, DEF_IRMHASH_KEY_SIZE, DEF_IRMHASH_KEY_SIZE, DEF_IRMHASH_DATA_SIZE, DEF_IRMHASH_CNT, 0)) == NULL)
	{
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -10;
	}

	log_print(LOGN_INFO, LH"BUFFER SIZE [%ld]", LT, MAX_TRACEBUF);

	return 1;
}

/*******************************************************************************
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

	log_print(LOGN_CRI,
		"SetUpSignal : SIGNAL HANDLER WAS INSTALLED[%d]", JiSTOPFlag);

    return;

} /* end of SetUpSignal */


/*******************************************************************************

*******************************************************************************/
void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    FinishFlag = sign;

	log_print(LOGN_CRI, "UserControlledSignal : [%d]", sign);
	FinishProgram();
} /* end of UserControlledSignal */


/*******************************************************************************

*******************************************************************************/
void FinishProgram()
{
	int i=0;
	for ( i=0; i<MAX_TRACE_CNT; i++ )
	{
		if ( g_pstTraceFileList->stTraceFile[i].fp!=NULL )
		{
			fclose( g_pstTraceFileList->stTraceFile[i].fp );
			g_pstTraceFileList->stTraceFile[i].fp = NULL;
		}

		if ( g_pstServerTraceFileList->stTraceFile[i].fp!=NULL )
		{
			fclose( g_pstServerTraceFileList->stTraceFile[i].fp );
			g_pstServerTraceFileList->stTraceFile[i].fp = NULL;
		}
	}
    log_print(LOGN_CRI,
		"FinishProgram : PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);

    exit(0);
} /* end of FinishProgram */


/*******************************************************************************

*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI,
			"IgnoreSignal : UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);

    signal(sign, IgnoreSignal);
} /* end of IgnoreSignal */

void InitTraceFile()
{
	memset( g_pstTraceFileList, 0, sizeof(st_TraceFileList) );

	Check_TraceInfo();
}

void InitServTraceFile()
{
    memset( g_pstServerTraceFileList, 0, sizeof(st_TraceFileList) );

    Check_ServTraceInfo();
}

