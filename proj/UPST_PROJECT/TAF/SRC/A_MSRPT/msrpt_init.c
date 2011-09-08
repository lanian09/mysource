/**		@file	msrp_init.c
 * 		- A_MSRPT 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpt_init.c,v 1.3 2011/09/05 12:26:40 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/05 12:26:40 $
 * 		@ref		msrp_init.c
 *
 * 		@section	Intro(소개)
 * 		- A_MSRPT 프로세스를 초기화 하는 함수들
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
#include "procid.h"
#include "sshmid.h"
#include "commdef.h"
#include "common_stg.h"
#include "filter.h"
#include "path.h"

// LIB
#include "mems.h"
#include "memg.h"	 /* MEMG_ID, MEMG_ALLOCED */
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "ipclib.h"

// .
#include "msrpt_func.h"
#include "msrpt_init.h"

/**
 *	Declare var.
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

extern st_Flt_Info	*flt_info;
extern int			guiTimerValue;

/**
 *	Declare extern func.
 */
extern void invoke_del(void *p);

/**
 *	Implement func.
 */

/** dInitMSRPT function.
 *
 *  dInitMSRPT Function
 *
 *  @return			S32
 *  @see			msrp_init.c
 *
 **/
S32 dInitMSRPT(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstMSRPTHASH, stTIMERNINFO **pstTIMERNINFO)
{
	/* Setup Signal */
	SetUpSignal();

	/* Init Shared Memory */
	if((*pstMEMSINFO = nifo_init_zone((U8*)"A_MSRPT", SEQ_PROC_A_MSRPT, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, "F=%s:%s.%d FAILED IN nifo_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, "F=%s:%s.%d FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            __FILE__, __FUNCTION__, __LINE__, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
		log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
		return -5;
	}

	/* MSRPT Hash Table 초기화 */
	/* MSRPT Transaction 관리를 위한 구조체를 만들어야 함 */
	if((*pstMSRPTHASH = hasho_init(
		S_SSHM_A_MSRPT, DEF_MSRPTTRANSKEY_SIZE, DEF_MSRPTTRANSKEY_SIZE, DEF_MSRPTTRANS_SIZE, MSRPT_TRANS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d hasho_init MSRPT", __FILE__, __FUNCTION__, __LINE__);
		return -4;
	}

	if((*pstTIMERNINFO = timerN_init(MSRPT_TRANS_CNT, DEF_MSRPTCOMMON_SIZE)) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}

	vMSRPTimerReConstruct(*pstMSRPTHASH, *pstTIMERNINFO);

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			msrp_init.c
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
 *  @see			msrp_init.c
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
 *  @see			msrp_init.c
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
 *  @see			msrp_init.c
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


void vMSRPTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int                 i;
	OFFSET              offset;
	stHASHONODE         *p;
	stMEMGNODEHDR       *pMEMGNODEHDR;

	st_MSRPT_COMMON		COMMON;
	st_MSRPT_TRANS_KEY	*pKey;
	st_MSRPT_TRANS		*pData;

	/* MSRP TRANS */
	log_print(LOGN_INFO, "REBUILD MSRP TIMER hashcnt=%u", pHASH->uiHashSize);
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_MSRP_TIMEOUT];
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (st_MSRPT_TRANS_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (st_MSRPT_TRANS *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD MSRP TIMER SIP=%d.%d.%d.%d:%u DIP=%d.%d.%d.%d.%u MSGID=%s",
						HIPADDR(pKey->uiCliIP), pKey->usCliPort, HIPADDR(pKey->uiSrvIP), pKey->usSrvPort, pKey->szMSGID);

				memcpy(&COMMON.stMSRPTTRANSKEY, pKey, DEF_MSRPTTRANSKEY_SIZE);
				pData->timerNID = timerN_add(pTIMER, invoke_del, (U8*)&COMMON, DEF_MSRPTCOMMON_SIZE, time(NULL) + guiTimerValue);

			}
			offset = p->offset_next;
		}
	}
}


/*
 * $Log: msrpt_init.c,v $
 * Revision 1.3  2011/09/05 12:26:40  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 01:35:33  uamyd
 * modified to runnable source
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/21 09:07:52  hhbaek
 * Commit TAF/SRC/ *
 *
 * Revision 1.2  2011/08/09 05:31:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.7  2011/05/09 14:19:31  dark264sh
 * A_MSRPT: A_CALL multi 처리
 *
 * Revision 1.6  2011/01/11 04:09:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:00  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.5  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.4  2009/07/15 16:42:10  dqms
 * ADD vMSRPTimerReConstruct()
 *
 * Revision 1.3  2009/06/29 13:23:41  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/12 15:51:47  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:13  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1  2008/09/18 06:48:10  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:15:16  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.3  2007/05/17 02:41:07  dark264sh
 * SEQ_PROC_CILOG => SEQ_PROC_CI_LOG, S_MSGQ_CILOG => S_MSGQ_CI_LOG 변경
 *
 * Revision 1.2  2007/05/10 04:52:41  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/05/07 01:48:09  dark264sh
 * INIT
 *
 */
