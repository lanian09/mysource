/**		@file	msrpm_func.c
 * 		- MSRP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpm_func.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:47 $
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

#include "loglib.h"
#include "utillib.h"
#include "nifo.h"

#include "common_stg.h"
#include "procid.h"

#include "msrpm_util.h"
#include "msrpm_msgq.h"
#include "msrpm_func.h"


S32 dProcMSRPM(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPMHASH, TCP_INFO *pstTCPINFO, U8 *pDATA)
{
	S32					dIsStart, dLen, dRet, i;
	
	st_MSRPM_TSESS_KEY	stMSRPMTSESSKEY;
	st_MSRPM_TSESS_KEY	*pstMSRPMTSESSKEY = &stMSRPMTSESSKEY;
	st_MSRPM_TSESS		stMSRPMTSESS;
	st_MSRPM_TSESS		*pstMSRPMTSESS = &stMSRPMTSESS;

	U8					*pTCPDATA, *pNODE, *pTMP;
	st_MSRPM_MSG_INFO	*pstMSGINFO;

	stHASHONODE			*pstHASHONODE;

	U8					szCIP[INET_ADDRSTRLEN];
	U8					szSIP[INET_ADDRSTRLEN];
	U8					szTID[MSRP_TID_SIZE];


	switch(pstTCPINFO->cTcpFlag)
	{
	case DEF_TCP_START:
		log_print(LOGN_DEBUG, "@@@ START CIP=%s:%d SIP=%s:%d",
				util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
				util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);

		/* Make Hash Key */
		MakeHashKey(pstTCPINFO, pstMSRPMTSESSKEY);

		pstMSRPMTSESS->uiTcpSynTime = pstTCPINFO->uiCapTime;
		pstMSRPMTSESS->uiTcpSynMTime = pstTCPINFO->uiCapMTime;
		pstMSRPMTSESS->ucSynRtx = pstTCPINFO->ucRtx;
		pstMSRPMTSESS->dL4FailCode = pstTCPINFO->usL4FailCode;
		pstMSRPMTSESS->usAppCode = pstTCPINFO->usAppCode;
		pstMSRPMTSESS->usL4Code = pstTCPINFO->usL4Code;
		pstMSRPMTSESS->usL7Code = pstTCPINFO->usL7Code;
		pstMSRPMTSESS->ucTcpClientStatus = pstTCPINFO->ucTcpClientStatus;
		pstMSRPMTSESS->ucTcpServerStatus = pstTCPINFO->ucTcpServerStatus;

		pstMSRPMTSESS->uiIPDataUpPktCnt = pstTCPINFO->uiIPDataUpPktCnt;
		pstMSRPMTSESS->uiIPDataDnPktCnt = pstTCPINFO->uiIPDataDnPktCnt;
		pstMSRPMTSESS->uiIPTotUpPktCnt = pstTCPINFO->uiIPTotUpPktCnt;
		pstMSRPMTSESS->uiIPTotDnPktCnt = pstTCPINFO->uiIPTotDnPktCnt;
		pstMSRPMTSESS->uiIPDataUpRetransCnt = pstTCPINFO->uiIPDataUpRetransCnt;
		pstMSRPMTSESS->uiIPDataDnRetransCnt = pstTCPINFO->uiIPDataDnRetransCnt;
		pstMSRPMTSESS->uiIPTotUpRetransCnt = pstTCPINFO->uiIPTotUpRetransCnt;
		pstMSRPMTSESS->uiIPTotDnRetransCnt = pstTCPINFO->uiIPTotDnRetransCnt;
		pstMSRPMTSESS->uiIPDataUpPktSize = pstTCPINFO->uiIPDataUpPktSize;
		pstMSRPMTSESS->uiIPDataDnPktSize = pstTCPINFO->uiIPDataDnPktSize;
		pstMSRPMTSESS->uiIPTotUpPktSize = pstTCPINFO->uiIPTotUpPktSize;
		pstMSRPMTSESS->uiIPTotDnPktSize = pstTCPINFO->uiIPTotDnPktSize;
		pstMSRPMTSESS->uiIPDataUpRetransSize = pstTCPINFO->uiIPDataUpRetransSize;
		pstMSRPMTSESS->uiIPDataDnRetransSize = pstTCPINFO->uiIPDataDnRetransSize;
		pstMSRPMTSESS->uiIPTotUpRetransSize = pstTCPINFO->uiIPTotUpRetransSize;
		pstMSRPMTSESS->uiIPTotDnRetransSize = pstTCPINFO->uiIPTotDnRetransSize;

		if((pstHASHONODE = hasho_add(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY, (U8 *)pstMSRPMTSESS)) == NULL)
		{
			if((pstHASHONODE = hasho_find(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY)) == NULL)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d EXIST SAME HASH NODE BUT NOT FIND ??? CIP=%s:%d SIP=%s:%d", 
						__FILE__, __FUNCTION__, __LINE__,
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				exit(0);
			}

			pstMSRPMTSESS = (st_MSRPM_TSESS *)nifo_ptr(pstMSRPMHASH, pstHASHONODE->offset_Data);

			if((dRet = dCloseMSRPMTSESS(pstMEMSINFO, pstMSRPMHASH, pstMSRPMTSESSKEY, pstMSRPMTSESS)) < 0)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d dCloseMSRPMTSESS CIP=%s:%d SIP=%s:%d dRet=%d", 
						__FILE__, __FUNCTION__, __LINE__, 
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
				return -11;
			}

			pstMSRPMTSESS = &stMSRPMTSESS;

			if((pstHASHONODE = hasho_add(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY, (U8 *)pstMSRPMTSESS)) == NULL)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d hasho_add NULL CIP=%s:%d SIP=%s:%d", 
						__FILE__, __FUNCTION__, __LINE__, 
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				return -12;
			}
		}

		pstMSRPMTSESS = (st_MSRPM_TSESS *)nifo_ptr(pstMSRPMHASH, pstHASHONODE->offset_Data);
		
		for(i = 0; i < MSRPM_MSG_INFO_CNT; i++)
		{
			pstMSGINFO = &pstMSRPMTSESS->stMSRPMMSGINFO[i];

			memset(pstMSGINFO, 0x00, DEF_MSRPMMSGINFO_SIZE);

			if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL) {
				log_print(LOGN_CRI, "F=%s:%s.%d nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d", 
						__FILE__, __FUNCTION__, __LINE__, 
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				hasho_del(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY);
				return -21;
			}

			if(nifo_tlv_alloc(pstMEMSINFO, pNODE, ETH_DATA_NUM, 0, DEF_MEMSET_OFF) == NULL) {
				log_print(LOGN_CRI, "F=%s:%s.%d nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d", 
						__FILE__, __FUNCTION__, __LINE__, 
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				nifo_node_delete(pstMEMSINFO, pNODE);
				hasho_del(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY);
				return -22;
			}	

			pstMSGINFO->offset_DATA = nifo_offset(pstMEMSINFO, pNODE);
			pstMSGINFO->dStatus = MSRPM_STATUS_WAIT;
		}	

		pstMSRPMTSESS->stMSRPMMSGINFO[0].uiNextSeq = pstTCPINFO->uiSeqNum;
		pstMSRPMTSESS->stMSRPMMSGINFO[1].uiNextSeq = pstTCPINFO->uiAckNum;
				
		break;

	case DEF_TCP_END:
		log_print(LOGN_DEBUG, "*** END CIP=%s:%d SIP=%s:%d",
				util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
				util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);

		/* Make Hash Key */
		MakeHashKey(pstTCPINFO, pstMSRPMTSESSKEY);

		/* Find Hash */
		if((pstHASHONODE = hasho_find(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY)) == NULL)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d hasho_find NULL CIP=%s:%d SIP=%s:%d", 
					__FILE__, __FUNCTION__, __LINE__, 
					util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
					util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
			return -31;
		}

		pstMSRPMTSESS = (st_MSRPM_TSESS *)nifo_ptr(pstMSRPMHASH, pstHASHONODE->offset_Data);

		UpCount(pstTCPINFO, pstMSRPMTSESS);
				
		if((dRet = dCloseMSRPMTSESS(pstMEMSINFO, pstMSRPMHASH, pstMSRPMTSESSKEY, pstMSRPMTSESS)) < 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d dCloseMSRPMTSESS CIP=%s:%d SIP=%s:%d dRet=%d", 
					__FILE__, __FUNCTION__, __LINE__, 
					util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
					util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
			return -32;
		}

		break;

	case DEF_TCP_DATA:
		log_print(LOGN_DEBUG, 
			"### DATA CIP=%s:%d SIP=%s:%d SIZE=%u SEQ=%u ACK=%u L4=%d FAIL=%ld RTX=%s TIME=%u.%u",
			util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
			util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort,
			pstTCPINFO->uiDataSize, pstTCPINFO->uiSeqNum, pstTCPINFO->uiAckNum,
			pstTCPINFO->usL4Code, pstTCPINFO->usL4FailCode, 
			PrintRtx(pstTCPINFO->ucRtx), pstTCPINFO->uiCapTime, pstTCPINFO->uiCapMTime);

		pTCPDATA = &pDATA[pstTCPINFO->uiSOffset];

		log_print(LOGN_INFO, "INPUT LEN=%d DATA\n%.*s", pstTCPINFO->uiDataSize, pstTCPINFO->uiDataSize, pTCPDATA);

		/* Make Hash Key */
		MakeHashKey(pstTCPINFO, pstMSRPMTSESSKEY);

		/* Find Hash */
		if((pstHASHONODE = hasho_find(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY)) == NULL)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d hasho_find NULL CIP=%s:%d SIP=%s:%d", 
					__FILE__, __FUNCTION__, __LINE__, 
					util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
					util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
			return -41;
		}

		pstMSRPMTSESS = (st_MSRPM_TSESS *)nifo_ptr(pstMSRPMHASH, pstHASHONODE->offset_Data);
		UpCount(pstTCPINFO, pstMSRPMTSESS);

		pstMSGINFO = &pstMSRPMTSESS->stMSRPMMSGINFO[((pstTCPINFO->ucRtx == DEF_FROM_CLIENT) ? 0 : 1)];
		if(pstMSGINFO->offset_DATA == 0) {
			log_print(LOGN_CRI, "MSGINFO DATA NULL");
			return -100;
		}
		pstMSGINFO->uiIPDataSize = (pstTCPINFO->ucRtx == DEF_FROM_CLIENT) ? pstMSRPMTSESS->uiIPDataUpPktSize : pstMSRPMTSESS->uiIPDataDnPktSize;

		if((pstMSGINFO->uiNextSeq != 0) && (pstMSGINFO->uiNextSeq != pstTCPINFO->uiSeqNum))
		{
			log_print(LOGN_WARN, "F=%s:%s.%d DIFF SEQ NEXTSEQ=%u RCVSEQ=%u LASTSEQ= %u CIP=%s:%d SIP=%s:%d TIME=%u.%u RTX=%s",
					__FILE__, __FUNCTION__, __LINE__,
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
			memset(szTID, 0x00, MSRP_TID_SIZE);
			dIsStart = dCheckStart(&pTCPDATA[dLen], pstTCPINFO->uiDataSize - dLen, szTID);

			switch(pstMSGINFO->dStatus)
			{
			case MSRPM_STATUS_WAIT:
				if(!dIsStart) {
					/* Drop Packet */
					log_print(LOGN_WARN, "F=%s:%s.%d WAIT HEADER BUT NOT HEADER CIP=%s:%d SIP=%s:%d", 
							__FILE__, __FUNCTION__, __LINE__, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					return -61;
				} else {
					pstMSGINFO->uiStartTime = pstTCPINFO->uiCapTime;
					pstMSGINFO->uiStartMTime = pstTCPINFO->uiCapMTime;
					memcpy(pstMSGINFO->szTID, szTID, MSRP_TID_LEN);
					pstMSGINFO->szTID[MSRP_TID_LEN] = 0x00;
					pstMSGINFO->dTIDLen = strlen((char*)pstMSGINFO->szTID);	
				}
				break;

			case MSRPM_STATUS_START:
				if(dIsStart) {
					log_print(LOGN_CRI, "F=%s:%s.%d ABNORMAL MAKE MSG CIP=%s:%d SIP=%s:%d", 
							__FILE__, __FUNCTION__, __LINE__, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					log_print(LOGN_INFO, "F=%s:%s.%d TCP_DATA=%.*s", __FILE__, __FUNCTION__, __LINE__, pstTCPINFO->uiDataSize, pTCPDATA);
					pTMP = (U8 *)nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA + NIFO_SIZE + TLV_SIZE); 
					log_print(LOGN_INFO, "F=%s:%s.%d BUFFER_DATA=%.*s", __FILE__, __FUNCTION__, __LINE__, pstMSGINFO->uiLen, pTMP);

					if((dRet = dCloseMSRPM(pstMEMSINFO, pstMSRPMTSESSKEY, pstMSRPMTSESS, pstMSGINFO, pstTCPINFO->ucRtx, MSRP_UERR_9300)) < 0) {
						log_print(LOGN_CRI, "F=%s:%s.%d dCloseMSRPM CIP=%s:%d SIP=%s:%d dRet=%d", 
								__FILE__, __FUNCTION__, __LINE__, 
								util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
								util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
						return -81;
					}

					if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL) {
						log_print(LOGN_CRI, "F=%s:%s.%d nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d", 
								__FILE__, __FUNCTION__, __LINE__, 
								util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
								util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
						return -82;
					}

					if(nifo_tlv_alloc(pstMEMSINFO, pNODE, ETH_DATA_NUM, 0, DEF_MEMSET_OFF) == NULL) {
						log_print(LOGN_CRI, "F=%s:%s.%d nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d", 
								__FILE__, __FUNCTION__, __LINE__, 
								util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
								util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
						nifo_node_delete(pstMEMSINFO, pNODE);
						return -83;
					}	
					pstMSGINFO->offset_DATA = nifo_offset(pstMEMSINFO, pNODE);
					pstMSGINFO->uiStartTime = pstTCPINFO->uiCapTime;
					pstMSGINFO->uiStartMTime = pstTCPINFO->uiCapMTime;
					memcpy(pstMSGINFO->szTID, szTID, MSRP_TID_LEN);
					pstMSGINFO->szTID[MSRP_TID_LEN] = 0x00;
					pstMSGINFO->dTIDLen = strlen((char*)pstMSGINFO->szTID);	
				}
				break;

			default:
				log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN STATUS=%d CIP=%s:%d SIP=%s:%d", 
						__FILE__, __FUNCTION__, __LINE__, pstMSGINFO->dStatus,
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
				return -71;
			}

			pstMSGINFO->uiLastUpdateTime = pstTCPINFO->uiCapTime;
			pstMSGINFO->uiLastUpdateMTime = pstTCPINFO->uiCapMTime;
			pstMSGINFO->uiAckTime = pstTCPINFO->uiAckTime;
			pstMSGINFO->uiAckMTime = pstTCPINFO->uiAckMTime;

			dRet = dFlowMSRPM(pstMEMSINFO, pstMSGINFO, &pTCPDATA[dLen], pstTCPINFO->uiDataSize - dLen, &dLen);
			log_print(LOGN_INFO, "CIP=%s:%d SIP=%s:%d dFlowMSRPM ENDSTATUS=%d",
						util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
						util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
			if(dRet == MSRPM_ENDSTATUS_END)
			{
//				pstMSGINFO->uiIPDataSize -= (pstTCPINFO->uiDataSize - dLen);
				if((dRet = dCloseMSRPM(pstMEMSINFO, pstMSRPMTSESSKEY, pstMSRPMTSESS, pstMSGINFO, pstTCPINFO->ucRtx, 0)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d dCloseMSRPM CIP=%s:%d SIP=%s:%d dRet=%d", 
							__FILE__, __FUNCTION__, __LINE__, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort, dRet);
					return -51;
				}

				if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, "F=%s:%s.%d nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d", 
							__FILE__, __FUNCTION__, __LINE__, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					return -52;
				}

				if(nifo_tlv_alloc(pstMEMSINFO, pNODE, ETH_DATA_NUM, 0, DEF_MEMSET_OFF) == NULL) {
					log_print(LOGN_CRI, "F=%s:%s.%d nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d", 
							__FILE__, __FUNCTION__, __LINE__, 
							util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
							util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
					nifo_node_delete(pstMEMSINFO, pNODE);
					return -53;
				}	

				pstMSGINFO->offset_DATA = nifo_offset(pstMEMSINFO, pNODE);
				pstMSGINFO->dStatus = MSRPM_STATUS_WAIT;
			}
			else if(dRet < 0)
			{
				/* 에러 발생 */
				log_print(LOGN_CRI, "F=%s:%s.%d dFlowMSRPM dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);

				for(i = 0; i < MSRPM_MSG_INFO_CNT; i++)
				{
					pstMSGINFO = &pstMSRPMTSESS->stMSRPMMSGINFO[i];
					nifo_node_delete(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));
				}
				hasho_del(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY);

				return -91;
			} 
			else
			{
				pstMSGINFO->dStatus = MSRPM_STATUS_START;
			}
		}

		break;

	default:
		log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN TCPFLAG=%d CIP=%s:%d SIP=%s:%d", 
				__FILE__, __FUNCTION__, __LINE__, pstTCPINFO->cTcpFlag,
				util_cvtipaddr(szCIP, pstTCPINFO->uiCliIP), pstTCPINFO->usCliPort,
				util_cvtipaddr(szSIP, pstTCPINFO->uiSrvIP), pstTCPINFO->usSrvPort);
		break;
	}

	return 0;
}

S32 dCloseMSRPM(stMEMSINFO *pstMEMSINFO, st_MSRPM_TSESS_KEY *pstMSRPMTSESSKEY, st_MSRPM_TSESS *pstMSRPMTSESS, st_MSRPM_MSG_INFO *pstMSGINFO, U8 ucRtx, U16 usFailCode)
{
	S32					dRet;
	U8					*pNODE, *pDATA;
	TEXT_INFO			*pstTEXTINFO;
	U32					uiLastSeq, uiNextSeq;

	U8					szCIP[INET_ADDRSTRLEN];
	U8					szSIP[INET_ADDRSTRLEN];

	log_print(LOGN_DEBUG, "CLOSE MSRP CIP=%s:%d SIP=%s:%d",
				util_cvtipaddr(szCIP, pstMSRPMTSESSKEY->uiCliIP), pstMSRPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPMTSESSKEY->uiSrvIP), pstMSRPMTSESSKEY->usSrvPort);

	if((pNODE = nifo_node_alloc(pstMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d nifo_node_alloc NULL CIP=%s:%d SIP=%s:%d",
				__FILE__, __FUNCTION__, __LINE__,
				util_cvtipaddr(szCIP, pstMSRPMTSESSKEY->uiCliIP), pstMSRPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPMTSESSKEY->uiSrvIP), pstMSRPMTSESSKEY->usSrvPort);
		nifo_node_delete(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));	
		memset(pstMSGINFO, 0x00, DEF_MSRPMMSGINFO_SIZE);
		return -1;
	}
	if((pstTEXTINFO = (TEXT_INFO *)nifo_tlv_alloc(pstMEMSINFO, pNODE, TEXT_INFO_DEF_NUM, TEXT_INFO_SIZE, DEF_MEMSET_ON)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d nifo_tlv_alloc NULL CIP=%s:%d SIP=%s:%d",
				__FILE__, __FUNCTION__, __LINE__,
				util_cvtipaddr(szCIP, pstMSRPMTSESSKEY->uiCliIP), pstMSRPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPMTSESSKEY->uiSrvIP), pstMSRPMTSESSKEY->usSrvPort);
		nifo_node_delete(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));	
		memset(pstMSGINFO, 0x00, DEF_MSRPMMSGINFO_SIZE);
		return -2;

	}

	pstTEXTINFO->offset = 0;
	pstTEXTINFO->len = pstMSGINFO->uiLen;
	pstTEXTINFO->uiStartTime = pstMSGINFO->uiStartTime;
	pstTEXTINFO->uiStartMTime = pstMSGINFO->uiStartMTime;
	pstTEXTINFO->uiLastUpdateTime = pstMSGINFO->uiLastUpdateTime;
	pstTEXTINFO->uiLastUpdateMTime = pstMSGINFO->uiLastUpdateMTime;
	pstTEXTINFO->uiAckTime = pstMSGINFO->uiAckTime;
	pstTEXTINFO->uiAckMTime = pstMSGINFO->uiAckMTime;
	pstTEXTINFO->clientIP = pstMSRPMTSESSKEY->uiCliIP;
	pstTEXTINFO->serverIP = pstMSRPMTSESSKEY->uiSrvIP;
	pstTEXTINFO->clientPort = pstMSRPMTSESSKEY->usCliPort;
	pstTEXTINFO->serverPort = pstMSRPMTSESSKEY->usSrvPort;
	pstTEXTINFO->protocol = DEF_PROTOCOL_TCP;
	pstTEXTINFO->IPDataSize = (pstMSGINFO->uiIPDataSize == 0) ? pstMSGINFO->uiLen : pstMSGINFO->uiIPDataSize;
	pstTEXTINFO->range = pstMSRPMTSESS->dRange;
	pstTEXTINFO->network = pstMSRPMTSESS->dNetwork;
	pstTEXTINFO->rawFileIndex = pstMSGINFO->uiRawFileIndex;
	pstTEXTINFO->rawPacketIndex = pstMSGINFO->uiRawPacketIndex;
	pstTEXTINFO->rtx = ucRtx;
	pstTEXTINFO->failcode = usFailCode;
	pstTEXTINFO->usL4Code = pstMSRPMTSESS->usL4Code;

	pDATA = (U8 *)nifo_get_value(pstMEMSINFO, ETH_DATA_NUM, pstMSGINFO->offset_DATA);
	log_print(LOGN_INFO, "SEND LEN=%d DATA\n%.*s", pstMSGINFO->uiLen, pstMSGINFO->uiLen, pDATA);

	InitCount(pstMSRPMTSESS, ucRtx);
	
	nifo_node_link_nont_next(pstMEMSINFO, pNODE, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));

	uiNextSeq = pstMSGINFO->uiNextSeq;
	uiLastSeq = pstMSGINFO->uiLastSeq;

	if((dRet = dSend_MSRPM_Data(pstMEMSINFO, SEQ_PROC_A_MSRPT, pNODE)) < 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d dSend_MSRPM_Data CIP=%s:%d SIP=%s:%d dRet=%d",
				__FILE__, __FUNCTION__, __LINE__,
				util_cvtipaddr(szCIP, pstMSRPMTSESSKEY->uiCliIP), pstMSRPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPMTSESSKEY->uiSrvIP), pstMSRPMTSESSKEY->usSrvPort, dRet);
		nifo_node_delete(pstMEMSINFO, pNODE);	
		memset(pstMSGINFO, 0x00, DEF_MSRPMMSGINFO_SIZE);
		pstMSGINFO->uiNextSeq = uiNextSeq;
		pstMSGINFO->uiLastSeq = uiLastSeq;
		return -3;
	} else {
		log_print(LOGN_INFO, "SEND MSG CIP=%s:%d SIP=%s:%d OFFSET=%ld", 
				util_cvtipaddr(szCIP, pstMSRPMTSESSKEY->uiCliIP), pstMSRPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPMTSESSKEY->uiSrvIP), pstMSRPMTSESSKEY->usSrvPort,
				nifo_offset(pstMEMSINFO, pNODE));
		memset(pstMSGINFO, 0x00, DEF_MSRPMMSGINFO_SIZE);
		pstMSGINFO->uiNextSeq = uiNextSeq;
		pstMSGINFO->uiLastSeq = uiLastSeq;
	}

	return 0;
}

