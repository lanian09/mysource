/**		@file	sipm_func.c
 * 		- SIP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: sipm_func.c,v 1.2 2011/09/05 12:26:41 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 12:26:41 $
 * 		@ref		sip_func.c
 * 		@todo		
 *
 * 		@section	Intro(소개)
 * 		- SIP Transaction을 관리 하는 함수들
 *
 * 		@section	Requirement
 * 		 @li	Nothing
 *
 **/

/**
 * Include headers
 */
// TOP
#include "procid.h"

// LIB
#include "loglib.h"
#include "utillib.h"
#include "common_stg.h"

// .
#include "sipm_func.h"

/**
 *	Implement func.
 */
S32 dProcSIPM(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstSIPMHASH, TCP_INFO *pstTCPINFO, U8 *pDATA)
{
	S32					dIsStart, dLen, dRet, i;
	
	st_SIPM_TSESS_KEY	stSIPMTSESSKEY;
	st_SIPM_TSESS_KEY	*pstSIPMTSESSKEY = &stSIPMTSESSKEY;
	st_SIPM_TSESS		stSIPMTSESS;
	st_SIPM_TSESS		*pstSIPMTSESS = &stSIPMTSESS;

	U8					*pTCPDATA, *pNODE, *pTMP;
	st_SIPM_MSG_INFO	*pstMSGINFO;

	stHASHONODE			*pstHASHONODE;

	U8					szCIP[INET_ADDRSTRLEN];
	U8					szSIP[INET_ADDRSTRLEN];

	/* Make Hash Key */
	MakeHashKey(pstTCPINFO, pstSIPMTSESSKEY);

	switch(pstTCPINFO->cTcpFlag)
	{
	case DEF_TCP_START:
		log_print(LOGN_DEBUG, "@@@ START CIP=%s:%d SIP=%s:%d",
				util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
				util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);


		pstSIPMTSESS->uiTcpSynTime = pstTCPINFO->uiCapTime;
		pstSIPMTSESS->uiTcpSynMTime = pstTCPINFO->uiCapMTime;
		pstSIPMTSESS->ucSynRtx = pstTCPINFO->ucRtx;
		pstSIPMTSESS->dL4FailCode = pstTCPINFO->usL4FailCode;
		pstSIPMTSESS->usAppCode = pstTCPINFO->usAppCode;
		pstSIPMTSESS->usL4Code = pstTCPINFO->usL4Code;
		pstSIPMTSESS->usL7Code = pstTCPINFO->usL7Code;
		pstSIPMTSESS->ucTcpClientStatus = pstTCPINFO->ucTcpClientStatus;
		pstSIPMTSESS->ucTcpServerStatus = pstTCPINFO->ucTcpServerStatus;

		pstSIPMTSESS->uiIPDataUpPktCnt = pstTCPINFO->uiIPDataUpPktCnt;
		pstSIPMTSESS->uiIPDataDnPktCnt = pstTCPINFO->uiIPDataDnPktCnt;
		pstSIPMTSESS->uiIPTotUpPktCnt = pstTCPINFO->uiIPTotUpPktCnt;
		pstSIPMTSESS->uiIPTotDnPktCnt = pstTCPINFO->uiIPTotDnPktCnt;
		pstSIPMTSESS->uiIPDataUpRetransCnt = pstTCPINFO->uiIPDataUpRetransCnt;
		pstSIPMTSESS->uiIPDataDnRetransCnt = pstTCPINFO->uiIPDataDnRetransCnt;
		pstSIPMTSESS->uiIPTotUpRetransCnt = pstTCPINFO->uiIPTotUpRetransCnt;
		pstSIPMTSESS->uiIPTotDnRetransCnt = pstTCPINFO->uiIPTotDnRetransCnt;
		pstSIPMTSESS->uiIPDataUpPktSize = pstTCPINFO->uiIPDataUpPktSize;
		pstSIPMTSESS->uiIPDataDnPktSize = pstTCPINFO->uiIPDataDnPktSize;
		pstSIPMTSESS->uiIPTotUpPktSize = pstTCPINFO->uiIPTotUpPktSize;
		pstSIPMTSESS->uiIPTotDnPktSize = pstTCPINFO->uiIPTotDnPktSize;
		pstSIPMTSESS->uiIPDataUpRetransSize = pstTCPINFO->uiIPDataUpRetransSize;
		pstSIPMTSESS->uiIPDataDnRetransSize = pstTCPINFO->uiIPDataDnRetransSize;
		pstSIPMTSESS->uiIPTotUpRetransSize = pstTCPINFO->uiIPTotUpRetransSize;
		pstSIPMTSESS->uiIPTotDnRetransSize = pstTCPINFO->uiIPTotDnRetransSize;

		if((pstHASHONODE = hasho_add(pstSIPMHASH, (U8 *)pstSIPMTSESSKEY, (U8 *)pstSIPMTSESS)) == NULL)
		{
			if((pstHASHONODE = hasho_find(pstSIPMHASH, (U8 *)pstSIPMTSESSKEY)) == NULL)
			{
				log_print(LOGN_CRI, LH"EXIST SAME HASH NODE BUT NOT FIND ??? CIP=%s:%d SIP=%s:%d", 
						LT,
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				exit(0);
			}

			pstSIPMTSESS = (st_SIPM_TSESS *)nifo_ptr(pstSIPMHASH, pstHASHONODE->offset_Data);

			if((dRet = dCloseSIPMTSESS(pstMEMSINFO, pstSIPMHASH, pstSIPMTSESSKEY, pstSIPMTSESS)) < 0)
			{
				log_print(LOGN_CRI, LH"dCloseSIPMTSESS CIP=%s:%d SIP=%s:%d dRet=%d", 
						LT, 
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
				return -11;
			}

			pstSIPMTSESS = &stSIPMTSESS;

			if((pstHASHONODE = hasho_add(pstSIPMHASH, (U8 *)pstSIPMTSESSKEY, (U8 *)pstSIPMTSESS)) == NULL)
			{
				log_print(LOGN_CRI, LH"hasho_add NULL CIP=%s:%d SIP=%s:%d", 
						LT, 
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				return -12;
			}
		}

		pstSIPMTSESS = (st_SIPM_TSESS *)nifo_ptr(pstSIPMHASH, pstHASHONODE->offset_Data);
		
		for(i = 0; i < SIPM_MSG_INFO_CNT; i++)
		{
			pstMSGINFO = &pstSIPMTSESS->stSIPMMSGINFO[i];

			memset(pstMSGINFO, 0x00, DEF_SIPMMSGINFO_SIZE);

			if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL) {
				log_print(LOGN_CRI, LH"nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d", 
						LT, 
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				hasho_del(pstSIPMHASH, (U8 *)pstSIPMTSESSKEY);
				return -21;
			}

			if(nifo_tlv_alloc(pstMEMSINFO, pNODE, ETH_DATA_NUM, 0, DEF_MEMSET_OFF) == NULL) {
				log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d", 
						LT, 
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				nifo_node_delete(pstMEMSINFO, pNODE);
				hasho_del(pstSIPMHASH, (U8 *)pstSIPMTSESSKEY);
				return -22;
			}	

			pstMSGINFO->offset_DATA = nifo_offset(pstMEMSINFO, pNODE);
			pstMSGINFO->offset_CurDATA = pstMSGINFO->offset_DATA;
		}	

		pstSIPMTSESS->stSIPMMSGINFO[0].uiNextSeq = pstTCPINFO->uiSeqNum;
		pstSIPMTSESS->stSIPMMSGINFO[1].uiNextSeq = pstTCPINFO->uiAckNum;
				
		break;

	case DEF_TCP_END:
		log_print(LOGN_DEBUG, "*** END CIP=%s:%d SIP=%s:%d",
				util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
				util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);


		/* Find Hash */
		if((pstHASHONODE = hasho_find(pstSIPMHASH, (U8 *)pstSIPMTSESSKEY)) == NULL)
		{
			log_print(LOGN_CRI, LH"hasho_find NULL CIP=%s:%d SIP=%s:%d", 
					LT, 
					util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
					util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
			return -31;
		}

		pstSIPMTSESS = (st_SIPM_TSESS *)nifo_ptr(pstSIPMHASH, pstHASHONODE->offset_Data);

		UpCount(pstTCPINFO, pstSIPMTSESS);
				
		if((dRet = dCloseSIPMTSESS(pstMEMSINFO, pstSIPMHASH, pstSIPMTSESSKEY, pstSIPMTSESS)) < 0)
		{
			log_print(LOGN_CRI, LH"dCloseSIPMTSESS CIP=%s:%d SIP=%s:%d dRet=%d", 
					LT, 
					util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
					util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
			return -32;
		}

		break;

	case DEF_TCP_DATA:
		log_print(LOGN_DEBUG, 
			"### DATA CIP=%s:%d SIP=%s:%d SIZE=%u SEQ=%u ACK=%u L4=%d FAIL=%d RTX=%s TIME=%u.%u",
			util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
			util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort,
			pstTCPINFO->uiDataSize, pstTCPINFO->uiSeqNum, pstTCPINFO->uiAckNum,
			pstTCPINFO->usL4Code, pstTCPINFO->usL4FailCode, 
			PrintRtx(pstTCPINFO->ucRtx), pstTCPINFO->uiCapTime, pstTCPINFO->uiCapMTime);

		pTCPDATA = &pDATA[pstTCPINFO->uiSOffset];

		log_print(LOGN_INFO, "INPUT LEN=%d DATA\n%.*s", pstTCPINFO->uiDataSize, pstTCPINFO->uiDataSize, pTCPDATA);

		/* Find Hash */
		if((pstHASHONODE = hasho_find(pstSIPMHASH, (U8 *)pstSIPMTSESSKEY)) == NULL)
		{
			log_print(LOGN_CRI, LH"hasho_find NULL CIP=%s:%d SIP=%s:%d", 
					LT, 
					util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
					util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
			return -41;
		}

		pstSIPMTSESS = (st_SIPM_TSESS *)nifo_ptr(pstSIPMHASH, pstHASHONODE->offset_Data);
		UpCount(pstTCPINFO, pstSIPMTSESS);

		pstMSGINFO = &pstSIPMTSESS->stSIPMMSGINFO[((pstTCPINFO->ucRtx == DEF_FROM_CLIENT) ? 0 : 1)];
		if(pstMSGINFO->offset_DATA == 0) {
			log_print(LOGN_CRI, "MSGINFO DATA NULL");
			return -100;
		}
		pstMSGINFO->uiIPDataSize = (pstTCPINFO->ucRtx == DEF_FROM_CLIENT) ? pstSIPMTSESS->uiIPDataUpPktSize : pstSIPMTSESS->uiIPDataDnPktSize;

		if((pstMSGINFO->uiNextSeq != 0) && (pstMSGINFO->uiNextSeq != pstTCPINFO->uiSeqNum))
		{
			log_print(LOGN_WARN, LH"DIFF SEQ NEXTSEQ=%u RCVSEQ=%u LASTSEQ= %u CIP=%s:%d SIP=%s:%d TIME=%u.%u RTX=%s",
					LT,
					pstMSGINFO->uiNextSeq, pstTCPINFO->uiSeqNum, pstMSGINFO->uiLastSeq,
					util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
					util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort,
					pstTCPINFO->uiCapTime, pstTCPINFO->uiCapMTime, PrintRtx(pstTCPINFO->ucRtx));
		}

		pstMSGINFO->uiNextSeq = pstTCPINFO->uiSeqNum + pstTCPINFO->uiDataSize;
		pstMSGINFO->uiLastSeq = pstTCPINFO->uiSeqNum;

		dLen = 0;

		while(dLen < pstTCPINFO->uiDataSize)
		{
			log_print(LOGN_INFO, "+++ START FLOW CIP=%s:%d SIP=%s:%d ENDSTATUS=%s:%d RLEN=%d LEN=%d",
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort,
						PrintEndStatus(pstMSGINFO->dEndStatus), pstMSGINFO->dEndStatus, pstTCPINFO->uiDataSize, dLen);

			dIsStart = dCheckStart(&pTCPDATA[dLen], pstTCPINFO->uiDataSize - dLen);

			switch(pstMSGINFO->dStatus)
			{
			case SIPM_STATUS_HDRWAIT:
				if(!dIsStart) {
					/* Drop Packet */
					log_print(LOGN_WARN, LH"WAIT HEADER BUT NOT HEADER CIP=%s:%d SIP=%s:%d", 
							LT, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					return -61;
				} else {
					pstMSGINFO->uiStartTime = pstTCPINFO->uiCapTime;
					pstMSGINFO->uiStartMTime = pstTCPINFO->uiCapMTime;
				}
				break;

			case SIPM_STATUS_HDRSTART:
			case SIPM_STATUS_BODYWAIT:
			case SIPM_STATUS_BODYSTART:
				if(dIsStart) {
					log_print(LOGN_CRI, LH"ABNORMAL MAKE MSG CIP=%s:%d SIP=%s:%d", 
							LT, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					log_print(LOGN_INFO, LH"ABNORMAL MAKE MSG LEN=%d TCP_DATA=\n%.*s", 
								LT, 
								pstTCPINFO->uiDataSize - dLen, pstTCPINFO->uiDataSize - dLen, &pTCPDATA[dLen]);
/*
					pTMP = (U8 *)nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA + NIFO_SIZE + TLV_SIZE); 
					log_print(LOGN_INFO, LH"ABNORMAL MAKE MSG LEN=%d BUFFER_DATA=\n%.*s", 
								LT, pstMSGINFO->uiTotalLen, pstMSGINFO->uiTotalLen, pTMP);
*/

					if((dRet = dCloseSIPM(pstMEMSINFO, pstSIPMTSESSKEY, pstSIPMTSESS, pstMSGINFO, pstTCPINFO->ucRtx, SIP_UERR_9300)) < 0) {
						log_print(LOGN_CRI, LH"dCloseSIPM CIP=%s:%d SIP=%s:%d dRet=%d", 
								LT, 
								util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
								util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
						return -81;
					}

					if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL) {
						log_print(LOGN_CRI, LH"nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d", 
								LT, 
								util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
								util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
						return -82;
					}

					if(nifo_tlv_alloc(pstMEMSINFO, pNODE, ETH_DATA_NUM, 0, DEF_MEMSET_OFF) == NULL) {
						log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d", 
								LT, 
								util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
								util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
						nifo_node_delete(pstMEMSINFO, pNODE);
						return -83;
					}	
					pstMSGINFO->offset_DATA = nifo_offset(pstMEMSINFO, pNODE);
					pstMSGINFO->offset_CurDATA = pstMSGINFO->offset_DATA;
					pstMSGINFO->uiStartTime = pstTCPINFO->uiCapTime;
					pstMSGINFO->uiStartMTime = pstTCPINFO->uiCapMTime;
				}
				break;

			default:
				log_print(LOGN_CRI, LH"UNKNOWN STATUS=%d CIP=%s:%d SIP=%s:%d", 
						LT, pstMSGINFO->dStatus,
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				return -71;
			}

			pstMSGINFO->uiLastUpdateTime = pstTCPINFO->uiCapTime;
			pstMSGINFO->uiLastUpdateMTime = pstTCPINFO->uiCapMTime;
			pstMSGINFO->uiAckTime = pstTCPINFO->uiAckTime;
			pstMSGINFO->uiAckMTime = pstTCPINFO->uiAckMTime;

			switch(pstMSGINFO->dStatus)
			{
			case SIPM_STATUS_HDRWAIT:
			case SIPM_STATUS_HDRSTART:
				dRet = dGetHDR(pstMEMSINFO, pstMSGINFO, &pTCPDATA[dLen], pstTCPINFO->uiDataSize - dLen, &dLen);
				log_print(LOGN_INFO, "CIP=%s:%d SIP=%s:%d dGetHDR ENDSTATUS=%s:%d RLEN=%d LEN=%d",
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, 
						PrintEndStatus(dRet), dRet, pstTCPINFO->uiDataSize, dLen);
				break;
			case SIPM_STATUS_BODYWAIT:
			case SIPM_STATUS_BODYSTART:
				dRet = dGetBODY(pstMEMSINFO, pstMSGINFO, &pTCPDATA[dLen], pstTCPINFO->uiDataSize - dLen, &dLen);
				log_print(LOGN_INFO, "CIP=%s:%d SIP=%s:%d dGetBODY ENDSTATUS=%s:%d RLEN=%d LEN=%d",
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, 
						PrintEndStatus(dRet), dRet, pstTCPINFO->uiDataSize, dLen);
				break;
			default:
				log_print(LOGN_CRI, LH"UNKNOWN STATUS=%d CIP=%s:%d SIP=%s:%d", 
						LT, pstMSGINFO->dStatus,
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				return -100;	
			}


			if(dRet < 0) {
				/* 에러 발생 */
				log_print(LOGN_CRI, LH"dGetHDR|dGetBODY dRet=%d CIP=%s:%d SIP=%s:%d", 
						LT, dRet,
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				return -101;
			}

			switch(dRet)
			{
			case SIPM_ENDSTATUS_EMPTY:
			case SIPM_ENDSTATUS_0D:
			case SIPM_ENDSTATUS_0D0A:
			case SIPM_ENDSTATUS_0D0A0D:
				pstMSGINFO->dStatus = SIPM_STATUS_HDRSTART;
				break;

			case SIPM_ENDSTATUS_BODY:
				pstMSGINFO->dStatus = SIPM_STATUS_BODYSTART;
				break;

			case SIPM_ENDSTATUS_0D0A0D0A:
				/* Header 끝 */
				log_print(LOGN_DEBUG, "CIP=%s:%d SIP=%s:%d HEADER END",
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
						
				pstMSGINFO->uiContentLen = 0;
				pTMP = (U8 *)nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA + NIFO_SIZE + TLV_SIZE); 
				pstMSGINFO->uiContentLen = dGetHDRINFO(pTMP, pstMSGINFO->uiHdrLen);
// log_print(LOGN_INFO, "==== ContentLen=%d LEN=%d DATA=\n%.*s", pstMSGINFO->uiContentLen, pstMSGINFO->uiTotalLen, pstMSGINFO->uiTotalLen, pTMP);
				if(pstMSGINFO->uiContentLen > 0) {
					log_print(LOGN_DEBUG, "CIP=%s:%d SIP=%s:%d HEADER END HAVE BODY Content-Length=%d",
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, pstMSGINFO->uiContentLen);
					pstMSGINFO->dStatus = SIPM_STATUS_BODYWAIT;	
					pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_BODY;
					break;
				}

				log_print(LOGN_DEBUG, "CIP=%s:%d SIP=%s:%d HEADER END NOT HAVE BODY",
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);

//				pstMSGINFO->uiIPDataSize -= (pstTCPINFO->uiDataSize - dLen);
				if((dRet = dCloseSIPM(pstMEMSINFO, pstSIPMTSESSKEY, pstSIPMTSESS, pstMSGINFO, pstTCPINFO->ucRtx, 0)) < 0) {
					log_print(LOGN_CRI, LH"dCloseSIPM CIP=%s:%d SIP=%s:%d dRet=%d", 
							LT, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
					return -51;
				}

				if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, LH"nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d", 
							LT, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					return -52;
				}

				if(nifo_tlv_alloc(pstMEMSINFO, pNODE, ETH_DATA_NUM, 0, DEF_MEMSET_OFF) == NULL) {
					log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d", 
							LT, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					nifo_node_delete(pstMEMSINFO, pNODE);
					return -53;
				}	

				pstMSGINFO->offset_DATA = nifo_offset(pstMEMSINFO, pNODE);
				pstMSGINFO->offset_CurDATA = pstMSGINFO->offset_DATA;
				break;

			case SIPM_ENDSTATUS_END:
				/* BODY 끝 */
				log_print(LOGN_DEBUG, "CIP=%s:%d SIP=%s:%d BODY END",
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);

//				pstMSGINFO->uiIPDataSize -= (pstTCPINFO->uiDataSize - dLen);
				if((dRet = dCloseSIPM(pstMEMSINFO, pstSIPMTSESSKEY, pstSIPMTSESS, pstMSGINFO, pstTCPINFO->ucRtx, 0)) < 0) {
					log_print(LOGN_CRI, LH"dCloseSIPM CIP=%s:%d SIP=%s:%d dRet=%d", 
							LT, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
					return -51;
				}

				if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, LH"nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d", 
							LT, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					return -52;
				}

				if(nifo_tlv_alloc(pstMEMSINFO, pNODE, ETH_DATA_NUM, 0, DEF_MEMSET_OFF) == NULL) {
					log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d", 
							LT, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					nifo_node_delete(pstMEMSINFO, pNODE);
					return -53;
				}	

				pstMSGINFO->offset_DATA = nifo_offset(pstMEMSINFO, pNODE);
				pstMSGINFO->offset_CurDATA = pstMSGINFO->offset_DATA;
				break;
			default:
				log_print(LOGN_CRI, LH"UNKNOWN ENDSTATUS=%d CIP=%s:%d SIP=%s:%d", 
						LT, pstMSGINFO->dEndStatus,
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				break;
			}
		}

		break;

	default:
		log_print(LOGN_CRI, LH"UNKNOWN TCPFLAG=%d CIP=%s:%d SIP=%s:%d", 
				LT, pstTCPINFO->cTcpFlag,
				util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
				util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
		break;
	}

	return 0;
}

S32 dCloseSIPM(stMEMSINFO *pstMEMSINFO, st_SIPM_TSESS_KEY *pstSIPMTSESSKEY, st_SIPM_TSESS *pstSIPMTSESS, st_SIPM_MSG_INFO *pstMSGINFO, U8 ucRtx, U16 usFailCode)
{
	S32					dRet;
	U8					*pNODE;
	TEXT_INFO			*pstTEXTINFO;
	U32					uiLastSeq, uiNextSeq;

	U8					szCIP[INET_ADDRSTRLEN];
	U8					szSIP[INET_ADDRSTRLEN];

	log_print(LOGN_DEBUG, "CLOSE SIP CIP=%s:%d SIP=%s:%d",
				util_cvtipaddr(szCIP, pstSIPMTSESSKEY->uiCliIP), pstSIPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstSIPMTSESSKEY->uiSrvIP), pstSIPMTSESSKEY->usSrvPort);

	if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, LH"nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d",
				LT,
				util_cvtipaddr(szCIP, pstSIPMTSESSKEY->uiCliIP), pstSIPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstSIPMTSESSKEY->uiSrvIP), pstSIPMTSESSKEY->usSrvPort);
		nifo_node_delete(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));	
		memset(pstMSGINFO, 0x00, DEF_SIPMMSGINFO_SIZE);
		return -1;
	}
	if((pstTEXTINFO = (TEXT_INFO *)nifo_tlv_alloc(pstMEMSINFO, pNODE, TEXT_INFO_DEF_NUM, TEXT_INFO_SIZE, DEF_MEMSET_ON)) == NULL)
	{
		log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d",
				LT,
				util_cvtipaddr(szCIP, pstSIPMTSESSKEY->uiCliIP), pstSIPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstSIPMTSESSKEY->uiSrvIP), pstSIPMTSESSKEY->usSrvPort);
		nifo_node_delete(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));	
		memset(pstMSGINFO, 0x00, DEF_SIPMMSGINFO_SIZE);
		return -2;

	}

	pstTEXTINFO->offset = 0;
	pstTEXTINFO->len = pstMSGINFO->uiTotalLen;
	pstTEXTINFO->uiStartTime = pstMSGINFO->uiStartTime;
	pstTEXTINFO->uiStartMTime = pstMSGINFO->uiStartMTime;
	pstTEXTINFO->uiLastUpdateTime = pstMSGINFO->uiLastUpdateTime;
	pstTEXTINFO->uiLastUpdateMTime = pstMSGINFO->uiLastUpdateMTime;
	pstTEXTINFO->uiAckTime = pstMSGINFO->uiAckTime;
	pstTEXTINFO->uiAckMTime = pstMSGINFO->uiAckMTime;
	pstTEXTINFO->clientIP = pstSIPMTSESSKEY->uiCliIP;
	pstTEXTINFO->serverIP = pstSIPMTSESSKEY->uiSrvIP;
	pstTEXTINFO->clientPort = pstSIPMTSESSKEY->usCliPort;
	pstTEXTINFO->serverPort = pstSIPMTSESSKEY->usSrvPort;
	pstTEXTINFO->protocol = DEF_PROTOCOL_TCP;
	pstTEXTINFO->IPDataSize = (pstMSGINFO->uiIPDataSize == 0) ? pstMSGINFO->uiTotalLen : pstMSGINFO->uiIPDataSize;;
	pstTEXTINFO->range = pstSIPMTSESS->dRange;
	pstTEXTINFO->network = pstSIPMTSESS->dNetwork;
	pstTEXTINFO->rtx = ucRtx;
	pstTEXTINFO->failcode = usFailCode;
	pstTEXTINFO->usL4Code = pstSIPMTSESS->usL4Code;

	InitCount(pstSIPMTSESS, ucRtx);
	
	nifo_node_link_nont_next(pstMEMSINFO, pNODE, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));

	uiNextSeq = pstMSGINFO->uiNextSeq;
	uiLastSeq = pstMSGINFO->uiLastSeq;

	log_print(LOGN_INFO, "SEND DATA ContentLen=%d LEN=%d DATA=\n%.*s", 
				pstMSGINFO->uiContentLen, pstMSGINFO->uiHdrLen, pstMSGINFO->uiHdrLen, 
				nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA + NIFO_SIZE + TLV_SIZE));

	if((dRet = dSend_SIPM_Data(pstMEMSINFO, SEQ_PROC_A_SIPT, pNODE)) < 0) {
		log_print(LOGN_CRI, LH"dSend_SIPM_Data CIP=%s:%d SIP=%s:%d dRet=%d",
				LT,
				util_cvtipaddr(szCIP, pstSIPMTSESSKEY->uiCliIP), pstSIPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstSIPMTSESSKEY->uiSrvIP), pstSIPMTSESSKEY->usSrvPort, dRet);
		nifo_node_delete(pstMEMSINFO, pNODE);	
		memset(pstMSGINFO, 0x00, DEF_SIPMMSGINFO_SIZE);
		pstMSGINFO->uiNextSeq = uiNextSeq;
		pstMSGINFO->uiLastSeq = uiLastSeq;
		return -3;
	} else {
		log_print(LOGN_INFO, "SEND MSG CIP=%s:%d SIP=%s:%d OFFSET=%u", 
				util_cvtipaddr(szCIP, pstSIPMTSESSKEY->uiCliIP), pstSIPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstSIPMTSESSKEY->uiSrvIP), pstSIPMTSESSKEY->usSrvPort,
				nifo_offset(pstMEMSINFO, pNODE));
		memset(pstMSGINFO, 0x00, DEF_SIPMMSGINFO_SIZE);
		pstMSGINFO->uiNextSeq = uiNextSeq;
		pstMSGINFO->uiLastSeq = uiLastSeq;
	}

	return 0;
}

S32 dCloseSIPMTSESS(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstSIPMHASH, st_SIPM_TSESS_KEY *pstSIPMTSESSKEY, st_SIPM_TSESS *pstSIPMTSESS)
{
	S32					i, dRet;
	st_SIPM_MSG_INFO	*pstMSGINFO;

	U8					szCIP[INET_ADDRSTRLEN];
	U8					szSIP[INET_ADDRSTRLEN];

	log_print(LOGN_DEBUG, "CLOSE TSESS CIP=%s:%d SIP=%s:%d",
				util_cvtipaddr(szCIP, pstSIPMTSESSKEY->uiCliIP), pstSIPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstSIPMTSESSKEY->uiSrvIP), pstSIPMTSESSKEY->usSrvPort);

	for(i = 0; i < SIPM_MSG_INFO_CNT; i++)
	{
		pstMSGINFO = &pstSIPMTSESS->stSIPMMSGINFO[i];

		if(pstMSGINFO->dStatus != SIPM_STATUS_HDRWAIT) {
			if((dRet = dCloseSIPM(pstMEMSINFO, pstSIPMTSESSKEY, pstSIPMTSESS, pstMSGINFO, i+1, 0)) < 0) {
				log_print(LOGN_CRI, LH"dCloseSIPM CIP=%s:%d SIP=%s:%d dRet=%d", 
						LT,
						util_cvtipaddr(szCIP, pstSIPMTSESSKEY->uiCliIP), pstSIPMTSESSKEY->usCliPort,
						util_cvtipaddr(szSIP, pstSIPMTSESSKEY->uiSrvIP), pstSIPMTSESSKEY->usSrvPort, dRet);
			}
		} else {
			if(pstMSGINFO->offset_DATA != 0) {
				nifo_node_delete(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));
			}
		}
	}

	hasho_del(pstSIPMHASH, (U8 *)pstSIPMTSESSKEY);

	return 0;
}

