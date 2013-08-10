/**     @file   call_func.c
 *      - Call Session을 관리 하는 프로세스
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: call_func.c,v 1.2 2011/09/04 08:04:25 dhkim Exp $
 *
 *      @Author     $Author: dhkim $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/04 08:04:25 $
 *      @warning    .
 *      @ref        call_msgq.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - CALL Session을 관리 하는 프로세스
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
 **/

#include <sys/time.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "mems.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "hasho.h"

#include "Analyze_Ext_Abs.h"


// PROJECT
#include "procid.h"
#include "filter.h"
#include "common_stg.h"
#include "capdef.h"

// .
#include "call_func.h"
#include "call_msgq.h"

extern stMEMSINFO	*pMEMSINFO;
extern stHASHOINFO  *pHASHOINFO;
extern stTIMERNINFO *pTIMERNINFO;

extern st_Flt_Info 	*flt_info;
extern int			guiTimerValue;

extern U64		CreateSessCnt;
extern U64		DelSessCnt;

extern U32          gATCPCnt;
extern U32          gAINETCnt;
extern U32          gAITCPCnt;

/** Create_Call_Session function.
 *
 *  Create_Call_Session Function
 *
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSndMsgQ : Send the Msg to the next process
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            call_msgq.c
 *
 **/
OFFSET Create_Call_Session(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASHOINFO, stTIMERNINFO *pTIMERNINFO, 
		LOG_COMMON *pLOG_COMMON, int type, int len, char *data)
{
	U8						*pNode;
	OFFSET 					offset;
	struct timeval			stNowTime;

	LOG_SIGNAL 				*pLOGSIGNAL;
	CALL_TIMER_ARG			aCALL_TIMER_ARG, *pCALL_TIMER_ARG;

	CALL_SESSION_HASH_DATA 	*pCALL_SESSION_HASH_DATA;


	pCALL_TIMER_ARG = &aCALL_TIMER_ARG;

	if((pNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "[%s] ERROR [%s.%d] CALL_SESSION_HASH_DATA nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	if((pCALL_SESSION_HASH_DATA = 
				(CALL_SESSION_HASH_DATA *) nifo_tlv_alloc(pMEMSINFO, pNode, CALL_SESSION_HASH_DATA_DEF_NUM, CALL_SESSION_HASH_DATA_SIZE, DEF_MEMSET_ON)) == NULL) {
		log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pCALL_SESSION_HASH_DATA nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_unlink_nont(pMEMSINFO, pNode);
		nifo_cont_delete(pMEMSINFO, pNode);
		return -2;
	}
	offset = nifo_offset(pMEMSINFO, pNode);

	pCALL_SESSION_HASH_DATA->PAGE_OLD_STATE = 0;
	pCALL_SESSION_HASH_DATA->PAGE_STATE = STS_NEW_PAGE;
	pCALL_SESSION_HASH_DATA->PAGE_ID = 1;
	pCALL_SESSION_HASH_DATA->IV_PAGE_ID = 1;
	pCALL_SESSION_HASH_DATA->pHASHOINFO = pHASHOINFO;
	pCALL_SESSION_HASH_DATA->pMEMSINFO = pMEMSINFO;
	pCALL_SESSION_HASH_DATA->pTIMERNINFO = pTIMERNINFO;
	pCALL_SESSION_HASH_DATA->func1 = Send_Page_Session_LOG;
	pCALL_SESSION_HASH_DATA->isServiceFlag = 0;

	pCALL_TIMER_ARG->ClientIP 			= pLOG_COMMON->uiClientIP;
	pCALL_TIMER_ARG->CallTime 			= pLOG_COMMON->uiCallTime;
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
	pCALL_SESSION_HASH_DATA->timerNID 	= 
		timerN_add(pTIMERNINFO, invoke_del_CALL, (U8 *)pCALL_TIMER_ARG, sizeof(CALL_TIMER_ARG), time(NULL) + guiTimerValue);

	gettimeofday(&stNowTime, NULL);

	/* nifo로 node를 할당하여 준다. 각기 단독 node로... */
	/* pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS = */
	/* pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS =    &&   Page의 state의 초기화(INIT)    ==> 이 부분을 함수로 만들었으면 함.*/
	/* CALL NODE 할당 및 LOG_CALL_TRANS TLV 할당 */
	if((pNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_DIALUP_SESS nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
		return -3;
	}

	if((pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS = 
				(LOG_DIALUP_SESS *) nifo_tlv_alloc(pMEMSINFO, pNode, LOG_DIALUP_SESS_DEF_NUM, LOG_DIALUP_SESS_SIZE, DEF_MEMSET_ON)) == NULL) {
		log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_DIALUP_SESS nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_unlink_nont(pMEMSINFO, pNode);
		nifo_cont_delete(pMEMSINFO, pNode);
		return -4;
	}
	memcpy(pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS, pLOG_COMMON, LOG_COMMON_SIZE);
	pCALL_SESSION_HASH_DATA->offset_DIALUP = nifo_offset(pMEMSINFO, pNode);
	pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->OpStartTime = stNowTime.tv_sec;
	pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->OpStartMTime = stNowTime.tv_usec;

	/* DIAL-UP ACCESS CHECK */
	if( type == RADIUS_START_NUM ) {
		pLOGSIGNAL = (LOG_SIGNAL *)data;
		pCALL_SESSION_HASH_DATA->uiPDSNIP = pLOGSIGNAL->uiNasIP;
		if( pLOGSIGNAL->usServiceType == LOG_DIALUP_SESS_DEF_NUM ) 
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->usSvcL4Type = L4_PHONE;
	}

	if((pNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_CALL_TRANS nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
		return -5;
	}
	if((pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS = 
				(LOG_CALL_TRANS *) nifo_tlv_alloc(pMEMSINFO, pNode, LOG_CALL_TRANS_DEF_NUM, LOG_CALL_TRANS_SIZE, DEF_MEMSET_ON)) == NULL) {
		log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_CALL_TRANS nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_unlink_nont(pMEMSINFO, pNode);
		nifo_cont_delete(pMEMSINFO, pNode);
		return -6;
	}
	pCALL_SESSION_HASH_DATA->offset_CALL = nifo_offset(pMEMSINFO, pNode);
	pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS->OpStartTime = stNowTime.tv_sec;
	pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS->OpStartMTime = stNowTime.tv_usec;

#if 0	
	log_print(LOGN_DEBUG, "#### OFFSET:%ld pCALL_SESSION_HASH_DATA:%p pLOG_CALL_TRANS:%p pLOG_DIALUP_SESS:%p", 
			offset, pCALL_SESSION_HASH_DATA, pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS, pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS);
#endif
	
	CreateSessCnt++;

	return offset;
}

int Init_Call_Session(CALL_SESSION_HASH_DATA *pCALL_SESSION_HASH_DATA, stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASHOINFO, stTIMERNINFO *pTIMERNINFO, LOG_COMMON *pLOG_COMMON, int type, int len, char *data)
{
	struct timeval			stNowTime;

	CALL_TIMER_ARG			aCALL_TIMER_ARG, *pCALL_TIMER_ARG;

	pCALL_TIMER_ARG = &aCALL_TIMER_ARG;


	pCALL_SESSION_HASH_DATA->PAGE_OLD_STATE = 0;
	pCALL_SESSION_HASH_DATA->PAGE_STATE = STS_NEW_PAGE;
	pCALL_SESSION_HASH_DATA->PAGE_ID = 1;
	pCALL_SESSION_HASH_DATA->IV_PAGE_ID = 1;
	pCALL_SESSION_HASH_DATA->pHASHOINFO = pHASHOINFO;
	pCALL_SESSION_HASH_DATA->pMEMSINFO = pMEMSINFO;
	pCALL_SESSION_HASH_DATA->pTIMERNINFO = pTIMERNINFO;
	pCALL_SESSION_HASH_DATA->func1 = Send_Page_Session_LOG;
	pCALL_SESSION_HASH_DATA->isServiceFlag = 0;

	pCALL_TIMER_ARG->ClientIP = pLOG_COMMON->uiClientIP;
	pCALL_TIMER_ARG->CallTime = pLOG_COMMON->uiCallTime;
//	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
//	pCALL_SESSION_HASH_DATA->timerNID = timerN_add(pTIMERNINFO, invoke_del_CALL, (U8 *)pCALL_TIMER_ARG, sizeof(CALL_TIMER_ARG), time(NULL) + guiTimerValue);

	gettimeofday(&stNowTime, NULL);

	pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->OpStartTime = stNowTime.tv_sec;
	pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->OpStartMTime = stNowTime.tv_usec;

	pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS->OpStartTime = stNowTime.tv_sec;
	pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS->OpStartMTime = stNowTime.tv_usec;

#if 0	
	log_print(LOGN_DEBUG, "#### OFFSET:%ld pCALL_SESSION_HASH_DATA:%p pLOG_CALL_TRANS:%p pLOG_DIALUP_SESS:%p", 
			offset, pCALL_SESSION_HASH_DATA, pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS, pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS);
#endif
	
	return 1;
}

void invoke_del_CALL(void *p)
{
	CALL_TIMER_ARG  		*pCALL_TIMER_ARG;
	CALL_SESSION_HASH_DATA 	*pCALL_SESSION_HASH_DATA;
	CALL_DUP_HASH_DATA 		*pstCallDupList;
	stHASHONODE				*pHASHNODE;
	STIME					baseCallTime;
	STIME					LastPktTime;
	int						CallIndex = 0;
	U32						dSeqProcID;
	stTIMERNKEY				*pstTIMERNKEY;

	pCALL_TIMER_ARG = (CALL_TIMER_ARG *) p;
	baseCallTime = pCALL_TIMER_ARG->CallTime;

	log_print(LOGN_INFO, "@@@ TIMER TIMEOUT CIP:%d.%d.%d.%d CALLTIME:%d ", 
			HIPADDR(pCALL_TIMER_ARG->ClientIP), baseCallTime);

	if((pHASHNODE = hasho_find(pHASHOINFO, (U8 *) &pCALL_TIMER_ARG->ClientIP)) != NULL) {

		pstCallDupList = (CALL_DUP_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHNODE->offset_Data);
		/* Next Call 이 있으면 CallTime을 비교하여 콜을 결정한다. */
		if(pstCallDupList->NextCallSessF) {
			if(baseCallTime == pstCallDupList->NextCallTime) {
				CallIndex = 1;
			}
		}

		pCALL_SESSION_HASH_DATA =
			(CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[CallIndex]);
		pstTIMERNKEY = (stTIMERNKEY *)&pCALL_SESSION_HASH_DATA->timerNID;
		log_print(LOGN_DEBUG, "@@@ INVOKE TIMEOUT CALL:%d CIP:%d.%d.%d.%d TIMEKEY:%d CALLTIME:%d NTIME:%ld S:%d C:%d", 
				CallIndex, HIPADDR(pCALL_TIMER_ARG->ClientIP), pstTIMERNKEY->sTimeKey, baseCallTime, time(NULL), 
				pCALL_SESSION_HASH_DATA->isStopFlag, pCALL_SESSION_HASH_DATA->isCallType);
		
		/* 
		 * 타임아웃으로 종료시에 A_INET, A_ITCP 프로세스에 종료 시그널 보내기
		 */
		if (pCALL_SESSION_HASH_DATA->isStopFlag != DEF_CALLSTATE_FIN && 
			pCALL_SESSION_HASH_DATA->isStopFlag != DEF_CALLSTATE_RECV) 
		{
			log_print(LOGN_DEBUG, "@@@ RETIMER SETTING CALL:%d CIP:%d.%d.%d.%d S:%d C:%d]", 
					CallIndex, HIPADDR(pCALL_TIMER_ARG->ClientIP), pCALL_SESSION_HASH_DATA->isStopFlag, pCALL_SESSION_HASH_DATA->isCallType);
			if(CallIndex == 0) {
				if (pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_FIN)
					pstCallDupList->CurrCallWaitF = 0;
				if (pstCallDupList->NextCallSessF == 0)
					LastPktTime = 0;
				else
					LastPktTime = pstCallDupList->CurrLastPktTime;
			} else {
				if (pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_FIN)
					pstCallDupList->NextCallWaitF = 0;
				LastPktTime = 0;
			}


			/* 현재상태가 DORMENT 상태이면 호를 지우지 않는다. 
			 */
			if (pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_DORM) {

				dSeqProcID = SEQ_PROC_A_TCP + (pCALL_TIMER_ARG->ClientIP % gATCPCnt);
				Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
				dSeqProcID = SEQ_PROC_A_ITCP + (pCALL_TIMER_ARG->ClientIP % gAITCPCnt);
				Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
				dSeqProcID = SEQ_PROC_A_INET + (pCALL_TIMER_ARG->ClientIP % gAINETCnt);
				Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
				Send_Clear_Msg(SEQ_PROC_A_SIPT, pCALL_TIMER_ARG->ClientIP, LastPktTime);

				pCALL_SESSION_HASH_DATA->isStopFlag = DEF_CALLSTATE_DFIN;
				guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT];
			}
			else if (pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_DFIN) {
				pCALL_SESSION_HASH_DATA->isStopFlag = DEF_CALLSTATE_RECV;
				pCALL_SESSION_HASH_DATA->isCallType = DEF_CALL_NORMAL;
				guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_RCALL_TIMEOUT];
			}
			else if (pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_INIT) {
				dSeqProcID = SEQ_PROC_A_TCP + (pCALL_TIMER_ARG->ClientIP % gATCPCnt);
				Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
				dSeqProcID = SEQ_PROC_A_ITCP + (pCALL_TIMER_ARG->ClientIP % gAITCPCnt);
				Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
				dSeqProcID = SEQ_PROC_A_INET + (pCALL_TIMER_ARG->ClientIP % gAINETCnt);
				Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
				Send_Clear_Msg(SEQ_PROC_A_SIPT, pCALL_TIMER_ARG->ClientIP, LastPktTime);

				pCALL_SESSION_HASH_DATA->isStopFlag = DEF_CALLSTATE_FIN;
				guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT];
			}

			log_print(LOGN_DEBUG, ">>> RETIMER SETTING CALL:%d CIP:%d.%d.%d.%d S:%d C:%d TIME[%d]", 
					CallIndex, HIPADDR(pCALL_TIMER_ARG->ClientIP), pCALL_SESSION_HASH_DATA->isStopFlag, pCALL_SESSION_HASH_DATA->isCallType, guiTimerValue);

			pCALL_SESSION_HASH_DATA->timerNID = timerN_add(pTIMERNINFO, invoke_del_CALL, (U8 *)pCALL_TIMER_ARG, sizeof(CALL_TIMER_ARG), time(NULL) + guiTimerValue);
			return;
		}
		/* 착신 데이타를 받은 상태에서 RADIUS 시그널이 없을 경우 종료 시그널을 보낸다.  */
		else if (pCALL_SESSION_HASH_DATA->isCallType == DEF_CALL_RECALL) {
			log_print(LOGN_DEBUG, "@@@ RECALL SETTING CALL:%d CIP:%d.%d.%d.%d S:%d C:%d]", 
					CallIndex, HIPADDR(pCALL_TIMER_ARG->ClientIP), pCALL_SESSION_HASH_DATA->isStopFlag, pCALL_SESSION_HASH_DATA->isCallType);
			if(CallIndex == 0) {
				if (pstCallDupList->NextCallSessF == 0)
					LastPktTime = 0;
				else
					LastPktTime = pstCallDupList->CurrLastPktTime;
			} else {
				LastPktTime = 0;
			}

			dSend_INET_Signal(pCALL_SESSION_HASH_DATA, pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallTime, pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallMTime, STOP_PI_RECALL_NUM);


			dSeqProcID = SEQ_PROC_A_TCP + (pCALL_TIMER_ARG->ClientIP % gATCPCnt);
			Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
			dSeqProcID = SEQ_PROC_A_ITCP + (pCALL_TIMER_ARG->ClientIP % gAITCPCnt);
			Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
			dSeqProcID = SEQ_PROC_A_INET + (pCALL_TIMER_ARG->ClientIP % gAINETCnt);
			Send_Clear_Msg(dSeqProcID, pCALL_TIMER_ARG->ClientIP, LastPktTime);
			Send_Clear_Msg(SEQ_PROC_A_SIPT, pCALL_TIMER_ARG->ClientIP, LastPktTime);

			pCALL_SESSION_HASH_DATA->isStopFlag = DEF_CALLSTATE_DFIN;
			guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT];

			pCALL_SESSION_HASH_DATA->timerNID = timerN_add(pTIMERNINFO, invoke_del_CALL, (U8 *)pCALL_TIMER_ARG, sizeof(CALL_TIMER_ARG), time(NULL) + guiTimerValue);
			return;
		}

		Send_Page_Session_LOG((void *)pCALL_SESSION_HASH_DATA);
		if(pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->usPlatformType == DEF_PLATFORM_PHONE) {
			Send_Dialup_Session_LOG(pCALL_SESSION_HASH_DATA);
		}

