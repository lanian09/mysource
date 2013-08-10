/**     @file   sipt_func.c
 *      - SIPT Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: sipt_func.c,v 1.2 2011/09/06 12:46:38 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:38 $
 *      @ref        sipt_util.c
 *      @todo       Nothing
 *
 *      @section    Intro(소개)
 *      - SIPT Service Processing
 *
 *      @section    Requirement
 *       @li Nothing
 *
 **/

/**
 * Include headers
 */
#include "common_stg.h"
#include "procid.h"
#include "filter.h"

// LIB
#include "loglib.h"

// TAF
#include "sip.h"

// .
#include "sipt_sess.h"
#include "sipt_msgq.h"
#include "sipt_util.h"
#include "sipt_func.h"

/**
 *	Declare variables
 */
extern st_Flt_Info	*flt_info;
extern int			guiTimerValue;

extern stHASHOINFO		*pSESSKEYINFO;
extern stHASHONODE		*pSESSKEYNODE;

SESSKEY_TBL				*pstSESSKEYTBL;
SESSKEY_LIST			*pstSESSKEYList;
SESSKEY_LIST			*pstNEWSESSKEYList;

/**
 *	Declare external func
 */
extern void invoke_del(void *p);
extern int Delete_SessList(SIP_INFO_KEY *pSIPINFOKEY);

/**
 *	Impl. func.
 */
