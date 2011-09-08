/**		@file	call_init.c
 * 		- A_CALL 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: call_init.c,v 1.4 2011/09/07 04:22:44 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/07 04:22:44 $
 * 		@warning	nothing
 * 		@ref		call_init.c call_maic.c
 * 		@todo		nothing
 *
 * 		@section	Intro(소개)
 * 		- A_CALL 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>

// LIB
#include "typedef.h"
#include "commdef.h"	/* MAX_MP_NUM, FILE LIST */
#include "utillib.h"
#include "loglib.h"
#include "filelib.h"
#include "ipclib.h"
#include "memg.h"
#include "mems.h"		/* stMEMSINFO */
#include "cifo.h"		/* stCIFO */
#include "gifo.h"		/* gifo_init_group() */
#include "nifo.h"		/* nifo_init_zone() */
#include "hasho.h"
#include "timerN.h"

#include "Analyze_Ext_Abs.h"

// PROEJCT
#include "sshmid.h"	/* S_SSHM_ID */
#include "procid.h"	/* SEQ_PROC_ID */
#include "path.h"	/* LOG_PATH */
#include "filter.h"
#include "common_stg.h"
#include "capdef.h"

// .
#include "call_init.h"
#include "call_func.h"
#include "call_msgq.h"

extern stCIFO	*gpCIFO;

extern S32		giFinishSignal;
extern S32		giStopFlag;
extern S32		guiSeqProcID;

extern st_Flt_Info  *flt_info;

U32				gATCPCnt = 0;
U32				gAINETCnt = 0;
U32				gAITCPCnt = 0;

extern char		gszMyProc[32];

/** dInitCALL function.
 *
 *  dInitCALL Function
 *
 *  @return			S32
 *  @see			call_init.c call_main.c
 *
 **/
