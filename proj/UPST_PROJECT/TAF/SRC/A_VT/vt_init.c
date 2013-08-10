/*******************************************************************************
 *		@file	call_init.c
 * 		- A_CALL 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: vt_init.c,v 1.3 2011/09/06 12:46:40 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/06 12:46:40 $
 * 		@warning	nothing
 * 		@ref		call_init.c call_maic.c
 * 		@todo		nothing
 *
 * 		@section	Intro(소개)
 * 		- A_CALL 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
*******************************************************************************/

/**
 *	Include headers
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>

// TOP
#include "typedef.h"
#include "common_stg.h"
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "filter.h"

// LIB headers
#include "loglib.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "ipclib.h"

// .
#include "vt_init.h"

/**
 *	Declare var.
 */
int         		gAINETCnt = 0;
int         		gACALLCnt = 0;

extern S32			giFinishSignal;
extern S32			giStopFlag;
extern U64			nifo_create;
extern U64			nifo_del;
extern st_Flt_Info	*flt_info;
extern int			guiTimerValue;
extern stCIFO		*gpCIFO;

/**
 *	Declare extern func.
 */
extern void invoke_del_CALL(void *p);		/* vt_main.c */

/**
 *	Implement func.
 */

/** dInitVT function.
 *
 *  dInitVT Function
 *
 *  @return			S32
 *
 **/
S32 dInitVT(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO)
{
	/* Setup Signal */
	SetUpSignal();

	*pMEMSINFO = nifo_init_zone((U8*)"A_VT", SEQ_PROC_A_VT, FILE_NIFO_ZONE);
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

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
        return -3;
    }

	/* VT Hash Table 초기화 */
	if((*pHASHOINFO = hasho_init(S_SSHM_A_VT, TAG_KEY_LOG_COMMON_SIZE, TAG_KEY_LOG_COMMON_SIZE, 
		CALL_SESSION_HASH_DATA_SIZE, VT_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -5;
	}

	if((*pTIMERNINFO = timerN_init(VT_SESS_CNT, sizeof(st_CALLTimer))) == NULL) { 
		log_print(LOGN_CRI, LH"timerN_init NULL", LT);
		return -6;
	}

	vVTTimerReConstruct(*pHASHOINFO, *pTIMERNINFO);

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
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
 *
 **/
void FinishProgram(void)
{
#ifdef MEM_TEST
    log_print(LOGN_CRI, "CREATE CNT[%llu] DELETE CNT[%llu]", nifo_create, nifo_del);
#endif
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

/** vIPFRAGTimerReConstruct function.
 *
 *  vIPFRAGTimerReConstruct Function
 *
 *	@param	*pHASH	:	stHASHOINFO
 *	@param	*pTIMER	:	stTIMERNINFO
 *
 *  @return			void
 *
 **/
void vVTTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int					i;
	OFFSET				offset;
	stHASHONODE			*p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	st_CALLTimer			COMMON;
	st_SIPSessKey			*pKey;
	VT_SESSION_HASH_DATA	*pData;

	/* IPFRAG */
	log_print(LOGN_INFO, "REBUILD VT TIMER hashcnt=%u", pHASH->uiHashSize);
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_VT_TIMEOUT];
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (st_SIPSessKey *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (VT_SESSION_HASH_DATA *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD VT TIMER IP=%d.%d.%d.%d:%u", 
						HIPADDR(pKey->uiClientIP), pKey->uiClientIP);

				COMMON.ClientIP = pKey->uiClientIP;
				pData->timerNID = timerN_add(pTIMER, invoke_del_CALL, (U8*)&COMMON, sizeof(st_CALLTimer), time(NULL) + guiTimerValue);

			}
			offset = p->offset_next;
		}
	}
}
/*
 * $Log: vt_init.c,v $
 * Revision 1.3  2011/09/06 12:46:40  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 01:35:33  uamyd
 * modified to runnable source
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/11 08:09:11  hhbaek
 * Commit A_VT
 *
 * Revision 1.3  2011/08/09 09:06:42  hhbaek
 * Commit A_VT
 *
 * Revision 1.2  2011/08/09 05:20:01  hhbaek
 * Commit A_VT service block
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.13  2011/05/09 13:01:43  dark264sh
 * A_VT: A_CALL multi 처리
 *
 * Revision 1.12  2011/05/06 04:08:15  jsyoon
 * *** empty log message ***
 *
 * Revision 1.11  2011/01/11 04:09:10  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.10  2009/08/22 18:37:09  jsyoon
 * INVITE 외의 MSG는 SIPD -> A_VT -> A_IM
 *
 * Revision 1.9  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.8  2009/07/15 15:59:44  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2009/07/15 15:39:31  dark264sh
 * A_VT vVTTimerReConstruct 추가
 *
 * Revision 1.6  2009/07/05 15:39:40  dqms
 * *** empty log message ***
 *
 * Revision 1.5  2009/06/29 13:26:43  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/15 09:04:57  jsyoon
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/12 14:34:12  jsyoon
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/11 19:27:20  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1  2009/06/10 21:46:09  jsyoon
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
