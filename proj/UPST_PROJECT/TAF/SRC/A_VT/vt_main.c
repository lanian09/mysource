/**		@file	vt_main.c
 * 		- VT 정보를 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: vt_main.c,v 1.2 2011/09/06 12:46:40 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:40 $
 * 		@warning	.
 * 		@ref		vt_main.c vt_init.c
 * 		@todo		VT의 RTP 패킷은 어떻게 필터링 할 것인가?
 *
 * 		@section	Intro(소개)
 * 		- VT 정보를 관리 하는 프로세스
 *
 **/

/**
 *	Include headers
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "sshmid.h"
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "filter.h"
#include "capdef.h"

// LIB
#include "config.h"
#include "typedef.h"
#include "Analyze_Ext_Abs.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

// .
#include "vt_init.h"
#include "vt_msgq.h"

/**
 *	Declare var.
 */
S32				giFinishSignal;			/**< Finish Signal */
S32				giStopFlag;				/**< main loop Flag 0: Stop, 1: Loop */
stMEMSINFO		*gpMEMSINFO;		/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;
stHASHOINFO		*gpHASHOINFO;		/**< new interface 관리 구조체 */
stTIMERNINFO	*gpTIMERNINFO;		/**< new interface 관리 구조체 */
st_Flt_Info		*flt_info;
int				guiTimerValue;
int				guiTimerClose;

extern int    	gAINETCnt;
extern int    	gACALLCnt;

/**
 *	Declare func.
 */
S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime);
U8 *PrintTYPE(S32 type);
U8 *PrintTcpFlag(S32 tcpflag);
S32 Send_VT_Session_LOG(VT_SESSION_HASH_DATA *pstVTSESSDATA);
void invoke_del_CALL(void *p);
void VT_Sess_Init(LOG_VT_SESS *pLOGVTSESS, LOG_SIP_TRANS *pLOGSIPTRANS);
int dCloseVTSess(LOG_VT_SESS *pLOGVTSESS);
LOG_VT_SESS *VT_Session_Process( int type, int len, char *data, int *dSndIMF);
int VT_Session_Update(char *data);
void dInitDupList( st_DupList *pstList );
int dSearchDupList( st_DupList *pstList, unsigned short usIdentification, unsigned short usFragmentOffset );
int RTP_Process(OFFSET offset, Capture_Header_Msg *pCAPHEAD ,INFO_ETH *pINFOETH);
int dRtpQosJitter(UINT uiTimestamp, VT_SESSION_HASH_DATA *pData, Capture_Header_Msg *pstCAPHEAD, INFO_ETH *pInfoEth);
int dRtpQosLoss(UINT uiSequence, VT_SESSION_HASH_DATA *pData, Capture_Header_Msg *pstCAPHEAD, INFO_ETH *pInfoEth);

/**
 *	Implement func.
 */

/** dHttpInit function.
 *
 *  dHttpInit Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime)
{
	S64		gapTime;

	gapTime = (((S64)endtime * 1000000 + (S64)endmtime) - ((S64)starttime * 1000000 + (S64)startmtime));

	if(gapTime < 0)
		gapTime = 0;

	return gapTime;
}


U8 *PrintTYPE(S32 type)
{
	switch(type)
	{
	case LOG_HTTP_TRANS_DEF_NUM: 		return "LOG_HTTP_TRANS";
	case LOG_TCP_SESS_DEF_NUM: 			return "LOG_TCP_SESS";
	case TCP_INFO_DEF_NUM: 				return "TCP_INFO";
	case BODY_DEF_NUM: 					return "BODY";
	case LOG_RTSP_TRANS_DEF_NUM:		return "LOG_RTSP_TRANS";
	case LOG_PAGE_TRANS_DEF_NUM:		return "LOG_PAGE_TRANS";
	case LOG_CALL_TRANS_DEF_NUM:		return "LOG_CALL_TRANS";
	case LOG_JNC_TRANS_DEF_NUM:			return "LOG_JNC_TRANS";
	case LOG_VOD_SESS_DEF_NUM:			return "LOG_VOD_SESS";
	default: 							return "UNKNOWN";
	}
}

U8 *PrintTcpFlag(S32 tcpflag)
{
	switch(tcpflag)
	{
	case DEF_TCP_START: 				return "TCP_START";
	case DEF_TCP_END: 					return "TCP_END";
	default: 							return "UNKNOWN";
	}
}	

#if 0
S32 dGetCALLQID(U32 uiClientIP)
{
	S32		idx = 0;

	idx = uiClientIP % gACALLCnt;

	return dCALLQID[idx];
}
#endif

S32 Send_VT_Session_LOG(VT_SESSION_HASH_DATA *pstVTSESSDATA)
{
	S32				dRet;
	//S32				qid;
	S32				dSeqProcID;
	OFFSET 			offset;

	LOG_VT_SESS		*pLOG_VT_SESS;

	struct timeval  stNowTime;
	U8				*pLOGVTNODE;

	offset = pstVTSESSDATA->offset;
	pLOG_VT_SESS = pstVTSESSDATA->pLOG_VT_SESS;
	if(!pLOG_VT_SESS) return 0;

	log_print(LOGN_DEBUG, "@@@ Send_VT_SESS CIP[%d.%d.%d.%d]", HIPADDR(pLOG_VT_SESS->uiClientIP) );

	/* pLOG_VT_SESS가 끝날때 수행하는 부분 */
	gettimeofday(&stNowTime, NULL);
	pLOG_VT_SESS->OpEndTime = stNowTime.tv_sec;
	pLOG_VT_SESS->OpEndMTime = stNowTime.tv_usec;

	pLOG_VT_SESS->SessGapTime = GetGapTime(pLOG_VT_SESS->LastPktTime, pLOG_VT_SESS->LastPktMTime, 
			pLOG_VT_SESS->SessStartTime, pLOG_VT_SESS->SessStartMTime);

	if(pLOG_VT_SESS->LastUserErrCode == 0) {
		if(pLOG_VT_SESS->RTPUpCnt == 0 && pLOG_VT_SESS->RTPDnCnt == 0) {
			pLOG_VT_SESS->LastUserErrCode = VT_UERR_NOPLAY;
		}
		else if(pLOG_VT_SESS->LastMethod != SIP_MSG_BYE) {
			pLOG_VT_SESS->LastUserErrCode = VT_UERR_TIMEOUT;
		}
	}

	LOG_VT_SESS_Prt("VT SESSION LOG ", pLOG_VT_SESS);
//	pLOGVTNODE = nifo_ptr(gpMEMSINFO, offset);
	pLOGVTNODE = nifo_ptr(gpMEMSINFO, nifo_get_offset_node(gpMEMSINFO, (U8 *)pstVTSESSDATA->pLOG_VT_SESS));

	//qid = dGetCALLQID(pLOG_VT_SESS->uiClientIP);
	dSeqProcID = SEQ_PROC_A_CALL + (pLOG_VT_SESS->uiClientIP % gACALLCnt);
	//if((dRet = dSend_VT_Data(gpMEMSINFO, qid, pLOGVTNODE)) < 0) {
	if((dRet = dSend_VT_Data(gpMEMSINFO, dSeqProcID, pLOGVTNODE)) < 0) {
		log_print(LOGN_CRI, LH"MSGQ WRITE FAILE[%d][%s]", LT, dRet, strerror(-dRet));
		return -5;
	}
	pstVTSESSDATA->pLOG_VT_SESS = 0;

	return 0;
}


void invoke_del_CALL(void *p)
{
	st_CALLTimer  			*pstCALLTimer;
	VT_SESSION_HASH_DATA 	*pstVTSESSDATA;
	stHASHONODE				*pHASHNODE;

	pstCALLTimer = (st_CALLTimer *) p;

	log_print(LOGN_DEBUG, "@@@ TIMER TIMEOUT CIP[%d.%d.%d.%d]", HIPADDR(pstCALLTimer->ClientIP) );

	if((pHASHNODE = hasho_find(gpHASHOINFO, (U8 *) &pstCALLTimer->ClientIP)) != NULL) { 
		pstVTSESSDATA = (VT_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHNODE->offset_Data);
		log_print(LOGN_DEBUG, "@@@ INVOKE TIMEOUT CIP[%d.%d.%d.%d]   NTIME[%ld]", HIPADDR(pstCALLTimer->ClientIP) , time(NULL) );

		Send_VT_Session_LOG(pstVTSESSDATA);
		if(pstVTSESSDATA->pLOG_VT_SESS){
			U8 *__pNODE;
			__pNODE = nifo_ptr(gpMEMSINFO, nifo_get_offset_node(gpMEMSINFO, (U8 *)pstVTSESSDATA->pLOG_VT_SESS));
			nifo_node_delete(gpMEMSINFO, __pNODE);
			pstVTSESSDATA->pLOG_VT_SESS = NULL;
		}
		hasho_del(gpHASHOINFO, (U8 *) &pstCALLTimer->ClientIP);
	} else {
		log_print(LOGN_CRI, "INVOKE TIMEOUT BUT NODE NULL CIP[%d.%d.%d.%d]   NTIME[%ld]", HIPADDR(pstCALLTimer->ClientIP) , time(NULL) );
	}
}

