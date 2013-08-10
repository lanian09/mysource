/**		@file	sipt_init.c
 * 		- A_SIPT 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: sipt_init.c,v 1.3 2011/09/06 12:46:38 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/06 12:46:38 $
 * 		@ref		sipt_init.c sipt_maic.c
 *
 * 		@section	Intro(소개)
 * 		- A_SIPT 프로세스를 초기화 하는 함수들
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
#include <sys/ipc.h>
#include <sys/shm.h>

// TOP
#include "common_stg.h"
#include "path.h"
#include "commdef.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "filter.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "ipclib.h"
#include "utillib.h"
#include "memg.h"

// INC
#include "sipt_sess.h"
#include "sipt_init.h"

/**
 * Declare variables
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

extern st_Flt_Info	*flt_info;
extern int			guiTimerValue;

extern SESSKEY_TBL 	*pstSESSKEYTBL;
extern stHASHOINFO	*pSESSKEYINFO;

/**
 *	Declare external func.
 */
extern void invoke_del(void *p);

/**
 *	Impl func.
 */
int Init_SIPTKeyShm( )
{
	log_print( LOGN_CRI, "INIT SIPT KEY SHM KEY:%u", S_SSHM_SIPT_LIST);
	if( shm_init(S_SSHM_SIPT_LIST, SESSKEY_TBL_SIZE, (void**)&pstSESSKEYTBL) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(SIPT_LIST=%d)", S_SSHM_SIPT_LIST);
        return -1;
    }
    return 0;
}

/** dInitMEKUN function.
 *
 *  dInitMEKUN Function
 *
 *  @return			S32
 *  @see			sipt_init.c sipt_main.c
 *
 **/
S32 dInitSIPT(stMEMSINFO **pMEMSINFO, stHASHOINFO **pSIPHASH, stTIMERNINFO **pTIMERNINFO)
{
	int 	dRet;

	SetUpSignal();

	if((*pMEMSINFO = nifo_init_zone((U8*)"A_SIPT", SEQ_PROC_A_SIPT, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
        return -3;
    }


	if((*pSIPHASH = hasho_init(S_SSHM_A_SIPT, SIP_INFO_KEY_SIZE, SIP_INFO_KEY_SIZE, 
		SIP_INFO_SIZE, SIP_INFO_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -4;
	}

	if((*pTIMERNINFO = timerN_init(SIP_INFO_CNT, SIP_COMMON_SIZE)) == NULL) {
		log_print(LOGN_CRI, LH"timerN_init NULL", LT);
		return -5;
	}

	/* SIP TRANSACTION CLEAR */
	if((pSESSKEYINFO = hasho_init( S_SSHM_SIPT_SESS, STOP_CALL_KEY_SIZE, STOP_CALL_KEY_SIZE, 
					SESSKEY_LIST_SIZE, CALL_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -6;
	}
	
	if( (dRet = Init_SIPTKeyShm() ) < 0 ) {
		log_print( LOGN_INFO, LH"ERROR IN Init_SIPTKeyShm() dRet:%d", LT, dRet );
		return -7;
	} else if (dRet == 0) {
		InitSIPTKeyList();
	}

	vSIPTimerReConstruct(*pSIPHASH, *pTIMERNINFO);

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			sipt_init.c sipt_main.c
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
 *  @see			sipt_init.c sipt_main.c
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
 *  @see			sipt_init.c sipt_main.c
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
 *  @see			sipt_init.c sipt_main.c
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

void vSIPTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int					i;
	OFFSET				offset;
	stHASHONODE			*p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	SIP_COMMON			COMMON;
	SIP_INFO_KEY		*pKey;
	SIP_INFO			*pData;

	U8					szSIP[INET_ADDRSTRLEN];
	U8					szDIP[INET_ADDRSTRLEN];

	/* SIP */
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_SIP_TIMEOUT];
	log_print(LOGN_INFO, "REBUILD SIP TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (SIP_INFO_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (SIP_INFO *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD SIP TIMER CALLID=%s RTX=%u CSEQ=%u CSEQTYPE=%u FROMTAG=%s CLIIP=%s:%u SVRIP=%s:%u", 
						pKey->CallID, pKey->RTX, pKey->CSeq, pKey->SeqType, pKey->FromTag,
						util_cvtipaddr(szSIP, pKey->ClientIP), pKey->ClientIP, util_cvtipaddr(szDIP, pKey->ServerIP), pKey->ServerIP);

				memcpy(&COMMON.SIPINFOKEY, pKey, SIP_INFO_KEY_SIZE);
				pData->timerNID = timerN_add(pTIMER, invoke_del, (U8*)&COMMON, SIP_COMMON_SIZE, time(NULL) + guiTimerValue);

			}
			offset = p->offset_next;
		}
	}
}

/*
 * $Log: sipt_init.c,v $
 * Revision 1.3  2011/09/06 12:46:38  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 01:35:33  uamyd
 * modified to runnable source
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/17 13:13:55  hhbaek
 * A_SIPT
 *
 * Revision 1.3  2011/08/09 08:17:41  uamyd
 * add blocks
 *
 * Revision 1.2  2011/08/09 05:31:09  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.10  2011/01/11 04:09:10  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.9  2009/08/17 18:27:19  jsyoon
 * *** empty log message ***
 *
 * Revision 1.8  2009/08/17 15:08:01  jsyoon
 * 세션관리 메모리를 공유메모리로 변경
 *
 * Revision 1.7  2009/08/06 06:56:09  dqms
 * 로그레벨 공유메모리로 수정
 *
 * Revision 1.6  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.5  2009/07/15 16:44:07  dark264sh
 * A_SIPT vSIPTimerReConstruct 추가
 *
 * Revision 1.4  2009/07/05 15:39:40  dqms
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/29 13:25:54  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/05 05:30:16  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:34  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1  2008/09/18 07:21:33  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.3  2007/05/17 02:38:21  dark264sh
 * SEQ_PROC_CILOG => SEQ_PROC_CI_LOG, S_MSGQ_CILOG => S_MSGQ_CI_LOG 변경
 *
 * Revision 1.2  2007/03/07 01:12:06  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/03/05 00:37:21  dark264sh
 * *** empty log message ***
 *
 */