#if 0  /* NOT SEND CALL LOG TO TAM */
		Send_Call_Session_LOG(pCALL_SESSION_HASH_DATA);
#endif
		if(pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS) {
			U8 *__pNODE;
			__pNODE = nifo_ptr(pCALL_SESSION_HASH_DATA->pMEMSINFO, nifo_get_offset_node(pCALL_SESSION_HASH_DATA->pMEMSINFO, (U8 *) pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS));
			nifo_node_delete(pCALL_SESSION_HASH_DATA->pMEMSINFO, __pNODE);
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS = NULL;
			pCALL_SESSION_HASH_DATA->offset_DIALUP = 0;
		}
		if(pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS) {
			U8 *__pNODE;
			__pNODE = nifo_ptr(pCALL_SESSION_HASH_DATA->pMEMSINFO, nifo_get_offset_node(pCALL_SESSION_HASH_DATA->pMEMSINFO, (U8 *) pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS));
			nifo_node_delete(pCALL_SESSION_HASH_DATA->pMEMSINFO, __pNODE);
			pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS = NULL;
			pCALL_SESSION_HASH_DATA->offset_CALL = 0;
		}
		if(pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS){
			U8 *__pNODE;
			__pNODE = nifo_ptr(pCALL_SESSION_HASH_DATA->pMEMSINFO, nifo_get_offset_node(pCALL_SESSION_HASH_DATA->pMEMSINFO, (U8 *) pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS));
			nifo_node_delete(pCALL_SESSION_HASH_DATA->pMEMSINFO, __pNODE);
			pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS = NULL;
			pCALL_SESSION_HASH_DATA->offset_PAGE = 0;
		}
		if(pCALL_SESSION_HASH_DATA->aPAGE_DATA.pBODY){
			U8 *__pNODE;
			__pNODE = nifo_ptr(pCALL_SESSION_HASH_DATA->pMEMSINFO, nifo_get_offset_node(pCALL_SESSION_HASH_DATA->pMEMSINFO, (U8 *) pCALL_SESSION_HASH_DATA->aPAGE_DATA.pBODY));
			nifo_node_delete(pCALL_SESSION_HASH_DATA->pMEMSINFO, __pNODE);
			pCALL_SESSION_HASH_DATA->aPAGE_DATA.pBODY = NULL;
			pCALL_SESSION_HASH_DATA->aPAGE_DATA.offset_BODY = 0;
		}
		if(pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY){
			U8 *__pNODE;
			__pNODE = nifo_ptr(pCALL_SESSION_HASH_DATA->pMEMSINFO, nifo_get_offset_node(pCALL_SESSION_HASH_DATA->pMEMSINFO, (U8 *) pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY));
			nifo_node_delete(pCALL_SESSION_HASH_DATA->pMEMSINFO, __pNODE);
			pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY = NULL;
			pCALL_SESSION_HASH_DATA->aPAGE_DATA.offset_InBODY = 0;
		}
		nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pstCallDupList->CallDupList[CallIndex]));

		/* CURR CALL을 정리할때 NEXT CALL이 있으면 NEXT 정보를 CURR 로 이동한다. */
		if(CallIndex == 0) {
			if(pstCallDupList->NextCallSessF) {
				pstCallDupList->CallDupList[0] = pstCallDupList->CallDupList[1];
				pstCallDupList->CurrCallSessF = pstCallDupList->NextCallSessF;
				pstCallDupList->CurrCallWaitF = pstCallDupList->NextCallWaitF;
				pstCallDupList->CurrCallTime = pstCallDupList->NextCallTime;

				pstCallDupList->NextCallSessF = 0;
				pstCallDupList->NextCallWaitF = 0;
				pstCallDupList->NextCallTime = 0;
				pstCallDupList->CallDupList[1] = 0;
			} else {
				pstCallDupList->CurrCallSessF = 0;
				pstCallDupList->CurrCallWaitF = 0;
				pstCallDupList->CurrCallTime = 0;
				pstCallDupList->CallDupList[0] = 0;
			}
		} 
		/* NEXT CALL을 정리할때 CURR CALL을 함께 정리한다. */
		else {
			pstCallDupList->NextCallSessF = 0;
			pstCallDupList->NextCallWaitF = 0;
			pstCallDupList->NextCallTime = 0;
			pstCallDupList->CallDupList[1] = 0;
 
			if(pstCallDupList->CurrCallSessF) {
				pCALL_SESSION_HASH_DATA =
					(CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[0]);
				pCALL_SESSION_HASH_DATA->timerNID = 
					timerN_update(pTIMERNINFO, pCALL_SESSION_HASH_DATA->timerNID, time(NULL) + 1);
			}
		}
		
		if( pstCallDupList->CurrCallSessF == 0 && pstCallDupList->NextCallSessF == 0 ) {
			hasho_del(pHASHOINFO, (U8 *) &pCALL_TIMER_ARG->ClientIP);
			DelSessCnt++;
		}
	} else {
		log_print(LOGN_CRI, "INVOKE TIMEOUT BUT NODE NULL CIP:%d.%d.%d.%d CALLTIME:%d NTIME:%ld ", 
				HIPADDR(pCALL_TIMER_ARG->ClientIP), baseCallTime, time(NULL));
	}
}

