/**		@file	inet_init.c
 * 		- Internet 사용량을 계산 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: inet_init.c,v 1.2 2011/09/04 12:16:51 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 12:16:51 $
 * 		@ref		inet_init.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- Internet 사용량을 계산 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>
#include <signal.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"
#include "path.h"
#include "filter.h"

// LIB
#include "memg.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "utillib.h"

// .
#include "inet_func.h"
#include "inet_init.h"

/**
 * Declare variables
 */
st_Flt_Info				*flt_info;

extern stCIFO			*gpCIFO;
extern S32				giFinishSignal;
extern S32				giStopFlag;
extern S32				guiSeqProcID;
extern char				gszMyProc[32];

extern S32				gATCPCnt;
extern S32				gACALLCnt;

extern unsigned char   *Send_Node_Head[MAX_MP_NUM];
extern unsigned int    Diff_Node_Cnt[MAX_MP_NUM];
extern unsigned int    Send_Node_Cnt[MAX_MP_NUM];
extern unsigned int    Collection_Cnt[MAX_MP_NUM];
extern unsigned int    old_time[MAX_MP_NUM];
extern unsigned int    check_pkt[MAX_MP_NUM];

/**
 *	Declare external func.
 */
extern void invoke_del(void *p);		/* main.c */

/** SetUpSignal function.
 *
 *	SetUpSignal Function
 *
 *	@return			void
 *	@see			inet_init.c inet_main.c inet_api.h
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
 *	UserControlledSignal Function
 *
 *	@param	isign	:	signal
 *
 *	@return			void
 *	@see			inet_init.c inet_main.c inet_api.h
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
 *	FinishProgram Function
 *
 *	@return			void
 *	@see			inet_init.c inet_main.c
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
 *	IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *	@return			void
 *	@see			inet_init.c inet_main.c inet_api.h
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

/** vINETTimerReConstruct function.
 *
 *	vINETTimerReConstruct Function
 *
 *	@param	*pHASH	:	stHASHOINFO
 *	@param	*pTIMER	:	stTIMERNINFO
 *
 *	@return			void
 *	@see			inet_init.c inet_main.c inet_api.h
 *
 **/
void vINETTimerReConstruct(stHASHOINFO *pINETHASH, stTIMERNINFO *pTIMER)
{
	int				i;
	OFFSET			offset;
	stHASHONODE		*p;
	stMEMGNODEHDR	*pMEMGNODEHDR;
	INET_KEY		*pINETKEY;
	INET_DATA		*pINETDATA;
	TIMER_COMMON	TIMERCOMMON;
	U8				szCIP[INET_ADDRSTRLEN];
	U8				szSIP[INET_ADDRSTRLEN];

	log_print(LOGN_CRI, "REBUILD INET TIMER hashcnt=%u", pINETHASH->uiHashSize);
	for(i = 0; i < pINETHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)nifo_ptr(pINETHASH, pINETHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)nifo_ptr(pINETHASH, offset);

			pINETKEY = (INET_KEY *)nifo_ptr(pINETHASH, p->offset_Key);
			pINETDATA = (INET_DATA *)nifo_ptr(pINETHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD INET TIMER CIP=%s SIP=%s:%d",
					util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);

				memcpy(&TIMERCOMMON.INETKEY, pINETKEY, INET_KEY_SIZE);
				pINETDATA->timerNID = timerN_add(pTIMER, invoke_del, (U8 *)&TIMERCOMMON, sizeof(TIMER_COMMON), time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_INET_TIMEOUT]);
			}
			offset = p->offset_next;
		}
	}
}

/** dInitINET function.
 *
 *  dInitINET Function
 *
 *  @return         S32
 *  @see            inet_init.c inet_main.c inet_api.h
 *
 **/
S32 dInitINET(stMEMSINFO **pMEMSINFO, stHASHOINFO **pCALLHASH, stHASHOINFO **pINETHASH, stTIMERNINFO **pTIMER, S32 dProcNum)
{
	int			i;
	int			dATCPCnt, dACALLCnt;
	U32			uiMsgQKey;
	char		szMsgQName[32];

	SetUpSignal();

	if((*pMEMSINFO = nifo_init_zone((U8*)gszMyProc, guiSeqProcID, FILE_NIFO_ZONE)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	for(i = 0; i < MAX_MP_NUM; i++) {
        /* BUFFERING */
        Send_Node_Head[i] = NULL;
        Diff_Node_Cnt[i] = 0;
        Send_Node_Cnt[i] = 0;
        Collection_Cnt[i] = COLLECTION_MIN;
        old_time[i] = 0;
        check_pkt[i] = 0;
    }

	gATCPCnt = get_block_num(FILE_MC_INIT, "A_ITCP");
	log_print(LOGN_INFO, "INIT., A_ITCP ProcCount[%d]", gATCPCnt);

	gACALLCnt = get_block_num(FILE_MC_INIT, "A_CALL");
    log_print(LOGN_INFO, "INIT., A_CALL ProcCount[%d]", gACALLCnt);


	*pINETHASH = hasho_init(S_SSHM_INETHASH0 + dProcNum, INET_KEY_SIZE, INET_KEY_SIZE, INET_DATA_SIZE, INET_HASH_CNT, 0);
	if( *pINETHASH == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -3;
	}

	if((*pCALLHASH = hasho_init(S_SSHM_INETCALL0 + dProcNum, INET_CALL_KEY_SIZE, INET_CALL_KEY_SIZE, INET_CALL_DATA_SIZE, CALL_SESS_CNT, 0)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -4;
	}

	if((*pTIMER = timerN_init(INET_HASH_CNT, sizeof(TIMER_COMMON))) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d timerN_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}

	vINETTimerReConstruct(*pINETHASH, *pTIMER);

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
        return -6;
    }

	return 0;
}

/*
 * 	$Log: inet_init.c,v $
 * 	Revision 1.2  2011/09/04 12:16:51  hhbaek
 * 	*** empty log message ***
 * 	
 * 	Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * 	NEW OAM SYSTEM
 * 	
 * 	Revision 1.4  2011/08/21 14:04:33  uamyd
 * 	added prea
 * 	
 * 	Revision 1.3  2011/08/17 13:00:01  hhbaek
 * 	A_INET
 * 	
 * 	Revision 1.2  2011/08/10 09:57:43  uamyd
 * 	modified and block added
 * 	
 * 	Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * 	init DQMS2
 * 	
 * 	Revision 1.4  2011/05/11 07:44:08  jsyoon
 * 	*** empty log message ***
 * 	
 * 	Revision 1.3  2011/05/09 09:35:13  dark264sh
 * 	A_INET: A_CALL multi 처리
 * 	
 * 	Revision 1.2  2011/04/13 14:15:36  dark264sh
 * 	A_INET: dSendINETLog 처리
 * 	
 * 	Revision 1.1  2011/04/13 13:14:38  dark264sh
 * 	A_INET 추가
 * 	
 */
