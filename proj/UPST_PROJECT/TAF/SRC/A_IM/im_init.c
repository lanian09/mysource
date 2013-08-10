/**		@file	im_init.c
 * 		- A_IM Process Initialize
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		@ID 		$Id: im_init.c,v 1.3 2011/09/05 01:35:32 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/05 01:35:32 $
 * 		@warning	nothing
 * 		@ref		in_init.c im_maic.c
 * 		@todo		nothing
 *
 * 		@section	Intro(소개)
 * 		- A_IM Process Initialize
 *
 * 		@section	Requirement
 *
 **/

/**
 *	Include headers
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "path.h"
#include "filter.h"

// LIB
#include "config.h"
#include "common_stg.h"
#include "memg.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

#include "loglib.h"
#include "ipclib.h"

// .
#include "im_init.h"

/**
 *	Declare var.
 */
extern S32			giFinishSignal;
extern S32			giStopFlag;
extern S32			gACALLCnt;
extern U64			nifo_create;
extern U64			nifo_del;
extern st_Flt_Info	*flt_info;
extern int			guiTimerValue;
extern stCIFO		*gpCIFO;

/**
 *	Declare func.
 */
extern void invoke_del_CALL(void *p);

/** dInitVT function.
 *
 *  dInitVT Function
 *
 *  @return			S32
 *  @see			call_init.c call_main.c
 *
 **/
S32 dInitIM(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO)
{
	//int			i;
	//int			dACALLCnt;
	//U32			uiMsgQKey;
	//char		szMsgQName[32];

	/* Setup Signal */
	SetUpSignal();

	*pMEMSINFO = nifo_init_zone((U8*)"A_IM", SEQ_PROC_A_IM, FILE_NIFO_ZONE);
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

#if 0
	if((*pMEMSINFO = nifo_init(S_SSHM_NIFO, S_SEMA_NIFO, "A_IM", SEQ_PROC_A_IM)) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d nifo_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if((dMyQID = nifo_msgq_init(S_MSGQ_A_IM)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_msgq_init S_MSGQ_A_IM dRet[%d][%s]", 
			__FILE__, __FUNCTION__, __LINE__, dMyQID, strerror(-dMyQID));
		return -2;
	}

	if((dCILOGQID = nifo_msgq_init(S_MSGQ_CI_LOG)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_msgq_init S_MSGQ_CI_LOG dRet[%d][%s]", 
			__FILE__, __FUNCTION__, __LINE__, dCILOGQID, strerror(-dCILOGQID));
		return -3;
	}

	dACALLCnt = dGetBlockNums(MC_INIT, "A_CALL");
	for(i = 0; i < MAX_SMP_NUM; i++)
	{
		dCALLQID[i] = -1;
	}

	uiMsgQKey = S_MSGQ_A_CALL0;

	for(i = 0; i < dACALLCnt; i++)
	{
		sprintf(szMsgQName, "A_CALL%d", i);

		if((dCALLQID[i] = nifo_msgq_init(uiMsgQKey)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] nifo_msgq_init CALL [PROC:%s QID:%d]", 
				__FILE__, __FUNCTION__, __LINE__, szMsgQName, dCALLQID[i]);
			return -4;
		}
		gACALLCnt++;
		uiMsgQKey++;
		log_print(LOGN_INFO, "INIT., A_CALL[%d] MSGQ[%d] TOT[%d]", i, dCALLQID[i], gACALLCnt);
	}
#endif

	/* IM Hash Table 초기화 */
	if((*pHASHOINFO = hasho_init(S_SSHM_A_IM, TAG_KEY_LOG_COMMON_SIZE, TAG_KEY_LOG_COMMON_SIZE, 
					CALL_SESSION_HASH_DATA_SIZE, IM_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}

	if((*pTIMERNINFO = timerN_init(IM_SESS_CNT, sizeof(st_CALLTimer))) == NULL) { 
		log_print(LOGN_CRI, "[%s][%s.%d] timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -6;
	}

	vIMSESSTimerReConstruct(*pHASHOINFO, *pTIMERNINFO);

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

void vIMSESSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int						i;
	OFFSET					offset;
	stHASHONODE				*p;
	stMEMGNODEHDR			*pMEMGNODEHDR;

	st_CALLTimer			COMMON;
	st_IMSessKey			*pKey;
	IM_SESSION_HASH_DATA	*pData;

	/* IM SESS */
	log_print(LOGN_INFO, "REBUILD IMSESS TIMER hashcnt=%u", pHASH->uiHashSize);
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_IM_TIMEOUT];
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (st_IMSessKey *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (IM_SESSION_HASH_DATA *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD IMSESS TIMER SIP=%d.%d.%d.%d", HIPADDR(pKey->uiClientIP));

				memcpy(&COMMON.ClientIP, &pKey->uiClientIP, ST_CALLTIMER_SIZE);
				pData->timerNID = timerN_add(pTIMER, invoke_del_CALL, (U8*)&COMMON, ST_CALLTIMER_SIZE, time(NULL) + guiTimerValue);
			}
			offset = p->offset_next;
		}
	}
}

/*
 * $Log: im_init.c,v $
 * Revision 1.3  2011/09/05 01:35:32  uamyd
 * modified to runnable source
 *
 * Revision 1.2  2011/09/04 12:16:51  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/11 08:06:55  hhbaek
 * Commit A_IM
 *
 * Revision 1.2  2011/08/09 09:08:20  hhbaek
 * Commit A_IM
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.8  2011/05/09 11:47:37  dark264sh
 * A_IM: A_CALL multi 처리
 *
 * Revision 1.7  2011/01/11 04:09:07  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.6  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.5  2009/07/15 16:20:38  dqms
 * ADD vIMSESSTimerReConstruct()
 *
 * Revision 1.4  2009/07/05 15:39:40  dqms
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/29 13:21:49  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/15 09:05:08  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1  2009/06/13 11:38:45  jsyoon
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
