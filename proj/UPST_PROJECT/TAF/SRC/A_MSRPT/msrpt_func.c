/**		@file	msrp_func.c
 * 		- MSRP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpt_func.c,v 1.4 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
 * 		@ref		msrp_func.c
 * 		@todo		
 *
 * 		@section	Intro(소개)
 * 		- MSRP Transaction을 관리 하는 함수들
 *
 * 		@section	Requirement
 * 		 @li	Nothing
 *
 **/

/**
 *	Include headers
 */
#include <sys/time.h>
#include <unistd.h>

// TOP
#include "common_stg.h"
#include "filter.h"
#include "procid.h"
#include "sshmid.h"

// LIB
#include "loglib.h"
#include "utillib.h"
#include "tools.h"

// .
#include "msrpt_util.h"
#include "msrpt_msgq.h"
#include "msrpt_func.h"

/**
 *	Declare var.
 */
extern st_Flt_Info	*flt_info;
extern int			guiTimerValue;

/**
 *	Declare extern func.
 */
extern void invoke_del(void *p);

/**
 *	Implement func.
 */
S32 dProcMSRPTTrans(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPTHASH, stTIMERNINFO *pstTIMERNINFO, TEXT_INFO *pstTEXTINFO, U8 *pDATA)
{
	S32						dHdrLen, dBodyLen;
	U8						*pNODE;
	LOG_MSRP_TRANS			*pLOG;

	st_MSRPT_INFO			stMSRPTINFO;
	st_MSRPT_INFO			*pstMSRPTINFO = &stMSRPTINFO;
	st_MSRPT_TRANS_KEY		stMSRPTTRANSKEY;
	st_MSRPT_TRANS_KEY		*pstMSRPTTRANSKEY = &stMSRPTTRANSKEY;
	st_MSRPT_TRANS			stMSRPTTRANS;
	st_MSRPT_TRANS			*pstMSRPTTRANS = &stMSRPTTRANS;

	stHASHONODE				*pstHASHONODE;
	st_MSRPT_COMMON			stMSRPTCOMMON;
	
    U8						szCIP[INET_ADDRSTRLEN];
    U8						szSIP[INET_ADDRSTRLEN];

	struct timeval			stTime;

	log_print(LOGN_DEBUG, "RCV DATA CIP=%s:%d SIP=%s:%d LEN=%d",
				util_cvtipaddr(szCIP, pstTEXTINFO->clientIP), pstTEXTINFO->clientPort,
				util_cvtipaddr(szSIP, pstTEXTINFO->serverIP), pstTEXTINFO->serverPort,
				pstTEXTINFO->len);

	gettimeofday(&stTime, NULL);

	/* GET MSRPT INFO */
	memset(pstMSRPTINFO, 0x00, DEF_MSRPTINFO_SIZE);
	if(dGetMSRPTINFO(pDATA, pstTEXTINFO->len, pstMSRPTINFO) < 0) {
		log_print(LOGN_CRI, LH"dGetMSRPTINFO", LT);
		return -1;
	}

	dHdrLen = dGetLen(pstTEXTINFO->len, (char*)pDATA);
	dBodyLen = pstTEXTINFO->len - dHdrLen;
	if((dBodyLen = dBodyLen - (strlen((char*)pstMSRPTINFO->szTID) + 8 + 2)) > 0)	/* 8: -------TID$, 2: 0x0D0x0A */
	{
		pstMSRPTINFO->usBodyLen = dBodyLen;
	}

	log_print(LOGN_INFO, 
		"PARSING Method=%s:%d ContentType=%s SuccReport=%s FailReport=%s ToPath=%s FromPath=%s MSGID=%s ResCode=%d Finish=%s:%d BODYLEN=%d",
				PrintMethod(pstMSRPTINFO->usMethod), pstMSRPTINFO->usMethod, pstMSRPTINFO->szContentType, 
				PrintReport(pstMSRPTINFO->usSuccessReport), PrintReport(pstMSRPTINFO->usFailureReport),
				pstMSRPTINFO->szToPath, pstMSRPTINFO->szFromPath, pstMSRPTINFO->szMSGID, pstMSRPTINFO->usResCode,
				PrintEndFlag(pstMSRPTINFO->usEndFlag), pstMSRPTINFO->usEndFlag, pstMSRPTINFO->usBodyLen);

	/* MSRPT HASH KEY 할당 */
	memset(pstMSRPTTRANSKEY, 0x00, DEF_MSRPTTRANSKEY_SIZE);
	pstMSRPTTRANSKEY->uiCliIP = pstTEXTINFO->clientIP;
	pstMSRPTTRANSKEY->uiSrvIP = pstTEXTINFO->serverIP;
	pstMSRPTTRANSKEY->usCliPort = pstTEXTINFO->clientPort;	
	pstMSRPTTRANSKEY->usSrvPort = pstTEXTINFO->serverPort;	
	memcpy(pstMSRPTTRANSKEY->szMSGID, pstMSRPTINFO->szMSGID, MSRP_MSGID_LEN);
	pstMSRPTTRANSKEY->szMSGID[MSRP_MSGID_LEN] = 0x00;

	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_MSRP_TIMEOUT];

	/* 기존 Transaction 검색 */
	if((pstHASHONODE = hasho_find(pstMSRPTHASH, (U8 *)pstMSRPTTRANSKEY)) == NULL)
	{
		/* 기존 Transaction 존재 하지 않는 경우 */
		log_print(LOGN_DEBUG, "NOT HAVE HASH CIP=%s:%d SIP=%s:%d MSGID=%s",
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID);	

		/* LOG 생성 */
		if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL)
		{
			log_print(LOGN_CRI, LH"nifo_node_alloc CIP=%s:%d SIP=%s:%d MSGID=%s",
					LT,
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID);	
			return -2;	
		}

		if((pLOG = (LOG_MSRP_TRANS *)nifo_tlv_alloc(pstMEMSINFO, pNODE, LOG_MSRP_TRANS_DEF_NUM, LOG_MSRP_TRANS_SIZE, DEF_MEMSET_ON)) == NULL)
		{
			log_print(LOGN_CRI, LH"nifo_tlv_alloc CIP=%s:%d SIP=%s:%d MSGID=%s",
					LT,
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID);	
			return -3;	

		}

		pLOG->OpStartTime = stTime.tv_sec;
		pLOG->OpStartMTime = stTime.tv_usec;