S32 dCloseMSRPMTSESS(stMEMSINFO *pstMEMSINFO, stHASHOINFO *pstMSRPMHASH, st_MSRPM_TSESS_KEY *pstMSRPMTSESSKEY, st_MSRPM_TSESS *pstMSRPMTSESS)
{
	S32					i, dRet;
	st_MSRPM_MSG_INFO	*pstMSGINFO;

	U8					szCIP[INET_ADDRSTRLEN];
	U8					szSIP[INET_ADDRSTRLEN];

	log_print(LOGN_DEBUG, "CLOSE TSESS CIP=%s:%d SIP=%s:%d",
				util_cvtipaddr(szCIP, pstMSRPMTSESSKEY->uiCliIP), pstMSRPMTSESSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPMTSESSKEY->uiSrvIP), pstMSRPMTSESSKEY->usSrvPort);

	for(i = 0; i < MSRPM_MSG_INFO_CNT; i++)
	{
		pstMSGINFO = &pstMSRPMTSESS->stMSRPMMSGINFO[i];

		if(pstMSGINFO->dStatus == MSRPM_STATUS_START) {
			if((dRet = dCloseMSRPM(pstMEMSINFO, pstMSRPMTSESSKEY, pstMSRPMTSESS, pstMSGINFO, i+1, 0)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d dCloseMSRPM CIP=%s:%d SIP=%s:%d dRet=%d", 
						__FILE__, __FUNCTION__, __LINE__,
						util_cvtipaddr(szCIP, pstMSRPMTSESSKEY->uiCliIP), pstMSRPMTSESSKEY->usCliPort,
						util_cvtipaddr(szSIP, pstMSRPMTSESSKEY->uiSrvIP), pstMSRPMTSESSKEY->usSrvPort, dRet);
			}
		} else {
			if(pstMSGINFO->offset_DATA != 0) {
				nifo_node_delete(pstMEMSINFO, nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA));
			}
		}
	}

	hasho_del(pstMSRPMHASH, (U8 *)pstMSRPMTSESSKEY);

	return 0;
}