void Call_Session_Process( int type, int len, char *data)
{
	stHASHONODE     *pHASHONODE;
	LOG_COMMON 		*pLOG_COMMON;
	LOG_SIGNAL		*pLOGSIGNAL;	
	LOG_INET 		*pLOGINET;
	LOG_ITCP_SESS 	*pLOGITCPSESS;
	LOG_IHTTP_TRANS *pLOGIHTTPTRANS;

	TAG_KEY_LOG_COMMON		aTAG_KEY_LOG_COMMON, *pTAG_KEY_LOG_COMMON;
	CALL_TIMER_ARG			aCALL_TIMER_ARG, *pCALL_TIMER_ARG;
//	CALL_SESSION_HASH_DATA 	aCALL_SESSION_HASH_DATA;
	CALL_SESSION_HASH_DATA 	*pCALL_SESSION_HASH_DATA;
	CALL_SESSION_HASH_DATA 	*pCALL_SESSION_HASH_DATA_CURR;

	CALL_DUP_HASH_DATA 		stCallDupList;
	CALL_DUP_HASH_DATA 		*pstCallDupList;

	UINT 					dSeqProcID;
//	U8						*pNode;
	int						CallIndex = 0;
	int 					isContinue[2];
	int						dCALLTIMEOUT = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
	OFFSET 					dNewOffset;
	
	int 					isStart = 0;

	pCALL_TIMER_ARG = &aCALL_TIMER_ARG;

	// In the Call_session_Process function , there are processing everything related to LOG_COMMON
	// pLOG_COMMON = real LOG's first COMMON_KEY
	pLOG_COMMON = (LOG_COMMON *) data;
	pTAG_KEY_LOG_COMMON = &aTAG_KEY_LOG_COMMON;
	pTAG_KEY_LOG_COMMON->uiClientIP = pLOG_COMMON->uiClientIP;

	log_print(LOGN_DEBUG, "RCV LOG TYPE[%d][%s] LEN[%d] CIP[%d.%d.%d.%d] TIME[%u.%u]", type, 
			((type==START_CALL_NUM || type==STOP_CALL_NUM || type==RADIUS_START_NUM || type==LOG_PISIGNAL_DEF_NUM || type==START_PI_DATA_RECALL_NUM) 
			 ? PRINT_TAG_DEF_ALL_CALL_INPUT(type) : PRINT_DEF_NUM_table_log(type)), len,
			HIPADDR(pLOG_COMMON->uiClientIP), pLOG_COMMON->uiCallTime, pLOG_COMMON->uiCallMTime);

	if( (pHASHONODE = hasho_find(pHASHOINFO, (U8 *)pTAG_KEY_LOG_COMMON)) ) 
	{
		pstCallDupList = (CALL_DUP_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
		
		isContinue[0] = 0;
		isContinue[1] = 0;

		/* Next Call이 있을 경우 CallTime을 비교하여 Call 세션을 결정한다. */
		if(pstCallDupList->NextCallSessF) {
			if(pLOG_COMMON->uiCallTime >= pstCallDupList->NextCallTime ) {
				CallIndex = 1;
			} else if(type == LOG_SIP_TRANS_DEF_NUM && ((LOG_SIP_TRANS *)data)->method == SIP_MSG_INVITE) {
				if(pLOG_COMMON->uiCallTime >= pstCallDupList->CurrLastPktTime) {
					CallIndex = 1;
				}
			}
		} else {
			if(type == RADIUS_START_NUM) {
				pLOGSIGNAL = (LOG_SIGNAL *)data;
				/* 
				 * 3GPP2-Begin-Session == 1 이면 처음 호의 시작이며 이전 호가 있다면 정리한다. 
				 */
				if(pLOGSIGNAL->ucStopFlag==1) {
					if (pstCallDupList->CurrCallWaitF) {
						pCALL_SESSION_HASH_DATA = 
							(CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[0]);

						/* Radius Accounting Start Signal을 받았을 때 CallTime을 비교하여 
						 * 현재보다 이전 시간일 경우 새로운 콜을 생성하지 않는다. 
						 */
						if( pLOGSIGNAL->uiCallTime < pstCallDupList->CurrCallTime ) {
							log_print(LOGN_CRI, "DROP ACCT START MSG IN REVERSE ORDER CTIME[%d.%d] NTIME[%d.%d]",
									pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallTime,
									pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallMTime,
									pLOGSIGNAL->uiCallTime, pLOGSIGNAL->uiCallMTime); 
							return ;
						} else if ( pLOGSIGNAL->uiCallTime == pstCallDupList->CurrCallTime ) {
							if (pLOGSIGNAL->uiCallMTime <= pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallMTime) {
								log_print(LOGN_CRI, "DROP ACCT START MSG IN REVERSE ORDER CTIME[%d.%d] NTIME[%d.%d]",
										pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallTime,
										pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallMTime,
										pLOGSIGNAL->uiCallTime, pLOGSIGNAL->uiCallMTime); 
								return ;
							}
						}
						/* Clear MSG to A_TCP[], A_INET[], A_ITCP[] */
						dSeqProcID = SEQ_PROC_A_TCP + (pLOGSIGNAL->uiClientIP % gATCPCnt);
						Send_Clear_Msg(dSeqProcID, pLOGSIGNAL->uiClientIP, pstCallDupList->CurrLastPktTime);
						dSeqProcID = SEQ_PROC_A_ITCP + (pLOGSIGNAL->uiClientIP % gAITCPCnt);
						Send_Clear_Msg(dSeqProcID, pLOGSIGNAL->uiClientIP, pstCallDupList->CurrLastPktTime);
						dSeqProcID = SEQ_PROC_A_INET + (pLOGSIGNAL->uiClientIP % gAINETCnt);
						Send_Clear_Msg(dSeqProcID, pLOGSIGNAL->uiClientIP, pstCallDupList->CurrLastPktTime);
						Send_Clear_Msg(dSeqProcID, pLOGSIGNAL->uiClientIP, pstCallDupList->CurrLastPktTime);
						
						guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT];
						pCALL_SESSION_HASH_DATA->timerNID = 
							timerN_update(pTIMERNINFO, pCALL_SESSION_HASH_DATA->timerNID, time(NULL) + guiTimerValue);
						log_print(LOGN_INFO, "UPDATE CURR CALL TIMER IMSI[%s] STOPFLAG[%d] TIMEOUT[%d]", 
								pCALL_SESSION_HASH_DATA->aLOG_COMMON.szIMSI, pCALL_SESSION_HASH_DATA->isStopFlag, guiTimerValue);
					}

					dNewOffset = Create_Call_Session(pMEMSINFO, pHASHOINFO, pTIMERNINFO, pLOG_COMMON, type, len, data);
					if(dNewOffset < 0) {
						log_print(LOGN_CRI, "CAN NOT CREATE NEXT CALL Create_Call_Session() RET:%ld CIP:%d IMSI:%s ", 
								dNewOffset, pLOGSIGNAL->uiClientIP, pLOGSIGNAL->szIMSI );
						pstCallDupList->CallDupList[1] = 0;
						pstCallDupList->NextCallSessF = 0;
						return ;
					}
					pstCallDupList->CallDupList[1] = dNewOffset;
					pstCallDupList->NextCallTime = pLOG_COMMON->uiCallTime;
					pstCallDupList->NextCallSessF = 1;
					CallIndex = 1;
					isContinue[CallIndex] = 0;
	
					pCALL_SESSION_HASH_DATA_CURR = 
						(CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[0]);
					log_print(LOGN_DEBUG, "	CREATE NEXT CALL BEGIN[%d] : IP[%d.%d.%d.%d]	CURR[%s][%d.%d] NEXT[%s][%d.%d]", 
							pLOGSIGNAL->ucStopFlag,	
							HIPADDR(pLOGSIGNAL->uiClientIP), 
							pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.szIMSI,
							pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallTime, 
							pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallMTime, 
							pLOGSIGNAL->szIMSI, pLOGSIGNAL->uiCallTime, pLOGSIGNAL->uiCallMTime);
				} else {
					/* DORMANT 상태로 다음 시그널을 기다리는 상태이면 */
					if (pstCallDupList->CurrCallWaitF) {
						isContinue[CallIndex] = 1;
					} else {
						pCALL_SESSION_HASH_DATA_CURR = 
							(CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[0]);
						/* Radius Accounting Start Signal을 받았을 때 CallTime을 비교하여 
						 * 현재보다 이전 시간일 경우 새로운 콜을 생성하지 않는다. 
						 */
						if( pLOGSIGNAL->uiCallTime < pstCallDupList->CurrCallTime ) {
							log_print(LOGN_CRI, "DROP ACCT START MSG IN REVERSE ORDER CTIME[%d.%d] NTIME[%d.%d]",
									pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallTime,
									pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallMTime,
									pLOGSIGNAL->uiCallTime, pLOGSIGNAL->uiCallMTime); 
							return ;
						} else if ( pLOGSIGNAL->uiCallTime == pstCallDupList->CurrCallTime ) {
							if (pLOGSIGNAL->uiCallMTime <= pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallMTime) {
								log_print(LOGN_CRI, "DROP ACCT START MSG IN REVERSE ORDER CTIME[%d.%d] NTIME[%d.%d]",
										pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallTime,
										pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallMTime,
										pLOGSIGNAL->uiCallTime, pLOGSIGNAL->uiCallMTime); 
								return ;
							}
						}

						dNewOffset = Create_Call_Session(pMEMSINFO, pHASHOINFO, pTIMERNINFO, pLOG_COMMON, type, len, data);
						if(dNewOffset < 0) {
							log_print(LOGN_CRI, "CAN NOT CREATE NEXT CALL Create_Call_Session() RET:%ld CIP:%d IMSI:%s ", 
									dNewOffset, pLOGSIGNAL->uiClientIP, pLOGSIGNAL->szIMSI );
							pstCallDupList->CallDupList[1] = 0;
							pstCallDupList->NextCallSessF = 0;
							return ;
						}
						pstCallDupList->CallDupList[1] = dNewOffset;
						pstCallDupList->NextCallTime = pLOG_COMMON->uiCallTime;
						pstCallDupList->NextCallSessF = 1;
						CallIndex = 1;
						isContinue[CallIndex] = 0;

						log_print(LOGN_DEBUG, "	CREATE NEXT CALL BEGIN[%d]: IP[%d.%d.%d.%d]	CURR[%s][%d.%d] NEXT[%s][%d.%d]", 
								pLOGSIGNAL->ucStopFlag,	
								HIPADDR(pLOGSIGNAL->uiClientIP), 
								pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.szIMSI,
								pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallTime, 
								pCALL_SESSION_HASH_DATA_CURR->aLOG_COMMON.uiCallMTime, 
								pLOGSIGNAL->szIMSI, pLOGSIGNAL->uiCallTime, pLOGSIGNAL->uiCallMTime);

					}
				}
			} /* END RADIUS_START_NUM */ 
		}

		if (CallIndex) {
			if(pstCallDupList->NextLastPktTime < pLOG_COMMON->uiCallTime)
				pstCallDupList->NextLastPktTime = pLOG_COMMON->uiCallTime;
		} else {
			if(pstCallDupList->CurrLastPktTime < pLOG_COMMON->uiCallTime)
				pstCallDupList->CurrLastPktTime = pLOG_COMMON->uiCallTime;
		}

		pCALL_SESSION_HASH_DATA = 
			(CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[CallIndex]);

		log_print(LOGN_DEBUG, "	FOUND INDEX[%d] SVCFLAG[%d] IP[%d.%d.%d.%d] IMSI[%s] CallTime[%u.%u] LogTime[%u] S:%d C:%d",
				CallIndex, pCALL_SESSION_HASH_DATA->isServiceFlag,
				HIPADDR(pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiClientIP),
				pCALL_SESSION_HASH_DATA->aLOG_COMMON.szIMSI, 
				pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallTime, pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallMTime,
				(!CallIndex) ? pstCallDupList->CurrCallTime : pstCallDupList->NextCallTime,
				pCALL_SESSION_HASH_DATA->isStopFlag, pCALL_SESSION_HASH_DATA->isCallType);


		if(!pCALL_SESSION_HASH_DATA->isServiceFlag && pCALL_SESSION_HASH_DATA->isCallType >= DEF_CALL_RECALL) {
			switch(type) {
				case LOG_INET_DEF_NUM:
					pLOGINET = (LOG_INET *) data;
					log_print(LOGN_DEBUG, "	LOG_INET  UP[%d] DN[%d]", pLOGINET->uiUpPacketCnt, pLOGINET->uiDnPacketCnt);
					if(pLOGINET->uiUpPacketCnt && pLOGINET->uiDnPacketCnt) isStart=1;
					break;
				case LOG_ITCP_SESS_DEF_NUM:
					pLOGITCPSESS = (LOG_ITCP_SESS *) data;
					log_print(LOGN_DEBUG, "	LOG_ITCP_SESS  UP[%d] DN[%d]", pLOGITCPSESS->uiIPTotUpPktCnt, pLOGITCPSESS->uiIPTotDnPktCnt);
					if(pLOGITCPSESS->uiIPTotUpPktCnt && pLOGITCPSESS->uiIPTotDnPktCnt) isStart=1;
					break;
				case LOG_IHTTP_TRANS_DEF_NUM:
					pLOGIHTTPTRANS = (LOG_IHTTP_TRANS *) data;
					log_print(LOGN_DEBUG, "	LOG_IHTTP_TRANS  UP[%d] DN[%d]", pLOGIHTTPTRANS->uiIPTotUpPktCnt, pLOGIHTTPTRANS->uiIPTotDnPktCnt);
					if(pLOGIHTTPTRANS->uiIPTotUpPktCnt && pLOGIHTTPTRANS->uiIPTotDnPktCnt) isStart=1;
					break;
				default:
					isStart = 0;
					break;
			}
			if(isStart) {
				dSend_Service_Start_Signal(pCALL_SESSION_HASH_DATA, pLOG_COMMON->uiCallTime, pLOG_COMMON->uiCallMTime);
				pCALL_SESSION_HASH_DATA->isServiceFlag = 1;
			}
		}
		else if(!pCALL_SESSION_HASH_DATA->isServiceFlag && type != LOG_PISIGNAL_DEF_NUM && type != RADIUS_START_NUM) {
			dSend_Service_Start_Signal(pCALL_SESSION_HASH_DATA, pLOG_COMMON->uiCallTime, pLOG_COMMON->uiCallMTime);
			pCALL_SESSION_HASH_DATA->isServiceFlag = 1;
		} /* 착신대기 상태에서 데이타를 받았을 경우 */
		else if (type == START_PI_DATA_RECALL_NUM && 
				pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_RECV) {
//				(pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_DORM || pCALL_SESSION_HASH_DATA->isStopFlag != DEF_CALLSTATE_DFIN)) {
			dSend_INET_Signal(pCALL_SESSION_HASH_DATA, pLOG_COMMON->uiCallTime, pLOG_COMMON->uiCallMTime, type);
			pCALL_SESSION_HASH_DATA->isCallType = DEF_CALL_RECALL;

			Init_Call_Session(pCALL_SESSION_HASH_DATA, pMEMSINFO, pHASHOINFO, pTIMERNINFO, pLOG_COMMON, type, len, data);

			/* TODO: CALL HASH DATA를 초기화하는 로직을 그대로 해줘야 한다 */
			pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallTime = pLOG_COMMON->uiCallTime;
			pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallMTime = pLOG_COMMON->uiCallMTime;
			pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiAccStartTime = pLOG_COMMON->uiCallTime;
			pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiAccStartMTime = pLOG_COMMON->uiCallMTime;

			dCALLTIMEOUT = flt_info->stTimerInfo.usTimerInfo[PI_RCALL_SIGWAIT];
			pCALL_SESSION_HASH_DATA->timerNID = 
				timerN_update(pTIMERNINFO, pCALL_SESSION_HASH_DATA->timerNID, time(NULL) + dCALLTIMEOUT);
		}
		else if (type == RADIUS_START_NUM && pCALL_SESSION_HASH_DATA->isCallType == DEF_CALL_RECALL) {
			dSend_INET_Signal(pCALL_SESSION_HASH_DATA, pLOG_COMMON->uiCallTime, pLOG_COMMON->uiCallMTime, START_PI_SIG_RECALL_NUM);
			pCALL_SESSION_HASH_DATA->isCallType = DEF_CALL_RECALL_1;
		}
 
		/* Accounting Request Stop 시그널을 받은 이후에는 다른 로그가 와도 타이머를 업데이트 하지 않는다. */
		if (isContinue[CallIndex] == 1) {
			pCALL_SESSION_HASH_DATA->isStopFlag = DEF_CALLSTATE_INIT;
		}
		if (pCALL_SESSION_HASH_DATA->isStopFlag != DEF_CALLSTATE_FIN) {
			if ( type == LOG_PISIGNAL_DEF_NUM ) {
				pLOGSIGNAL = (LOG_SIGNAL *)data;

				log_print(LOGN_INFO, "CHANGE SIGNAL CALLTIME: %u.%u --> %u.%u", 
						pLOGSIGNAL->uiCallTime, pLOGSIGNAL->uiCallMTime,
						pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallTime, 
						pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallMTime);

				pLOGSIGNAL->uiCallTime = pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallTime;
				pLOGSIGNAL->uiCallMTime = pCALL_SESSION_HASH_DATA->aLOG_COMMON.uiCallMTime;

				if(pLOGSIGNAL->ucAcctType == DEF_MSG_ACCREQ_STOP || pLOGSIGNAL->ucAcctType == DEF_MSG_ACCREQ_LINKSTOP) {

					if (pCALL_SESSION_HASH_DATA->isCallType >= DEF_CALL_RECALL) {
						dSend_INET_Signal(pCALL_SESSION_HASH_DATA, pLOG_COMMON->uiCallTime, pLOG_COMMON->uiCallMTime, STOP_PI_RECALL_NUM);
					}

					/* 
					 *	IF 3GPP2-Session-Continue == 1 -> 다음에 호가 유지됨 
					 */
					if (pLOGSIGNAL->ucStopFlag == 0) {
						dCALLTIMEOUT = flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT];
						pCALL_SESSION_HASH_DATA->isStopFlag = DEF_CALLSTATE_FIN;
					} else {
						dCALLTIMEOUT = flt_info->stTimerInfo.usTimerInfo[PI_DORM_TIMEOUT];
						pstCallDupList->CurrCallWaitF = 1;
						pCALL_SESSION_HASH_DATA->isStopFlag = DEF_CALLSTATE_DORM;
					}
				} 	
				pCALL_SESSION_HASH_DATA->timerNID = 
					timerN_update(pTIMERNINFO, pCALL_SESSION_HASH_DATA->timerNID, time(NULL) + dCALLTIMEOUT);
				log_print(LOGN_INFO, "UPDATE EXIST TIMER IMSI[%s] STOPFLAG[%d] TIMEOUT[%d] S:%d C:%d", 
						pCALL_SESSION_HASH_DATA->aLOG_COMMON.szIMSI, pCALL_SESSION_HASH_DATA->isStopFlag, dCALLTIMEOUT,
						pCALL_SESSION_HASH_DATA->isStopFlag, pCALL_SESSION_HASH_DATA->isCallType);

			} else {
				if (pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_INIT) {
					pCALL_SESSION_HASH_DATA->timerNID = 
						timerN_update(pTIMERNINFO, pCALL_SESSION_HASH_DATA->timerNID, time(NULL) + dCALLTIMEOUT);
					log_print(LOGN_INFO, "UPDATE EXIST TIMER IMSI[%s] STOPFLAG[%d] TIMEOUT[%d] S:%d C:%d", 
							pCALL_SESSION_HASH_DATA->aLOG_COMMON.szIMSI, pCALL_SESSION_HASH_DATA->isStopFlag, dCALLTIMEOUT, 
							pCALL_SESSION_HASH_DATA->isStopFlag, pCALL_SESSION_HASH_DATA->isCallType);
				}
			}

			if(type == LOG_PISIGNAL_DEF_NUM) {
				return;
			}
		}

		LOG_TCP_SESS		*pTCPLOG;
		LOG_IV				*pIVLOG;
		LOG_ITCP_SESS		*pITCPLOG;
		LOG_IHTTP_TRANS		*pIHTTPLOG;

		switch(type)
		{
		case LOG_TCP_SESS_DEF_NUM:
			pTCPLOG = (LOG_TCP_SESS *)data;
			if((pTCPLOG->usPlatformType == DEF_PLATFORM_STREAM) && (pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS->LastPlatformType == DEF_PLATFORM_STREAM)) {
				pTCPLOG->usSvcL4Type = pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS->LastSvcL4Type;	
			} else if((pTCPLOG->usSvcL4Type == L4_WIPI) && (pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS->LastSvcL7Type == APP_DOWN)) {
				pTCPLOG->usPlatformType = pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS->LastPlatformType;
			}
			break;
		case LOG_ITCP_SESS_DEF_NUM:
			pITCPLOG = (LOG_ITCP_SESS *)data;
			if(pCALL_SESSION_HASH_DATA->isCallType >= DEF_CALL_RECALL) {
				pITCPLOG->usSvcL4Type = L4_INET_TCP_RECV;
			}
			break;
		case LOG_IHTTP_TRANS_DEF_NUM:
			pIHTTPLOG = (LOG_IHTTP_TRANS *)data;
			if(pCALL_SESSION_HASH_DATA->isCallType >= DEF_CALL_RECALL) {
				pIHTTPLOG->usSvcL4Type = L4_INET_HTTP_RECV;
			}
			break;
		case LOG_IV_DEF_NUM:
			pIVLOG = (LOG_IV *)data;
			pIVLOG->uiPageID = pCALL_SESSION_HASH_DATA->IV_PAGE_ID;
			pCALL_SESSION_HASH_DATA->IV_PAGE_ID++;	
			break;
		default:
			break;
		}
	} 
	/* 호 세션이 없을 경우 */
	else {
		if(type == RADIUS_START_NUM) {
			memset((char *)&stCallDupList, 0x0, sizeof(CALL_DUP_HASH_DATA));
			pstCallDupList = &stCallDupList;

			if((pHASHONODE = hasho_add(pHASHOINFO, (U8 *)pTAG_KEY_LOG_COMMON, (U8 *)pstCallDupList)) == NULL) {
				log_print(LOGN_CRI, "[%s.%d] ERROR : CALL DUP LIST hasho_add NULL", __FUNCTION__, __LINE__);
//				TAG_KEY_LOG_COMMON_Prt((S8 *)__FUNCTION__, pTAG_KEY_LOG_COMMON);
//				LOG_COMMON_Prt((S8 *)__FUNCTION__, pLOG_COMMON);
				exit(10);
			} else {
				pstCallDupList = (CALL_DUP_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
				memset(pstCallDupList, 0x00, sizeof(CALL_DUP_HASH_DATA));

				dNewOffset = Create_Call_Session(pMEMSINFO, pHASHOINFO, pTIMERNINFO, pLOG_COMMON, type, len, data);
				if(dNewOffset < 0) {
					log_print(LOGN_CRI, "CAN NOT CREATE CALL Create_Call_Session() RET:%ld CIP:%d IMSI:%s ", 
							dNewOffset, pLOG_COMMON->uiClientIP, pLOG_COMMON->szIMSI );
					pstCallDupList->CallDupList[0] = 0;
					pstCallDupList->CurrCallSessF = 0;
					hasho_del(pHASHOINFO, (U8 *)pTAG_KEY_LOG_COMMON);
					DelSessCnt++;
					return ;
				}
				pstCallDupList->CallDupList[0] = dNewOffset;
				pstCallDupList->CurrCallTime = pLOG_COMMON->uiCallTime;
				pstCallDupList->CurrCallSessF = 1;

				log_print(LOGN_DEBUG, "	CREATE NEW CALL CIP[%d.%d.%d.%d]", HIPADDR(pLOG_COMMON->uiClientIP));
			}

			pCALL_SESSION_HASH_DATA = 
				(CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[0]);
		} else {
			log_print(LOGN_INFO, "	BYPASS CIP[%d.%d.%d.%d]", HIPADDR(pLOG_COMMON->uiClientIP));
			return ;
		}
	}

	if(pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS == NULL) {
		log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_CALL_TRANS is NULL : lack of memory", __FILE__, __FUNCTION__, __LINE__);
		return ;
	}
	if(pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS == NULL) {
		log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_DIALUP_SESS is NULL : lack of memory", __FILE__, __FUNCTION__, __LINE__);
		return ;
	}

	// LOG_COMMON Update
	{
		LOG_COMMON	aaLOG_COMMON;
		memcpy(&aaLOG_COMMON, &pCALL_SESSION_HASH_DATA->aLOG_COMMON, LOG_COMMON_SIZE);
//		LOG_COMMON_Prt("LOG_TYPE1-1", &pCALL_SESSION_HASH_DATA->aLOG_COMMON);
		ASSOCIATION_LOG_COMMON_with_LOG_COMMON((LOG_COMMON *) &pCALL_SESSION_HASH_DATA->aLOG_COMMON , pLOG_COMMON);
//		LOG_COMMON_Prt("LOG_TYPE1-2", &pCALL_SESSION_HASH_DATA->aLOG_COMMON);

		if(pLOG_COMMON->szHostName[0] != 0x00) {
			memcpy(pCALL_SESSION_HASH_DATA->aLOG_COMMON.szHostName, pLOG_COMMON->szHostName, MAX_HOSTNAME_LEN);
			pCALL_SESSION_HASH_DATA->aLOG_COMMON.szHostName[MAX_HOSTNAME_LEN] = 0x00;
		}

		if(pLOG_COMMON->szBrowserInfo[0] != 0x00) {
			memcpy(pCALL_SESSION_HASH_DATA->aLOG_COMMON.szBrowserInfo, pLOG_COMMON->szBrowserInfo, MAX_BROWSERINFO_LEN);
			pCALL_SESSION_HASH_DATA->aLOG_COMMON.szBrowserInfo[MAX_BROWSERINFO_LEN] = 0x00;
		}

		if(pLOG_COMMON->szModel[0] != 0x00) {
			memcpy(pCALL_SESSION_HASH_DATA->aLOG_COMMON.szModel, pLOG_COMMON->szModel, MAX_MODEL_LEN);
			pCALL_SESSION_HASH_DATA->aLOG_COMMON.szModel[MAX_MODEL_LEN] = 0x00;
		}
#ifdef COMPARE
		Compare_LOG_COMMON("Compare after applying LOG_COMMON",&aaLOG_COMMON,&pCALL_SESSION_HASH_DATA->aLOG_COMMON);
#endif
	}
#ifdef COMPARE
	Compare_LOG_COMMON("Compare LOG_COMMON2 before memcpy to call",pLOG_COMMON,&pCALL_SESSION_HASH_DATA->aLOG_COMMON);
#endif
	/* INPUT data의 앞을 새롭게 채운다. 꼭 해주어야함. 중요함. */
	memcpy((char *) pLOG_COMMON,(char *) &pCALL_SESSION_HASH_DATA->aLOG_COMMON,LOG_COMMON_SIZE);   
	// It is important that this LOG should be sent to the CILOG. 
	// In CILOG , LOG will be written to the log_file.


//	LOG_CALL_TRANS Update
//	LOG_PAGE_TRANS is processing in Page_Process()
	{
		LOG_CALL_TRANS	aaLOG_CALL_TRANS;
		memcpy(&aaLOG_CALL_TRANS,pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS,LOG_CALL_TRANS_SIZE);
		ASSOCIATION_LOG_CALL_TRANS(pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS, type , data);
#ifdef COMPARE
		Compare_LOG_CALL_TRANS("Compare after applying CALL" , &aaLOG_CALL_TRANS,pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS);
#endif
	}

//	LOG_CALL_TRANS_Prt("PRINT LOG_CALL_TRANS", pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS);
}