//		pLOG->usPlatformType = pstTEXTINFO->usL4Code / 1000 * 1000;
		pLOG->usSvcL4Type = pstTEXTINFO->usL4Code;
		pLOG->usSvcL7Type = dGetL7TYPE(pstTEXTINFO->rtx);
		pLOG->usPlatformType = dGetPlatformType(pLOG->usSvcL4Type, pLOG->usSvcL7Type);
		pLOG->uiClientIP = pstTEXTINFO->clientIP;

		memset(pstMSRPTTRANS, 0x00, DEF_MSRPTTRANS_SIZE);
		pstMSRPTTRANS->offset_LOG = nifo_offset(pstMEMSINFO, (U8 *)pLOG);
		pstMSRPTTRANS->offset_NODE = nifo_offset(pstMEMSINFO, (U8 *)pNODE);

		if((pstHASHONODE = hasho_add(pstMSRPTHASH, (U8 *)pstMSRPTTRANSKEY, (U8 *)pstMSRPTTRANS)) == NULL)
		{
			log_print(LOGN_CRI, LH"hasho_add CIP=%s:%d SIP=%s:%d MSGID=%s",
					LT,
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID);	
			nifo_node_delete(pstMEMSINFO, pNODE);
			return -4;
		}
		else
		{
			pstMSRPTTRANS = (st_MSRPT_TRANS *)nifo_ptr(pstMSRPTHASH, pstHASHONODE->offset_Data);
			memcpy(&stMSRPTCOMMON.stMSRPTTRANSKEY, pstMSRPTTRANSKEY, DEF_MSRPTTRANSKEY_SIZE);
			pstMSRPTTRANS->timerNID = timerN_add(pstTIMERNINFO, invoke_del, (U8 *)&stMSRPTCOMMON, DEF_MSRPTCOMMON_SIZE, time(NULL) + guiTimerValue );
		}
	}
	else
	{
		/* 기존 Transaction 존재 하는 경우 */
		log_print(LOGN_DEBUG, "HAVE HASH CIP=%s:%d SIP=%s:%d MSGID=%s",
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID);	

		pstMSRPTTRANS = (st_MSRPT_TRANS *)nifo_ptr(pstMSRPTHASH, pstHASHONODE->offset_Data);
		pstMSRPTTRANS->timerNID = timerN_update(pstTIMERNINFO, pstMSRPTTRANS->timerNID, time(NULL) + guiTimerValue );	
	}

	dFlowMSRPTTrans(pstMEMSINFO, pstMSRPTINFO, pstMSRPTTRANSKEY, pstMSRPTTRANS, pstTEXTINFO);

	log_print(LOGN_DEBUG, "FINISH STATUS=%d:%s", pstMSRPTTRANS->ucFinish, PrintFinish(pstMSRPTTRANS->ucFinish));

	switch(pstMSRPTTRANS->ucFinish)
	{
	case MSRPT_ACT_CONTINUE:
		/* CF */
		//dProcMSRPCF(pstMEMSINFO, pstMSRPTINFO, pstMSRPTTRANSKEY, pstMSRPTTRANS, pstTEXTINFO);
		break;

	case MSRPT_ACT_FINISH:
		/* CF */
		//dProcMSRPCF(pstMEMSINFO, pstMSRPTINFO, pstMSRPTTRANSKEY, pstMSRPTTRANS, pstTEXTINFO);
		dCloseMSRPTTrans(pstMEMSINFO, pstMSRPTHASH, pstMSRPTTRANSKEY, pstMSRPTTRANS);
		timerN_del(pstTIMERNINFO, pstMSRPTTRANS->timerNID);
		break;

	case MSRPT_ACT_DELETE:
		nifo_node_delete(pstMEMSINFO, pNODE);
		timerN_del(pstTIMERNINFO, pstMSRPTTRANS->timerNID);
		hasho_del(pstMSRPTHASH, (U8 *)pstMSRPTTRANSKEY);
		break;

	default:
		log_print(LOGN_CRI, LH"dFlowMSRPTTrans UNKNOWN CIP=%s:%d SIP=%s:%d MSGID=%s dRet=%d", 
				LT,
				util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
				pstMSRPTTRANSKEY->szMSGID, pstMSRPTTRANS->ucFinish);	
		break;
	}

	return 0;
}