S32 dInitCALL(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO)
{
	char 	szProcName[32];
	S32		uiShmCALLKey;
	int 	dLen, ProcNum;

	/* Setup Signal */
	SetUpSignal();

	dLen = strlen(gszMyProc);
	ProcNum = atoi(&gszMyProc[dLen-1]);
	uiShmCALLKey = S_SSHM_A_CALL0 + ProcNum;
	sprintf(szProcName, "A_CALL%d", ProcNum);

	*pMEMSINFO = nifo_init_zone((U8*)szProcName, guiSeqProcID, FILE_NIFO_ZONE);
    if( *pMEMSINFO == NULL ){
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

	log_print(LOGN_CRI, "INIT., PROC[%s] ProcNum[%d]", gszMyProc, ProcNum);

	/* Multiple Process Queue Clear */
	/* A_TCP */
	gATCPCnt = get_block_num(FILE_MC_INIT, "A_TCP");
	log_print(LOGN_CRI, "INIT., A_TCP: ProcCount[%d]", gATCPCnt);

	/* A_INET */
	gAINETCnt = get_block_num(FILE_MC_INIT, "A_INET");
	log_print(LOGN_CRI, "INIT., A_INET: ProcCount[%d]", gAINETCnt);

	/* A_ITCP */
	gAITCPCnt = get_block_num(FILE_MC_INIT, "A_ITCP");
	log_print(LOGN_CRI, "INIT., A_ITCP: ProcCount[%d]", gAITCPCnt);

	if( shm_init(S_SSHM_FLT_INFO, sizeof(st_Flt_Info), (void**)&flt_info) < 0 ){
		log_print(LOGN_CRI,LH"ERROR IN shm_init(FLT_INFO=0x%x)"EH, LT, S_SSHM_FLT_INFO, ET);
		return -3;
	}	

	/* CALL Hash Table 초기화 */
	if((*pHASHOINFO = hasho_init(uiShmCALLKey, TAG_KEY_LOG_COMMON_SIZE, TAG_KEY_LOG_COMMON_SIZE, 
		CALL_DUP_HASH_DATA_SIZE, CALL_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}

	if((*pTIMERNINFO = timerN_init(CALL_SESS_CNT, sizeof(CALL_TIMER_ARG))) == NULL) { 
		log_print(LOGN_CRI, "[%s][%s.%d] timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -6;
	}

	vCALLTimerReConstruct(*pMEMSINFO, *pHASHOINFO, *pTIMERNINFO);

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			call_init.c call_main.c
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
 *  @see			call_init.c call_main.c
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
 *  @see			call_init.c call_main.c
 *
 **/
void FinishProgram(void)
{
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
 *  @see			call_init.c call_main.c
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

/*******************************************************************************

*******************************************************************************/
void vCALLTimerReConstruct(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int					i, Index;
	OFFSET				offset;
	stHASHONODE			*p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	CALL_TIMER_ARG			COMMON;
	TAG_KEY_LOG_COMMON		*pKey;
	CALL_DUP_HASH_DATA		*pData;
	CALL_SESSION_HASH_DATA	*pSessData;
	
	int	dCALLTIMEOUT = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];

	/* IPFRAG */
	log_print(LOGN_INFO, "REBUILD CALL TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (TAG_KEY_LOG_COMMON *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (CALL_DUP_HASH_DATA *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD CALL TIMER CLIENT IP=%d.%d.%d.%d", HIPADDR(pKey->uiClientIP));
				COMMON.ClientIP = pKey->uiClientIP;
				
				for(Index=0; Index<2; Index++) {
					if(pData->CallDupList[Index] != 0) {
						pSessData = (CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pData->CallDupList[Index]);
					} else {
						continue;
					}
					pSessData->timerNID = timerN_add(pTIMER, invoke_del_CALL, (U8 *)&COMMON, sizeof(CALL_TIMER_ARG), time(NULL) + dCALLTIMEOUT);

					pSessData->pMEMSINFO = pMEMSINFO;
					pSessData->pHASHOINFO = pHASH;
					pSessData->pTIMERNINFO = pTIMER;

					if(pSessData->offset_CALL != 0) {
						pSessData->pLOG_CALL_TRANS = (LOG_CALL_TRANS *)nifo_get_value(pMEMSINFO, LOG_CALL_TRANS_DEF_NUM, pSessData->offset_CALL);
					} else {
						pSessData->pLOG_CALL_TRANS = NULL;
					}
					if(pSessData->offset_PAGE != 0) {
						pSessData->pLOG_PAGE_TRANS = (LOG_PAGE_TRANS *)nifo_get_value(pMEMSINFO, LOG_PAGE_TRANS_DEF_NUM, pSessData->offset_PAGE);
					} else {
						pSessData->pLOG_PAGE_TRANS = NULL;
					}
					if(pSessData->offset_DIALUP != 0) {
						pSessData->pLOG_DIALUP_SESS = (LOG_DIALUP_SESS *)nifo_get_value(pMEMSINFO, LOG_DIALUP_SESS_DEF_NUM, pSessData->offset_DIALUP);
					} else {
						pSessData->pLOG_DIALUP_SESS = NULL;
					}
					if(pSessData->aPAGE_DATA.offset_BODY != 0) {
						pSessData->aPAGE_DATA.pBODY = (BODY *)nifo_get_value(pMEMSINFO, BODY_DEF_NUM, pSessData->aPAGE_DATA.offset_BODY);
					} else {
						pSessData->aPAGE_DATA.pBODY = NULL;
					}
					if(pSessData->aPAGE_DATA.offset_InBODY != 0) {
						pSessData->aPAGE_DATA.pInBODY = (BODY *)nifo_get_value(pMEMSINFO, BODY_DEF_NUM, pSessData->aPAGE_DATA.offset_InBODY);
					} else {
						pSessData->aPAGE_DATA.pInBODY = NULL;
					}

					pSessData->func1 = Send_Page_Session_LOG;
				}

			}
			offset = p->offset_next;
		}
	}
}


/*
 * $Log: call_init.c,v $
 * Revision 1.4  2011/09/07 04:22:44  uamyd
 * corrected .. etc
 *
 * Revision 1.3  2011/09/04 12:20:32  dhkim
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/04 08:04:25  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 07:21:31  hhbaek
 * *** empty log message ***
 *
 * Revision 1.3  2011/08/18 04:18:30  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/17 07:15:03  dcham
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.19  2011/05/11 23:11:08  jsyoon
 * *** empty log message ***
 *
 * Revision 1.18  2011/05/09 15:14:06  jsyoon
 * *** empty log message ***
 *
 * Revision 1.17  2011/04/18 15:42:03  jsyoon
 * *** empty log message ***
 *
 * Revision 1.16  2011/01/11 04:09:05  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.15  2010/05/04 12:12:25  dqms
 * *** empty log message ***
 *
 * Revision 1.14  2009/08/06 06:56:09  dqms
 * 로그레벨 공유메모리로 수정
 *
 * Revision 1.13  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.12  2009/08/01 05:40:02  dqms
 * TIMER 함수 호출 할 때 CallTime 비교하여 현재 콜 선택
 *
 * Revision 1.11  2009/07/31 07:38:04  jsyoon
 * vCALLTimerReConstruct 중복콜 구조로 수정
 *
 * Revision 1.10  2009/07/17 09:56:20  jsyoon
 * CALL_STOP_NUM 메세지 처리
 *
 * Revision 1.9  2009/07/16 16:06:19  dqms
 * *** empty log message ***
 *
 * Revision 1.8  2009/07/16 15:38:50  jsyoon
 * A_CALL 재시작 할때 포인터를 복구하도록 offset 추가
 *
 * Revision 1.7  2009/07/16 14:31:41  dqms
 * *** empty log message ***
 *
 * Revision 1.6  2009/07/15 13:17:33  dqms
 * ADD vCALLTimerReConstruct()
 *
 * Revision 1.5  2009/06/29 13:19:09  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/16 15:16:00  jsyoon
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/08 02:46:22  jsyoon
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/05 05:30:16  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:21  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.1  2007/08/21 12:52:37  dark264sh
 * no message
 *
 * Revision 1.7  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.6  2006/11/07 08:55:24  shlee
 * hasho_init function
 *
 * Revision 1.5  2006/10/20 10:00:53  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2006/10/18 08:53:31  dark264sh
 * nifo debug 코드 추가
 *
 * Revision 1.3  2006/10/12 08:26:25  cjlee
 * *** empty log message ***
 *
 * Revision 1.2  2006/10/11 11:52:33  dark264sh
 * PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 * Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 * no message
 *
 * Revision 1.3  2006/10/09 08:53:36  cjlee
 * *** empty log message ***
 *
 * Revision 1.2  2006/10/09 02:40:14  dark264sh
 * CALL MSGQID 변경
 *
 * Revision 1.1  2006/10/09 01:55:58  dark264sh
 * INIT
 *
 */