S32 dFlowMSRPM(stMEMSINFO *pstMEMSINFO, st_MSRPM_MSG_INFO *pstMSGINFO, U8 *szInData, S32 dDataSize, S32 *pdLen)
{
	S32			i;
	TLV			*pstTLV;
	U8			*pOUT;

	pstTLV = (TLV *)nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA + NIFO_SIZE);

	for(i = 0; i < dDataSize; i++)
	{
		switch(pstMSGINFO->dEndStatus)
		{
		case MSRPM_ENDSTATUS_EMPTY:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_H1:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H2;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_H2:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H3;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_H3:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H4;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_H4:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H5;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_H5:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H6;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_H6:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H7;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_H7:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H7;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[0]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T1;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T1:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T1]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T2;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T2:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T2]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T3;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T3:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T3]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T4;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T4:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T4]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T5;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T5:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T5]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T6;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T6:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T6]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T7;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T7:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T7]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T8;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T8:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T8]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T9;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T9:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T9]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T10;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T10:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T10]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T11;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T11:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T11]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T12;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T12:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T12]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T13;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T13:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T13]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T14;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T14:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T14]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T15;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T15:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T15]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T16;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T16:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T16]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T17;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T17:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T17]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T18;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T18:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T18]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T19;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T19:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T19]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T20;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T20:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T20]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T21;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T21:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T21]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T22;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T22:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T22]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T23;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T23:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T23]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T24;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T24:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T24]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T25;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T25:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T25]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T26;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T26:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T26]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T27;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T27:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T27]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T28;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T28:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T28]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T29;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T29:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T29]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T30;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T30:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T30]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T31;
					if(pstMSGINFO->dTIDLen == pstMSGINFO->dEndStatus) pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T31:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			default:	
				if(szInData[i] == pstMSGINFO->szTID[MSRPM_ENDSTATUS_T31]) {
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_T;
				} else {	
					pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				}
				break;
			}
			break;

		case MSRPM_ENDSTATUS_T:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			case '#':
			case '$':
			case '+':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_F;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_F:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H1;
				break;
			case 0x0D:
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_0D;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		case MSRPM_ENDSTATUS_0D:
			switch(szInData[i])
			{
			case '-':
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_H6;
				break;
			case 0x0A:
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_END;
				break;
			default:	
				pstMSGINFO->dEndStatus = MSRPM_ENDSTATUS_EMPTY;
				break;
			}
			break;

		default:
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN END_STATUS=%d", __FILE__, __FUNCTION__, __LINE__, pstMSGINFO->dEndStatus);
			break;
		}

		pOUT = nifo_ptr(pstMEMSINFO, pstMSGINFO->offset_DATA);

		if(((NIFO *)pOUT)->maxoffset == ((NIFO *)pOUT)->lastoffset) {
			log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW", __FILE__, __FUNCTION__, __LINE__);
			return -1;
		}

		pOUT[((NIFO *)pOUT)->lastoffset] = szInData[i];
		((NIFO *)pOUT)->lastoffset++;
		pstTLV->len++;

		(*pdLen)++;
		pstMSGINFO->uiLen++;

		if(pstMSGINFO->dEndStatus == MSRPM_ENDSTATUS_END) {
			return MSRPM_ENDSTATUS_END;
		}
		
	}

	return 0;
}

