/**		@file	tcp_init.c
 * 		- A_TCP 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: tcp_init.c,v 1.5 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.5 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
 * 		@ref		tcp_init.c
 *
 * 		@section	Intro(소개)
 * 		- A_TCP 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

/* SYS HEADER */
#include <stdio.h>
#include <errno.h>
#include <signal.h>
/* LIB HEADER */
#include "cifo.h"		/* stCIFO */
#include "gifo.h"		/* gifo_init_group() */
#include "nifo.h"		/* nifo_init_zone() */
#include "commdef.h"
#include "loglib.h"
#include "utillib.h"
#include "filelib.h"
#include "ipclib.h"
#include "memg.h" 		/* MEMG_ID, MEMG_ALLOCED */
#include "hasho.h"
#include "timerN.h"
/* PRO HEADER */
#include "common_stg.h"
#include "procid.h"
#include "sshmid.h"
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
#include "filter.h"
#include "func_time_check.h"
/* OAM HEADER */
/* LOC HEADER */
#include "tcp_sess.h"
#include "tcp_util.h"
#include "tcp_func.h"
#include "tcp_init.h"

extern stCIFO	    *gpCIFO;

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


int Init_TCPKeyShm( int ProcNum )
{
	UINT 	uiShmSESSKey;

	uiShmSESSKey = S_SSHM_TCP_LIST0 + ProcNum;

	log_print( LOGN_CRI, "INIT TCP CALL SHM KEY:%u PROCNO:%d", uiShmSESSKey, ProcNum);

	if( shm_init(uiShmSESSKey, SESSKEY_TBL_SIZE, (void**)&pstSESSKEYTBL) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN shm_init(TCP_LIST=%d)", LT, uiShmSESSKey);
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
	int		dRet, dLen, ProcNum;
	UINT 	uiShmTCPKey, uiShmSESSKey;

	SetUpSignal();

	dLen = strlen(gszMyProc);
	ProcNum = atoi(&gszMyProc[dLen-1]);
	uiShmTCPKey = S_SSHM_A_TCP0 + ProcNum;
	uiShmSESSKey = S_SSHM_SESS_KEY0 + ProcNum;


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
	gAHTTPCnt = get_block_num(FILE_MC_INIT, "A_HTTP");
	log_print(LOGN_INFO, "INIT., A_HTTP ProcCount[%d]", gAHTTPCnt);

	gACALLCnt = get_block_num(FILE_MC_INIT, "A_CALL");
	log_print(LOGN_INFO, "INIT., A_CALL ProcCount[%d]", gACALLCnt);

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
        return -3;
    }


	/* TCP Hash Table 초기화 */
	/* TCP 세션 관리를 위한 구조체를 만들어야 함 */
	if((*pHASHOINFO = hasho_init( uiShmTCPKey, TCP_SESS_KEY_SIZE, TCP_SESS_KEY_SIZE, 
					TCP_SESS_SIZE, TCP_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -4;
	}

	if((*pTIMERNINFO = timerN_init(TCP_SESS_CNT, sizeof(TCP_COMMON))) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}

	vTCPTimerReConstruct(*pHASHOINFO, *pTIMERNINFO);
	
	/* TCP SESSION CLEAR */
	if((pSESSKEYINFO = hasho_init( uiShmSESSKey, STOP_CALL_KEY_SIZE, STOP_CALL_KEY_SIZE, 
									SESSKEY_LIST_SIZE, CALL_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -6;
	}

	if( (dRet = Init_TCPKeyShm(ProcNum) ) < 0 ) {
		log_print( LOGN_INFO, "[%s.%d] ERROR IN Init_TCPKeyShm() dRet:%d", __FUNCTION__, __LINE__, dRet );
		return -7;
	} else if (dRet == 0) {
		InitTCPKeyList();
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
 * $Log: tcp_init.c,v $
 * Revision 1.5  2011/09/07 06:30:48  hhbaek
 * *** empty log message ***
 *
 * Revision 1.4  2011/09/07 04:59:33  uamyd
 * modified
 *
 * Revision 1.3  2011/09/06 06:44:52  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 01:35:33  uamyd
 * modified to runnable source
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.7  2011/08/21 09:07:53  hhbaek
 * Commit TAF/SRC/ *
 *
 * Revision 1.6  2011/08/17 07:25:47  dcham
 * *** empty log message ***
 *
 * Revision 1.5  2011/08/11 04:38:31  uamyd
 * modified and block added
 *
 * Revision 1.4  2011/08/10 09:57:44  uamyd
 * modified and block added
 *
 * Revision 1.3  2011/08/09 05:31:11  uamyd
 * modified
 *
 * Revision 1.2  2011/08/05 09:04:50  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.18  2011/05/09 15:03:35  dark264sh
 * A_TCP: A_CALL multi 처리
 *
 * Revision 1.17  2011/01/11 04:09:10  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.16  2009/08/17 18:27:47  jsyoon
 * *** empty log message ***
 *
 * Revision 1.15  2009/08/17 16:05:06  jsyoon
 * 세션관리 메모리를 공유메모리로 변경
 *
 * Revision 1.14  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.13  2009/08/01 08:46:11  dqms
 * *** empty log message ***
 *
 * Revision 1.12  2009/07/29 12:42:11  jsyoon
 * TCP_STOP pTCPINFO->uiCapTime = pTCPSESS->uiLastUpdateTime
 *
 * Revision 1.11  2009/07/29 07:02:34  dqms
 * *** empty log message ***
 *
 * Revision 1.10  2009/07/22 06:25:21  dqms
 * *** empty log message ***
 *
 * Revision 1.9  2009/07/15 16:18:32  dark264sh
 * A_TCP vTCPTimerReConstruct 추가
 *
 * Revision 1.8  2009/07/15 16:11:13  dqms
 * 멀티프로세스 수정
 *
 * Revision 1.7  2009/06/29 13:26:10  dark264sh
 * *** empty log message ***
 *
 * Revision 1.6  2009/06/19 08:54:48  jsyoon
 * MULTIPLE HASHKEY, LOG
 *
 * Revision 1.5  2009/06/16 11:58:36  dqms
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/16 06:11:07  dqms
 * Support Multiple Process
 *
 * Revision 1.3  2009/06/15 08:45:42  jsyoon
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/08 18:54:42  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:33  dqms
 * Init TAF_RPPI
 *
 * Revision 1.4  2008/11/24 07:05:47  dark264sh
 * WIPI ONLINE 처리
 *
 * Revision 1.3  2008/09/18 07:47:39  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.2  2008/06/18 10:39:34  dark264sh
 * A_TCP IV관련 routing 처리
 *
 * Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.2  2007/09/03 05:29:18  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/08/21 12:54:17  dark264sh
 * no message
 *
 * Revision 1.8  2006/12/01 09:37:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.6  2006/11/07 08:43:48  shlee
 * hasho_init function
 *
 * Revision 1.5  2006/11/03 08:30:02  dark264sh
 * A_TCP에 func_time_check 추가
 *
 * Revision 1.4  2006/10/20 10:02:43  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2006/10/18 08:53:31  dark264sh
 * nifo debug 코드 추가
 *
 * Revision 1.2  2006/10/11 11:52:33  dark264sh
 * PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 * Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 * no message
 *
 * Revision 1.13  2006/10/09 02:40:14  dark264sh
 * CALL MSGQID 변경
 *
 * Revision 1.12  2006/09/15 02:53:44  dark264sh
 * LOG 데이터를 HTTP가 아닌 PAGE로 보내도록 수정
 * nifo_msg_read에서 현재는 자연스러운 처리가 돼지 않음.
 * 향후 nifo_msg_read를 수정해서 처리해야 할꺼 같음.
 *
 * Revision 1.11  2006/09/13 04:30:25  dark264sh
 * strerror 잘못 찍는 부분 수정
 *
 * Revision 1.10  2006/09/06 11:43:28  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2006/08/24 04:08:06  dark264sh
 * HTTP 기본 Flow 구성
 *
 * Revision 1.8  2006/08/21 10:58:20  dark264sh
 * dInitTcp 함수의 파라미터 변경
 *
 * Revision 1.7  2006/08/21 10:56:26  dark264sh
 * dInitTcp 함수의 파라미터 변경
 *
 * Revision 1.6  2006/08/21 10:47:28  dark264sh
 * timerN_init 에러 핸들링 추가
 *
 * Revision 1.5  2006/08/21 07:29:51  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2006/08/21 05:44:43  dark264sh
 * no message
 *
 * Revision 1.3  2006/08/21 05:23:01  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2006/08/21 03:07:38  dark264sh
 * no message
 *
 * Revision 1.1  2006/08/18 10:17:39  dark264sh
 * TCP INIT
 *
 */
