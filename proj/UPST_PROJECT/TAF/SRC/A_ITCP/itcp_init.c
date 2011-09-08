/**		@file	tcp_init.c
 * 		- A_TCP 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: itcp_init.c,v 1.3 2011/09/05 12:26:40 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/05 12:26:40 $
 * 		@ref		tcp_init.c
 *
 * 		@section	Intro(소개)
 * 		- A_TCP 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>

// DQMS headers
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "path.h"
#include "filter.h"

// LIB
#include "memg.h"		/* MEMG_ID, MEMG_ALLOCED */
#include "cifo.h"       /* stCIFO */
#include "gifo.h"       /* gifo_init_group() */
#include "nifo.h"       /* nifo_init_zone() */
#include "loglib.h"
#include "filelib.h"
#include "ipclib.h"

// TAF headers
#include "func_time_check.h"

// .
#include "itcp_sess.h"
#include "itcp_init.h"

/**
 * Declare variables
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;
extern UINT			guiSeqProcID;

extern st_FuncTimeCheckList	*pFUNC;

extern SESSKEY_TBL 	*pstSESSKEYTBL;
extern stHASHOINFO 	*pSESSKEYINFO;
extern int			guiTimerValue;

extern st_Flt_Info 	*flt_info;

extern char 		gszMyProc[32];

int 				gAHTTPCnt = 0;
int 				gACALLCnt = 0;

/**
 *	Declare external func.
 */
extern void invoke_del(void *p);

/**
 *	Implement func.
 */