S32 dGetHDR(stMEMSINFO *pstMEMSINFO, st_SIPM_MSG_INFO *pstMSGINFO, U8 *szInData, S32 dDataSize, S32 *pdLen)
{
	S32			i;
	TLV			*pstTLV;
	U8			*pOUT;

	pstTLV = (TLV *)nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA + NIFO_SIZE);

	for(i = 0; i < dDataSize; i++)
	{
		switch(pstMSGINFO->dEndStatus)
		{
		case SIPM_ENDSTATUS_EMPTY:
			switch(szInData[i])
			{
			case 0x0D:
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_0D;
				break;
			default:	
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case SIPM_ENDSTATUS_0D:
			switch(szInData[i])
			{
			case 0x0A:
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_0D0A;
				break;
			case 0x0D:
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_0D;
				break;
			default:	
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case SIPM_ENDSTATUS_0D0A:
			switch(szInData[i])
			{
			case 0x0D:
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_0D0A0D;
				break;
			default:	
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case SIPM_ENDSTATUS_0D0A0D:
			switch(szInData[i])
			{
			case 0x0A:
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_0D0A0D0A;
				break;
			case 0x0D:
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_0D;
				break;
			default:	
				pstMSGINFO->dEndStatus = SIPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		default:
			log_print(LOGN_CRI, LH"UNKNOWN END_STATUS=%d", LT, pstMSGINFO->dEndStatus);
			break;
		}

		pOUT = nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_CurDATA);

		if(((NIFO *)pOUT)->maxoffset == ((NIFO *)pOUT)->lastoffset) {

			/* 새로운 cont Node 할당 */
			if((pOUT = nifo_node_alloc(pstMEMSINFO)) == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", LT);
				return -2;
			}

			nifo_node_link_cont_prev(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA), pOUT);
			pstMSGINFO->offset_CurDATA = nifo_offset(pstMEMSINFO, pOUT);
		}

		pOUT = nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_CurDATA);

		pOUT[((NIFO *)pOUT)->lastoffset] = szInData[i];
		((NIFO *)pOUT)->lastoffset++;
		pstTLV->len++;

		(*pdLen)++;
		pstMSGINFO->uiTotalLen++;
		pstMSGINFO->uiHdrLen++;

		if(pstMSGINFO->dEndStatus == SIPM_ENDSTATUS_0D0A0D0A) {
			return SIPM_ENDSTATUS_0D0A0D0A;
		}
		
	}

	return pstMSGINFO->dEndStatus;
}