void VT_Sess_Init(LOG_VT_SESS *pLOGVTSESS, LOG_SIP_TRANS *pLOGSIPTRANS) 
{
	struct timeval  stNowTime;

	gettimeofday(&stNowTime, NULL);

	memcpy(pLOGVTSESS, pLOGSIPTRANS, LOG_COMMON_SIZE);

	pLOGVTSESS->LastMethod 		= pLOGSIPTRANS->method;
	memcpy(&pLOGVTSESS->CallID[0], &pLOGSIPTRANS->CallID[0], SIP_CALLID_LEN);
	pLOGVTSESS->CallID[SIP_CALLID_LEN] = 0x00;


	pLOGVTSESS->SessStartTime 	= pLOGSIPTRANS->TransStartTime;
	pLOGVTSESS->SessStartMTime 	= pLOGSIPTRANS->TransStartMTime;
	pLOGVTSESS->LastPktTime 	= pLOGSIPTRANS->TransEndTime;
	pLOGVTSESS->LastPktMTime 	= pLOGSIPTRANS->TransEndMTime;

	pLOGVTSESS->ClientPort		= pLOGSIPTRANS->SrcPort;
	pLOGVTSESS->ServerIP		= pLOGSIPTRANS->DestIP;
	pLOGVTSESS->ServerPort		= pLOGSIPTRANS->DestPort;

	pLOGVTSESS->LastResCode 	= pLOGSIPTRANS->ResCode;			/* 200 OK */

	pLOGVTSESS->usPlatformType 	= DEF_PLATFORM_VT;
	pLOGVTSESS->usSvcL4Type 	= L4_VT;
	pLOGVTSESS->usSvcL7Type 	= pLOGSIPTRANS->usSvcL7Type;

	switch(pLOGSIPTRANS->ResCode)                                
	{
		case 200:
		case 301:   /* Moved Permanently */
		case 402:   /* Payment Required */
		case 405:   /* Method Not Allowed */
		case 406:   /* Not Acceptable */
		case 415:   /* Unsupported Media Type */
		case 484:   /* Address Incomplete */
		case 486:   /* Busy here : 통화중 */
		case 487:   /* Request terminated : 발신 취소 */
		case 488:   /* Not Acceptable Here */
		case 603:   /* Decline : 수신자 거절 */
		case 606:   /* Not Acceptable */
			pLOGVTSESS->SetupEndTime 	= pLOGSIPTRANS->TransEndTime; 			
			pLOGVTSESS->SetupEndMTime 	= pLOGSIPTRANS->TransEndMTime;
			break;
		default:
			pLOGVTSESS->L7FailCode = VT_UERR_NOSETUP;
			pLOGVTSESS->LastUserErrCode = pLOGVTSESS->L7FailCode;
			break;
	}

	pLOGVTSESS->AudioPort 		= pLOGSIPTRANS->AudioPort;
	pLOGVTSESS->VideoPort 		= pLOGSIPTRANS->VideoPort;
	memcpy(&pLOGVTSESS->AudioProto[0], &pLOGSIPTRANS->AudioProto[0], SIP_PROTO_LEN);
	memcpy(&pLOGVTSESS->VideoProto[0], &pLOGSIPTRANS->VideoProto[0], SIP_PROTO_LEN);

	/* SET DATA TRAFFIC SIZE */
	pLOGVTSESS->TotalReqCnt				+= pLOGSIPTRANS->TotalReqCnt;
	pLOGVTSESS->TotalResCnt				+= pLOGSIPTRANS->TotalResCnt;
	pLOGVTSESS->SkipResCnt				+= pLOGSIPTRANS->SkipResCnt;
	pLOGVTSESS->RetransReqCnt			+= pLOGSIPTRANS->RetransReqCnt;
	pLOGVTSESS->RetransResCnt			+= pLOGSIPTRANS->RetransResCnt;

	pLOGVTSESS->uiIPDataUpPktCnt		+= pLOGSIPTRANS->TotalReqCnt;
	pLOGVTSESS->uiIPDataDnPktCnt		+= pLOGSIPTRANS->TotalResCnt;
	pLOGVTSESS->uiIPTotUpPktCnt			+= pLOGSIPTRANS->TotalReqCnt;
	pLOGVTSESS->uiIPTotDnPktCnt			+= pLOGSIPTRANS->TotalResCnt;
	pLOGVTSESS->uiIPDataUpRetransCnt	+= pLOGSIPTRANS->RetransReqCnt;
	pLOGVTSESS->uiIPDataDnRetransCnt	+= pLOGSIPTRANS->RetransResCnt;
	pLOGVTSESS->uiIPTotUpRetransCnt		+= pLOGSIPTRANS->RetransReqCnt;
	pLOGVTSESS->uiIPTotDnRetransCnt		+= pLOGSIPTRANS->RetransResCnt;

	pLOGVTSESS->uiIPDataUpPktSize		+= pLOGSIPTRANS->ReqDataSize;
	pLOGVTSESS->uiIPDataDnPktSize		+= pLOGSIPTRANS->ResDataSize;
	pLOGVTSESS->uiIPTotUpPktSize		+= pLOGSIPTRANS->ReqIPDataSize;
	pLOGVTSESS->uiIPTotDnPktSize		+= pLOGSIPTRANS->ResIPDataSize;

	pLOGVTSESS->OpStartTime = stNowTime.tv_sec;
	pLOGVTSESS->OpStartMTime = stNowTime.tv_usec;

	return;
}

int dCloseVTSess(LOG_VT_SESS *pLOGVTSESS)
{
	stHASHONODE				*pHASHNODE;
	VT_SESSION_HASH_DATA 	*pstVTSESSDATA;

	log_print(LOGN_DEBUG, "	CLOSE VT SESSION CIP[%d.%d.%d.%d]", HIPADDR(pLOGVTSESS->uiClientIP) );

	if((pHASHNODE = hasho_find(gpHASHOINFO, (U8 *) &pLOGVTSESS->uiClientIP)) != NULL) {
		pstVTSESSDATA = (VT_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHNODE->offset_Data);

		/* BYE 이후에 오는 RTP트래픽을 위해 1초의 타임아웃 시간을 대기한다 */
		guiTimerClose = flt_info->stTimerInfo.usTimerInfo[PI_TCP_RSTWAIT];
		pstVTSESSDATA->timerNID = 
			timerN_update(gpTIMERNINFO, pstVTSESSDATA->timerNID, time(NULL) + guiTimerClose);
#if 0
		Send_VT_Session_LOG(pstVTSESSDATA);
		if(pstVTSESSDATA->pLOG_VT_SESS) {
			U8 *pDELNode;

			pDELNode = nifo_ptr(gpMEMSINFO, pstVTSESSDATA->offset);
			nifo_node_delete(gpMEMSINFO, pDELNode);
			pstVTSESSDATA->pLOG_VT_SESS = NULL;
		}
		timerN_del(gpTIMERNINFO, pstVTSESSDATA->timerNID);
		hasho_del(gpHASHOINFO, (U8 *) &pLOGVTSESS->uiClientIP);
#endif
	}
	return 1;
}