S32 dFlowMSRPTTrans(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, st_MSRPT_TRANS_KEY *pstMSRPTTRANSKEY, st_MSRPT_TRANS *pstMSRPTTRANS, TEXT_INFO *pstTEXTINFO)
{
	S32						dMsgStatus;
	LOG_MSRP_TRANS			*pLOG;

    U8						szCIP[INET_ADDRSTRLEN];
    U8						szSIP[INET_ADDRSTRLEN];

	dMsgStatus = dGetMsgStatus(pstMSRPTINFO->usMethod, pstMSRPTINFO->usEndFlag);

	log_print(LOGN_DEBUG, "FLOW CIP=%s:%d SIP=%s:%d MSGID=%s OLD_STATUS=%s:%d MSG_STATUS=%s:%d END_FLAG=%s:%d",
				util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
				pstMSRPTTRANSKEY->szMSGID, PrintStatus(pstMSRPTTRANS->dStatus), 
				pstMSRPTTRANS->dStatus, PrintMsgStatus(dMsgStatus), dMsgStatus,
				PrintEndFlag(pstMSRPTINFO->usEndFlag), pstMSRPTINFO->usEndFlag);
				
	pLOG = (LOG_MSRP_TRANS *)nifo_ptr(pstMEMSINFO, pstMSRPTTRANS->offset_LOG);

	switch(pstMSRPTTRANS->dStatus)
	{
	case MSRPT_STATUS_WAIT:
		switch(dMsgStatus)
		{
		case MSRPT_MSG_REQ_END:
		case MSRPT_MSG_REQ_CONTINUE:
		case MSRPT_MSG_REQ_NOTHING:
			InitLog(pstMEMSINFO, pstMSRPTINFO, pstTEXTINFO, pLOG);
			SetNormalReq(pstMEMSINFO, pstMSRPTINFO, pstTEXTINFO, pLOG);
			pstMSRPTTRANS->dStatus = MSRPT_STATUS_START;
			pstMSRPTTRANS->ucFinish = MSRPT_ACT_CONTINUE;
			break;

		case MSRPT_MSG_REQ_ABORT:
			InitLog(pstMEMSINFO, pstMSRPTINFO, pstTEXTINFO, pLOG);
			SetNormalReq(pstMEMSINFO, pstMSRPTINFO, pstTEXTINFO, pLOG);
			pstMSRPTTRANS->ucFinish = MSRPT_ACT_FINISH;
			pstMSRPTTRANS->usFailCode = MSRP_UERR_9100;
			break;

		case MSRPT_MSG_RES:
			pstMSRPTTRANS->ucFinish = MSRPT_ACT_DELETE;
			break;

		case MSRPT_MSG_REPORT:
			pstMSRPTTRANS->ucFinish = MSRPT_ACT_DELETE;
			break;

		default:
			log_print(LOGN_CRI, LH"MSRPT_STATUS_WAIT UNKNOWN CIP=%s:%d SIP=%s:%d MSGID=%s MSG_STATUS=%d Method=%d", 
					LT, 
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID,
					dMsgStatus, pstMSRPTINFO->usMethod);
			pstMSRPTTRANS->ucFinish = MSRPT_ACT_DELETE;
			break;
		}
		break;

	case MSRPT_STATUS_START:
		switch(dMsgStatus)
		{
		case MSRPT_MSG_REQ_END:
		case MSRPT_MSG_REQ_CONTINUE:
		case MSRPT_MSG_REQ_NOTHING:
			SetNormalReq(pstMEMSINFO, pstMSRPTINFO, pstTEXTINFO, pLOG);
			break;

		case MSRPT_MSG_REQ_ABORT:
			SetNormalReq(pstMEMSINFO, pstMSRPTINFO, pstTEXTINFO, pLOG);
			pstMSRPTTRANS->ucFinish = MSRPT_ACT_FINISH;
			pstMSRPTTRANS->usFailCode = MSRP_UERR_9100;
			break;

		case MSRPT_MSG_RES:
			SetNormalRes(pstMEMSINFO, pstMSRPTINFO, pstTEXTINFO, pLOG);
			break;

		case MSRPT_MSG_REPORT:
			SetNormalReport(pstMEMSINFO, pstMSRPTINFO, pstTEXTINFO, pLOG);
			break;

		default:
			log_print(LOGN_CRI, LH"MSRPT_STATUS_REQ UNKNOWN CIP=%s:%d SIP=%s:%d MSGID=%s MSG_STATUS=%d Method=%d", 
					LT, 
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID,
					dMsgStatus, pstMSRPTINFO->usMethod);
			pstMSRPTTRANS->ucFinish = MSRPT_ACT_FINISH;
			pstMSRPTTRANS->usFailCode = MSRP_UERR_9300;
			break;
		}
		break;

	default:
		log_print(LOGN_CRI, LH"UNKNOWN CIP=%s:%d SIP=%s:%d MSGID=%s OLD_STATUS=%d Method=%d", 
				LT, 
				util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
				pstMSRPTTRANSKEY->szMSGID,
				pstMSRPTTRANS->dStatus, pstMSRPTINFO->usMethod);
		pstMSRPTTRANS->ucFinish = MSRPT_ACT_FINISH;
		pstMSRPTTRANS->usFailCode = MSRP_UERR_9300;
		break;
	}

	return 0;
}