U16 Make_Page_UserError(LOG_PAGE_TRANS *pLOG_PAGE_TRANS)
{
	int page_err = 0;
	int tcp_err = 0;
	int http_err = 0;


	// RespCode가  200번대의 성공이 아니면 1300 에러
	// RespCode가 304이면 성공으로 ...
	if(  (( (int) (pLOG_PAGE_TRANS->LastResCode/100) ) != 2) && 
			(pLOG_PAGE_TRANS->LastResCode != 304) ) {		
		page_err = 1300;
	}

	if(pLOG_PAGE_TRANS->LastL7FailCode > 0) {

		switch(pLOG_PAGE_TRANS->LastL4FailCode) {
			case LONGLAST_SYN_TRANS:	// TCP : timeout (LONGLAST)
			case LONGLAST_SYNACK_TRANS:	// TCP : timeout (LONGLAST)
			case LONGLAST_NOFIN_TRANS:	// TCP : timeout (LONGLAST)
				page_err = 1300;		// PAGE_ERR_BASE
				tcp_err = 50;			// PAGE_LONGLAST_ERR
				break;
			case TCP_ERR_RST_E1_SYN:	// TCP : Client Reset
			case TCP_ERR_RST_E1_SYNACK:	// TCP : Client Reset
			case TCP_ERR_RST_E1_NOFIN:	// TCP : Client Reset
				page_err = 1300;
				tcp_err = 40;			// PAGE_CLI_RST_ERR
				break;
			case TCP_ERR_RST_E2_SYN:	// TCP : Client Reset
			case TCP_ERR_RST_E2_SYNACK:	// TCP : Client Reset
			case TCP_ERR_RST_E2_NOFIN:	// TCP : Client Reset
				page_err = 1300;
				tcp_err = 20;			// PAGE_SER_RST_ERR
				break;
			case LONGLAST_FIN_E1:		// TCP : timeout (LONGLAST)
			case TCP_ERR_RST_E1_FIN_E1:	// TCP : Client Reset
			case TCP_ERR_RST_E2_FIN_E1:	// TCP : Client Reset
			case TCP_NOERR_FIN_E1:		// TCP : CLI-FIN
				page_err = 1300;
				tcp_err = 30;			// PAGE_CLI_FIN_ERR
				break;
			case LONGLAST_FIN_E2:		// TCP : timeout (LONGLAST)
			case TCP_ERR_RST_E1_FIN_E2:	// TCP : Client Reset
			case TCP_ERR_RST_E2_FIN_E2:	// TCP : Client Reset
			case TCP_NOERR_FIN_E2:		// TCP : SER-FIN
				page_err = 1300;
				tcp_err = 10;			// PAGE_SER_FIN_ERR
				break;
			case ABNORMAL_TRANS:		// TCP : Abnormal
				page_err = 1300;		
				tcp_err = 9;			// TCP ABNORMAL
				break;
			case TCP_SUCCESS:		// TCP : Data Sending
				break;
			default :
				log_print(LOGN_CRI, "ERROR NOT DEFINED TCP Error L4FailCode=%ld",
					pLOG_PAGE_TRANS->LastL4FailCode);
				break;
		}

		switch(pLOG_PAGE_TRANS->LastL7FailCode){
			case HTTP_UERR_900:		// 서버로부터 오류를 나타내는 응답 코드가 온 경우 (응답 코드 400 이상)
				page_err = 1300;
				if( (int) (pLOG_PAGE_TRANS->LastResCode/100) == 5)
					http_err = 5;
				else
					http_err = 6;
				break;
			case HTTP_UERR_910:		// Req 메시지가 완료되지 않고 Transaction이 종료된 경우
			case HTTP_UERR_911:		// Req 메시지가 예정된 크기보다 작게 전송된 후 그 다음 Req메시지가 전송된 경우
				page_err = 1300;
				http_err = 7;
				break;
			case HTTP_UERR_920:		// 완료된 Req 메시지에 대해 서버 측의 ACK가 전송되지 않은 경우
				page_err = 1300;
				http_err = 1;
				break;
			case HTTP_UERR_930:		// 서버 측의 ACK가 전송되었으나 응답메시지의 전송이 시작되지 않은 경우
				page_err = 1300;
				http_err = 2;
				break;
			case HTTP_UERR_940:		// 응답메시지가 시작된 후 응답메시지 전송이 완료되지 않고 Transaction이 종료된 경우
			case HTTP_UERR_941:		// Res 메시지가 예정된 크기보다 작게 전송된 후 그 다음 Res 메시지가 전송된 경우
				page_err = 1300;
				http_err = 3;
				break;
			case HTTP_UERR_950:		// 응답메시지가 완료되었지만 MN에서 ACK가 전송되지 않고 트랜잭션이 종료된 경우
				page_err = 1300;
				http_err = 4;
				break;
			case HTTP_UERR_960:		// Req 메시지에서 잘못된 Method 또는 알수 없는 Method가 있는 경우
				page_err = 1300;
				http_err = 8;
				break;
			case HTTP_UERR_970:		// Seq 번호가 맞지 않아서 정리된 경우
				page_err = 1300;
				http_err = 9;
				break;
			default :
				log_print(LOGN_CRI, "ERROR NOT DEFINED HTTP Error L7FailCode=%d",
					pLOG_PAGE_TRANS->LastL7FailCode);
				break;
		}

	}

	return (U16) (page_err + tcp_err + http_err);
}