LOG_VT_SESS *VT_Session_Process( int type, int len, char *data, int *dSndIMF)
{
	st_SIPSessKey 	stKey;
	st_SIPSessKey 	*pstKey;
	LOG_SIP_TRANS 	*pLOGSIPTRANS;
	LOG_VT_SESS 	*pLOGVTSESS = NULL;

	stHASHONODE     		*pHASHONODE;
	VT_SESSION_HASH_DATA 	stVTSESSDATA;
	VT_SESSION_HASH_DATA 	*pstVTSESSDATA;

	st_CALLTimer			stCALLTimer, *pstCALLTimer;

	UCHAR		*pNode;


	pstCALLTimer = &stCALLTimer;

	pLOGSIPTRANS = (LOG_SIP_TRANS *) data;

	memset(&stKey, 0, sizeof(st_SIPSessKey));
	pstKey = &stKey;

	pstKey->uiClientIP = pLOGSIPTRANS->uiClientIP;
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_VT_TIMEOUT];


	log_print(LOGN_DEBUG, "RCV LOG TYPE[%d][%s] LEN[%d] IP[%d.%d.%d.%d]", 
			type, PRINT_DEF_NUM_table_log(type), len, HIPADDR(pLOGSIPTRANS->uiClientIP));

	/* 1. 기존 세션이 있으면 */
	if( (pHASHONODE = hasho_find(gpHASHOINFO, (U8 *)pstKey)) ) {

		pstVTSESSDATA = (VT_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
		pLOGVTSESS = pstVTSESSDATA->pLOG_VT_SESS;

		/* 필터에 L4_SIP_CSCF 로 등록되어 있으므로 VT 타입으로 변경 */
		pLOGSIPTRANS->usPlatformType = DEF_PLATFORM_VT;
		pLOGSIPTRANS->usSvcL4Type = L4_VT;

		if( pLOGSIPTRANS->method == SIP_MSG_INVITE && 
				strcmp(pLOGVTSESS->CallID, pLOGSIPTRANS->CallID) != 0 ) {
			/* 1.2 CALLID가 다른 경우 이전 세션을 정리하고 새로운 세션을 생성한다 */

			Send_VT_Session_LOG(pstVTSESSDATA);
			if(pstVTSESSDATA->pLOG_VT_SESS) {
				U8 *pDELNode;

				pDELNode = nifo_ptr(gpMEMSINFO, pstVTSESSDATA->offset);
				nifo_node_delete(gpMEMSINFO, pDELNode);
				pstVTSESSDATA->pLOG_VT_SESS = NULL;
			}
			timerN_del(gpTIMERNINFO, pstVTSESSDATA->timerNID);
			hasho_del(gpHASHOINFO, (U8 *) &pstKey->uiClientIP);

			log_print(LOGN_DEBUG, "	SESSION EXSIT, BUT NOT SAME CALL-ID PREV[%s] CURR[%s]", pLOGVTSESS->CallID, pLOGSIPTRANS->CallID);
	

			/* 1.3 Create New VT Session */
			memset((char *)&stVTSESSDATA, 0x0, sizeof(VT_SESSION_HASH_DATA));
			pstVTSESSDATA = &stVTSESSDATA;

			if((pHASHONODE = hasho_add(gpHASHOINFO, (U8 *)pstKey, (U8 *)pstVTSESSDATA)) == NULL) {
				log_print(LOGN_CRI, LH"ERROR : hasho_add NULL", LT);
				return NULL;
			} else {
				pstVTSESSDATA = (VT_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
				memset(pstVTSESSDATA, 0x00, VT_SESSION_HASH_DATA_SIZE);

				dInitDupList( &pstVTSESSDATA->stUpList );
				dInitDupList( &pstVTSESSDATA->stDownList );

				pstCALLTimer->ClientIP = pLOGSIPTRANS->uiClientIP;
				pstVTSESSDATA->timerNID = timerN_add(gpTIMERNINFO, invoke_del_CALL, (U8 *)pstCALLTimer, sizeof(st_CALLTimer), time(NULL) + guiTimerValue);

				/* LOG_VT_SESS TLV 할당 */
				if((pNode = nifo_node_alloc(gpMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, LH"ERROR pLOG_VT_SESS nifo_node_alloc NULL", LT);
					return NULL;
				}
				if((pstVTSESSDATA->pLOG_VT_SESS = (LOG_VT_SESS *) nifo_tlv_alloc(gpMEMSINFO, pNode, LOG_VT_SESS_DEF_NUM, LOG_VT_SESS_SIZE, DEF_MEMSET_ON)) == NULL) {
					log_print(LOGN_CRI, LH"ERROR pLOG_VT_SESS nifo_tlv_alloc NULL", LT);
					nifo_node_unlink_nont(gpMEMSINFO, pNode);
					nifo_cont_delete(gpMEMSINFO, pNode);
					return NULL;
				}
				pLOGVTSESS = pstVTSESSDATA->pLOG_VT_SESS;

				pstVTSESSDATA->offset = nifo_offset (gpMEMSINFO, pstVTSESSDATA->pLOG_VT_SESS);

				/* pLOG_VT_SESS 생성시 초기화 */
				VT_Sess_Init(pLOGVTSESS, pLOGSIPTRANS);

				pLOGSIPTRANS->isUsed = 1;

				log_print(LOGN_INFO, "	CREATE NEW VT SESSION : IP[%d.%d.%d.%d] METHOD[%d]", 
						HIPADDR(pstKey->uiClientIP), pLOGSIPTRANS->method);
			}
		} else {
			if(pLOGSIPTRANS->method == SIP_MSG_INVITE) {
				pLOGVTSESS->SetupEndTime    = pLOGSIPTRANS->AckTime;	/* INVITE -> Trying -> Ringing -> 200OK -> ACK */
				pLOGVTSESS->SetupEndMTime   = pLOGSIPTRANS->AckMTime;
			}
			
			/* TRAFFIC */
			pLOGVTSESS->TotalReqCnt				+= pLOGSIPTRANS->TotalReqCnt;
			pLOGVTSESS->TotalResCnt				+= pLOGSIPTRANS->TotalResCnt;
			pLOGVTSESS->SkipResCnt				+= pLOGSIPTRANS->SkipResCnt;
			pLOGVTSESS->RetransReqCnt			+= pLOGSIPTRANS->RetransReqCnt;
			pLOGVTSESS->RetransResCnt			+= pLOGSIPTRANS->RetransResCnt;

			pLOGVTSESS->uiIPDataUpPktCnt		+= pLOGSIPTRANS->TotalReqCnt;
			pLOGVTSESS->uiIPDataDnPktCnt		+= pLOGSIPTRANS->TotalResCnt;
			pLOGVTSESS->uiIPTotUpPktCnt			+= pLOGSIPTRANS->TotalReqCnt;
			pLOGVTSESS->uiIPTotDnPktCnt			+= pLOGSIPTRANS->TotalResCnt;
			pLOGVTSESS->uiIPDataUpRetransCnt	+= pLOGSIPTRANS->RetransReqCnt;
			pLOGVTSESS->uiIPDataDnRetransCnt	+= pLOGSIPTRANS->RetransResCnt;
			pLOGVTSESS->uiIPTotUpRetransCnt		+= pLOGSIPTRANS->RetransReqCnt;
			pLOGVTSESS->uiIPTotDnRetransCnt		+= pLOGSIPTRANS->RetransResCnt;

			pLOGVTSESS->uiIPDataUpPktSize		+= pLOGSIPTRANS->ReqDataSize;
			pLOGVTSESS->uiIPDataDnPktSize		+= pLOGSIPTRANS->ResDataSize;
			pLOGVTSESS->uiIPTotUpPktSize		+= pLOGSIPTRANS->ReqIPDataSize;
			pLOGVTSESS->uiIPTotDnPktSize		+= pLOGSIPTRANS->ResIPDataSize;


			pLOGVTSESS->LastMethod		= pLOGSIPTRANS->method;
			pLOGVTSESS->LastPktTime 	= pLOGSIPTRANS->TransEndTime;
			pLOGVTSESS->LastPktMTime 	= pLOGSIPTRANS->TransEndMTime;

			pLOGVTSESS->LastResCode     = pLOGSIPTRANS->ResCode;            	
//			pLOGVTSESS->LastUserErrCode	= pLOGSIPTRANS->LastUserErrCode;
			
			pstVTSESSDATA->timerNID = 
				timerN_update(gpTIMERNINFO, pstVTSESSDATA->timerNID, time(NULL) + guiTimerValue);

			pLOGSIPTRANS->isUsed = 1;

			log_print(LOGN_DEBUG, "	UPDATE EXIST VT SESSION : IP[%d.%d.%d.%d] METHOD[%d]", 
					HIPADDR(pstKey->uiClientIP), pLOGSIPTRANS->method);
		}
	} else {
		/* 2. 기존 세션이 없으면 */
		if ( pLOGSIPTRANS->method == SIP_MSG_INVITE ) {
			memset((char *)&stVTSESSDATA, 0x0, sizeof(VT_SESSION_HASH_DATA));
			pstVTSESSDATA = &stVTSESSDATA;

			if((pHASHONODE = hasho_add(gpHASHOINFO, (U8 *)pstKey, (U8 *)pstVTSESSDATA)) == NULL) {
				log_print(LOGN_CRI, LH"ERROR : hasho_add NULL", LT);
				return NULL;
			} else {
				pstVTSESSDATA = (VT_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
				memset(pstVTSESSDATA, 0x00, VT_SESSION_HASH_DATA_SIZE);

				dInitDupList( &pstVTSESSDATA->stUpList );
				dInitDupList( &pstVTSESSDATA->stDownList );

				pstCALLTimer->ClientIP = pLOGSIPTRANS->uiClientIP;
				pstVTSESSDATA->timerNID = timerN_add(gpTIMERNINFO, invoke_del_CALL, (U8 *)pstCALLTimer, sizeof(st_CALLTimer), time(NULL) + guiTimerValue);

				/* LOG_VT_SESS TLV 할당 */
				if((pNode = nifo_node_alloc(gpMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, LH"ERROR pLOG_VT_SESS nifo_node_alloc NULL", LT);
					return NULL;
				}
				if((pstVTSESSDATA->pLOG_VT_SESS = (LOG_VT_SESS *) nifo_tlv_alloc(gpMEMSINFO, pNode, LOG_VT_SESS_DEF_NUM, LOG_VT_SESS_SIZE, DEF_MEMSET_ON)) == NULL) {
					log_print(LOGN_CRI, LH"ERROR pLOG_VT_SESS nifo_tlv_alloc NULL", LT);
					nifo_node_unlink_nont(gpMEMSINFO, pNode);
					nifo_cont_delete(gpMEMSINFO, pNode);
					return NULL;
				}
				pLOGVTSESS = pstVTSESSDATA->pLOG_VT_SESS;

				pstVTSESSDATA->offset = nifo_offset (gpMEMSINFO, pstVTSESSDATA->pLOG_VT_SESS);

				/* pLOG_VT_SESS 생성시 초기화 */
				VT_Sess_Init(pLOGVTSESS, pLOGSIPTRANS);
			}
			pLOGSIPTRANS->isUsed = 1;
			log_print(LOGN_DEBUG, "	CREATE NEW VT SESSION : IP[%d.%d.%d.%d] METHOD[%d]", 
					HIPADDR(pstKey->uiClientIP), pLOGSIPTRANS->method);
		} else {
			log_print(LOGN_INFO, "BYPASS TO A_IM TYPE:[%s] IP[%d.%d.%d.%d] METHOD[%d]", 
					PrintTYPE(type), HIPADDR(pstKey->uiClientIP),
pLOGSIPTRANS->method);
			*dSndIMF = 1;
			return 0;
		}
	}
#if 0	
	if(pLOGVTSESS && pLOGVTSESS->LastMethod == SIP_MSG_BYE) {
		log_print(LOGN_DEBUG, "RECEIVE BYE CLOSE VT SESSION CIP[%d.%d.%d.%d]", HIPADDR(pLOGVTSESS->uiClientIP) );
		dCloseVTSess(pLOGVTSESS);
	}
#endif

	return pLOGVTSESS;
}

int VT_Session_Update(char *data)
{
	st_SIPSessKey 	stKey;
	st_SIPSessKey 	*pstKey;

	CALL_KEY        *pCALLKEY = NULL;
	stHASHONODE     *pHASHONODE;

	VT_SESSION_HASH_DATA    *pstVTSESS;
	

	pCALLKEY = (CALL_KEY *) data;

	memset(&stKey, 0, sizeof(st_SIPSessKey));
	pstKey = &stKey;

	pstKey->uiClientIP = pCALLKEY->uiSrcIP;
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_VT_TIMEOUT];

	/* 1. Found IM Session */
	if( (pHASHONODE = hasho_find(gpHASHOINFO, (U8 *)pstKey)) ) {

		pstVTSESS = (VT_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
		pstVTSESS->timerNID =
			timerN_update(gpTIMERNINFO, pstVTSESS->timerNID, time(NULL) + guiTimerValue);

		log_print(LOGN_DEBUG, "	UPDATE TIMER VT SESSION : IP[%d.%d.%d.%d] METHOD[%d] Call-ID[%s] TIMEOUT[%d]",
				HIPADDR(pstVTSESS->pLOG_VT_SESS->uiClientIP), pstVTSESS->pLOG_VT_SESS->LastMethod,
				pstVTSESS->pLOG_VT_SESS->CallID, DEF_WAIT_TIMEOUT);
	} else {
		log_print(LOGN_DEBUG, "	NOT FOUND VT SESSION : IP[%d.%d.%d.%d]", HIPADDR(pstKey->uiClientIP));
	}

	return 0;
}


#define DEF_DUP_ID  1
#define NON_DUP_ID  2

void dInitDupList( st_DupList *pstList )
{
	int     i;

	memset( pstList, 0x00, sizeof(st_DupList) );

	for( i=0; i<MAX_DUP_NODE; i++ ) {
		if( i < (MAX_DUP_NODE-1) )
			pstList->stNode[i].usNext = i+1;
		else
			pstList->stNode[i].usNext = 0;

		if( (MAX_DUP_NODE-1-i) > 0 )
			pstList->stNode[MAX_DUP_NODE-1-i].usPrev = MAX_DUP_NODE-2-i;
		else
			pstList->stNode[MAX_DUP_NODE-1-i].usPrev = MAX_DUP_NODE-1;

	}
}

int dSearchDupList( st_DupList *pstList, unsigned short usIdentification, unsigned short usFragmentOffset )
{                                                              
	unsigned short  usCount, usIndex;

	usCount = pstList->usCurrentCount;
	usIndex = pstList->usLastIndex;

#ifdef __DEBUG__
	log_print( LOGN_INFO, "	##### CNT[%d] IDX[%d] IDEN[%5d, %5d] FRAG[%5d, %5d]", 
			usCount, usIndex, 
			pstList->stNode[usIndex].usIdentification, usIdentification, 
			pstList->stNode[usIndex].usFragmentOffset, usFragmentOffset);
#endif

	while( usCount > 0 ) {
		if(usIdentification == pstList->stNode[usIndex].usIdentification ) {
			if( usFragmentOffset == pstList->stNode[usIndex].usFragmentOffset ) {
				log_print( LOGN_INFO, "	##### DUPLICATE PACKET IDEN:%5d FRAG:%5d INDEX:%02d",
						usIdentification, usFragmentOffset, usIndex );
				return DEF_DUP_ID;
			}
		}       

		usIndex = pstList->stNode[usIndex].usPrev;
		usCount--;      
	}                   
	/*          
	 * DEL FIRST & INSERT NEW
	 */
	if( pstList->usFirstIndex == pstList->stNode[pstList->usLastIndex].usNext ) {
		/* LIST FULL */
		pstList->usFirstIndex = pstList->stNode[pstList->usFirstIndex].usNext;
		pstList->usLastIndex = pstList->stNode[pstList->usLastIndex].usNext;

		pstList->stNode[pstList->usLastIndex].usIdentification = usIdentification;
		pstList->stNode[pstList->usLastIndex].usFragmentOffset = usFragmentOffset;
	}
	else {
		pstList->usLastIndex = pstList->stNode[pstList->usLastIndex].usNext;

		pstList->stNode[pstList->usLastIndex].usIdentification = usIdentification;
		pstList->stNode[pstList->usLastIndex].usFragmentOffset = usFragmentOffset;

		pstList->usCurrentCount++;
	}

	return 0;
}

int RTP_Process(OFFSET offset, Capture_Header_Msg *pCAPHEAD ,INFO_ETH *pINFOETH)
{
	int 			dRet = 0;
	int 			dRtcpFlag = 0, uiLength = 0, dRtxFlag;
	USHORT 			usRTPAudioPort, usRTPVideoPort;
	stHASHONODE     *pHASHONODE;
	st_SIPSessKey 	stKey;
	st_SIPSessKey 	*pstKey;

	pst_RTCP_COMM 	pstRTCP_COMM;
	pst_RTCP_SR 	pstRTCP_SR = NULL;
	pst_RTCP_RR 	pstRTCP_RR = NULL;
	UCHAR 			ucReportCnt = 0;
	PRTP			pRTP;
	UCHAR			*pucData;
	UCHAR 			*pucRTCPData;

	UINT 			uiTempIP;

	VT_SESSION_HASH_DATA *pstVTSESSDATA;

	log_print(LOGN_INFO, "%s : bRtxType = %d, IPHeaderLen = %d, UDP DataLen=%d",	
		(char *) __FUNCTION__, pCAPHEAD->bRtxType, pINFOETH->stIP.wIPHeaderLen, pINFOETH->stUDPTCP.wDataLen);

	dRtxFlag = pCAPHEAD->bRtxType;
	if(dRtxFlag == DEF_FROM_SERVER) {
		uiTempIP = pINFOETH->stIP.dwSrcIP;
		pINFOETH->stIP.dwSrcIP = pINFOETH->stIP.dwDestIP;
		pINFOETH->stIP.dwDestIP = uiTempIP;
	}

	memset(&stKey, 0, sizeof(st_SIPSessKey));
	pstKey = &stKey;

	pstKey->uiClientIP = pINFOETH->stIP.dwSrcIP;
	pstKey->uiReserved = 0;

	if( (pHASHONODE = hasho_find(gpHASHOINFO, (UCHAR *)pstKey)) == NULL) {

		log_print(LOGN_INFO, "	NOT EXIST : VT SESSION SRC IP[%d.%d.%d.%d] DEST IP[%d.%d.%d.%d]", 
				HIPADDR(pstKey->uiClientIP), HIPADDR(pINFOETH->stIP.dwDestIP));

		pstKey->uiClientIP = pINFOETH->stIP.dwDestIP;
		if( (pHASHONODE = hasho_find(gpHASHOINFO, (UCHAR *)pstKey)) == NULL) {

			log_print(LOGN_INFO, "	NOT EXIST : VT SESSION SRC IP[%d.%d.%d.%d] DEST IP[%d.%d.%d.%d]", 
					HIPADDR(pstKey->uiClientIP), HIPADDR(pINFOETH->stIP.dwSrcIP));
			return -1;
		} else {
			dRtxFlag = DEF_FROM_SERVER;
			uiTempIP = pINFOETH->stIP.dwSrcIP;
			pINFOETH->stIP.dwSrcIP = pINFOETH->stIP.dwDestIP;
			pINFOETH->stIP.dwDestIP = uiTempIP;
		}
	}

	// PORT 1235, 1255 RTCP 패킷 
	if( (dRtcpFlag = pINFOETH->stUDPTCP.wSrcPort % 2) != 0 ) {
		pucData = nifo_get_value(gpMEMSINFO, ETH_DATA_NUM, offset);
		pucRTCPData = &pucData[pINFOETH->offset];

		while( uiLength < pINFOETH->stUDPTCP.wDataLen ) {

			pstRTCP_COMM = (pst_RTCP_COMM)&pucRTCPData[uiLength];
	
			ucReportCnt = pstRTCP_COMM->ucVerCnt & 0x1f;
			pstRTCP_COMM->usLength  = htons(pstRTCP_COMM->usLength)*4;
			pstRTCP_COMM->uiSSRC    = htonl(pstRTCP_COMM->uiSSRC);

			log_print( LOGN_INFO, "	[VT_SESSION_RTCP] COMMON VERCNT:%u TYPE:%u LEN:%u SSRC:%u REPORT:%d [%u:%u]",
					pstRTCP_COMM->ucVerCnt, pstRTCP_COMM->ucType, pstRTCP_COMM->usLength, pstRTCP_COMM->uiSSRC,
					ucReportCnt, uiLength, pINFOETH->stUDPTCP.wDataLen );

			if( pstRTCP_COMM->ucType == DEF_SR ) {
				/* SENDER REPORT */
				pstRTCP_SR  = (pst_RTCP_SR)&pucRTCPData[DEF_RTCP_COMM_SIZE + uiLength];

				pstRTCP_SR->uiRTPTime       = htonl(pstRTCP_SR->uiRTPTime);
				pstRTCP_SR->uiSendPktCnt    = htonl(pstRTCP_SR->uiSendPktCnt);
				pstRTCP_SR->uiSendOctCnt    = htonl(pstRTCP_SR->uiSendOctCnt);

				log_print( LOGN_INFO, "	[VT_SESSION_RTCP] SR NTP:%llu RTP:%u PKT:%u OCT:%u",
						pstRTCP_SR->llNTPTime, pstRTCP_SR->uiRTPTime, pstRTCP_SR->uiSendPktCnt, pstRTCP_SR->uiSendOctCnt );

				if (ucReportCnt) {
					pstRTCP_RR  = (pst_RTCP_RR)&pucRTCPData[DEF_RTCP_COMM_SIZE + DEF_RTCP_SRHDR_SIZE];

					pstRTCP_RR->uiSSRC      = htonl(pstRTCP_RR->uiSSRC);
					pstRTCP_RR->uiLostInfo  = htonl(pstRTCP_RR->uiLostInfo) & 0x00ffffff;
					pstRTCP_RR->uiSeqNum    = htonl(pstRTCP_RR->uiSeqNum);
					pstRTCP_RR->uiJitter    = htonl(pstRTCP_RR->uiJitter);
					pstRTCP_RR->uiLSR       = htonl(pstRTCP_RR->uiLSR);
					pstRTCP_RR->uiDLSR      = htonl(pstRTCP_RR->uiDLSR);

					if(pstRTCP_RR->uiLostInfo > 0xffff) {
						log_print( LOGN_WARN, "	[RTCP] SR REPORT SSRC:%u LOST:%u SEQ:%u JIT:%u LSR:%u DLSR:%u",
								pstRTCP_RR->uiSSRC, pstRTCP_RR->uiLostInfo, pstRTCP_RR->uiSeqNum,
								pstRTCP_RR->uiJitter, pstRTCP_RR->uiLSR, pstRTCP_RR->uiDLSR );
					}
					log_print( LOGN_INFO, "	[VT_SESSION_RTCP] SR REPORT SSRC:%u LOST:%u SEQ:%u JIT:%u LSR:%u DLSR:%u",
							pstRTCP_RR->uiSSRC, pstRTCP_RR->uiLostInfo, pstRTCP_RR->uiSeqNum,
							pstRTCP_RR->uiJitter, pstRTCP_RR->uiLSR, pstRTCP_RR->uiDLSR );
				}

				uiLength += pstRTCP_COMM->usLength + 4;

				continue;
			}
			else if( pstRTCP_COMM->ucType == DEF_RR && ucReportCnt ) {
				/* RECEIVER REPORT */
				log_print( LOGN_INFO, "	[VT_SESSION_RTCP] RECEIVER REPORT" );

				pstRTCP_RR  = (pst_RTCP_RR)&pucRTCPData[DEF_RTCP_COMM_SIZE + uiLength];

				pstRTCP_RR->uiSSRC      = htonl(pstRTCP_RR->uiSSRC);
				pstRTCP_RR->uiLostInfo  = htonl(pstRTCP_RR->uiLostInfo) & 0x00ffffff;
				pstRTCP_RR->uiSeqNum    = htonl(pstRTCP_RR->uiSeqNum);
				pstRTCP_RR->uiJitter    = htonl(pstRTCP_RR->uiJitter);
				pstRTCP_RR->uiLSR       = htonl(pstRTCP_RR->uiLSR);
				pstRTCP_RR->uiDLSR      = htonl(pstRTCP_RR->uiDLSR);

				if(pstRTCP_RR->uiLostInfo > 0xffff) {
					log_print( LOGN_WARN, "	[RTCP] RR REPORT SSRC:%u LOST:%u SEQ:%u JIT:%u LSR:%u DLSR:%u",
							pstRTCP_RR->uiSSRC, pstRTCP_RR->uiLostInfo, pstRTCP_RR->uiSeqNum,
							pstRTCP_RR->uiJitter, pstRTCP_RR->uiLSR, pstRTCP_RR->uiDLSR );
				}
				log_print( LOGN_INFO, "[VT_SESSION_RTCP] RR SSRC:%u LOST:%u SEQ:%u JIT:%u LSR:%u DLSR:%u",
						pstRTCP_RR->uiSSRC, pstRTCP_RR->uiLostInfo, pstRTCP_RR->uiSeqNum,
						pstRTCP_RR->uiJitter, pstRTCP_RR->uiLSR, pstRTCP_RR->uiDLSR );

			}
			else {
				/* ETC */
				log_print( LOGN_INFO, "	IP:%d.%d.%d.%d:%u RTCP:%u", 
					HIPADDR(stKey.uiClientIP), pINFOETH->stUDPTCP.wSrcPort, pstRTCP_COMM->ucType );
				
				uiLength += pstRTCP_COMM->usLength + 4;
				continue;
			}

			uiLength += pstRTCP_COMM->usLength + 4;
		}
	}

	/* 1. SRC IP로 UDP 세션을 검색하고 중복패킷 리스트에 추가한다. */
	log_print(LOGN_INFO, "	%s : VT SESSION EXIST IP[%d.%d.%d.%d]", (dRtxFlag == DEF_FROM_CLIENT) ? "UP" : "DN", HIPADDR(pstKey->uiClientIP));
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_VT_TIMEOUT];

	pstVTSESSDATA = (VT_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
	pstVTSESSDATA->timerNID = timerN_update(gpTIMERNINFO, pstVTSESSDATA->timerNID, time(NULL) + guiTimerValue);
	
	usRTPAudioPort = pstVTSESSDATA->pLOG_VT_SESS->AudioPort;
	usRTPVideoPort = pstVTSESSDATA->pLOG_VT_SESS->VideoPort;

	if(dRtxFlag == DEF_FROM_CLIENT) { // UP   
		dRet = dSearchDupList( &pstVTSESSDATA->stUpList, pINFOETH->stIP.usIdent, pINFOETH->stIP.usIPFrag);
	} else {
		dRet = dSearchDupList( &pstVTSESSDATA->stDownList, pINFOETH->stIP.usIdent, pINFOETH->stIP.usIPFrag);
	}

	/* 2. 중복된 패킷이 있으면 DEST IP로 검색한다 */
	if(dRet == DEF_DUP_ID) {
		pstKey->uiClientIP = pINFOETH->stIP.dwDestIP;
		pstKey->uiReserved = 0;

		if( (pHASHONODE = hasho_find(gpHASHOINFO, (UCHAR *)pstKey)) != NULL) {
			log_print(LOGN_INFO, "	DUPLICATE %s : VT SESSION EXIST SRC IP[%d.%d.%d.%d] DEST[%d.%d.%d.%d]", 
					(dRtxFlag == DEF_FROM_CLIENT) ? "UP" : "DN", HIPADDR(pstKey->uiClientIP), HIPADDR(pINFOETH->stIP.dwSrcIP));
			pstVTSESSDATA = (VT_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
		} else {
			log_print(LOGN_WARN, "	NOT EXIST : DUPLICATE VT SESSION SRC IP[%d.%d.%d.%d] DEST[%d.%d.%d.%d]", 
					HIPADDR(pstKey->uiClientIP), HIPADDR(pINFOETH->stIP.dwSrcIP));
			return 100;
		}

		/* DUPLICATE Packet인 경우 Update할 필드 방향을 바꾸기 위하여 RtxFlag를 반대로 설정한다. */
		if(dRtxFlag == DEF_FROM_CLIENT)
			dRtxFlag = DEF_FROM_SERVER;
		else
			dRtxFlag = DEF_FROM_CLIENT;
	}

	/* 2. 중복된 패킷이 없으면 정상 처리한다. */
	if(dRtxFlag == DEF_FROM_CLIENT) {
		if ( dRtcpFlag ) {
#if 0
			if( pstRTCP_RR ) {
				/* CHECK UP MAX JITTER */
				if( pstVTSESSDATA->pLOG_VT_SESS->UpMaxJitter < pstRTCP_RR->uiJitter ) {
					pstVTSESSDATA->pLOG_VT_SESS->UpMaxJitter = pstRTCP_RR->uiJitter;
				}
				if(htonl(pstRTCP_RR->uiLostInfo) <= pstVTSESSDATA->pLOG_VT_SESS->RTPUpCnt) {
					pstVTSESSDATA->pLOG_VT_SESS->RTPUpLossCnt = htonl(pstRTCP_RR->uiLostInfo);
				}
				if(pINFOETH->stUDPTCP.wSrcPort == usRTPAudioPort+1) {
					pstVTSESSDATA->pLOG_VT_SESS->AudioUpLossCnt = pstVTSESSDATA->pLOG_VT_SESS->RTPUpLossCnt;
				}
				else {
					pstVTSESSDATA->pLOG_VT_SESS->VideoUpLossCnt = pstVTSESSDATA->pLOG_VT_SESS->RTPUpLossCnt;
				}
			}
#endif
		}
		else {
			pucData = nifo_get_value(gpMEMSINFO, ETH_DATA_NUM, offset);
			pRTP = (PRTP)&pucData[pINFOETH->offset];

			dRtpQosJitter(TOUINT(pRTP->Timestamp), pstVTSESSDATA, pCAPHEAD, pINFOETH);
			dRtpQosLoss(TOUSHORT(pRTP->Sequence), pstVTSESSDATA, pCAPHEAD, pINFOETH);

			if(pINFOETH->stUDPTCP.wSrcPort == usRTPAudioPort) {
				pstVTSESSDATA->pLOG_VT_SESS->AudioUpCnt++;
				pstVTSESSDATA->pLOG_VT_SESS->AudioUpSize += pINFOETH->stUDPTCP.wDataLen;
			}
			else {
				pstVTSESSDATA->pLOG_VT_SESS->VideoUpCnt++;
				pstVTSESSDATA->pLOG_VT_SESS->VideoUpSize += pINFOETH->stUDPTCP.wDataLen;
			}
			pstVTSESSDATA->pLOG_VT_SESS->uiIPDataUpPktCnt++;
			pstVTSESSDATA->pLOG_VT_SESS->uiIPTotUpPktCnt++;
			pstVTSESSDATA->pLOG_VT_SESS->uiIPDataUpPktSize += pINFOETH->stUDPTCP.wDataLen;
			pstVTSESSDATA->pLOG_VT_SESS->uiIPTotUpPktSize += pINFOETH->stIP.wTotalLength;

			pstVTSESSDATA->pLOG_VT_SESS->RTPUpCnt ++;
			pstVTSESSDATA->pLOG_VT_SESS->RTPUpDataSize += pINFOETH->stUDPTCP.wDataLen;
		}
	}
	else {
		if ( dRtcpFlag ) {
#if 0
			if( pstRTCP_RR ) {
				/* CHECK DN MAX JITTER */
				if( pstVTSESSDATA->pLOG_VT_SESS->DnMaxJitter < pstRTCP_RR->uiJitter ) {
					pstVTSESSDATA->pLOG_VT_SESS->DnMaxJitter = pstRTCP_RR->uiJitter;
				}
				if(htonl(pstRTCP_RR->uiLostInfo) <= pstVTSESSDATA->pLOG_VT_SESS->RTPDnCnt) {
					pstVTSESSDATA->pLOG_VT_SESS->RTPDnLossCnt = htonl(pstRTCP_RR->uiLostInfo);
				}
				if(pINFOETH->stUDPTCP.wSrcPort == usRTPAudioPort+1) {
					pstVTSESSDATA->pLOG_VT_SESS->AudioDownLossCnt = pstVTSESSDATA->pLOG_VT_SESS->RTPDnLossCnt;
				}
				else {
					pstVTSESSDATA->pLOG_VT_SESS->VideoDownLossCnt = pstVTSESSDATA->pLOG_VT_SESS->RTPDnLossCnt;
				}
			}
#endif
		}
		else {
			pucData = nifo_get_value(gpMEMSINFO, ETH_DATA_NUM, offset);
			pRTP = (PRTP)&pucData[pINFOETH->offset];

			dRtpQosJitter(TOUINT(pRTP->Timestamp), pstVTSESSDATA, pCAPHEAD, pINFOETH);
			dRtpQosLoss(TOUSHORT(pRTP->Sequence), pstVTSESSDATA, pCAPHEAD, pINFOETH);

			if(pINFOETH->stUDPTCP.wSrcPort == usRTPAudioPort) {
				pstVTSESSDATA->pLOG_VT_SESS->AudioDownCnt++;
				pstVTSESSDATA->pLOG_VT_SESS->AudioDownSize += pINFOETH->stUDPTCP.wDataLen;
			}
			else {
				pstVTSESSDATA->pLOG_VT_SESS->VideoDownCnt++;
				pstVTSESSDATA->pLOG_VT_SESS->VideoDownSize += pINFOETH->stUDPTCP.wDataLen;
			}
			pstVTSESSDATA->pLOG_VT_SESS->uiIPDataDnPktCnt++;
			pstVTSESSDATA->pLOG_VT_SESS->uiIPTotDnPktCnt++;
			pstVTSESSDATA->pLOG_VT_SESS->uiIPDataDnPktSize += pINFOETH->stUDPTCP.wDataLen;
			pstVTSESSDATA->pLOG_VT_SESS->uiIPTotDnPktSize += pINFOETH->stIP.wTotalLength;

			pstVTSESSDATA->pLOG_VT_SESS->RTPDnCnt ++;
			pstVTSESSDATA->pLOG_VT_SESS->RTPDnDataSize += pINFOETH->stUDPTCP.wDataLen;
		}
	}

	pstVTSESSDATA->pLOG_VT_SESS->LastPktTime = pCAPHEAD->curtime;
	pstVTSESSDATA->pLOG_VT_SESS->LastPktMTime = pCAPHEAD->ucurtime;

	return 1;
}

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			vt_main.c vt_init.c
 *
 **/

S32 main(S32 argc, S8 *argv[])
{
	S32						dRet;		/**< 함수 Return 값 */
	OFFSET					offset;
	U8						*pNode;
	U8						*pNextNode, *pCurNode;
	U8						*p, *data;
	S32						type, len, ismalloc, udpFlag, startFlag, dSndIMF;

	LOG_SIP_TRANS			*pLOGSIPTRANS;
	LOG_VT_SESS     		*pLOGVTSESS = NULL;

	Capture_Header_Msg		*pCAPHEAD;
	INFO_ETH				*pINFOETH;

	//S32 					dSNDQID;
	S32						dSNDProcKey;

	char    vERSION[7] = "R3.0.0";

	/* dAppLog 초기화 */
#if 0
	InitAppLog(getpid(), SEQ_PROC_A_VT, LOG_PATH"A_VT", "A_VT");
	if( (dRet = Init_shm_common()) < 0) {
		log_print(LOGN_CRI, LH"dInit_shm_common dRet[%d]", LT, dRet);
		exit(0);
	}
#endif

	/* Log 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_VT, LOG_PATH"/A_VT/", "A_VT");

	/* A_CALL 초기화 */
	if((dRet = dInitVT(&gpMEMSINFO, &gpHASHOINFO , &gpTIMERNINFO)) < 0)
	{
		log_print(LOGN_CRI, LH"dInitVT dRet[%d]", LT, dRet);
		exit(0);
	}

	//if((dRet = set_version(SEQ_PROC_A_VT, vERSION)) < 0 ) {
	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_VT, vERSION)) < 0)
	{
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_VT, vERSION);
	}

	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_VT_TIMEOUT];
	log_print(LOGN_CRI, "START A_VT VERSION: %s SESSCNT: %d TIMEOUT: %d CALLCNT: %d", vERSION, VT_SESS_CNT, guiTimerValue, gACALLCnt);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		timerN_invoke(gpTIMERNINFO);

		/* nifo ==> gifo */
		//if((offset = nifo_msg_read(gpMEMSINFO, dMyQID, NULL)) > 0) {
		if((offset = gifo_read(gpMEMSINFO, gpCIFO, SEQ_PROC_A_VT)) > 0)
		{
			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(gpMEMSINFO, offset);
			pNextNode = pNode;

			pLOGSIPTRANS = NULL;
			pCAPHEAD = NULL;
			pINFOETH = NULL;
			udpFlag = 0;
			startFlag = 0;
			dSndIMF = 0;
			//dSNDQID = 0;
			dSNDProcKey = 0;

			do {
				p = pNextNode;
				pCurNode = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(gpMEMSINFO, pCurNode, &type, &len, &data, &ismalloc, &p)) < 0)
						break;

					log_print(LOGN_INFO, "####################################################################");
					log_print(LOGN_INFO, "RCV LOG TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", type, 
							(type==START_CALL_NUM || type==CLEAR_CALL_NUM) ? PRINT_TAG_DEF_ALL_CALL_INPUT(type) : PRINT_TAG_DEF_ALL_CALL_INPUT(type), 
							len, (ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");

					switch(type)
					{
						case CLEAR_CALL_NUM:
							VT_Session_Update(data);
						case START_CALL_NUM:
							startFlag = 1;
							break;
						case LOG_SIP_TRANS_DEF_NUM:
							pLOGVTSESS = VT_Session_Process(type,len,data, &dSndIMF);
							//dSNDQID = dCALLQID[((LOG_COMMON *)data)->uiClientIP % gACALLCnt];
							dSNDProcKey = SEQ_PROC_A_CALL + (((LOG_COMMON *)data)->uiClientIP % gACALLCnt);
							//							LOG_SIP_TRANS_Prt("PRINT LOG_SIP_TRANS", (LOG_SIP_TRANS *)data);
							break;
						case CAP_HEADER_NUM:
							pCAPHEAD = (Capture_Header_Msg *)data;
							break;
						case INFO_ETH_NUM:
							pINFOETH = (INFO_ETH *)data;
							break;
						case ETH_DATA_NUM:
							break;
						default:
							log_print(LOGN_WARN, "????? UNKNOWN TYPE[%d]", type);
							break;
					}

					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}

				/* RTP :::  UDP인 경우에 pCAPHEAD pINFOETH : 이 2가지가 존재해야 한다. */ 
				if(pCAPHEAD && pINFOETH) {
					dRet = RTP_Process(nifo_offset(gpMEMSINFO, pCurNode), pCAPHEAD, pINFOETH);
					if(dRet<0) {
						udpFlag = 0;
						if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
							//dSNDQID = dINETQID[pINFOETH->stIP.dwSrcIP % gAINETCnt];
							dSNDProcKey = SEQ_PROC_A_INET + (pINFOETH->stIP.dwSrcIP % gAINETCnt);
						} else {
							//dSNDQID = dINETQID[pINFOETH->stIP.dwDestIP % gAINETCnt];
							dSNDProcKey = SEQ_PROC_A_INET + (pINFOETH->stIP.dwDestIP % gAINETCnt);
						}
					}
					else
						udpFlag = 1;

					pCAPHEAD = NULL;
					pINFOETH = NULL;
				}

				pNextNode = (U8 *)nifo_entry(nifo_ptr(gpMEMSINFO, ((NIFO *)pCurNode)->nont.offset_next), NIFO, nont);

				if(udpFlag == 1 || startFlag == 1) {
					nifo_node_unlink_nont(gpMEMSINFO, pCurNode);
					nifo_node_delete(gpMEMSINFO, pCurNode);
					udpFlag = 0;
					startFlag = 0;
				} else if ( dSndIMF == 1 ) {
					//if((dRet = dSend_VT_Data(gpMEMSINFO, dIMQID, pCurNode)) < 0) {
					if((dRet = dSend_VT_Data(gpMEMSINFO, SEQ_PROC_A_IM, pCurNode)) < 0) {
						log_print(LOGN_CRI, LH"ERROR MSGQ WRITE FAILE[%d][%s]", 
								LT, dRet, strerror(-dRet));
					}
					dSndIMF = 0;
				} else {
					//if((dRet = dSend_VT_Data(gpMEMSINFO, dSNDQID, pCurNode)) < 0) {
					if((dRet = dSend_VT_Data(gpMEMSINFO, dSNDProcKey, pCurNode)) < 0) {
						log_print(LOGN_CRI, LH"ERROR MSGQ WRITE FAILE[%d][%s]", 
								LT, dRet, strerror(-dRet));
					}
				}
				if(pLOGVTSESS && pLOGVTSESS->LastMethod == SIP_MSG_BYE) {
					log_print(LOGN_DEBUG, "	RECEIVE BYE CLOSE VT SESSION CIP[%d.%d.%d.%d]", HIPADDR(pLOGVTSESS->uiClientIP) );
					dCloseVTSess(pLOGVTSESS);
				}


			} while(pNode != pNextNode);

		} else {
			usleep(0);
		}
	}

	FinishProgram();

	return 0;
}