S32 dCloseMSRPTTrans(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPTHASH, st_MSRPT_TRANS_KEY *pstMSRPTTRANSKEY, st_MSRPT_TRANS *pstMSRPTTRANS)
{
	U64						duration;
	U8						*pNODE;
	LOG_MSRP_TRANS			*pLOG;

    U8						szCIP[INET_ADDRSTRLEN];
    U8						szSIP[INET_ADDRSTRLEN];

	struct timeval			stTime;

	log_print(LOGN_DEBUG, "CLOSE TRANS CIP=%s:%d SIP=%s:%d MSGID=%s",
				util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
				pstMSRPTTRANSKEY->szMSGID);	

	gettimeofday(&stTime, NULL);

	pNODE = (U8 *)nifo_ptr(pstMEMSINFO, pstMSRPTTRANS->offset_NODE);
	pLOG = (LOG_MSRP_TRANS *)nifo_ptr(pstMEMSINFO, pstMSRPTTRANS->offset_LOG);

	/* ResGapTime */
	STG_DiffTIME64(pLOG->ResTime, pLOG->ResMTime, pLOG->TransStartTime, pLOG->TransStartMTime, &duration);
	pLOG->ResGapTime = duration;

	/* ReportGapTime */
	STG_DiffTIME64(pLOG->ReportTime, pLOG->ReportMTime, pLOG->ResTime, pLOG->ResMTime, &duration);
	pLOG->ReportGapTime = duration; 

	/* TransGapTime */
	STG_DiffTIME64(pLOG->TransEndTime, pLOG->TransEndMTime, pLOG->TransStartTime, pLOG->TransStartMTime, &duration);
	/* pLOG->TransGapTime = duration; */

	pLOG->L7RstCode = pLOG->ReportCode;
	pLOG->L7RstCode = (pLOG->L7RstCode > 0) ? pLOG->L7RstCode : pLOG->ResCode;

	pLOG->OpEndTime = stTime.tv_sec;
	pLOG->OpEndMTime = stTime.tv_usec;

	if(pstMSRPTTRANS->usFailCode > 0) {
		pLOG->LastUserErrCode = pstMSRPTTRANS->usFailCode;
	} else if(pLOG->SuccessReport == MSRP_FLAG_YES) {
		if(pLOG->TotalReportCnt == 0) {
			pLOG->LastUserErrCode = MSRP_UERR_9200;
		}
	} else if(pLOG->TotalReqCnt != pLOG->TotalResCnt) {
		pLOG->LastUserErrCode = MSRP_UERR_9200;
	} else if(pLOG->L7RstCode >= 400) {
		pLOG->LastUserErrCode = MSRP_UERR_9000;	
	}

//	LOG_MSRP_TRANS_Prt("PRINT LOG_MSRP_TRANS", pLOG);

	dSend_MSRPT_Data(pstMEMSINFO, SEQ_PROC_A_IM, pNODE);
	hasho_del(pstMSRPTHASH, (U8 *)pstMSRPTTRANSKEY);
	
	return 0;
}