int Init_TCPKeyShm( int ProcNum )
{
    int     shm_id;
	UINT 	uiShmSESSKey;

	uiShmSESSKey = S_SSHM_ITCP_LIST0 + ProcNum;

	log_print( LOGN_CRI, "INIT TCP CALL SHM KEY:%u PROCNO:%d", uiShmSESSKey, ProcNum);

	if( shm_init( uiShmSESSKey, SESSKEY_TBL_SIZE, (void**)pstSESSKEYTBL ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(ITCP_LIST=%d)", LT, uiShmSESSKey);
		return -1;
	}

    return 0;
}

/** dInitTcp function.
 *
 *  dInitTcp Function
 *
 *  @return			S32
 *  @see			tcp_init.c tcp_main.c tcp_api.h
 *
 **/
S32 dInitTcp(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO)
{
	int				dRet;
	int 			i, dLen, ProcNum;
	int 			dAHTTPCnt, dACALLCnt;
	UINT 			uiShmTCPKey, uiShmSESSKey;
	char 			szMsgQName[32];

	SetUpSignal();

	dLen = strlen(gszMyProc);
	ProcNum = atoi(&gszMyProc[dLen-1]);
	uiShmTCPKey = S_SSHM_A_ITCP0 + ProcNum;
	uiShmSESSKey = S_SSHM_ISESS_KEY0 + ProcNum;

	if((*pMEMSINFO = nifo_init_zone((U8*)gszMyProc, guiSeqProcID, FILE_NIFO_ZONE)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }


    log_print(LOGN_INFO, "INIT., PROC[%s] SeqProcID[%d]", gszMyProc, guiSeqProcID);

    /* Multiple Process : YOON.20090616 */
    gAHTTPCnt = get_block_num(FILE_MC_INIT, "A_IHTTP");
    log_print(LOGN_INFO, "INIT., A_HTTP ProcCount[%d]", gAHTTPCnt);

    gACALLCnt = get_block_num(FILE_MC_INIT, "A_CALL");
    log_print(LOGN_INFO, "INIT., A_CALL ProcCount[%d]", gACALLCnt);


	/* TCP Hash Table 초기화 */
	/* TCP 세션 관리를 위한 구조체를 만들어야 함 */
	if((*pHASHOINFO = hasho_init( uiShmTCPKey, TCP_SESS_KEY_SIZE, TCP_SESS_KEY_SIZE, 
					TCP_SESS_SIZE, ITCP_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -3;
	}

	if((*pTIMERNINFO = timerN_init(ITCP_SESS_CNT, sizeof(TCP_COMMON))) == NULL) {
		log_print(LOGN_CRI, LH"timerN_init NULL", LT);
		return -4;
	}

	vTCPTimerReConstruct(*pHASHOINFO, *pTIMERNINFO);
	
	/* TCP SESSION CLEAR */
	if((pSESSKEYINFO = hasho_init( uiShmSESSKey, STOP_CALL_KEY_SIZE, STOP_CALL_KEY_SIZE, 
									SESSKEY_LIST_SIZE, CALL_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -5;
	}

	if( (dRet = Init_TCPKeyShm(ProcNum) ) < 0 ) {
		log_print( LOGN_INFO, LH"ERROR IN Init_TCPKeyShm() dRet:%d", LT, dRet );
		return -6;
	} else if (dRet == 0) {
		InitTCPKeyList();
	}

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
        return -7;
    }

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c tcp_api.h
 *
 **/
void SetUpSignal(void)
{
	giStopFlag = 1;

	/* WANTED SIGNALS */
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT, UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP, IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, IgnoreSignal);
}

/** UserControlledSignal function.
 *
 *  UserControlledSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c tcp_api.h
 *
 **/
void UserControlledSignal(S32 isign)
{
	giStopFlag = 0;
	giFinishSignal = isign;
	log_print(LOGN_CRI, "User Controlled Signal Req = %d", isign);
}

/** FinishProgram function.
 *
 *  FinishProgram Function
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c
 *
 **/
void FinishProgram(void)
{
PRINT_FUNC_TIME_CHECK(pFUNC);
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
	exit(0);
}

/** IgnoreSignal function.
 *
 *  IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c tcp_api.h
 *
 **/
void IgnoreSignal(S32 isign)
{
	if (isign != SIGALRM)
	{
		log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);
	}

	signal(isign, IgnoreSignal);
}

/** vTCPTimerReConstruct function.
 *
 *  vTCPTimerReConstruct Function
 *
 *	@param	*pHASH	:	stHASHOINFO
 *	@param	*pTIMER	:	stTIMERNINFO
 *
 *  @return			void
 *  @see			tcp_init.c tcp_main.c tcp_api.h
 *
 **/
void vTCPTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int					i;
	OFFSET				offset;
	stHASHONODE			*p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	TCP_COMMON			COMMON;
	TCP_SESS_KEY		*pKey;
	TCP_SESS			*pData;

	U8					szSIP[INET_ADDRSTRLEN];
	U8					szDIP[INET_ADDRSTRLEN];

	/* TCP */
	log_print(LOGN_INFO, "REBUILD TCP TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (TCP_SESS_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (TCP_SESS *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD TCP TIMER SIP=%s:%u SPORT=%u DIP=%s.%u DPORT=%u", 
						util_cvtipaddr(szSIP, pKey->uiSIP), pKey->uiSIP, pKey->usSPort,
						util_cvtipaddr(szDIP, pKey->uiDIP), pKey->uiDIP, pKey->usDPort);

				memcpy(&COMMON.TCPSESSKEY, pKey, TCP_SESS_KEY_SIZE);
				guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_TCP_TIMEOUT];
				pData->timerNID = timerN_add(pTIMER, invoke_del, (U8*)&COMMON, TCP_SESS_KEY_SIZE, time(NULL) + guiTimerValue);

			}
			offset = p->offset_next;
		}
	}
}

/*
 * $Log: itcp_init.c,v $
 * Revision 1.3  2011/09/05 12:26:40  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 05:32:59  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/17 13:02:39  hhbaek
 * A_ITCP
 *
 * Revision 1.3  2011/08/11 04:38:30  uamyd
 * modified and block added
 *
 * Revision 1.2  2011/08/10 09:57:44  uamyd
 * modified and block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.2  2011/05/09 09:58:03  dark264sh
 * A_ITCP: A_CALL multi 처리
 *
 * Revision 1.1  2011/04/12 02:51:50  dark264sh
 * A_ITCP 추가
 *
 */