S32 dGetBODY(stMEMSINFO *pstMEMSINFO, st_SIPM_MSG_INFO *pstMSGINFO, U8 *szInData, S32 dDataSize, S32 *pdLen)
{
	S32			i;
	TLV			*pstTLV;
	U8			*pOUT;

	pstTLV = (TLV *)nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA + NIFO_SIZE);

	for(i = 0; i < dDataSize; i++)
	{
		pOUT = nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_CurDATA);

		if(((NIFO *)pOUT)->maxoffset == ((NIFO *)pOUT)->lastoffset) {

			/* 새로운 cont Node 할당 */
			if((pOUT = nifo_node_alloc(pstMEMSINFO)) == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", LT);
				return -2;
			}

			nifo_node_link_cont_prev(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA), pOUT);
			pstMSGINFO->offset_CurDATA = nifo_offset(pstMEMSINFO, pOUT);
		}

		pOUT = nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_CurDATA);

		pOUT[((NIFO *)pOUT)->lastoffset] = szInData[i];
		((NIFO *)pOUT)->lastoffset++;
		pstTLV->len++;

		(*pdLen)++;
		pstMSGINFO->uiTotalLen++;
		pstMSGINFO->uiBodyLen++;

		if(pstMSGINFO->uiContentLen == pstMSGINFO->uiBodyLen) {
			return SIPM_ENDSTATUS_END;
		}
		
	}

	return pstMSGINFO->dEndStatus;
}

/*
 * $Log: sipm_func.c,v $
 * Revision 1.2  2011/09/05 12:26:41  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/18 01:38:45  hhbaek
 * A_SIPM
 *
 * Revision 1.2  2011/08/09 05:31:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.3  2011/01/11 04:09:09  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/08/19 14:07:40  pkg
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:36  dqms
 * Init TAF_RPPI
 *
 * Revision 1.3  2009/01/28 07:20:29  dark264sh
 * A_SIPM 데이터 merge시 multi node 사용 하도록 변경
 *
 * Revision 1.2  2008/12/29 11:38:02  dark264sh
 * SIP MSRP IPDataSize 버그 수정
 *
 * Revision 1.1  2008/09/18 07:19:52  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.4  2007/06/20 16:13:04  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2007/06/20 08:59:02  dark264sh
 * nifo node full시 할당 받지 않은 node를 삭제하는 문제
 *
 * Revision 1.2  2007/06/20 04:57:21  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/05/10 02:57:30  dark264sh
 * A_SIPM (TCP Merge) 추가
 *
 */

