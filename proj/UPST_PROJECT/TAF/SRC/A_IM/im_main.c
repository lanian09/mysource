/**		@file	im_main.c
 * 		-  IM Session Management
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		@ID 		$Id: im_main.c,v 1.4 2011/09/07 04:59:33 uamyd Exp $
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/07 04:59:33 $
 * 		@warning	.
 * 		@ref		im_main.c im_init.c
 * 		@todo		SIP, MSRP, XCAP
 *
 * 		@section	Intro(소개)
 * 		- IM Session Management
 *
 **/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "common_stg.h"
#include "path.h"
#include "sshmid.h"
#include "capdef.h"
#include "filter.h"

// LIB
#include "config.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "Analyze_Ext_Abs.h"

// .
#include "im_init.h"
#include "im_msgq.h"

/**
 *	Define constants
 */
#define ISSAME 				0

/**
 *	Declare var.
 */
S32					giFinishSignal;			/**< Finish Signal */
S32					giStopFlag;				/**< main loop Flag 0: Stop, 1: Loop */
S32					gACALLCnt = 0;
int					guiTimerValue;
static stMEMSINFO	*gpMEMSINFO;			/**< new interface 관리 구조체 */
static stHASHOINFO	*gpHASHOINFO;			/**< new interface 관리 구조체 */
static stTIMERNINFO	*gpTIMERNINFO;			/**< new interface 관리 구조체 */
stCIFO				*gpCIFO;
st_Flt_Info			*flt_info;

/**
 *	Implement func.
 */
S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime)
{   
	S64     gapTime;

	gapTime = (((S64)endtime * 1000000 + (S64)endmtime) - ((S64)starttime * 1000000 + (S64)startmtime));

	if(gapTime < 0)
		gapTime = 0;

	return gapTime;
}       

char *PrintTYPE(S32 type)
{
	switch(type)
	{
	case LOG_SIP_TRANS_DEF_NUM: 		return "LOG_SIP";
	case LOG_MSRP_TRANS_DEF_NUM: 		return "LOG_MSRP";
	case LOG_HTTP_TRANS_DEF_NUM:		return "LOG_XCAP";
	default: 							return "UNKNOWN";
	}
}

char *PrintTcpFlag(S32 tcpflag)
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