/* tvTime 구조체는 캡쳐 시간을 가지는 정보임 */
int dRtpQosJitter(UINT uiTimestamp, VT_SESSION_HASH_DATA *pData, Capture_Header_Msg *pstCAPHEAD, INFO_ETH *pInfoEth)
{
	struct timeval stDiffRecvTime, tvTime;
	int diff, jitter;
	unsigned int subTimestamp;

	tvTime.tv_sec = pstCAPHEAD->curtime;
	tvTime.tv_usec = pstCAPHEAD->ucurtime;

	log_print(LOGN_INFO, "PORTINFO: SRC[%u] DST[%u] AUDIO[%u] VIDEO[%u]",
			pInfoEth->stUDPTCP.wSrcPort, pInfoEth->stUDPTCP.wDestPort,
			pData->pLOG_VT_SESS->AudioPort, pData->pLOG_VT_SESS->VideoPort);

	if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER ) {
		if( pInfoEth->stUDPTCP.wSrcPort == pData->pLOG_VT_SESS->AudioPort ) {
			if( pData->tvAudioDnOldTime.tv_sec == 0 && pData->tvAudioDnOldTime.tv_usec == 0 )
				pData->tvAudioDnOldTime = tvTime;
			timersub(&tvTime, &pData->tvAudioDnOldTime, &stDiffRecvTime);
			if( stDiffRecvTime.tv_sec < 0 ) {
				return -1;
			}

			/* 1ms = 8 sampling */
			if( pData->uiAudioDnOldTimestamp == 0 )
				pData->uiAudioDnOldTimestamp = uiTimestamp;
			if( (subTimestamp = uiTimestamp - pData->uiAudioDnOldTimestamp) < 0) {
				pData->uiAudioDnOldTimestamp = uiTimestamp;
				pData->tvAudioDnOldTime = tvTime;
				return -2;
			}

			diff = abs((stDiffRecvTime.tv_sec * 8) - subTimestamp);
			jitter = pData->uiAudioDnJitter;
			pData->uiAudioDnJitter = jitter + ((diff - jitter)/16);

			/* init */
			pData->uiAudioDnOldTimestamp = uiTimestamp;
			pData->tvAudioDnOldTime = tvTime;

			/* CHECK MAX JITTER */
			if( pData->pLOG_VT_SESS->DnMaxJitter < pData->uiAudioDnJitter )
				pData->pLOG_VT_SESS->DnMaxJitter = pData->uiAudioDnJitter;
			log_print(LOGN_INFO, "CAP[%ld.%06ld] A_JITTER[%u:%u] V_JITTER[%u:%u] MAXJITTER[%u:%u]",
					pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
					pData->uiAudioDnJitter, pData->uiAudioUpJitter,
					pData->uiVideoDnJitter, pData->uiVideoUpJitter,
					pData->pLOG_VT_SESS->DnMaxJitter, pData->pLOG_VT_SESS->UpMaxJitter);
		}
		else {
			if( pData->tvVideoDnOldTime.tv_sec == 0 && pData->tvVideoDnOldTime.tv_usec == 0 )
				pData->tvVideoDnOldTime = tvTime;
			timersub(&tvTime, &pData->tvVideoDnOldTime, &stDiffRecvTime);
			if( stDiffRecvTime.tv_sec < 0 ) {
				return -1;
			}

			/* 1ms = 8 sampling */
			if( pData->uiVideoDnOldTimestamp == 0 )
				pData->uiVideoDnOldTimestamp = uiTimestamp;
			if( (subTimestamp = uiTimestamp - pData->uiVideoDnOldTimestamp) < 0) {
				pData->uiVideoDnOldTimestamp = uiTimestamp;
				pData->tvVideoDnOldTime = tvTime;
				return -2;
			}

			diff = abs((stDiffRecvTime.tv_sec * 8) - subTimestamp);
			jitter = pData->uiVideoDnJitter;
			pData->uiVideoDnJitter = jitter + ((diff - jitter)/16);

			/* init */
			pData->uiVideoDnOldTimestamp = uiTimestamp;
			pData->tvVideoDnOldTime = tvTime;

			/* CHECK MAX JITTER */
			if( pData->pLOG_VT_SESS->DnMaxJitter < pData->uiVideoDnJitter )
				pData->pLOG_VT_SESS->DnMaxJitter = pData->uiVideoDnJitter;
			log_print(LOGN_INFO, "CAP[%ld.%06ld] A_JITTER[%u:%u] V_JITTER[%u:%u] MAXJITTER[%u:%u]",
					pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
					pData->uiAudioDnJitter, pData->uiAudioUpJitter,
					pData->uiVideoDnJitter, pData->uiVideoUpJitter,
					pData->pLOG_VT_SESS->DnMaxJitter, pData->pLOG_VT_SESS->UpMaxJitter);
		}
	}
	else {
		if( pInfoEth->stUDPTCP.wDestPort == pData->pLOG_VT_SESS->AudioPort ) {

			if( pData->tvAudioUpOldTime.tv_sec == 0 && pData->tvAudioUpOldTime.tv_usec == 0 )
				pData->tvAudioUpOldTime = tvTime;
			timersub(&tvTime, &pData->tvAudioUpOldTime, &stDiffRecvTime);
			if( stDiffRecvTime.tv_sec < 0 ) {
				return -1;
			}

			/* 1ms = 8 sampling */
			if( pData->uiAudioUpOldTimestamp == 0 )
				pData->uiAudioUpOldTimestamp = uiTimestamp;
			if( (subTimestamp = uiTimestamp - pData->uiAudioUpOldTimestamp) < 0) {
				pData->uiAudioUpOldTimestamp = uiTimestamp;
				pData->tvAudioUpOldTime = tvTime;
				return -2;
			}

			diff = abs((stDiffRecvTime.tv_sec * 8) - subTimestamp);
			jitter = pData->uiAudioUpJitter;
			pData->uiAudioUpJitter = jitter + ((diff - jitter)/16);

			/* init */
			pData->uiAudioUpOldTimestamp = uiTimestamp;
			pData->tvAudioUpOldTime = tvTime;

			/* CHECK MAX JITTER */
			if( pData->pLOG_VT_SESS->UpMaxJitter < pData->uiAudioUpJitter )
				pData->pLOG_VT_SESS->UpMaxJitter = pData->uiAudioUpJitter;
			log_print(LOGN_INFO, "CAP[%ld.%06ld] A_JITTER[%u:%u] V_JITTER[%u:%u] MAXJITTER[%u:%u]",
					pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
					pData->uiAudioDnJitter, pData->uiAudioUpJitter,
					pData->uiVideoDnJitter, pData->uiVideoUpJitter,
					pData->pLOG_VT_SESS->DnMaxJitter, pData->pLOG_VT_SESS->UpMaxJitter);
		}
		else {
			if( pData->tvVideoUpOldTime.tv_sec == 0 && pData->tvVideoUpOldTime.tv_usec == 0 )
				pData->tvVideoUpOldTime = tvTime;
			timersub(&tvTime, &pData->tvVideoUpOldTime, &stDiffRecvTime);
			if( stDiffRecvTime.tv_sec < 0 ) {
				return -1;
			}

			/* 1ms = 8 sampling */
			if( pData->uiVideoUpOldTimestamp == 0 )
				pData->uiVideoUpOldTimestamp = uiTimestamp;
			if( (subTimestamp = uiTimestamp - pData->uiVideoUpOldTimestamp) < 0) {
				pData->uiVideoUpOldTimestamp = uiTimestamp;
				pData->tvVideoUpOldTime = tvTime;
				return -2;
			}

			diff = abs((stDiffRecvTime.tv_sec * 8) - subTimestamp);
			jitter = pData->uiVideoUpJitter;
			pData->uiVideoUpJitter = jitter + ((diff - jitter)/16);

			/* init */
			pData->uiVideoUpOldTimestamp = uiTimestamp;
			pData->tvVideoUpOldTime = tvTime;

			/* CHECK MAX JITTER */
			if( pData->pLOG_VT_SESS->UpMaxJitter < pData->uiVideoUpJitter )
				pData->pLOG_VT_SESS->UpMaxJitter = pData->uiVideoUpJitter;
			log_print(LOGN_INFO, "CAP[%ld.%06ld] A_JITTER[%u:%u] V_JITTER[%u:%u] MAXJITTER[%u:%u]",
					pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
					pData->uiAudioDnJitter, pData->uiAudioUpJitter,
					pData->uiVideoDnJitter, pData->uiVideoUpJitter,
					pData->pLOG_VT_SESS->DnMaxJitter, pData->pLOG_VT_SESS->UpMaxJitter);
		}
	}

	return 0;
}