int Page_Process( LOG_HTTP_TRANS *pLOG_HTTP_TRANS, BODY *pBODY)
{
	stHASHONODE     *pHASHONODE;
	struct timeval  stNowTime;
	LOG_COMMON *pLOG_COMMON;
	TAG_KEY_LOG_COMMON aTAG_KEY_LOG_COMMON , *pTAG_KEY_LOG_COMMON;
	CALL_SESSION_HASH_DATA *pCALL_SESSION_HASH_DATA;
	U8 *pPAGENODE;
	CALL_DUP_HASH_DATA		*pstCallDupList;
	int				CallIndex = 0;

	pLOG_COMMON = (LOG_COMMON *) pLOG_HTTP_TRANS;
	pTAG_KEY_LOG_COMMON = &aTAG_KEY_LOG_COMMON;
	pTAG_KEY_LOG_COMMON->uiClientIP = pLOG_COMMON->uiClientIP;

	if( (pHASHONODE = hasho_find(pHASHOINFO, (U8 *)pTAG_KEY_LOG_COMMON)) != NULL) { 
		log_print(LOGN_INFO, "%s : hasho_find",(char *) __FUNCTION__);
		pstCallDupList = (CALL_DUP_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		/* Next Call이 있을 경우 CallTime을 비교하여 Call 세션을 결정한다. */
		if(pstCallDupList->NextCallSessF) {
			if(pLOG_HTTP_TRANS->uiTcpSynTime >= pstCallDupList->NextCallTime) {
				CallIndex = 1;
			}
		}

		pCALL_SESSION_HASH_DATA = (CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[CallIndex]);

//		pCALL_SESSION_HASH_DATA = (CALL_SESSION_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
		if(pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY){
			log_print(LOGN_CRI, "%s : ERROR pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY must null. but, pInBODY is not NULL",(char *) __FUNCTION__);
		} else {
			pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY = pBODY;
			pCALL_SESSION_HASH_DATA->aPAGE_DATA.offset_InBODY = nifo_get_offset_node(pMEMSINFO, (U8 *)pBODY);
		}
		if(pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS == 0) {
			log_print(LOGN_INFO, "%s : alloc LOG_PAGE_TRANS",(char *) __FUNCTION__);
			/* PAGE NODE 할당 및 LOG_PAGE_TRANS TLV 할당 */
			if((pPAGENODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
				log_print(LOGN_CRI, "ERROR : [%s][%s.%d] pLOG_PAGE_TRANS nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
				pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY = NULL;
				pCALL_SESSION_HASH_DATA->aPAGE_DATA.offset_InBODY = 0;
				return -1;
			}
			if((pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS = (LOG_PAGE_TRANS *) nifo_tlv_alloc(pMEMSINFO, pPAGENODE, LOG_PAGE_TRANS_DEF_NUM, LOG_PAGE_TRANS_SIZE, DEF_MEMSET_ON)) == NULL) {
				log_print(LOGN_CRI, "ERROR : [%s][%s.%d] pLOG_PAGE_TRANS nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
				pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY = NULL;
				pCALL_SESSION_HASH_DATA->aPAGE_DATA.offset_InBODY = 0;
				nifo_node_delete(pMEMSINFO, pPAGENODE);
				return -2;
			}
	
			pCALL_SESSION_HASH_DATA->offset_PAGE = nifo_offset(pMEMSINFO, pPAGENODE);
			/* pLOG_PAGE_TRANS 생성시 초기화 */
			gettimeofday(&stNowTime, NULL);
			pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->OpStartTime = stNowTime.tv_sec;
			pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->OpStartMTime = stNowTime.tv_usec;

			// Page의 state의 초기화(INIT)    ==> 이 부분을 함수로 만들었으면 함.
			// 함수 안의 인자를 STS_NEW_PAGE로 놓으면 여기서부터 출발하며 자동으로 PAGE_STATE에 값이 들어감.
			pCALL_SESSION_HASH_DATA->PAGE_OLD_STATE = 0;
			pCALL_SESSION_HASH_DATA->PAGE_STATE = STS_NEW_PAGE;

			// send 할때 
			pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->PageID = pCALL_SESSION_HASH_DATA->PAGE_ID;
			pCALL_SESSION_HASH_DATA->PAGE_ID ++;

			memcpy((char *)pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS, &pCALL_SESSION_HASH_DATA->aLOG_COMMON, LOG_COMMON_SIZE);
		}

		pCALL_SESSION_HASH_DATA->LastPktTime = pLOG_HTTP_TRANS->uiLastPktTime;
		pCALL_SESSION_HASH_DATA->LastPktMTime = pLOG_HTTP_TRANS->uiLastPktMTime;

		flow_PAGE_state_go(pCALL_SESSION_HASH_DATA,LOG_HTTP_TRANS_DEF_NUM,sizeof(LOG_HTTP_TRANS),pLOG_HTTP_TRANS,pCALL_SESSION_HASH_DATA->PAGE_STATE,YES);
		// ??? log와 page관련된 정보는 여기서 모두 update합시다.
//		CALL_SESSION_HASH_DATA_Prt("after FLOW_PAGE : CALL_SESSION_HASH_DATA_Prt",pCALL_SESSION_HASH_DATA);

		// LOG_PAGE_TRANS Update
		{
			LOG_PAGE_TRANS	aaLOG_PAGE_TRANS;
			memcpy(&aaLOG_PAGE_TRANS,pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS,LOG_PAGE_TRANS_SIZE);
			ASSOCIATION_LOG_PAGE_TRANS(pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS, LOG_HTTP_TRANS_DEF_NUM, (S8 *)pLOG_HTTP_TRANS);

			pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->LastL7LastPktTime = pCALL_SESSION_HASH_DATA->LastPktTime;
			pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->LastL7LastPktMTime = pCALL_SESSION_HASH_DATA->LastPktMTime;
			pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->uiServerIP = pLOG_HTTP_TRANS->uiServerIP;
#ifdef COMPARE
			Compare_LOG_PAGE_TRANS("Compare after applying PAGE", &aaLOG_PAGE_TRANS,pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS);
#endif
			pLOG_HTTP_TRANS->uiPageID = pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->PageID;
		}

		if(pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY != NULL){
			//      pTHIS->aPAGE_DATA.pInBODY 메모리 해제
			U8 *__pNODE;
			__pNODE = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *) pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY));
			nifo_node_delete(pMEMSINFO, __pNODE);
			pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY = NULL;
			pCALL_SESSION_HASH_DATA->aPAGE_DATA.offset_InBODY = 0;
		}
	} else {
		log_print(LOGN_WARN, "ERROR : %s : call session is not exist. but , this http log comes in. we can't process this case. %d.%d.%d.%d",
				(char *) __FUNCTION__,HIPADDR(pTAG_KEY_LOG_COMMON->uiClientIP));
		return -3;
	}

	return 0;
}

char *getSigString(int type)
{
	switch(type) {
		case START_PI_DATA_RECALL_NUM:
			return "START_PI_DATA_RECALL_NUM";
		case START_RP_DATA_RECALL_NUM:
			return "START_RP_DATA_RECALL_NUM";
		case START_PI_SIG_RECALL_NUM:
			return "START_PI_SIG_RECALL_NUM";
		case START_RP_SIG_RECALL_NUM:
			return "START_RP_SIG_RECALL_NUM";
		case STOP_PI_RECALL_NUM:
			return "STOP_PI_RECALL_NUM";
		case STOP_RP_RECALL_NUM:
			return "STOP_RP_RECALL_NUM";
		default:
			return "UNKNOWN";
	}
}

/* 
   ETC_Process()
   Dial-up Internet Access
 */
void ETC_Process(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH)
{
	stHASHONODE				*pHASHONODE;
	TAG_KEY_LOG_COMMON 		aTAG_KEY_LOG_COMMON, *pTAG_KEY_LOG_COMMON;
	CALL_SESSION_HASH_DATA	*pCALL_SESSION_HASH_DATA;
	LOG_COMMON 				*pLOG_COMMON;
	CALL_DUP_HASH_DATA		*pstCallDupList;
	int						CallIndex = 0;
	IP4 					uiServerIP;


	log_print(LOGN_DEBUG, "RCV PKT RTX[%d] CIP[%d.%d.%d.%d] SIP[%d.%d.%d.%d] HEADERLEN[%d] DATALEN[%d]",
			pCAPHEAD->bRtxType, HIPADDR(pINFOETH->stIP.dwSrcIP), HIPADDR(pINFOETH->stIP.dwDestIP), 
			pINFOETH->stIP.wIPHeaderLen, pINFOETH->stUDPTCP.wDataLen);

	pTAG_KEY_LOG_COMMON = &aTAG_KEY_LOG_COMMON;
	if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
		pTAG_KEY_LOG_COMMON->uiClientIP = pINFOETH->stIP.dwSrcIP;
		uiServerIP = pINFOETH->stIP.dwDestIP;
	} else {
		pTAG_KEY_LOG_COMMON->uiClientIP = pINFOETH->stIP.dwDestIP;
		uiServerIP = pINFOETH->stIP.dwSrcIP;
	}

	if( (pHASHONODE = hasho_find(pHASHOINFO, (U8 *)pTAG_KEY_LOG_COMMON)) != NULL) {

		pstCallDupList = (CALL_DUP_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		/* Next Call이 있을 경우 CallTime을 비교하여 Call 세션을 결정한다. */
		if(pstCallDupList->NextCallSessF) {
			if(pCAPHEAD->curtime >= pstCallDupList->NextCallTime) {
				CallIndex = 1;
			}
		}

		pCALL_SESSION_HASH_DATA = (CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[CallIndex]);

//		pCALL_SESSION_HASH_DATA = (CALL_SESSION_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		if( pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_INIT ) {
			guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
			pCALL_SESSION_HASH_DATA->timerNID = 
				timerN_update(pTIMERNINFO, pCALL_SESSION_HASH_DATA->timerNID, time(NULL) + guiTimerValue);
			log_print(LOGN_INFO, "UPDATE EXIST TIMER IMSI[%s] STOPFLAG[%d] TIMEOUT[%d]", 
					pCALL_SESSION_HASH_DATA->aLOG_COMMON.szIMSI, pCALL_SESSION_HASH_DATA->isStopFlag, guiTimerValue);
		}
	
		/* Checking Platform Type & L4 Type */
		pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->usPlatformType = DEF_PLATFORM_PHONE;

		if (pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->usSvcL4Type == 0)
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->usSvcL4Type = L4_PHONE_ETC;

		log_print(LOGN_DEBUG, "	FOUND DIALUP CIP[%d.%d.%d.%d] SIP[%d.%d.%d.%d] IMSI[%s]	TIME[%u.%u]", 
				HIPADDR(pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->uiClientIP), 
				HIPADDR(uiServerIP), 
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->szIMSI, 
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->uiCallTime, 
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->uiCallMTime);

		if(!pCALL_SESSION_HASH_DATA->isServiceFlag) {
			dSend_Service_Start_Signal(pCALL_SESSION_HASH_DATA, pCAPHEAD->curtime, pCAPHEAD->ucurtime);
			pCALL_SESSION_HASH_DATA->isServiceFlag = 1;
		}

		if(pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS != 0) {
			if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {	// UP
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->UpPktCnt++;
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->UpPktSize += pINFOETH->stIP.wTotalLength;
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->UpDataSize += pINFOETH->stUDPTCP.wDataLen;
			} else {	// DOWN
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->DnPktCnt++;
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->DnPktSize += pINFOETH->stIP.wTotalLength;
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->DnDataSize += pINFOETH->stUDPTCP.wDataLen;
			}
			if( pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->SessStartTime == 0 ) {
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->SessStartTime = pCAPHEAD->curtime;
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->SessStartMTime = pCAPHEAD->ucurtime;
			}
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->LastPktTime = pCAPHEAD->curtime;
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->LastPktMTime = pCAPHEAD->ucurtime;
		} else {
			log_print(LOGN_CRI, "ERROR : %s : DIALUP ACCESS SESSION EXIST BUT pLOG_DIALUP_SESS IS NOT EXIST IP:%d.%d.%d.%d",
					(char *) __FUNCTION__, HIPADDR(pTAG_KEY_LOG_COMMON->uiClientIP));
		}

		if(pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS != 0) {
			pLOG_COMMON = (LOG_COMMON *) pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS;
//			LOG_COMMON_Prt("VERIFY LOG_CALL_TRANS", pLOG_COMMON);
		}
		if(pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS != 0){
			pLOG_COMMON = (LOG_COMMON *) pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS;
//			LOG_COMMON_Prt("VERIFY LOG_PAGE_TRANS", pLOG_COMMON);
		} 
	} else {
		log_print(LOGN_INFO, "INFO : %s : DIALUP ACCESS is not exist IP:%d.%d.%d.%d",
				(char *) __FUNCTION__, HIPADDR(pTAG_KEY_LOG_COMMON->uiClientIP));
	}
}