S32 Send_IM_Session_LOG(IM_SESSION_HASH_DATA *pstIMSESS)
{
	S32				dRet;
	//S32				qid;
	S32				dSeqProcID;

	OFFSET 			offset;
	U8				*pLOGIMNODE;

	LOG_IM_SESS		*pLOGIMSESS;

	struct timeval  stNowTime;


	offset = pstIMSESS->offset;
	pLOGIMSESS = pstIMSESS->pLOG_IM_SESS;

	log_print(LOGN_DEBUG, "@@@ Send_IM_SESS CIP[%d.%d.%d.%d] OFFSET[%ld]", 
			HIPADDR(pLOGIMSESS->uiClientIP), offset );

	/* pLOGIMSESS가 끝날때 수행하는 부분 */
	gettimeofday(&stNowTime, NULL);
	pLOGIMSESS->OpEndTime = stNowTime.tv_sec;
	pLOGIMSESS->OpEndMTime = stNowTime.tv_usec;

	pLOGIMSESS->SessGapTime = GetGapTime(pLOGIMSESS->LastPktTime, pLOGIMSESS->LastPktMTime,
			pLOGIMSESS->SessStartTime, pLOGIMSESS->SessStartMTime);

	if(pLOGIMSESS->LastUserErrCode == 0) {
		if(pLOGIMSESS->MSRPTotalReqCnt == 0 && pLOGIMSESS->MSRPTotalResCnt == 0) {
			pLOGIMSESS->LastUserErrCode = IM_UERR_NOPLAY;
		}
		else if(pLOGIMSESS->LastMethod != SIP_MSG_BYE) {
			pLOGIMSESS->LastUserErrCode = IM_UERR_TIMEOUT;
		}
	}

//	LOG_IM_SESS_Prt("IM SESSION LOG ", pLOGIMSESS);
//	pLOGIMNODE = nifo_ptr(gpMEMSINFO, offset);
	pLOGIMNODE = nifo_ptr(gpMEMSINFO, nifo_get_offset_node(gpMEMSINFO, (U8 *)pstIMSESS->pLOG_IM_SESS));

	//qid = dGetCALLQID(pLOGIMSESS->uiClientIP);
	dSeqProcID = SEQ_PROC_A_CALL + (pLOGIMSESS->uiClientIP % gACALLCnt);
	//if((dRet = dSend_IM_Data(gpMEMSINFO, qid, pLOGIMNODE)) < 0) {
	if((dRet = dSend_IM_Data(gpMEMSINFO, dSeqProcID, pLOGIMNODE)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
		return -5;
	}
	pstIMSESS->pLOG_IM_SESS = 0;

	return 0;
}


void invoke_del_CALL(void *p)
{
	st_CALLTimer  			*pstCALLTimer;
	IM_SESSION_HASH_DATA 	*pstIMSESS;
	stHASHONODE				*pHASHNODE;

	pstCALLTimer = (st_CALLTimer *) p;

	log_print(LOGN_DEBUG, "@@@ TIMER TIMEOUT CIP[%d.%d.%d.%d]", HIPADDR(pstCALLTimer->ClientIP) );

	if((pHASHNODE = hasho_find(gpHASHOINFO, (U8 *) &pstCALLTimer->ClientIP)) != NULL) { 
		pstIMSESS = (IM_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHNODE->offset_Data);
		log_print(LOGN_DEBUG, "@@@ INVOKE TIMEOUT CIP[%d.%d.%d.%d]   NTIME[%ld]", HIPADDR(pstCALLTimer->ClientIP) , time(NULL) );

		Send_IM_Session_LOG(pstIMSESS);
		if(pstIMSESS->pLOG_IM_SESS){
			U8 *__pNODE;
			__pNODE = nifo_ptr(gpMEMSINFO, nifo_get_offset_node(gpMEMSINFO, (U8 *)pstIMSESS->pLOG_IM_SESS));
			nifo_node_delete(gpMEMSINFO, __pNODE);
			pstIMSESS->pLOG_IM_SESS = NULL;
		}
		hasho_del(gpHASHOINFO, (U8 *) &pstCALLTimer->ClientIP);
	} else {
		log_print(LOGN_CRI, "INVOKE TIMEOUT BUT NODE NULL CIP[%d.%d.%d.%d]   NTIME[%ld]", HIPADDR(pstCALLTimer->ClientIP) , time(NULL) );
	}
}

void IM_Sess_Traffic(LOG_IM_SESS *pLOGIMSESS, int type, unsigned char *pDATA)
{
	LOG_SIP_TRANS 	*pLOGSIP;
	LOG_MSRP_TRANS 	*pLOGMSRP;
	LOG_HTTP_TRANS 	*pLOGXCAP;

	switch(type)
	{
		case LOG_SIP_TRANS_DEF_NUM: 	/* SIP */
			pLOGSIP= (LOG_SIP_TRANS *) pDATA;

			pLOGIMSESS->ReqDataSize			+= pLOGSIP->ReqDataSize;                                
			pLOGIMSESS->ResDataSize			+= pLOGSIP->ResDataSize;                                
			pLOGIMSESS->ReqIPDataSize		+= pLOGSIP->ReqIPDataSize;                               
			pLOGIMSESS->ResIPDataSize		+= pLOGSIP->ResIPDataSize;                              

			break;
		case LOG_MSRP_TRANS_DEF_NUM: 	/* MSRP */
			pLOGMSRP= (LOG_MSRP_TRANS *) pDATA;

			pLOGIMSESS->ReqBodySize			+= pLOGMSRP->ReqBodySize;                                
			pLOGIMSESS->ResBodySize			+= pLOGMSRP->ResBodySize;                                 
			pLOGIMSESS->ReqDataSize			+= pLOGMSRP->ReqDataSize;                                
			pLOGIMSESS->ResDataSize			+= pLOGMSRP->ResDataSize;                                
			pLOGIMSESS->ReqIPDataSize		+= pLOGMSRP->ReqIPDataSize;                               
			pLOGIMSESS->ResIPDataSize		+= pLOGMSRP->ResIPDataSize;                              
			pLOGIMSESS->RetransReqDataSize 	+= pLOGMSRP->RetransReqDataSize;                         
			pLOGIMSESS->RetransResDataSize 	+= pLOGMSRP->RetransResDataSize;                         

			break;
		case LOG_HTTP_TRANS_DEF_NUM: 	/* XCAP */
			pLOGXCAP= (LOG_HTTP_TRANS *) pDATA;

			pLOGIMSESS->ReqBodySize			+= pLOGXCAP->uiUpBodySize;                                
			pLOGIMSESS->ResBodySize			+= pLOGXCAP->uiDnBodySize;                                 
			pLOGIMSESS->ReqDataSize			+= pLOGXCAP->uiTcpUpBodySize;                                
			pLOGIMSESS->ResDataSize			+= pLOGXCAP->uiTcpDnBodySize;                                
			pLOGIMSESS->ReqIPDataSize		+= pLOGXCAP->uiIPDataUpPktSize;                               
			pLOGIMSESS->ResIPDataSize		+= pLOGXCAP->uiIPDataDnPktSize;                              
			pLOGIMSESS->RetransReqDataSize 	+= pLOGXCAP->uiIPDataUpRetransSize;                         
			pLOGIMSESS->RetransResDataSize 	+= pLOGXCAP->uiIPDataDnRetransSize; 

			break;
		default:
			log_print(LOGN_CRI, "IMPOSSIBLE TYPE[%d]", type);
			break;
	}

	return ;
}

void IM_Sess_Update(LOG_IM_SESS *pLOGIMSESS, int type, unsigned char *pDATA)
{
	struct timeval  stNowTime;

	LOG_SIP_TRANS 	*pLOGSIP;
	LOG_MSRP_TRANS 	*pLOGMSRP;
	LOG_HTTP_TRANS 	*pLOGXCAP;

	gettimeofday(&stNowTime, NULL);

	switch(type)
	{
		case LOG_SIP_TRANS_DEF_NUM: 	/* SIP */
			pLOGSIP= (LOG_SIP_TRANS *) pDATA;

			pLOGIMSESS->LastMethod			= pLOGSIP->method;
			pLOGIMSESS->LastResCode			= pLOGSIP->ResCode;
			pLOGIMSESS->L7FailCode			= pLOGSIP->L7FailCode;

			pLOGIMSESS->SkipResCnt			+= pLOGSIP->SkipResCnt;

			pLOGIMSESS->RetransReqCnt		+= pLOGSIP->RetransReqCnt;
			pLOGIMSESS->RetransResCnt		+= pLOGSIP->RetransResCnt;

			pLOGIMSESS->LastPktTime = pLOGSIP->TransEndTime;
			pLOGIMSESS->LastPktMTime = pLOGSIP->TransEndMTime;

			break;
		case LOG_MSRP_TRANS_DEF_NUM: 	/* MSRP */
			pLOGMSRP= (LOG_MSRP_TRANS *) pDATA;

			pLOGIMSESS->MSRPTotalReqCnt 	+= pLOGMSRP->TotalReqCnt;
			pLOGIMSESS->MSRPTotalResCnt 	+= pLOGMSRP->TotalResCnt;
			pLOGIMSESS->MSRPTotalReportCnt 	+= pLOGMSRP->TotalReportCnt;
			pLOGIMSESS->MSRPRetransReqCnt 	+= pLOGMSRP->RetransReqCnt;
			pLOGIMSESS->MSRPRetransResCnt 	+= pLOGMSRP->RetransResCnt;

			pLOGIMSESS->ReportBodySize 		+= pLOGMSRP->ReportBodySize;                              
			pLOGIMSESS->ReportDataSize 		+= pLOGMSRP->ReportDataSize;                             
			pLOGIMSESS->ReportIPDataSize 	+= pLOGMSRP->ReportIPDataSize;                           

			pLOGIMSESS->LastPktTime = pLOGMSRP->TransEndTime;
			pLOGIMSESS->LastPktMTime = pLOGMSRP->TransEndMTime;

			break;
		case LOG_HTTP_TRANS_DEF_NUM: 	/* XCAP */
			pLOGXCAP= (LOG_HTTP_TRANS *) pDATA;

			pLOGIMSESS->XCAPTotalReqCnt 	+= pLOGXCAP->uiIPDataUpPktCnt;
			pLOGIMSESS->XCAPTotalResCnt 	+= pLOGXCAP->uiIPDataDnPktCnt;
			pLOGIMSESS->XCAPRetransReqCnt 	+= pLOGXCAP->uiIPDataUpRetransCnt;
			pLOGIMSESS->XCAPRetransResCnt 	+= pLOGXCAP->uiIPDataDnRetransCnt;

			pLOGIMSESS->LastPktTime = pLOGXCAP->uiLastPktTime;
			pLOGIMSESS->LastPktMTime = pLOGXCAP->uiLastPktMTime;

			break;
		default:
			log_print(LOGN_CRI, "IMPOSSIBLE TYPE[%d]", type);
			break;
	}

    /* SET DATA TRAFFIC SIZE */
    pLOGIMSESS->TotalReqCnt			+= pLOGIMSESS->MSRPTotalReqCnt + pLOGIMSESS->XCAPTotalReqCnt;
    pLOGIMSESS->TotalResCnt			+= pLOGIMSESS->MSRPTotalResCnt + pLOGIMSESS->XCAPTotalResCnt;
    pLOGIMSESS->RetransReqCnt		+= pLOGIMSESS->MSRPRetransReqCnt + pLOGIMSESS->XCAPRetransReqCnt;
    pLOGIMSESS->RetransResCnt		+= pLOGIMSESS->MSRPRetransReqCnt + pLOGIMSESS->XCAPRetransReqCnt;

	IM_Sess_Traffic(pLOGIMSESS, type, pDATA);


	return;
}

void IM_Sess_Init(LOG_IM_SESS *pLOGIMSESS, LOG_SIP_TRANS *pLOGSIP) 
{
	struct timeval  stNowTime;

	gettimeofday(&stNowTime, NULL);

	memcpy(pLOGIMSESS, pLOGSIP, LOG_COMMON_SIZE);

	pLOGIMSESS->SessStartTime 	= pLOGSIP->TransStartTime;
	pLOGIMSESS->SessStartMTime 	= pLOGSIP->TransStartMTime;
	pLOGIMSESS->LastPktTime 	= pLOGSIP->TransEndTime;
	pLOGIMSESS->LastPktMTime 	= pLOGSIP->TransEndMTime;

	pLOGIMSESS->usPlatformType 	= DEF_PLATFORM_IM;
	pLOGIMSESS->usSvcL4Type 	= pLOGSIP->usSvcL4Type;
	pLOGIMSESS->usSvcL7Type 	= pLOGSIP->usSvcL7Type;

	memcpy(&pLOGIMSESS->CallID[0], &pLOGSIP->CallID[0], SIP_CALLID_LEN);
	pLOGIMSESS->CallID[SIP_CALLID_LEN] = 0x00;

	pLOGIMSESS->LastMethod 		= pLOGSIP->method;
	pLOGIMSESS->LastResCode 	= pLOGSIP->ResCode;

	switch(pLOGSIP->ResCode) 
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
			pLOGIMSESS->SetupEndTime    = pLOGSIP->TransEndTime;            
			pLOGIMSESS->SetupEndMTime   = pLOGSIP->TransEndMTime;
			break;
		default:
			pLOGIMSESS->L7FailCode = IM_UERR_NOSETUP;
			pLOGIMSESS->LastUserErrCode = pLOGIMSESS->L7FailCode;
			break;
	}

    /* SET DATA TRAFFIC SIZE */
    pLOGIMSESS->TotalReqCnt 	+= pLOGSIP->TotalReqCnt;
    pLOGIMSESS->TotalResCnt 	+= pLOGSIP->TotalResCnt;
	pLOGIMSESS->SkipResCnt 		+= pLOGSIP->SkipResCnt;
    pLOGIMSESS->RetransReqCnt 	+= pLOGSIP->RetransReqCnt;
    pLOGIMSESS->RetransResCnt 	+= pLOGSIP->RetransResCnt;

	pLOGIMSESS->ReqDataSize		+= pLOGSIP->ReqDataSize;                                
	pLOGIMSESS->ResDataSize		+= pLOGSIP->ResDataSize;                                
	pLOGIMSESS->ReqIPDataSize	+= pLOGSIP->ReqIPDataSize;                               
	pLOGIMSESS->ResIPDataSize	+= pLOGSIP->ResIPDataSize;                              


	/* Operation Time */
	pLOGIMSESS->OpStartTime = stNowTime.tv_sec;
	pLOGIMSESS->OpStartMTime = stNowTime.tv_usec;

    pLOGIMSESS->ClientPort      = pLOGSIP->SrcPort;
    pLOGIMSESS->ServerIP        = pLOGSIP->DestIP;
    pLOGIMSESS->ServerPort      = pLOGSIP->DestPort;

	return;
}