/*
 * $Log: msrpm_func.c,v $
 * Revision 1.3  2011/09/07 06:30:47  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 05:43:37  uamyd
 * MSRPM modified
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:52  hhbaek
 * Commit TAF/SRC
 *
 * Revision 1.3  2011/08/17 12:12:17  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/08 11:05:43  uamyd
 * modified block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.3  2011/01/11 04:09:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/08/19 14:09:59  pkg
 * A_SIPM, A_MSRPM nifo node full 방어 코드 추가
 *
 * Revision 1.1.1.1  2009/05/26 02:14:39  dqms
 * Init TAF_RPPI
 *
 * Revision 1.2  2008/12/29 11:38:33  dark264sh
 * SIP MSRP IPDataSize 버그 수정
 *
 * Revision 1.1  2008/09/18 06:35:03  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.10  2007/06/20 16:13:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2007/06/20 08:58:28  dark264sh
 * nifo node full시 할당 받지 않은 node를 삭제하는 문제
 *
 * Revision 1.8  2007/06/20 04:57:57  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2007/06/13 12:37:14  dark264sh
 * 가변 TID 처리 max len = 32
 *
 * Revision 1.6  2007/06/13 07:23:38  dark264sh
 * *** empty log message ***
 *
 * Revision 1.5  2007/06/13 06:28:51  dark264sh
 * A_MSRPM 상태값 잘못 체크하는 부분 수정
 *
 * Revision 1.4  2007/06/13 04:50:36  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2007/06/12 04:21:18  dark264sh
 * MSRP Message Overflow bug 수정
 *
 * Revision 1.2  2007/05/10 04:51:31  dark264sh
 * A_MSRPM, A_MSRPT 버그 수정
 *
 * Revision 1.1  2007/05/07 01:46:17  dark264sh
 * INIT
 *
 */