S32 dProcSIPStart(stMEMSINFO *pMEMSINFO, SIP_INFO_KEY *pSIPINFOKEY, SIP_INFO *pSIPINFO, TSIP_INFO *pTSIPINFO, U8 *pDATA, U32 len)
{
	U8				*pNODE;
	LOG_SIP_TRANS	*pStartLOG, *pLOG;	
	UINT 			dSeqProcID;
	int 			dRet;
	U8          	contact[SIP_CONTACT_SIZE];
	U8          	accept[SIP_CONTACT_SIZE];
	USHORT 			msgport = 0;
	S32				msgtype = 0;
	S32				event = 0;
	int 			dSvcType = DEF_PLATFORM_VT;

	if((pNODE = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, LH"nifo_node_alloc CallID=%s CSeq=%u FromTag=%s", 
				LT, pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
		return -1;
	}

	if((pStartLOG = (LOG_SIP_TRANS *)nifo_tlv_alloc(pMEMSINFO, pNODE, START_CALL_NUM, LOG_SIP_TRANS_SIZE, DEF_MEMSET_ON)) == NULL) 
	{
		log_print(LOGN_CRI, LH"nifo_tlv_alloc CallID=%s CSeq=%u FromTag=%s", 
				LT, pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
		return -2;
	}

	pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);

	memcpy(pStartLOG, pLOG, LOG_SIP_TRANS_SIZE);

	dRet = sip((char*)pDATA, len, (char*)&pStartLOG->URI[0], (char*)&pStartLOG->SIPFrom[0], (char*)&pStartLOG->SIPTo[0], &pStartLOG->SessID, &pStartLOG->AudioPort, &pStartLOG->VideoPort, (char*)&pStartLOG->UserAgent[0], (char*)&pStartLOG->Auth_nonce[0], (char*)&pStartLOG->UserName[0], (char*)&pStartLOG->AudioProto[0], (char*)&pStartLOG->VideoProto[0], &msgtype, &event, (char*)contact, (char*)accept, &msgport);
	if(dRet != 0) {
		log_print(LOGN_CRI, "PARSING ERROR [%d][\n%.*s]", len, len, pDATA);
		return -1;
	}
	sip_service((char*)accept, strlen((char*)accept), &dSvcType);

	if (dSvcType == DEF_PLATFORM_IM) {
		pLOG->usPlatformType = DEF_PLATFORM_IM;
		pStartLOG->usPlatformType = DEF_PLATFORM_IM;
		dSeqProcID = SEQ_PROC_A_IM;
	} else {
		pLOG->usPlatformType = DEF_PLATFORM_VT;
		pLOG->usSvcL4Type = L4_VT;
		pStartLOG->usPlatformType = DEF_PLATFORM_VT;
		pStartLOG->usSvcL4Type = L4_VT;
		dSeqProcID = SEQ_PROC_A_VT;
	}

//	LOG_SIP_TRANS_Prt("START_SIP_TRANS_MSG", pStartLOG);
	dSend_SIPT_Data(pMEMSINFO, dSeqProcID, pNODE);

	log_print(LOGN_DEBUG, "SEND START MSG CallID[%s] CSeq[%u] FromTag[%s] PLATFORM[%ld]",
			pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag, pStartLOG->usPlatformType);

	return 0;
}

/** dProcSIPTrans function.
 *
 *  ddProcSIPTrans Function
 *
 *  @param  *data : 분석할 SIP Message Data
 *  @param  len : 분석할 SIP Message Data Length
 *  @param  *pSIPINFOKEY : 분석된 값을 받기 위한 구조체
 *  @param  *msgtype : SIP Message Type
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(Message 분석 실패)
 *  @see            sipt_msgq.c
 *
 **/
S32 dProcSIPTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pSIPHASH, stTIMERNINFO *pTIMER, TEXT_INFO *pTEXTINFO, U8 *pDATA, U32 len, OFFSET offset)
{
	int 			dRet;
	S32				msgtype = 0, isNewTrans = 0, isNewSess = 0;
	U32				ip;
	U8				*pNODE;
	LOG_SIP_TRANS	*pLOG;

	SIP_INFO_KEY	SIPINFOKEY;
	SIP_INFO_KEY	*pSIPINFOKEY = &SIPINFOKEY;
	SIP_INFO		SIPINFO;
	SIP_INFO		*pSIPINFO = &SIPINFO;
	TSIP_INFO		TSIPINFO;
	TSIP_INFO		*pTSIPINFO = &TSIPINFO;
	
	SIP_COMMON		SIPCOMMON;
	stHASHONODE		*pHASHNODE;

	CALL_KEY		stCALLKEY;
	stHASHONODE		*pSESSKEYNODE;

	
	/* SIPT KEY */
	memset(pSIPINFOKEY, 0x00, SIP_INFO_KEY_SIZE);
	if(dGetSIPTransKey(pDATA, len, pSIPINFOKEY, &msgtype, &ip) < 0) {
		log_print(LOGN_CRI, LH"dGetSIPTransKey", LT);
		return -1;
	}

	if(pTEXTINFO->usL4Code == L4_SIP_CSCF || pTEXTINFO->usL4Code == L4_VT) {
		pSIPINFOKEY->ClientIP = pTEXTINFO->clientIP;
		pSIPINFOKEY->ServerIP = pTEXTINFO->serverIP;
	} else {
		pSIPINFOKEY->ClientIP = 0;
		pSIPINFOKEY->ServerIP = 0;
	}

	log_print(LOGN_DEBUG, "RCV MSG MSGTYPE=%s:%d CallID=%s CSeq=%u FromTag=%s CIP=%u SIP=%u", 
		PrintMsgType(msgtype), msgtype, pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, 
		pSIPINFOKEY->FromTag, pSIPINFOKEY->ClientIP, pSIPINFOKEY->ServerIP);

	if((pTSIPINFO->status = dGetMsgType(msgtype)) < 0) {
		log_print(LOGN_CRI, LH"dGetMsgType MSGTYPE=%s:%d CallID=%s CSeq=%u FromTag=%s", 
			LT, PrintMsgType(msgtype), 
			msgtype, pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
		return -2;
	}

	switch(pTSIPINFO->status)
	{
	case TSIP_RES_SKIP:
	case TSIP_RES_NORMAL:
		pSIPINFOKEY->RTX = (pTEXTINFO->rtx == DEF_FROM_CLIENT) ? DEF_FROM_SERVER : DEF_FROM_CLIENT;
		break;
	case TSIP_REQ_NORMAL:
	case TSIP_REQ_INVITE:
	case TSIP_REQ_ACK:
		pSIPINFOKEY->RTX = pTEXTINFO->rtx;
		break;
	default:
		pSIPINFOKEY->RTX = 0;
		break;
	}
	pTSIPINFO->msgtype = msgtype;
	pTSIPINFO->delete_node = YES;
	pTSIPINFO->pTEXTINFO = pTEXTINFO;
	pTSIPINFO->pMEMSINFO = pMEMSINFO;
	pTSIPINFO->offset_DATA = offset;
	pTSIPINFO->AddMsg = AddMsg;
	pTSIPINFO->InitLog = InitLog;
	pTSIPINFO->SetResSkip = SetResSkip;
	pTSIPINFO->SetResNormal = SetResNormal;
	pTSIPINFO->SetRetransReq = SetRetransReq;
	pTSIPINFO->SetRetransRes = SetRetransRes;
	pTSIPINFO->SetAck = SetAck;
	pTSIPINFO->SetRetransAck = SetRetransAck;
	pTSIPINFO->SetReqDataSize = SetReqDataSize;
	pTSIPINFO->SetResDataSize = SetResDataSize;

	/* SETTING TIMER */
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_SIP_TIMEOUT];

	/* 기존 Transaction 존재 하지 않는 경우 */
	if((pHASHNODE = hasho_find(pSIPHASH, (U8 *)pSIPINFOKEY)) == NULL)
	{
		log_print(LOGN_DEBUG, "NOT HAVE HASH MSGTYPE=%s:%d STATUS=%s:%d CallID=%s CSeq=%u FromTag=%s",
				PrintMsgType(msgtype), msgtype, PrintStatus(pTSIPINFO->status), pTSIPINFO->status,
				pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
		pSIPINFO->TSIP_STATE = TSIP_MSG_WAIT;
		pSIPINFO->TSIP_OLD_STATE = 0;
		pSIPINFO->finish_trans = NO;
		pSIPINFO->start_trans = NO;
		pSIPINFO->failcode = 0;
		pSIPINFO->endstatus = 0;

		if((pNODE = nifo_node_alloc(pMEMSINFO)) == NULL)
		{
			log_print(LOGN_CRI, LH"nifo_node_alloc MSGTYPE=%s:%d STATUS=%s:%d CallID=%s CSeq=%u FromTag=%s", 
					LT, PrintMsgType(msgtype), msgtype, 
					PrintStatus(pTSIPINFO->status), pTSIPINFO->status, pSIPINFOKEY->CallID, 
					pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
			return -3;
		}

		if((pLOG = (LOG_SIP_TRANS *)nifo_tlv_alloc(pMEMSINFO, pNODE, LOG_SIP_TRANS_DEF_NUM, LOG_SIP_TRANS_SIZE, DEF_MEMSET_ON)) == NULL) 
		{
			log_print(LOGN_CRI, LH"nifo_tlv_alloc MSGTYPE=%s:%d STATUS=%s:%d CallID=%s CSeq=%u FromTag=%s", 
					LT, PrintMsgType(msgtype), msgtype, 
					PrintStatus(pTSIPINFO->status), pTSIPINFO->status, pSIPINFOKEY->CallID, 
					pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
			return -4;
		}

		memcpy(pLOG->CallID, pSIPINFOKEY->CallID, SIP_CALLID_LEN);
		pLOG->CallID[SIP_CALLID_LEN] 	= 0x00;
		pLOG->CSeq 						= pSIPINFOKEY->CSeq;
		pLOG->Protocol 					= pTEXTINFO->protocol;
		pLOG->uiClientIP 				= pTEXTINFO->clientIP;
		pLOG->usSvcL4Type 				= pTEXTINFO->usL4Code;
		pLOG->usSvcL7Type 				= dGetL7TYPE(pTEXTINFO->rtx);
//		pLOG->usPlatformType 			= dGetPlatformType(pLOG->usSvcL4Type, pLOG->usSvcL7Type);
		pLOG->usPlatformType 			= 0;

		pSIPINFO->offset_LOG = nifo_offset(pMEMSINFO, (U8 *)pLOG);
		pSIPINFO->offset_NODE = nifo_offset(pMEMSINFO, pNODE);

		if((pHASHNODE = hasho_add(pSIPHASH, (U8 *)pSIPINFOKEY, (U8 *)pSIPINFO)) == NULL) {
			log_print(LOGN_CRI, LH"hash_add MSGTYPE=%s:%d STATUS=%s:%d CallID=%s CSeq=%u FromTag=%s", 
					LT, PrintMsgType(msgtype), msgtype, 
					PrintStatus(pTSIPINFO->status), pTSIPINFO->status, pSIPINFOKEY->CallID, 
					pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
			nifo_node_delete(pMEMSINFO, pNODE);
			return -5;
		} else {
			pSIPINFO = (SIP_INFO *)nifo_ptr(pSIPHASH, pHASHNODE->offset_Data);
			memcpy(&SIPCOMMON.SIPINFOKEY, pSIPINFOKEY, SIP_INFO_KEY_SIZE);
			pSIPINFO->timerNID = timerN_add(pTIMER, invoke_del, (U8 *)&SIPCOMMON, SIP_COMMON_SIZE, time(NULL) + guiTimerValue);
		}
		
		if(msgtype == SIP_MSG_INVITE) 
			isNewTrans = 1;

		isNewSess = 1;
	}
	/* 기존 Transaction 존재 하는 경우 */
	else
	{
		log_print(LOGN_DEBUG, "HAVE HASH MSGTYPE=%s:%d STATUS=%s:%d CallID=%s CSeq=%u FromTag=%s",
			PrintMsgType(msgtype), msgtype, PrintStatus(pTSIPINFO->status), pTSIPINFO->status,
			pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
		pSIPINFO = (SIP_INFO *)nifo_ptr(pSIPHASH, pHASHNODE->offset_Data);
		pSIPINFO->timerNID = timerN_update(pTIMER, pSIPINFO->timerNID, time(NULL) + guiTimerValue);
	}

	flow_TSIP_state_go(pSIPINFO, TSIP_INFO_DEF_NUM, TSIP_INFO_SIZE, pTSIPINFO, pSIPINFO->TSIP_STATE, YES);

	if(pSIPINFO->start_trans == NO) {
		log_print(LOGN_DEBUG, "NOT START MSG MSGTYPE=%s:%d STATUS=%s:%d CallID=%s CSeq=%u FromTag=%s",
			PrintMsgType(msgtype), msgtype, PrintStatus(pTSIPINFO->status), pTSIPINFO->status,
			pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
		nifo_node_delete(pMEMSINFO, pNODE);	
		hasho_del(pSIPHASH, (U8 *)pSIPINFOKEY);
		timerN_del(pTIMER, pSIPINFO->timerNID);
		return -21;
	}

	/* ADD SESSION KEY LIST */
	if(isNewSess == 1 && pSIPINFOKEY->ClientIP != 0) {
		stCALLKEY.uiSrcIP = pTEXTINFO->clientIP;
		stCALLKEY.uiReserved = 0;

		if( (pSESSKEYNODE = hasho_find(pSESSKEYINFO, (U8 *)&stCALLKEY)) == NULL )
		{
			/* ADD SIP_INFO_KEY LIST */
			dRet = dGetSessKeyList(&pstNEWSESSKEYList);
			if(dRet < 0) {
				log_print(LOGN_CRI, LH"#### dGetStackOnly() dRet[%d]", LT, dRet);
				return 0;
			}
			memcpy(&pstNEWSESSKEYList->stSessKey, pSIPINFOKEY, SIP_INFO_KEY_SIZE);
			pstNEWSESSKEYList->SessStartTime = pTEXTINFO->uiStartTime;
			if((pSESSKEYNODE = hasho_add(pSESSKEYINFO, (U8 *)&stCALLKEY, (U8 *)pstNEWSESSKEYList)) == NULL) {
				log_print(LOGN_CRI, LH"#### hasho_add NULL", LT);
			} else {
				log_print(LOGN_DEBUG, "#### ADD HASH NEW CALL CIP:%d.%d.%d.%d START:%u",
						HIPADDR(stCALLKEY.uiSrcIP), pstNEWSESSKEYList->SessStartTime);
			}
		} else {
			pstSESSKEYList = (SESSKEY_LIST *)nifo_ptr(pSESSKEYINFO, pSESSKEYNODE->offset_Data);

			dRet = dGetSessKeyList(&pstNEWSESSKEYList);
			if(dRet < 0) {
				log_print(LOGN_CRI, "#### ERROR dGetStackOnly() dRet[%d]", dRet);
				return 0;
			}
			memcpy(&pstNEWSESSKEYList->stSessKey, pSIPINFOKEY, SIP_INFO_KEY_SIZE);
			pstNEWSESSKEYList->SessStartTime = pTEXTINFO->uiStartTime;
			dRet = dAddSessKeyNext(pstSESSKEYList, pstNEWSESSKEYList);
			if(dRet < 0) {
				log_print(LOGN_CRI, "#### SESSKEY INSERT ERROR][RET]:%d", dRet);
				FreeSessKeyList(pstNEWSESSKEYList);
				return 0;
			} else {
				log_print(LOGN_DEBUG, "#### ADD HASH EXIST CALL CIP:%d.%d.%d.%d START:%u",
						HIPADDR(stCALLKEY.uiSrcIP), pstNEWSESSKEYList->SessStartTime);
			}
		}
	}

	/*
	 *	Start Message
	 */
	if(isNewTrans)
		dProcSIPStart(pMEMSINFO, pSIPINFOKEY, pSIPINFO, pTSIPINFO, pDATA, len);

//	if((pSIPINFO->finish_trans == YES) && (pSIPINFO->failcode > 0)) {
	if( pSIPINFO->finish_trans == YES ) {
		log_print(LOGN_DEBUG, "FINISH TRANS MSGTYPE=%s:%d STATUS=%s:%d CallID=%s CSeq=%u FromTag=%s",
			PrintMsgType(msgtype), msgtype, PrintStatus(pTSIPINFO->status), pTSIPINFO->status,
			pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
		dCloseSIPTrans(pMEMSINFO, pSIPHASH, pSIPINFOKEY, pSIPINFO);
		timerN_del(pTIMER, pSIPINFO->timerNID);
		if(pSIPINFOKEY->ClientIP != 0) {
			Delete_SessList(pSIPINFOKEY);
		}
	}

	if(pTSIPINFO->delete_node == YES) {
		return -14;
	}

	return 0;
}

S32 dCloseSIPTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pSIPHASH, SIP_INFO_KEY *pSIPINFOKEY, SIP_INFO *pSIPINFO)
{
	U64				duration;
	U8				*pNODE;
	LOG_SIP_TRANS	*pLOG;

	log_print(LOGN_DEBUG, "CLOSE TRANS CallID=%s CSeq=%u FromTag=%s",
		pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);

	pNODE = nifo_ptr(pMEMSINFO, pSIPINFO->offset_NODE);
	pLOG= (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);

	/* FirstResGapTime */
	STG_DiffTIME64(pLOG->FirstResTime, pLOG->FirstResMTime, pLOG->TransStartTime, pLOG->TransStartMTime, &duration);
	pLOG->FirstResGapTime = duration;
//	pLOG->FirstResGapTime = duration / 1000;

	/* ResGapTime */
	STG_DiffTIME64(pLOG->ResTime, pLOG->ResMTime, pLOG->TransStartTime, pLOG->TransStartMTime, &duration);
	pLOG->ResGapTime = duration;
//	pLOG->ResGapTime = duration / 1000;

	/* AckGapTime */
	STG_DiffTIME64(pLOG->AckTime, pLOG->AckMTime, pLOG->ResTime, pLOG->ResMTime, &duration);
	pLOG->AckGapTime = duration;
//	pLOG->AckGapTime = duration / 1000;

	/* TransGapTime */
	STG_DiffTIME64(pLOG->TransEndTime, pLOG->TransEndMTime, pLOG->TransStartTime, pLOG->TransStartMTime, &duration);
//	pLOG->TransGapTime = duration;
//	pLOG->TransGapTime = duration / 1000;

	/* EndStatus */
	pLOG->EndStatus = pSIPINFO->endstatus;

	/* L7FailCode */
	if(pSIPINFO->finish_trans == YES) {
		if(pSIPINFO->failcode > 0) {
			pLOG->L7FailCode = pSIPINFO->failcode;
		} else if(pLOG->ResCode >= 400) {
			switch(pLOG->ResCode)
			{	
				/* LGT 요청에 의한 성공 처리 */
				case 401:	/* Unauthorized */
				case 402:	/* Payment Required */
				case 405:	/* Method Not Allowed */
				case 406:	/* Not Acceptable */
				case 415:	/* Unsupported Media Type */
				case 484:	/* Address Incomplete */
				case 486:	/* Busy here : 통화중 */
				case 487:	/* Request terminated : 발신 취소 */
				case 603:	/* Decline : 수신자 거절 */
				case 606:	/* Not Acceptable */
					break;
				default:
					pLOG->L7FailCode = SIP_UERR_8000;
					break;
			}
		}
	} else {
		/* TIME OUT 종료 */
		pLOG->L7FailCode = SIP_UERR_8200;
	}

	pLOG->LastUserErrCode = pLOG->L7FailCode;

//	LOG_SIP_TRANS_Prt("PRINT LOG_SIP_TRANS", pLOG);

	dSend_SIPT_Data(pMEMSINFO, SEQ_PROC_A_SIP, pNODE);
	hasho_del(pSIPHASH, (U8 *)pSIPINFOKEY);

	return 0;
}


/**
 *  $Log: sipt_func.c,v $
 *  Revision 1.2  2011/09/06 12:46:38  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/17 13:13:55  hhbaek
 *  A_SIPT
 *
 *  Revision 1.3  2011/08/10 09:57:44  uamyd
 *  modified and block added
 *
 *  Revision 1.2  2011/08/09 05:31:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.15  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.14  2009/09/11 11:55:18  jsyoon
 *  401, 402, 405, 406, 415, 484 성공처리
 *
 *  Revision 1.13  2009/08/23 13:06:46  jsyoon
 *  INVITE MSGTYPE VT일 경우 SvcL4Type 변경
 *
 *  Revision 1.12  2009/08/22 18:37:09  jsyoon
 *  INVITE 외의 MSG는 SIPD -> A_VT -> A_IM
 *
 *  Revision 1.11  2009/08/20 18:37:56  pkg
 *  A_SIPT Call Session관리 List 버그 수정
 *
 *  Revision 1.10  2009/08/19 12:28:41  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.9  2009/08/17 17:32:00  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.8  2009/08/17 15:08:01  jsyoon
 *  세션관리 메모리를 공유메모리로 변경
 *
 *  Revision 1.7  2009/08/15 11:29:38  pkg
 *  *** empty log message ***
 *
 *  Revision 1.6  2009/08/11 11:10:18  dqms
 *  서비스 종료시 pSIPINFO->failcode를 체크하지 않음
 *
 *  Revision 1.5  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.4  2009/08/04 12:08:17  dqms
 *  TIMER를 공유메모리로 변경
 *
 *  Revision 1.3  2009/07/15 16:39:16  dqms
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/07/05 15:39:40  dqms
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:35  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.7  2009/02/12 04:43:00  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.6  2009/02/11 06:20:24  dark264sh
 *  SIP Key 변경 (SeqType 추가) | 구간 세분화 | 486,487,603 성공 처리
 *
 *  Revision 1.5  2009/01/28 14:57:29  dark264sh
 *  A_SIPT node free 버그 수정
 *
 *  Revision 1.4  2008/10/30 04:48:37  dark264sh
 *  no message
 *
 *  Revision 1.3  2008/09/19 07:08:16  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2008/09/19 06:50:18  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2008/09/18 07:21:33  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.17  2007/06/28 05:23:54  dark264sh
 *  ClientIP 구하는 방식을 GiCSCF, CSCF로 나누어 구하도록 변경
 *
 *  Revision 1.16  2007/06/27 12:14:10  dark264sh
 *  SIP Transaction Key 변경
 *
 *  Revision 1.15  2007/06/20 07:18:23  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.14  2007/03/29 12:46:56  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.13  2007/03/29 12:19:11  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.12  2007/03/29 06:59:28  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.11  2007/03/28 15:47:11  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.10  2007/03/28 14:28:21  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.9  2007/03/28 07:01:30  dark264sh
 *  A_SIPT Timeout 처리 되는 경우 EndTime 처리 못하는 문제 해결 log_print 수정
 *
 *  Revision 1.8  2007/03/26 07:16:09  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2007/03/25 13:36:17  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.6  2007/03/24 12:46:39  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2007/03/24 07:49:48  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2007/03/22 12:20:28  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2007/03/14 09:29:30  dark264sh
 *  remove prefix
 *
 *  Revision 1.2  2007/03/07 01:12:06  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/03/05 00:37:21  dark264sh
 *  *** empty log message ***
 *
 */
          
