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
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// DQMS
#include "capdef.h"			/* st_beacon_hdr */
#include "procid.h"
#include "path.h"
#include "msgdef.h"			/* MID_FLT_IRM */

// LIB
#include "loglib.h"
#include "sshmid.h"
#include "verlib.h"
#include "mems.h"
#include "gifo.h"
#include "cifo.h"


// .
#include "trace_comm.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
char				szVersion[7] = "R4.0.0";

int 				JiSTOPFlag;
int 				FinishFlag;

int 				dMyQid;

/****** 서버 IP TRACE를 위해 ******/
pst_TraceFileList 	g_pstTraceFileList;
st_TraceList		*trace_tbl;

pst_TraceFileList 	g_pstServerTraceFileList;
st_TraceList		*ServerTrace_tbl;

stMEMSINFO			*pMEMSINFO;
stCIFO				*pCIFO;
stHASHOINFO			*pIRMINFO;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/
extern int dInit_Proc(void);
extern void FinishProgram();
extern void DisTrace(int level);
extern void DisServerTrace(int level);
extern int Check_TraceInfo();
extern S32 dMakeIRMHash();
extern int Check_ServTraceInfo();
extern int dTraceProc(const char* szData);
extern int FlushData(pst_TraceFile pstTraceFile);



int main()
{
	time_t	 			chktime, curtime, writetime;
	int 				dRet;
	int 				i;
	pst_TraceFile		pstTraceFile;
	OFFSET				offset;
	U8					*pDATA, *pNODE;
	NOTIFY_SIG			*pNOTIFY;

	// Initialize log
	dRet = log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_M_TRACE, LOG_PATH"/M_TRACE", "M_TRACE");
	if(dRet < 0)
	{
		log_print(LOGN_WARN, LH"MAIN : Failed in Initialize LOGLIB Info [%d]", LT,  dRet);
		exit(-1);
	}

	// Set version
	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_M_TRACE, szVersion)) < 0)
	{
		log_print(LOGN_WARN, LH"MAIN : Failed in Initialize Version Info [%d]", LT,  dRet);
		exit(-2);
	}

	if((dRet = dInit_Proc()) < 0)
	{
		log_print(LOGN_CRI, "MAIN : FAILED dInit_Proc dRet=%d", dRet);
		exit(-3);
	}

	/* * INITIALUZE MESSAGE QUEUE */
	log_print(LOGN_CRI, "MAIN : INIT PROC SUCCESS START");

	/* 
	 * START MAIN WHILE LOOP 
	*/
	DisTrace( LOG_DEBUG );
	DisServerTrace( LOG_DEBUG );
	Check_TraceInfo();

	curtime = time(0);
	chktime = curtime+3;
	writetime = curtime;

	dMakeIRMHash();

	while(JiSTOPFlag)
	{
		curtime = time(0);
		if ( curtime>=chktime )
		{
			Check_TraceInfo();
			DisTrace( LOG_INFO );

			/**** 서버 트레이스 관련 ****/
			Check_ServTraceInfo();
			DisServerTrace( LOG_INFO );

			chktime=curtime+3;
		}

		if ( writetime < curtime )
		{
			for ( i=0; i<MAX_TRACE_CNT; i++)
			{
				pstTraceFile = &g_pstTraceFileList->stTraceFile[i];

				if ( pstTraceFile->fp!=NULL 
						&& pstTraceFile->dUsedCnt>0 
						&& pstTraceFile->lastuptime+5<curtime)
					FlushData( pstTraceFile );
			}

			/**** 서버레음� 관**/
			for ( i=0; i<MAX_TRACE_CNT; i++)
			{
				pstTraceFile = &g_pstServerTraceFileList->stTraceFile[i];

				if ( pstTraceFile->fp!=NULL
						&& pstTraceFile->dUsedCnt>0
						&& pstTraceFile->lastuptime+5<curtime)
					FlushData( pstTraceFile );
			}

			writetime = curtime;
		}

		for( i=0; i<10000 && JiSTOPFlag; i++ )
		{
			//if((offset = nifo_msg_read(pMEMSINFO, dMyQid, NULL)) > 0) {
			offset = gifo_read(pMEMSINFO, pCIFO, SEQ_PROC_M_TRACE);
			if(offset > 0)
			{
				pDATA = nifo_get_value(pMEMSINFO, st_TraceMsgHdr_DEF_NUM, offset);
				pNODE = nifo_ptr(pMEMSINFO, offset);
				if(pDATA != NULL) {
					dTraceProc(pDATA);
				}
				else {
					pNOTIFY = (NOTIFY_SIG *)nifo_get_value(pMEMSINFO, NOTIFY_SIG_DEF_NUM, offset);
					if(pNOTIFY != NULL) {
						switch(pNOTIFY->uiType)
						{
							case MID_FLT_IRM:
								log_print(LOGN_CRI, "### RCV NOTIFY SIGNAL FLT IRM");
								dMakeIRMHash();
								break;
							default:
								log_print(LOGN_CRI, "[%s][%s.%d] UNKNOWN Type[%d]", __FILE__, __FUNCTION__, __LINE__, pNOTIFY->uiType);
								break;
						}
					}
				}
				nifo_node_delete(pMEMSINFO, pNODE);
			} else {
				usleep(0);
				break;
			}
		}

	} /* while-loop end */

	FinishProgram();

	return 1;
} /* end of main */