int dRtpQosLoss(UINT uiSequence, VT_SESSION_HASH_DATA *pData, Capture_Header_Msg *pstCAPHEAD, INFO_ETH *pInfoEth)
{
	int     loss;

	if( pstCAPHEAD->bRtxType == DEF_FROM_SERVER ) {
		if( pInfoEth->stUDPTCP.wSrcPort == pData->pLOG_VT_SESS->AudioPort ) {
			if( pData->uiAudioDnOldSequence == 0 )
				pData->uiAudioDnOldSequence = uiSequence;

			loss = uiSequence - pData->uiAudioDnOldSequence;
			if(loss == 1) {
				pData->uiAudioDnOldSequence = uiSequence;
			}
			else if(loss > 1) {
				pData->pLOG_VT_SESS->AudioDownLossCnt += loss - 1;
				pData->uiAudioDnOldSequence = uiSequence;
				pData->pLOG_VT_SESS->RTPDnLossCnt
					= pData->pLOG_VT_SESS->AudioDownLossCnt+pData->pLOG_VT_SESS->VideoDownLossCnt;

				log_print(LOGN_INFO, "LOSSCNT1: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
						pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
						pData->pLOG_VT_SESS->RTPUpLossCnt, pData->pLOG_VT_SESS->RTPDnLossCnt);
			}
			else if(loss < 0) {
				/* overflow process */
				if(loss < -100) {
					pData->uiAudioDnOldSequence = uiSequence;
				}
				else {
					if( pData->pLOG_VT_SESS->AudioDownLossCnt > 0)
						pData->pLOG_VT_SESS->AudioDownLossCnt--;
					pData->pLOG_VT_SESS->RTPDnLossCnt
						= pData->pLOG_VT_SESS->AudioDownLossCnt+pData->pLOG_VT_SESS->VideoDownLossCnt;
					log_print(LOGN_INFO, "LOSSCNT2: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
							pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
							pData->pLOG_VT_SESS->RTPUpLossCnt, pData->pLOG_VT_SESS->RTPDnLossCnt);
				}
			}
		}
		else {
			if( pData->uiVideoDnOldSequence == 0 )
				pData->uiVideoDnOldSequence = uiSequence;

			loss = uiSequence - pData->uiVideoDnOldSequence;
			if(loss == 1) {
				pData->uiVideoDnOldSequence = uiSequence;
			}
			else if(loss > 1) {
				pData->pLOG_VT_SESS->VideoDownLossCnt += loss - 1;
				pData->uiVideoDnOldSequence = uiSequence;
				pData->pLOG_VT_SESS->RTPDnLossCnt
					= pData->pLOG_VT_SESS->AudioDownLossCnt+pData->pLOG_VT_SESS->VideoDownLossCnt;

				log_print(LOGN_INFO, "LOSSCNT5: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
						pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
						pData->pLOG_VT_SESS->RTPUpLossCnt, pData->pLOG_VT_SESS->RTPDnLossCnt);
			}
			else if(loss < 0) {
				/* overflow process */
				if(loss < -100) {
					pData->uiVideoDnOldSequence = uiSequence;
				}
				else {
					if( pData->pLOG_VT_SESS->VideoDownLossCnt > 0)
						pData->pLOG_VT_SESS->VideoDownLossCnt--;
					pData->pLOG_VT_SESS->RTPDnLossCnt
						= pData->pLOG_VT_SESS->AudioDownLossCnt+pData->pLOG_VT_SESS->VideoDownLossCnt;
					log_print(LOGN_INFO, "LOSSCNT6: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
							pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
							pData->pLOG_VT_SESS->RTPUpLossCnt, pData->pLOG_VT_SESS->RTPDnLossCnt);
				}
			}
		}
	}
	else {
		if( pInfoEth->stUDPTCP.wDestPort == pData->pLOG_VT_SESS->AudioPort ) {
			if( pData->uiAudioUpOldSequence == 0 )
				pData->uiAudioUpOldSequence = uiSequence;

			loss = uiSequence - pData->uiAudioUpOldSequence;
			if(loss == 1) {
				pData->uiAudioUpOldSequence = uiSequence;
			}
			else if(loss > 1) {
				pData->pLOG_VT_SESS->AudioUpLossCnt += loss - 1;
				pData->uiAudioUpOldSequence = uiSequence;
				pData->pLOG_VT_SESS->RTPUpLossCnt
					= pData->pLOG_VT_SESS->AudioUpLossCnt+pData->pLOG_VT_SESS->VideoUpLossCnt;

				log_print(LOGN_INFO, "LOSSCNT3: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
						pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
						pData->pLOG_VT_SESS->RTPUpLossCnt, pData->pLOG_VT_SESS->RTPDnLossCnt);
			}
			else if(loss < 0) {
				/* overflow process */
				if(loss < -100) {
					pData->uiAudioUpOldSequence = uiSequence;
				}
				else {
					if( pData->pLOG_VT_SESS->AudioUpLossCnt > 0)
						pData->pLOG_VT_SESS->AudioUpLossCnt--;
					pData->pLOG_VT_SESS->RTPUpLossCnt
						= pData->pLOG_VT_SESS->AudioUpLossCnt+pData->pLOG_VT_SESS->VideoUpLossCnt;
					log_print(LOGN_INFO, "LOSSCNT4: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
							pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
							pData->pLOG_VT_SESS->RTPUpLossCnt, pData->pLOG_VT_SESS->RTPDnLossCnt);
				}
			}
		}
		else {
			if( pData->uiVideoUpOldSequence == 0 )
				pData->uiVideoUpOldSequence = uiSequence;

			loss = uiSequence - pData->uiVideoUpOldSequence;
			if(loss == 1) {
				pData->uiVideoUpOldSequence = uiSequence;
			}   
			else if(loss > 1) {
				pData->pLOG_VT_SESS->VideoUpLossCnt += loss - 1;
				pData->uiVideoUpOldSequence = uiSequence;
				pData->pLOG_VT_SESS->RTPUpLossCnt
					= pData->pLOG_VT_SESS->AudioUpLossCnt+pData->pLOG_VT_SESS->VideoUpLossCnt;

				log_print(LOGN_INFO, "LOSSCNT7: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
						pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
						pData->pLOG_VT_SESS->RTPUpLossCnt, pData->pLOG_VT_SESS->RTPDnLossCnt);
			}           
			else if(loss < 0) {
				/* overflow process */
				if(loss < -100) {
					pData->uiVideoUpOldSequence = uiSequence;
				}
				else {
					if( pData->pLOG_VT_SESS->VideoUpLossCnt > 0)
						pData->pLOG_VT_SESS->VideoUpLossCnt--;
					pData->pLOG_VT_SESS->RTPUpLossCnt
						= pData->pLOG_VT_SESS->AudioUpLossCnt+pData->pLOG_VT_SESS->VideoUpLossCnt;
					log_print(LOGN_INFO, "LOSSCNT8: CAPTIME[%ld.%06ld] UP[%u] DOWN[%u]",
							pstCAPHEAD->curtime, pstCAPHEAD->ucurtime,
							pData->pLOG_VT_SESS->RTPUpLossCnt, pData->pLOG_VT_SESS->RTPDnLossCnt);
				}
			}
		}
	}

	return 0;
}