/*
 * SIP, MSRP, XCAP Message 
 */
int IM_Session_Process( int type, int len, char *data)
{
	st_IMSessKey 	stKey;
	st_IMSessKey 	*pstKey;

	LOG_COMMON 		*pCOMMON;

	LOG_SIP_TRANS 	*pLOGSIP 	= NULL;
	LOG_MSRP_TRANS 	*pLOGMSRP 	= NULL;
	LOG_HTTP_TRANS 	*pLOGXCAP 	= NULL;

	LOG_IM_SESS 	*pLOGIMSESS;

	stHASHONODE     		*pHASHONODE;

	IM_SESSION_HASH_DATA 	stIMSESS;
	IM_SESSION_HASH_DATA 	*pstIMSESS;

	st_CALLTimer			stCALLTimer, *pstCALLTimer;

	unsigned char			*pNode;


	pstCALLTimer = &stCALLTimer;
	pCOMMON = (LOG_COMMON *) data;

	memset(&stKey, 0, sizeof(st_IMSessKey));
	pstKey = &stKey;

	pstKey->uiClientIP = pCOMMON->uiClientIP;

	switch(type)
	{
		case LOG_SIP_TRANS_DEF_NUM: 	/* SIP */
			pLOGSIP= (LOG_SIP_TRANS *) data;
			pLOGSIP->usPlatformType = DEF_PLATFORM_IM;
			break;
		case LOG_MSRP_TRANS_DEF_NUM: 	/* MSRP */
			pLOGMSRP= (LOG_MSRP_TRANS *) data;
			pLOGMSRP->usPlatformType = DEF_PLATFORM_IM;
			break;
		case LOG_HTTP_TRANS_DEF_NUM: 	/* XCAP */
			pLOGXCAP= (LOG_HTTP_TRANS *) data;
			pLOGXCAP->usPlatformType = DEF_PLATFORM_IM;
			break;
		default:
			log_print(LOGN_WARN, "IMPOSSIBLE TYPE[%d]", type);
			break;
	}

	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_IM_TIMEOUT];
	/* 1. Found IM Session */
	if( (pHASHONODE = hasho_find(gpHASHOINFO, (U8 *)pstKey)) ) {

		pstIMSESS = (IM_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
		pLOGIMSESS = pstIMSESS->pLOG_IM_SESS;

		log_print(LOGN_DEBUG, "FOUND IM SESSION : IP[%d.%d.%d.%d] METHOD[%d] Call-ID[%s]", 
				HIPADDR(pLOGIMSESS->uiClientIP), pLOGIMSESS->LastMethod, pLOGIMSESS->CallID);

		if( pLOGSIP && pLOGSIP->method == SIP_MSG_INVITE && 
				(strcmp(pLOGIMSESS->CallID, pLOGSIP->CallID) != ISSAME) ) {
			/* 1.1 New request should have different call-ID */
			log_print(LOGN_INFO, "SEND SAME CALLID IM SESSION : IP[%d.%d.%d.%d] METHOD[%d] Call-ID[%s]", 
					HIPADDR(pstKey->uiClientIP), pLOGSIP->method, pLOGIMSESS->CallID);

			Send_IM_Session_LOG(pstIMSESS);
			if(pstIMSESS->pLOG_IM_SESS) {
				U8 *pDELNode;

				pDELNode = nifo_ptr(gpMEMSINFO, pstIMSESS->offset);
				nifo_node_delete(gpMEMSINFO, pDELNode);
			}
			timerN_del(gpTIMERNINFO, pstIMSESS->timerNID);
			hasho_del(gpHASHOINFO, (U8 *) &pstKey->uiClientIP);
	
			/* 1.2 Create New IM Session */
			memset((char *)&stIMSESS, 0x0, sizeof(IM_SESSION_HASH_DATA));
			pstIMSESS = &stIMSESS;

			if((pHASHONODE = hasho_add(gpHASHOINFO, (U8 *)pstKey, (U8 *)pstIMSESS)) == NULL) {
				log_print(LOGN_CRI, "[%s.%d] ERROR : hasho_add NULL", __FUNCTION__, __LINE__);
				return -2;
			} else {
				pstIMSESS = (IM_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
				memset(pstIMSESS, 0x00, IM_SESSION_HASH_DATA_SIZE);

				pstCALLTimer->ClientIP = pLOGSIP->uiClientIP;
				pstIMSESS->timerNID = timerN_add(gpTIMERNINFO, invoke_del_CALL, (U8 *)pstCALLTimer, sizeof(st_CALLTimer), time(NULL) + guiTimerValue);

				/* LOG_IM_SESS TLV 할당 */
				if((pNode = nifo_node_alloc(gpMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_IM_SESS nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -2;
				}
				if((pstIMSESS->pLOG_IM_SESS = (LOG_IM_SESS *) nifo_tlv_alloc(gpMEMSINFO, pNode, LOG_IM_SESS_DEF_NUM, LOG_IM_SESS_SIZE, DEF_MEMSET_ON)) == NULL) {
					log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_IM_SESS nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					nifo_node_unlink_nont(gpMEMSINFO, pNode);
					nifo_cont_delete(gpMEMSINFO, pNode);
					return -3;
				}
				pstIMSESS->offset = nifo_offset (gpMEMSINFO, pstIMSESS->pLOG_IM_SESS);
				pLOGIMSESS = pstIMSESS->pLOG_IM_SESS;

				/* pLOG_IM_SESS 생성시 초기화 */
				IM_Sess_Init(pLOGIMSESS, pLOGSIP);
			}
			pLOGSIP->isUsed = 1;
			log_print(LOGN_INFO, "CREATE NEW IM SESSION : IP[%d.%d.%d.%d] METHOD[%d] Call-ID[%s]", 
					HIPADDR(pLOGIMSESS->uiClientIP), pLOGIMSESS->LastMethod, pLOGIMSESS->CallID);
		}

		if( pLOGSIP && pLOGSIP->method != SIP_MSG_INVITE ) {
			/* SET DATA TRAFFIC SIZE */
			pLOGIMSESS->TotalReqCnt 	+= pLOGSIP->TotalReqCnt;
			pLOGIMSESS->TotalResCnt 	+= pLOGSIP->TotalResCnt;
			pLOGIMSESS->SkipResCnt 		+= pLOGSIP->SkipResCnt;
			pLOGIMSESS->RetransReqCnt 	+= pLOGSIP->RetransReqCnt;
			pLOGIMSESS->RetransResCnt 	+= pLOGSIP->RetransResCnt;

			pLOGIMSESS->ReqDataSize		+= pLOGSIP->ReqDataSize;                                
			pLOGIMSESS->ResDataSize		+= pLOGSIP->ResDataSize;                                
			pLOGIMSESS->ReqIPDataSize	+= pLOGSIP->ReqIPDataSize;                               
			pLOGIMSESS->ResIPDataSize	+= pLOGSIP->ResIPDataSize;                              

			pLOGSIP->isUsed = 1;
		}
	}
	/* 2. Not Found IM Session */
	else {
		/* 2.1 Create New IM Session */
		if ( pLOGSIP && pLOGSIP->method == SIP_MSG_INVITE ) {
			memset((char *)&stIMSESS, 0x0, sizeof(IM_SESSION_HASH_DATA));
			pstIMSESS = &stIMSESS;

			if((pHASHONODE = hasho_add(gpHASHOINFO, (U8 *)pstKey, (U8 *)pstIMSESS)) == NULL) {
				log_print(LOGN_CRI, "[%s.%d] ERROR : hasho_add NULL", __FUNCTION__, __LINE__);
				return -2;
			} else {
				pstIMSESS = (IM_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
				memset(pstIMSESS, 0x00, IM_SESSION_HASH_DATA_SIZE);

				pstCALLTimer->ClientIP = pLOGSIP->uiClientIP;
				pstIMSESS->timerNID = timerN_add(gpTIMERNINFO, invoke_del_CALL, (U8 *)pstCALLTimer, sizeof(st_CALLTimer), time(NULL) + guiTimerValue);

				/* LOG_IM_SESS TLV 할당 */
				if((pNode = nifo_node_alloc(gpMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_IM_SESS nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -2;
				}
				if((pstIMSESS->pLOG_IM_SESS = (LOG_IM_SESS *) nifo_tlv_alloc(gpMEMSINFO, pNode, LOG_IM_SESS_DEF_NUM, LOG_IM_SESS_SIZE, DEF_MEMSET_ON)) == NULL) {
					log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pLOG_IM_SESS nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					nifo_node_unlink_nont(gpMEMSINFO, pNode);
					nifo_cont_delete(gpMEMSINFO, pNode);
					return -3;
				}
				pstIMSESS->offset = nifo_offset (gpMEMSINFO, pstIMSESS->pLOG_IM_SESS);
				pLOGIMSESS = pstIMSESS->pLOG_IM_SESS;

				/* pLOG_IM_SESS 생성시 초기화 */
				IM_Sess_Init(pLOGIMSESS, pLOGSIP);
			}
			pLOGSIP->isUsed = 1;
			log_print(LOGN_INFO, "CREATE NEW IM SESSION : IP[%d.%d.%d.%d] METHOD[%d] Call-ID[%s]", 
					HIPADDR(pLOGIMSESS->uiClientIP), pLOGIMSESS->LastMethod, pLOGIMSESS->CallID);
		} else {
			log_print(LOGN_INFO, "BYPASS MSG TYPE:[%s] IP[%d.%d.%d.%d]", PrintTYPE(type), HIPADDR(pstKey->uiClientIP));
			return 0;
		}
	}

	if(pstIMSESS) {
		if( pstIMSESS->pLOG_IM_SESS == NULL) {
			log_print(LOGN_CRI, "[%s] ERROR [%s.%d] pstIMSESS EXIST BUT pLOG_IM_SESS is NULL", __FILE__, __FUNCTION__, __LINE__);
			return -4;
		} else if( data ) {
			IM_Sess_Update(pLOGIMSESS, type, data);
			pstIMSESS->timerNID = 
				timerN_update(gpTIMERNINFO, pstIMSESS->timerNID, time(NULL) + guiTimerValue);
			log_print(LOGN_INFO, "UPDATE IM SESSION : IP[%d.%d.%d.%d] type[%s]", HIPADDR(pLOGIMSESS->uiClientIP), PrintTYPE(type));
		}

		if(pLOGSIP && pLOGSIP->method == SIP_MSG_BYE ) {
			log_print(LOGN_DEBUG, "RECEIVE SIP_MSG_BYE CLEAR IM SESSION : IP[%d.%d.%d.%d]", HIPADDR(pLOGIMSESS->uiClientIP));
			Send_IM_Session_LOG(pstIMSESS);
			if(pstIMSESS->pLOG_IM_SESS) {
				U8 *pDELNode;

				pDELNode = nifo_ptr(gpMEMSINFO, pstIMSESS->offset);
				nifo_node_delete(gpMEMSINFO, pDELNode);
			}
			timerN_del(gpTIMERNINFO, pstIMSESS->timerNID);
			hasho_del(gpHASHOINFO, (U8 *) &pstKey->uiClientIP);
		}
	}

	return 0;
}

int IM_Session_Update(char *data)
{
	st_IMSessKey 	stKey;
	st_IMSessKey 	*pstKey;

	CALL_KEY 		*pCALLKEY = NULL;
	stHASHONODE 	*pHASHONODE;

	IM_SESSION_HASH_DATA 	*pstIMSESS;


	pCALLKEY = (CALL_KEY *) data;

	memset(&stKey, 0, sizeof(st_IMSessKey));
	pstKey = &stKey;

	pstKey->uiClientIP = pCALLKEY->uiSrcIP;
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_IM_TIMEOUT];

	/* 1. Found IM Session */
	if( (pHASHONODE = hasho_find(gpHASHOINFO, (U8 *)pstKey)) ) {

		pstIMSESS = (IM_SESSION_HASH_DATA *)nifo_ptr(gpHASHOINFO, pHASHONODE->offset_Data);
	
		pstIMSESS->timerNID =
			timerN_update(gpTIMERNINFO, pstIMSESS->timerNID, time(NULL) + guiTimerValue);

		log_print(LOGN_INFO, "UPDATE TIMER IM SESSION : IP[%d.%d.%d.%d] METHOD[%d] Call-ID[%s] TIMEOUT[%d]", 
				HIPADDR(pstIMSESS->pLOG_IM_SESS->uiClientIP), pstIMSESS->pLOG_IM_SESS->LastMethod, 
				pstIMSESS->pLOG_IM_SESS->CallID, DEF_WAIT_TIMEOUT);
	} else {
		log_print(LOGN_INFO, "NOT FOUND IM SESSION : IP[%d.%d.%d.%d]", HIPADDR(pstKey->uiClientIP));
	}

	return 0;
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
	//S32						qid;
	S32						dSeqProcID;
	OFFSET					offset;
	U8						*pNode;
	U8						*pNextNode, *pCurNode;
	U8						*p, *data;
	S32						type, len, ismalloc, dSendFlag;

	Capture_Header_Msg		*pCAPHEAD;
	INFO_ETH				*pINFOETH;

    char    vERSION[7] = "R3.0.0";

	/* Log 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_VT, LOG_PATH"/A_VT/", "A_VT");

#if 0
	/* dAppLog 초기화 */
	InitAppLog(getpid(), SEQ_PROC_A_IM, LOG_"A_IM", "A_IM");

	if( (dRet = Init_shm_common()) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInit_shm_common dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}
#endif

	/* A_IM 초기화 */
	if((dRet = dInitIM(&gpMEMSINFO , &gpHASHOINFO , &gpTIMERNINFO)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInitIM dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	//if((dRet = set_version(SEQ_PROC_A_IM, vERSION)) < 0 )
	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_IM, vERSION)) < 0)
    {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_IM, vERSION);
    }
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_IM_TIMEOUT];
	log_print(LOGN_CRI, "START A_IM VERSION: %s SESSCNT: %d TIMEOUT: %d ", vERSION, IM_SESS_CNT, guiTimerValue);

	/* MAIN LOOP */
	while(giStopFlag)
	{
	 	timerN_invoke(gpTIMERNINFO);

		//if((offset = nifo_msg_read(gpMEMSINFO, dMyQID, NULL)) > 0) {
		if((offset = gifo_read(gpMEMSINFO, gpCIFO, SEQ_PROC_A_IM)) > 0)
		{
			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(gpMEMSINFO, offset);
			pNextNode = pNode;

			pCAPHEAD = NULL;
			pINFOETH = NULL;
			dSendFlag = 1;
			//qid = 0;
			dSeqProcID = 0;

			do {
				p = pNextNode;
				pCurNode = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(gpMEMSINFO, pCurNode, &type, &len, &data, &ismalloc, &p)) < 0)
						break;

					log_print(LOGN_INFO, 	"TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", type, 
							(type==CLEAR_CALL_NUM) ? PRINT_TAG_DEF_ALL_CALL_INPUT(type) : PRINT_DEF_NUM_table_log(type), len, 
							(ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");

					switch(type)
					{
						case CLEAR_CALL_NUM:				/* Session Clear Message from A_CALL */
							IM_Session_Update(data);
							dSendFlag = 0;
							break;
						case START_CALL_NUM:
							dSendFlag = 0;
							break;
						case LOG_SIP_TRANS_DEF_NUM: 	/* SIP */
						case LOG_MSRP_TRANS_DEF_NUM: 	/* MSRP */
						case LOG_HTTP_TRANS_DEF_NUM: 	/* XCAP */
							IM_Session_Process(type, len, data);
							//qid = dGetCALLQID(((LOG_COMMON *)data)->uiClientIP);
							dSeqProcID = SEQ_PROC_A_CALL + (((LOG_COMMON *)data)->uiClientIP % gACALLCnt);
							break;
						case CAP_HEADER_NUM:
						case INFO_ETH_NUM:
							break;
						default:
							log_print(LOGN_WARN, "????? UNKNOWN TYPE[%d]", type);
							break;
					}
					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}

				pNextNode = (U8 *)nifo_entry(nifo_ptr(gpMEMSINFO, ((NIFO *)pCurNode)->nont.offset_next), NIFO, nont);

				if (dSendFlag==0) {
					nifo_node_unlink_nont(gpMEMSINFO, pCurNode);
					nifo_node_delete(gpMEMSINFO, pCurNode);
					dSendFlag = 1;
				} else {
					//if((dRet = dSend_IM_Data(gpMEMSINFO, qid, pCurNode)) < 0) {
					if((dRet = dSend_IM_Data(gpMEMSINFO, dSeqProcID, pCurNode)) < 0) {
						log_print(LOGN_CRI, "[%s] ERROR [%s.%d] MSGQ WRITE FAILE[%d][%s]", 
								__FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					}
				}

			} while(pNode != pNextNode);

		} else {
			usleep(0);
		}
	}

	FinishProgram();

	return 0;
}


/*
 *  $Log: im_main.c,v $
 *  Revision 1.4  2011/09/07 04:59:33  uamyd
 *  modified
 *
 *  Revision 1.3  2011/09/05 01:35:32  uamyd
 *  modified to runnable source
 *
 *  Revision 1.2  2011/09/04 12:16:51  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.6  2011/08/25 07:25:47  uamyd
 *  nifo_msg_write api or log changed to gifo_write
 *
 *  Revision 1.5  2011/08/21 09:07:51  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.4  2011/08/21 07:21:31  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.3  2011/08/11 08:06:55  hhbaek
 *  Commit A_IM
 *
 *  Revision 1.2  2011/08/09 09:08:20  hhbaek
 *  Commit A_IM
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.24  2011/05/09 11:50:49  dark264sh
 *  A_IM: A_CALL multi 처리
 *
 *  Revision 1.23  2011/01/11 04:09:07  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.22  2009/09/07 13:27:14  jsyoon
 *  LGT요구 응답코드 성공처리
 *
 *  Revision 1.21  2009/08/28 07:34:43  jsyoon
 *  세션 및 타이머 정보 출력
 *
 *  Revision 1.20  2009/08/25 16:49:22  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.19  2009/08/25 12:16:03  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.18  2009/08/22 19:46:36  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.17  2009/08/22 16:15:36  pkg
 *  *** empty log message ***
 *
 *  Revision 1.16  2009/08/19 18:01:27  pkg
 *  IM, VT 서비스 에러코드 세팅 변경
 *
 *  Revision 1.15  2009/08/19 12:25:08  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.14  2009/08/16 14:16:57  jsyoon
 *  ADD usSvcL4Type
 *
 *  Revision 1.13  2009/08/15 16:34:45  jsyoon
 *  PROCESS START_CALL_NUM MSG
 *
 *  Revision 1.12  2009/08/10 11:07:52  dqms
 *  Add LOG_SIP_TRANS->isUsed Flag
 *
 *  Revision 1.11  2009/08/04 12:08:17  dqms
 *  TIMER를 공유메모리로 변경
 *
 *  Revision 1.10  2009/07/27 05:36:18  dqms
 *  MSG_WAIT_TIMEOUT 7로 수정
 *
 *  Revision 1.9  2009/07/22 05:08:10  dqms
 *  *** empty log message ***
 *
 *  Revision 1.8  2009/07/20 05:32:09  dqms
 *  ETC 트래픽 패스 변경
 *
 *  Revision 1.7  2009/07/19 12:04:13  dqms
 *  타이머 업데이트 및 콜스탑 메세지 처리
 *
 *  Revision 1.6  2009/07/17 09:56:20  jsyoon
 *  CALL_STOP_NUM 메세지 처리
 *
 *  Revision 1.5  2009/07/15 16:20:38  dqms
 *  ADD vIMSESSTimerReConstruct()
 *
 *  Revision 1.4  2009/07/05 15:39:40  dqms
 *  *** empty log message ***
 *
 *  Revision 1.3  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.2  2009/06/15 09:05:08  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1  2009/06/13 11:38:45  jsyoon
 *  *** empty log message ***
 *
 */