/* 
   INET_Process()
   Dial-up Internet Access for INETSESS
 */
void INET_Process(LOG_INET *pINETSESS)
{
	stHASHONODE				*pHASHONODE;
	TAG_KEY_LOG_COMMON 		aTAG_KEY_LOG_COMMON, *pTAG_KEY_LOG_COMMON;
	CALL_SESSION_HASH_DATA	*pCALL_SESSION_HASH_DATA;
	LOG_COMMON 				*pLOG_COMMON;
	CALL_DUP_HASH_DATA		*pstCallDupList;
	int						CallIndex = 0;
	IP4 					uiServerIP;


	log_print(LOGN_DEBUG, "RCV INET CIP[%d.%d.%d.%d] SIP[%d.%d.%d.%d] UPLEN[%u] DNLEN[%u]",
			HIPADDR(pINETSESS->uiClientIP), HIPADDR(pINETSESS->uiServerIP), 
			pINETSESS->uiUpPacketSize, pINETSESS->uiDnPacketSize);

	pTAG_KEY_LOG_COMMON = &aTAG_KEY_LOG_COMMON;
	pTAG_KEY_LOG_COMMON->uiClientIP = pINETSESS->uiClientIP;
	uiServerIP = pINETSESS->uiServerIP;

	if( (pHASHONODE = hasho_find(pHASHOINFO, (U8 *)pTAG_KEY_LOG_COMMON)) != NULL) {

		pstCallDupList = (CALL_DUP_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		/* Next Call이 있을 경우 CallTime을 비교하여 Call 세션을 결정한다. */
		if(pstCallDupList->NextCallSessF) {
			if(pINETSESS->uiFirstPktTime >= pstCallDupList->NextCallTime) {
				CallIndex = 1;
			}
		}

		pCALL_SESSION_HASH_DATA = (CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[CallIndex]);

//		pCALL_SESSION_HASH_DATA = (CALL_SESSION_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		if( pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_INIT ) {
			guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
			pCALL_SESSION_HASH_DATA->timerNID = 
				timerN_update(pTIMERNINFO, pCALL_SESSION_HASH_DATA->timerNID, time(NULL) + guiTimerValue);
			log_print(LOGN_INFO, "UPDATE EXIST TIMER IMSI[%s] STOPFLAG[%d] TIMEOUT[%d]", 
					pCALL_SESSION_HASH_DATA->aLOG_COMMON.szIMSI, pCALL_SESSION_HASH_DATA->isStopFlag, guiTimerValue);
		}
	
		/* Checking Platform Type & L4 Type */
		pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->usPlatformType = DEF_PLATFORM_PHONE;

		if (pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->usSvcL4Type == 0)
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->usSvcL4Type = L4_PHONE_ETC;

		log_print(LOGN_DEBUG, "	FOUND DIALUP CIP[%d.%d.%d.%d] SIP[%d.%d.%d.%d] IMSI[%s]	TIME[%u.%u]", 
				HIPADDR(pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->uiClientIP), 
				HIPADDR(uiServerIP), 
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->szIMSI, 
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->uiCallTime, 
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->uiCallMTime);
#if 0
		if(!pCALL_SESSION_HASH_DATA->isServiceFlag) {
			dSend_Service_Start_Signal(pCALL_SESSION_HASH_DATA, pCAPHEAD->curtime, pCAPHEAD->ucurtime);
			pCALL_SESSION_HASH_DATA->isServiceFlag = 1;
		}
#endif

		if(pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS != 0) {

			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->UpPktCnt += pINETSESS->uiUpPacketCnt;
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->UpPktSize += pINETSESS->uiUpPacketSize;
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->UpDataSize += pINETSESS->uiUpPacketSize;

			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->DnPktCnt += pINETSESS->uiDnPacketCnt;
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->DnPktSize += pINETSESS->uiDnPacketSize;
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->DnDataSize += pINETSESS->uiDnPacketSize;

			if( pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->SessStartTime == 0 ) {
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->SessStartTime = pINETSESS->uiFirstPktTime;
				pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->SessStartMTime = pINETSESS->uiFirstPktMTime;
			}
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->LastPktTime = pINETSESS->uiLastPktTime;
			pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS->LastPktMTime = pINETSESS->uiLastPktMTime;
		} else {
			log_print(LOGN_CRI, "ERROR : %s : DIALUP ACCESS SESSION EXIST BUT pLOG_DIALUP_SESS IS NOT EXIST IP:%d.%d.%d.%d",
					(char *) __FUNCTION__, HIPADDR(pTAG_KEY_LOG_COMMON->uiClientIP));
		}

		if(pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS != 0) {
			pLOG_COMMON = (LOG_COMMON *) pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS;
//			LOG_COMMON_Prt("VERIFY LOG_CALL_TRANS", pLOG_COMMON);
		}
		if(pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS != 0){
			pLOG_COMMON = (LOG_COMMON *) pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS;
//			LOG_COMMON_Prt("VERIFY LOG_PAGE_TRANS", pLOG_COMMON);
		} 
	} else {
		log_print(LOGN_INFO, "INFO : %s : DIALUP ACCESS is not exist IP:%d.%d.%d.%d",
				(char *) __FUNCTION__, HIPADDR(pTAG_KEY_LOG_COMMON->uiClientIP));
	}
}