/*
 *  $Log: vt_main.c,v $
 *  Revision 1.2  2011/09/06 12:46:40  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.5  2011/08/25 07:25:47  uamyd
 *  nifo_msg_write api or log changed to gifo_write
 *
 *  Revision 1.4  2011/08/11 08:09:11  hhbaek
 *  Commit A_VT
 *
 *  Revision 1.3  2011/08/09 09:06:42  hhbaek
 *  Commit A_VT
 *
 *  Revision 1.2  2011/08/09 05:20:01  hhbaek
 *  Commit A_VT service block
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.45  2011/05/11 19:40:24  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.44  2011/05/11 19:37:33  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.43  2011/05/09 13:02:05  dark264sh
 *  A_VT: A_CALL multi 처리
 *
 *  Revision 1.42  2011/05/06 21:32:11  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.41  2011/05/06 04:08:15  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.40  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.2  2010/11/14 14:31:10  jwkim96
 *  STP 작업 내용 반영.
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.39  2010/04/09 07:27:09  dqms
 *  A_VT RTP트래픽 업/다운 처리방법 수정
 *
 *  Revision 1.38  2009/10/01 05:35:45  pkg
 *  BYE 이후에 오는 RTP트래픽을 위해 1초의 타임아웃 시간을 대기한다
 *
 *  Revision 1.37  2009/09/28 08:47:50  pkg
 *  RTP 트래픽처리 할 때 timerN_update 실행 추가
 *
 *  Revision 1.36  2009/09/18 12:06:21  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.35  2009/09/18 11:58:07  pkg
 *  디버그로그 정리 및 LossCnt 처리
 *
 *  Revision 1.34  2009/09/07 13:27:32  jsyoon
 *  LGT요구 응답코드 성공처리
 *
 *  Revision 1.33  2009/08/28 07:34:34  jsyoon
 *  세션 및 타이머 정보 출력
 *
 *  Revision 1.32  2009/08/25 16:49:33  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.31  2009/08/25 12:16:14  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.30  2009/08/22 19:46:51  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.29  2009/08/22 19:27:08  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.28  2009/08/22 18:37:09  jsyoon
 *  INVITE 외의 MSG는 SIPD -> A_VT -> A_IM
 *
 *  Revision 1.27  2009/08/19 18:53:28  pkg
 *  *** empty log message ***
 *
 *  Revision 1.26  2009/08/19 18:00:59  pkg
 *  IM, VT 서비스 에러코드 세팅 변경
 *
 *  Revision 1.25  2009/08/19 12:30:12  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.24  2009/08/17 17:54:18  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.23  2009/08/17 13:19:48  jsyoon
 *  ReportCnt 가 있을 경우에만 RR 디코딩vt_main.cc
 *
 *  Revision 1.22  2009/08/15 20:26:50  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.21  2009/08/15 20:16:12  jsyoon
 *  RTP 세션 로직 변경
 *
 *  Revision 1.20  2009/08/15 09:26:32  jsyoon
 *  CHECK POINT pstRTCP_RR is NULL
 *
 *  Revision 1.19  2009/08/10 11:07:52  dqms
 *  Add LOG_SIP_TRANS->isUsed Flag
 *
 *  Revision 1.18  2009/08/04 12:08:17  dqms
 *  TIMER를 공유메모리로 변경
 *
 *  Revision 1.17  2009/07/27 05:36:38  dqms
 *  TCPSESS LIST 수정
 *
 *  Revision 1.16  2009/07/22 05:10:39  dqms
 *  *** empty log message ***
 *
 *  Revision 1.15  2009/07/20 05:32:09  dqms
 *  ETC 트래픽 패스 변경
 *
 *  Revision 1.14  2009/07/19 12:04:13  dqms
 *  타이머 업데이트 및 콜스탑 메세지 처리
 *
 *  Revision 1.13  2009/07/17 09:56:20  jsyoon
 *  CALL_STOP_NUM 메세지 처리
 *
 *  Revision 1.12  2009/07/15 17:10:56  dqms
 *  set_version 위치 및 Plastform Type 변경
 *
 *  Revision 1.11  2009/07/05 15:39:40  dqms
 *  *** empty log message ***
 *
 *  Revision 1.10  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.9  2009/06/25 17:41:12  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.8  2009/06/18 16:32:42  jsyoon
 *  오디오 비디오 트래픽 분리
 *
 *  Revision 1.7  2009/06/15 09:04:57  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.6  2009/06/14 08:39:23  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.5  2009/06/13 11:44:11  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.4  2009/06/13 11:39:46  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.3  2009/06/12 07:23:25  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/06/11 19:27:20  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1  2009/06/10 21:46:09  jsyoon
 *  *** empty log message ***
 *
 */