/*
 * $Log: msrpt_func.c,v $
 * Revision 1.4  2011/09/07 06:30:48  hhbaek
 * *** empty log message ***
 *
 * Revision 1.3  2011/09/05 12:26:40  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 01:35:33  uamyd
 * modified to runnable source
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.2  2011/08/09 05:31:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.9  2011/05/09 14:19:31  dark264sh
 * A_MSRPT: A_CALL multi 처리
 *
 * Revision 1.8  2011/01/11 04:09:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:00  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.7  2009/09/02 13:55:18  jsyoon
 * MSRP COUNT SETTING
 *
 * Revision 1.6  2009/08/30 08:11:06  jsyoon
 * *** empty log message ***
 *
 * Revision 1.5  2009/08/30 08:09:23  jsyoon
 * *** empty log message ***
 *
 * Revision 1.4  2009/08/19 12:27:14  pkg
 * LOG_XXX_Prt 함수 주석 처리
 *
 * Revision 1.3  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.2  2009/07/15 16:42:10  dqms
 * ADD vMSRPTimerReConstruct()
 *
 * Revision 1.1.1.1  2009/05/26 02:14:13  dqms
 * Init TAF_RPPI
 *
 * Revision 1.4  2008/10/30 04:47:36  dark264sh
 * no message
 *
 * Revision 1.3  2008/09/19 07:08:04  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2008/09/19 06:49:38  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2008/09/18 06:48:10  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:15:17  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.14  2007/07/03 08:49:00  dark264sh
 * *** empty log message ***
 *
 * Revision 1.13  2007/07/03 08:05:58  dark264sh
 * *** empty log message ***
 *
 * Revision 1.12  2007/07/03 07:09:46  dark264sh
 * *** empty log message ***
 *
 * Revision 1.11  2007/07/02 12:31:08  dark264sh
 * 초기화 문제 버그
 *
 * Revision 1.10  2007/06/27 07:41:57  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2007/06/25 16:13:56  dark264sh
 * *** empty log message ***
 *
 * Revision 1.8  2007/06/25 14:10:09  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2007/06/20 05:59:18  dark264sh
 * *** empty log message ***
 *
 * Revision 1.6  2007/06/20 05:46:53  dark264sh
 * CF_MSRP_MSG MessageString 형식 변경
 *
 * Revision 1.5  2007/06/18 10:32:19  dark264sh
 * SuccessReport, FailureReport를 잘못 판단하는 버그
 *
 * Revision 1.4  2007/06/13 14:31:59  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2007/06/12 06:49:33  dark264sh
 * CF Method 잘못 넣는 부분 수정
 *
 * Revision 1.2  2007/06/12 05:12:00  dark264sh
 * CF 처리
 *
 * Revision 1.1  2007/05/07 01:48:09  dark264sh
 * INIT
 *
 */