/**
 *  $Log: call_func.c,v $
 *  Revision 1.2  2011/09/04 08:04:25  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/18 04:18:30  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/17 07:15:03  dcham
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.11  2011/05/11 23:11:08  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.10  2011/04/30 19:50:57  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.9  2011/04/24 21:19:21  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.8  2011/04/24 16:37:02  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.7  2011/04/24 15:24:55  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.6  2011/01/11 04:09:05  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.5  2009/09/14 17:20:27  pkg
 *  NEXT CALL이 생성되었을 때 START SERVICE SIGNAL이 전송되지 않은 버그 수정
 *
 *  Revision 1.4  2009/08/16 16:13:07  jsyoon
 *  REMOVE COMPARE DEBUG_LOG
 *
 *  Revision 1.3  2009/08/04 12:08:17  dqms
 *  TIMER를 공유메모리로 변경
 *
 *  Revision 1.2  2009/08/01 05:40:02  dqms
 *  TIMER 함수 호출 할 때 CallTime 비교하여 현재 콜 선택
 *
 *  Revision 1.1  2009/07/31 06:17:35  jsyoon
 *  RADIUS Continue Session 처리
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:22  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.1  2007/08/21 12:52:37  dark264sh
 *  no message
 *
 *  Revision 1.2  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.1  2006/10/20 10:00:53  dark264sh
 *  *** empty log message ***
 *
 */
