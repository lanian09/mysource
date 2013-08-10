/**		@file	tcp_msgq.c
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: itcp_msgq.c,v 1.2 2011/09/05 12:26:40 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 12:26:40 $
 * 		@ref		tcp_msgq.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "procid.h"
#include "commdef.h"
#include "func_time_check.h"	/* st_FuncTimeCheckList */
#include "capdef.h"

// LIB headers
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "loglib.h"
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */

// TAF headers
#include "debug.h"				/* ATCP_SUBINFO */

// .
#include "itcp_msgq.h"

/** dTCPStartMsg function.
 *
 *  dTCPStartMsg Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pTCPSESSKEY : TCP SESS HASH KEY
 *  @param	*pTCPSESS : TCP SESS HASH DATA
 *  @param	*pINFOETH : ETH 정보
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see			tcp_msgq.c
 *
 **/
S32 dTCPStartMsg(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS, INFO_ETH *pINFOETH)
{
START_FUNC_TIME_CHECK(pFUNC, 20);
	S32				dRet;
	S32				dSeqProcID;
	U8				*pNode;
	TCP_INFO		*pTCPINFO;

	if((pNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "LHnifo_node_alloc NULL", LT);
END_FUNC_TIME_CHECK(pFUNC, 20);
		return -1;
	}

	if((pTCPINFO = (TCP_INFO *)nifo_tlv_alloc(pMEMSINFO, pNode, TCP_INFO_DEF_NUM, TCP_INFO_SIZE, DEF_MEMSET_OFF)) == NULL) {
		log_print(LOGN_CRI, "LHnifo_tlv_alloc NULL", LT);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 20);
		return -2;	
	}	

	pTCPINFO->uiDataSize = 0;
	pTCPINFO->uiSrvIP = pTCPSESSKEY->uiDIP;
	pTCPINFO->uiCliIP = pTCPSESSKEY->uiSIP;
	pTCPINFO->usSrvPort = pTCPSESSKEY->usDPort;
	pTCPINFO->usCliPort = pTCPSESSKEY->usSPort;
	pTCPINFO->uiSeqNum = pTCPSESS->uiNextReqSeq;
	pTCPINFO->uiAckNum = pTCPSESS->uiNextResSeq;
	pTCPINFO->uiSOffset = 0;
	pTCPINFO->uiCapTime = pTCPSESS->uiSessCreateTime;
	pTCPINFO->uiCapMTime = pTCPSESS->uiSessCreateMTime;
	pTCPINFO->uiAckTime = 0;
	pTCPINFO->uiAckMTime = 0;
	pTCPINFO->usAppCode = pTCPSESS->usAppCode;
	pTCPINFO->usL4Code = pTCPSESS->usL4Code;
	pTCPINFO->usL7Code = pTCPSESS->usL7Code;
	pTCPINFO->usL4FailCode = 0;
	pTCPINFO->ucRtx = (pTCPSESS->ucStatus == DEF_STATUS_DATA) ? DEF_FROM_NONE : pTCPSESS->ucSynRtx;
	pTCPINFO->cTcpFlag = DEF_TCP_START;
	pTCPINFO->cRetrans = DEF_RETRANS_OFF;
	pTCPINFO->ucProtocol = DEF_PROTOCOL_TCP;

	SetData(pTCPSESS, pTCPINFO);

	dSeqProcID = (pTCPSESS->dSndMsgQ > 0) ? pTCPSESS->dSndMsgQ : dGetCALLProcID(pTCPSESSKEY->uiSIP);

	pATCPSUBINFO->sigCnt++;

	if((dRet = dSend_TCP_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
		log_print(LOGN_CRI, "LHdSendData dRet[%d]", LT, dRet);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 20);
		return -3;
	}
	
END_FUNC_TIME_CHECK(pFUNC, 20);
	return 0;
}

S32 dSend_TCP_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pData)
{
	int 		i;
START_FUNC_TIME_CHECK(pFUNC, 21);
#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pData);
#else
	S32			dRet;

	for(i=0; i<gAHTTPCnt; i++) {
		if(dSeqProcID == ( SEQ_PROC_A_IHTTP + i ) ) {
			pATCPSUBINFO->httpSndCnt++;
			pATCPSUBINFO->allSndCnt++;
			break;
		}
	}	

	if(dSeqProcID > 0) {
        log_print(LOGN_DEBUG, "SEND DATA OFFSET[%d] dSeqProcID[%d]", nifo_offset(pMEMSINFO, pData), dSeqProcID);

        if((dRet = gifo_write(pMEMSINFO, gpCIFO, guiSeqProcID, dSeqProcID, nifo_offset(pMEMSINFO, pData))) < 0) {
            log_print(LOGN_CRI, LH"gifo_write. to =%d dRet[%d][%s]", LT, dSeqProcID, dRet, strerror(-dRet));
END_FUNC_TIME_CHECK(pFUNC, 21);
            return -1;
        }
    } else {
        log_print(LOGN_DEBUG, "SEND DATA SeqProcID[%d]", dSeqProcID);
        nifo_node_delete(pMEMSINFO, pData);
    }

#endif

END_FUNC_TIME_CHECK(pFUNC, 21);
	return 0;
}

/** dSendPayLoad function.
 *
 *  dSendPayLoad Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pTCPSESSKEY : TCP SESS HASH KEY
 *  @param	*pTCPSESS : TCP SESS HASH DATA
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(TLV NODE 생성 실패) -2(메시지 전송 실패)
 *  @see			tcp_msgq.c
 *
 **/
S32 dSendPayLoad(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS)
{
START_FUNC_TIME_CHECK(pFUNC, 22);
	S32				dRet;
	U32				uiBase;
	OFFSET			offset;
	U8				*pData, *pNext;
	INFO_ETH		*pINFOETH;
	TCP_INFO		*pTCPINFO;
	Capture_Header_Msg	*pCAPHEAD;
    U8				szSIP[INET_ADDRSTRLEN];
    U8				szDIP[INET_ADDRSTRLEN];


	pData = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ReqData);
	uiBase = pTCPSESS->uiSynSeq;
#ifdef DEBUG1
	if(pData != NULL) {
		log_print(LOGN_INFO, "LHBEFORE REQ PRINT SEQ", LT);
		nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
	} else {
		log_print(LOGN_INFO, "LHBEFORE REQ PRINT SEQ NO HAVE DATA", LT);
	}
#endif
	while(pData != NULL) {
		offset = nifo_offset(pMEMSINFO, pData);

		pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, offset);
		pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
		pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
		
		/**
		 *  SEQ가 MAX값을 넘어서 할당된 경우 처리 방안이 필요
		 * 	다른 변수를 두어 처리 하는 방안 고려
		 */
#ifdef DEBUG1
		log_print(LOGN_INFO, "LHREQ BASE[%u] NEXTREQSEQ[%u] RCVACK[%u] TCPINFO_SEQNUM[%u] TCPINFO_DATA[%d] TCPINFO_ACKTIME[%d] RCVACKTIME[%d]",
			LT, uiBase, pTCPSESS->uiNextReqSeq, pTCPSESS->uiRcvReqAck, pTCPINFO->uiSeqNum, pTCPINFO->uiDataSize, pTCPINFO->uiAckTime, pTCPSESS->uiRcvReqAckTime);
#endif
#if 0
		if((OFFSET_SEQ(uiBase, pTCPSESS->uiNextReqSeq) == OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)) 
			&& (pTCPINFO->uiAckTime > 0) 
			&& (OFFSET_SEQ(uiBase, pTCPSESS->uiRcvReqAck) >= OFFSET_SEQ(uiBase, (pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize)))) {
#endif	
#if 0
        if(((pTCPINFO->uiAckTime > 0)
                && (OFFSET_SEQ(uiBase, pTCPSESS->uiRcvReqAck) >= OFFSET_SEQ(uiBase, (pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize))))
            && (((OFFSET_SEQ(uiBase, pTCPSESS->uiNextReqSeq) == OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)))
                || (pTCPSESS->uiReqCount > 10))) {
#endif
#if 0
        if(((pTCPINFO->uiAckTime > 0) && (OFFSET_SEQ(uiBase, pTCPSESS->uiNextReqSeq) == OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)))
			|| ((pTCPINFO->uiAckTime > 0) && (pTCPINFO->uiAckTime < pTCPSESS->uiRcvReqAckTime))) {
#endif
#if 0
		log_print(LOGN_DEBUG, "#### REQ pTCPINFO->uiAckTime:%u.%u pTCPSESS->uiRcvReqAckTime:%u.%u BEFORE:%ld AFTER:%ld",
				pTCPINFO->uiAckTime, pTCPINFO->uiAckMTime, pTCPSESS->uiRcvReqAckTime, pTCPSESS->uiRcvReqAckMTime, 
				(long)pTCPINFO->uiAckTime * 1000000 + (long)pTCPINFO->uiAckMTime, 
				(long)pTCPSESS->uiRcvReqAckTime * 1000000 + (long)pTCPSESS->uiRcvReqAckMTime);
#endif
		if( (pTCPINFO->uiAckTime > 0) && 
				(((OFFSET_SEQ(uiBase, pTCPSESS->uiNextReqSeq) == OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum))) || 
				(((long)pTCPINFO->uiAckTime * 1000000 + (long)pTCPINFO->uiAckMTime) < ((long)pTCPSESS->uiRcvReqAckTime * 1000000 + (long)pTCPSESS->uiRcvReqAckMTime))) ) {

			/* FOR DEBUG */
			if((OFFSET_SEQ(uiBase, pTCPSESS->uiNextReqSeq) != OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)))
				pATCPSUBINFO->diffSeqCnt++;

			pNext = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pData)->nont.offset_next), NIFO, nont);
			nifo_node_unlink_nont(pMEMSINFO, pData);
			if(pData == pNext)
				pNext = NULL;	

			pTCPSESS->uiLastReqSeq = pTCPINFO->uiSeqNum; 						/* LAST SND SEQ-NUM UPDATE */
			pTCPSESS->uiNextReqSeq = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize; /* NEXT SND SEQ-NUM UPDATE */

			if((dRet = dSend_TCP_Data(pMEMSINFO, pTCPSESS->dSndMsgQ, pData)) < 0) {
				log_print(LOGN_CRI, 
				"LHFAIL SEND PAYLOAD SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]SEQ[%d]", 
					LT,
					util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
					util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
					pCAPHEAD->curtime, pCAPHEAD->ucurtime, PrintRtx(pTCPINFO->ucRtx),
					pTCPINFO->uiSeqNum);
			}

			pData = pNext;

			pTCPSESS->uiReqCount--;
			pTCPSESS->offset_ReqData = nifo_offset(pMEMSINFO, pData);

		} else {
			break;
		}
	}
#ifdef DEBUG1
	pData = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ReqData);
	if(pData != NULL) {
		log_print(LOGN_INFO, "LHAFTER REQ PRINT SEQ", LT);
		nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
	} else {
		log_print(LOGN_INFO, "LHAFTER REQ PRINT SEQ NO HAVE DATA", LT);
	}
#endif

	pData = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData);
	uiBase = pTCPSESS->uiSynAckSeq;
#ifdef DEBUG1
	if(pData != NULL) {
		log_print(LOGN_INFO, "LHBEFORE RES PRINT SEQ", LT);
		nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
	} else {
		log_print(LOGN_INFO, "LHBEFORE RES PRINT SEQ NO HAVE DATA", LT);
	}
#endif
	while(pData != NULL) {
		offset = nifo_offset(pMEMSINFO, pData);

		pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, offset);
		pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
		pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
		
		/**
		 *  SEQ가 MAX값을 넘어서 할당된 경우 처리 방안이 필요
		 * 	다른 변수를 두어 처리 하는 방안 고려
		 */
#ifdef DEBUG1
		log_print(LOGN_INFO, "LHRES BASE[%u] NEXTRESSEQ[%u] RCVACK[%u] TCPINFO_SEQNUM[%u] TCPINFO_DATA[%d] TCPINFO_ACKTIME[%d] RCVACKTIME[%d]",
			LT, uiBase, pTCPSESS->uiNextResSeq, pTCPSESS->uiRcvResAck, pTCPINFO->uiSeqNum, pTCPINFO->uiDataSize, pTCPINFO->uiAckTime, pTCPSESS->uiRcvResAckTime);
#endif

#if 0
		if((OFFSET_SEQ(uiBase, pTCPSESS->uiNextResSeq) == OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)) 
			&& (pTCPINFO->uiAckTime > 0)
			&& (OFFSET_SEQ(uiBase, pTCPSESS->uiRcvResAck) == OFFSET_SEQ(uiBase, (pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize)))) {
#endif
#if 0
        if(((pTCPINFO->uiAckTime > 0)
                && (OFFSET_SEQ(uiBase, pTCPSESS->uiRcvResAck) >= OFFSET_SEQ(uiBase, (pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize))))
            && (((OFFSET_SEQ(uiBase, pTCPSESS->uiNextResSeq) == OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)))
                || (pTCPSESS->uiResCount > 10))) {
#endif
#if 0
        if(((pTCPINFO->uiAckTime > 0) && (OFFSET_SEQ(uiBase, pTCPSESS->uiNextResSeq) == OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)))
			|| ((pTCPINFO->uiAckTime > 0) && (pTCPINFO->uiAckTime < pTCPSESS->uiRcvResAckTime))) {
#endif
#if 0
		log_print(LOGN_DEBUG, "#### RES pTCPINFO->uiAckTime:%u.%u pTCPSESS->uiRcvResAckTime:%u.%u BEFORE:%ld AFTER:%ld",
				pTCPINFO->uiAckTime, pTCPINFO->uiAckMTime, pTCPSESS->uiRcvResAckTime, pTCPSESS->uiRcvResAckMTime, 
				(long)pTCPINFO->uiAckTime * 1000000 + (long)pTCPINFO->uiAckMTime, 
				(long)pTCPSESS->uiRcvResAckTime * 1000000 + (long)pTCPSESS->uiRcvResAckMTime);
#endif
		if( (pTCPINFO->uiAckTime > 0) && 
				(((OFFSET_SEQ(uiBase, pTCPSESS->uiNextResSeq) == OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum))) || 
				(((long)pTCPINFO->uiAckTime * 1000000 + (long)pTCPINFO->uiAckMTime) < ((long)pTCPSESS->uiRcvResAckTime * 1000000 + (long)pTCPSESS->uiRcvResAckMTime))) ) {

			/* FOR DEBUG */
			if((OFFSET_SEQ(uiBase, pTCPSESS->uiNextResSeq) != OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)))
				pATCPSUBINFO->diffSeqCnt++;

			pNext = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pData)->nont.offset_next), NIFO, nont);
			nifo_node_unlink_nont(pMEMSINFO, pData);
			if(pData == pNext)
				pNext = NULL;	

			pTCPSESS->uiLastResSeq = pTCPINFO->uiSeqNum; 						/* LAST SND SEQ-NUM UPDATE */
			pTCPSESS->uiNextResSeq = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize; /* NEXT SND SEQ-NUM UPDATE */

			if((dRet = dSend_TCP_Data(pMEMSINFO, pTCPSESS->dSndMsgQ, pData)) < 0) {
				log_print(LOGN_CRI, 
				"LHFAIL SEND PAYLOAD SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]SEQ[%d]", 
					LT,
					util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
					util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
					pCAPHEAD->curtime, pCAPHEAD->ucurtime, PrintRtx(pTCPINFO->ucRtx),
					pTCPINFO->uiSeqNum);
			}

			pData = pNext;

			pTCPSESS->uiResCount--;
			pTCPSESS->offset_ResData = nifo_offset(pMEMSINFO, pData);

		} else {
			break;
		}
	}
#ifdef DEBUG1
	pData = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData);
	if(pData != NULL) {
		log_print(LOGN_INFO, "LHAFTER RES PRINT SEQ", LT);
		nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
	} else {
		log_print(LOGN_INFO, "LHAFTER RES PRINT SEQ NO HAVE DATA", LT);
	}
#endif

END_FUNC_TIME_CHECK(pFUNC, 22);
	return 0;
}

/** dSendAllData function.
 *
 *  dSendAllData Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pTCPSESSKEY : TCP SESS HASH KEY
 *  @param	*pTCPSESS : TCP SESS HASH DATA
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(TLV NODE 생성 실패) -2(메시지 전송 실패)
 *  @see			tcp_msgq.c
 *
 **/
S32 dSendAllData(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS)
{
START_FUNC_TIME_CHECK(pFUNC, 23);
	S32				dRet;
	U32				uiBase;
	OFFSET			offset;
	OFFSET			offset_LOGNODE;
	U8				*pNode, *pNextNode;
	TCP_INFO		*pTCPINFO;
    U8				szSIP[INET_ADDRSTRLEN];
    U8				szDIP[INET_ADDRSTRLEN];

	/* SEND ReqData */
	pNode = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ReqData);
	uiBase = pTCPSESS->uiSynSeq;
	while(pNode != NULL) {
		pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);		

		offset = nifo_offset(pMEMSINFO, pNode);
		pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
		pTCPINFO->usL4FailCode = pTCPSESS->usL4FailCode;

		/* FOR DEBUG */
		if((OFFSET_SEQ(uiBase, pTCPSESS->uiNextReqSeq) != OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)))
			pATCPSUBINFO->diffSeqCnt++;

		nifo_node_unlink_nont(pMEMSINFO, pNode);
		if(pNode == pNextNode)
			pNextNode = NULL;

		pTCPSESS->uiLastReqSeq = pTCPINFO->uiSeqNum; 						/* LAST SND SEQ-NUM UPDATE */
		pTCPSESS->uiNextReqSeq = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize; /* NEXT SND SEQ-NUM UPDATE */

		if((dRet = dSend_TCP_Data(pMEMSINFO, pTCPSESS->dSndMsgQ, pNode)) < 0) {
			log_print(LOGN_CRI, 
				"FAIL SEND REQDATA SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]SEQ[%d]RCV_ACK[%d]", 
				util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
				util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
				pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime, PrintRtx(pTCPINFO->ucRtx), pTCPINFO->uiSeqNum, pTCPINFO->uiAckNum);
			nifo_node_free(pMEMSINFO, pNode);
		}

		pNode = pNextNode;
	}	

	pTCPSESS->uiReqCount = 0;
	pTCPSESS->offset_ReqData = 0;

	/* SEND ResData */
	pNode = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData);
	uiBase = pTCPSESS->uiSynAckSeq;
	while(pNode != NULL) {
		pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);		

		offset = nifo_offset(pMEMSINFO, pNode);
		pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
		pTCPINFO->usL4FailCode = pTCPSESS->usL4FailCode;

		/* FOR DEBUG */
		if((OFFSET_SEQ(uiBase, pTCPSESS->uiNextResSeq) != OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum)))
			pATCPSUBINFO->diffSeqCnt++;

		nifo_node_unlink_nont(pMEMSINFO, pNode);
		if(pNode == pNextNode)
			pNextNode = NULL;

		pTCPSESS->uiLastResSeq = pTCPINFO->uiSeqNum; 						/* LAST SND SEQ-NUM UPDATE */
		pTCPSESS->uiNextResSeq = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize; /* NEXT SND SEQ-NUM UPDATE */

		if((dRet = dSend_TCP_Data(pMEMSINFO, pTCPSESS->dSndMsgQ, pNode)) < 0) {
			log_print(LOGN_CRI, 
				"FAIL SEND RESDATA SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]SEQ[%d]RCV_ACK[%d]", 
				util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
				util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
				pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime, PrintRtx(pTCPINFO->ucRtx), pTCPINFO->uiSeqNum, pTCPINFO->uiAckNum);
			nifo_node_free(pMEMSINFO, pNode);
		}

		pNode = pNextNode;
	}	

	pTCPSESS->uiResCount = 0;
	pTCPSESS->offset_ResData = 0;

	/* LOG */	
	offset_LOGNODE = nifo_get_offset_node(pMEMSINFO, nifo_ptr(pMEMSINFO, pTCPSESS->offset_LOG));
	pNode = nifo_ptr(pMEMSINFO, offset_LOGNODE);

	nifo_node_unlink_nont(pMEMSINFO, pNode);
	if((dRet = dSend_TCP_Data(pMEMSINFO, dGetCALLProcID(pTCPSESSKEY->uiSIP), pNode)) < 0) {
		log_print(LOGN_CRI, 
			"FAIL SEND LOGDATA SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
			util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
			util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort);
		nifo_node_free(pMEMSINFO, pNode);
	}

//	LOG_ITCP_SESS_Prt("PRINT LOG_ITCP_SESS", (LOG_ITCP_SESS *)nifo_ptr(pMEMSINFO, pTCPSESS->offset_LOG));

	pTCPSESS->offset_LOG = 0;

END_FUNC_TIME_CHECK(pFUNC, 23);
	return 0;
}

/** dTCPStartMsg function.
 *
 *  dTCPStartMsg Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pTCPSESSKEY : TCP SESS HASH KEY
 *  @param	*pTCPSESS : TCP SESS HASH DATA
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see			tcp_msgq.c
 *
 **/
S32 dTCPStopMsg(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS)
{
START_FUNC_TIME_CHECK(pFUNC, 24);
	S32				dRet;
	S32				dSeqProcID;
	U8				*pNode;
	TCP_INFO		*pTCPINFO;

	if((pNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "LHnifo_node_alloc NULL", LT);
END_FUNC_TIME_CHECK(pFUNC, 24);
		return -1;
	}

	if((pTCPINFO = (TCP_INFO *)nifo_tlv_alloc(pMEMSINFO, pNode, TCP_INFO_DEF_NUM, TCP_INFO_SIZE, DEF_MEMSET_OFF)) == NULL) {
		log_print(LOGN_CRI, "LHnifo_tlv_alloc NULL", LT);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 24);
		return -2;	
	}	

	pTCPINFO->uiDataSize = 0;
	pTCPINFO->uiSrvIP = pTCPSESSKEY->uiDIP;
	pTCPINFO->uiCliIP = pTCPSESSKEY->uiSIP;
	pTCPINFO->usSrvPort = pTCPSESSKEY->usDPort;
	pTCPINFO->usCliPort = pTCPSESSKEY->usSPort;
	pTCPINFO->uiSeqNum = 0;
	pTCPINFO->uiAckNum = 0;
	pTCPINFO->uiSOffset = 0;
	pTCPINFO->uiCapTime = pTCPSESS->uiLastUpdateTime;
	pTCPINFO->uiCapMTime = pTCPSESS->uiLastUpdateMTime;
	pTCPINFO->uiAckTime = 0;
	pTCPINFO->uiAckMTime = 0;
	pTCPINFO->usAppCode = pTCPSESS->usAppCode;
	pTCPINFO->usL4Code = pTCPSESS->usL4Code;
	pTCPINFO->usL7Code = pTCPSESS->usL7Code;
	pTCPINFO->usL4FailCode = pTCPSESS->usL4FailCode;
	pTCPINFO->ucRtx = pTCPSESS->ucSynRtx;
	pTCPINFO->cTcpFlag = DEF_TCP_END;
	pTCPINFO->cRetrans = DEF_RETRANS_OFF;
	pTCPINFO->ucProtocol = DEF_PROTOCOL_TCP;

	SetData(pTCPSESS, pTCPINFO);

	dSeqProcID = (pTCPSESS->dSndMsgQ > 0) ? pTCPSESS->dSndMsgQ : dGetCALLProcID(pTCPSESSKEY->uiSIP);

	pATCPSUBINFO->sigCnt++;

	if((dRet = dSend_TCP_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
		log_print(LOGN_CRI, "LHdSendData dRet[%d]", LT, dRet);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 24);
		return -3;
	}
END_FUNC_TIME_CHECK(pFUNC, 24);
	return 0;
}

S32 dSessUpdataMsg(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS)
{
START_FUNC_TIME_CHECK(pFUNC, 25);
	S32				dRet;
	U8				*pNode;
	TCP_INFO		*pTCPINFO;

	if((pNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "LHnifo_node_alloc NULL", LT);
END_FUNC_TIME_CHECK(pFUNC, 25);
		return -1;
	}

	if((pTCPINFO = (TCP_INFO *)nifo_tlv_alloc(pMEMSINFO, pNode, TCP_INFO_DEF_NUM, TCP_INFO_SIZE, DEF_MEMSET_OFF)) == NULL) {
		log_print(LOGN_CRI, "LHnifo_tlv_alloc NULL", LT);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 25);
		return -2;	
	}	

	pTCPINFO->uiDataSize = 0;
	pTCPINFO->uiSrvIP = pTCPSESSKEY->uiDIP;
	pTCPINFO->uiCliIP = pTCPSESSKEY->uiSIP;
	pTCPINFO->usSrvPort = pTCPSESSKEY->usDPort;
	pTCPINFO->usCliPort = pTCPSESSKEY->usSPort;
	pTCPINFO->uiSeqNum = 0;
	pTCPINFO->uiAckNum = 0;
	pTCPINFO->uiSOffset = 0;
	pTCPINFO->uiCapTime = 0;
	pTCPINFO->uiCapMTime = 0;
	pTCPINFO->uiAckTime = 0;
	pTCPINFO->uiAckMTime = 0;
	pTCPINFO->usAppCode = 0;
	pTCPINFO->usL4Code = 0;
	pTCPINFO->usL7Code = 0;
	pTCPINFO->usL4FailCode = 0;
	pTCPINFO->ucRtx = 0;
	pTCPINFO->cTcpFlag = DEF_TCP_UPDATE;
	pTCPINFO->cRetrans = DEF_RETRANS_OFF;
	pTCPINFO->ucProtocol = DEF_PROTOCOL_TCP;

	if((dRet = dSend_TCP_Data(pMEMSINFO, dGetCALLProcID(pTCPSESSKEY->uiSIP), pNode)) < 0) {
		log_print(LOGN_CRI, "LHdSendData dRet[%d]", LT, dRet);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 25);
		return -3;
	}
END_FUNC_TIME_CHECK(pFUNC, 25);
	return 0;
}

/** dTCPStartMsg function.
 *
 *  dTCPStartMsg Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pTCPSESSKEY : TCP SESS HASH KEY
 *  @param	*pTCPSESS : TCP SESS HASH DATA
 *  @param	*pINFOETH : ETH 정보
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see			tcp_msgq.c
 *
 **/
S32 dTCPReCallStartMsg(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS)
{
START_FUNC_TIME_CHECK(pFUNC, 26);
	S32				dRet;
	S32				dSeqProcID;
	U8				*pNode;
	LOG_SIGNAL		*pCOMMON;
	U8				szIP[INET_ADDRSTRLEN];

	if((pNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "LHnifo_node_alloc NULL", LT);
END_FUNC_TIME_CHECK(pFUNC, 26);
		return -1;
	}

	if((pCOMMON = (LOG_SIGNAL *)nifo_tlv_alloc(pMEMSINFO, pNode, START_PI_DATA_RECALL_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL) {
		log_print(LOGN_CRI, "LHnifo_tlv_alloc NULL", LT);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 26);
		return -2;	
	}	

	pCOMMON->uiClientIP = pTCPSESSKEY->uiSIP;
	pCOMMON->uiCallTime = pTCPSESS->uiSessCreateTime;
	pCOMMON->uiCallMTime = pTCPSESS->uiSessCreateMTime;

	dSeqProcID = dGetCALLProcID(pTCPSESSKEY->uiSIP);

	if((dRet = dSend_TCP_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
		log_print(LOGN_CRI, "LHdSendData dRet[%d]", LT, dRet);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 26);
		return -3;
	}
	else
	{
		log_print(LOGN_DEBUG, "SEND RECALL START IP=%s:%u", util_cvtipaddr(szIP, pCOMMON->uiClientIP), pCOMMON->uiClientIP);
	}
	
END_FUNC_TIME_CHECK(pFUNC, 26);
	return 0;
}

/**
 *  $Log: itcp_msgq.c,v $
 *  Revision 1.2  2011/09/05 12:26:40  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 13:02:39  hhbaek
 *  A_ITCP
 *
 *  Revision 1.2  2011/08/10 09:57:44  uamyd
 *  modified and block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.13  2011/05/12 01:47:06  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.12  2011/05/12 01:25:05  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.11  2011/05/12 01:18:17  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.10  2011/05/12 01:09:42  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.9  2011/05/12 01:02:16  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.8  2011/05/11 12:14:10  dark264sh
 *  A_ITCP log_print를 잘못 찍는 부분 수정
 *
 *  Revision 1.7  2011/05/09 09:59:34  dark264sh
 *  A_ITCP: A_CALL multi 처리
 *
 *  Revision 1.6  2011/04/17 15:05:13  dark264sh
 *  A_ITCP, A_IHTTP: log_print 수정
 *
 *  Revision 1.5  2011/04/16 13:35:04  dark264sh
 *  A_ITCP: ReCallStart 전송시 LOG_SIGNAL 사용하도록 변경
 *
 *  Revision 1.4  2011/04/16 12:00:22  dark264sh
 *  A_ITCP: SYN, SYNACK없이 세션을 생성한 경우 RTX = DEF_FROM_NONE 세팅
 *
 *  Revision 1.3  2011/04/16 10:28:38  dark264sh
 *  A_ITCP: 서버방향에 의해서 세션 생성시 RecallStart 전송
 *
 *  Revision 1.2  2011/04/14 11:21:55  dark264sh
 *  A_ITCP: LOG_TCP_SESS => LOG_ITCP_SESS 변경
 *
 *  Revision 1.1  2011/04/12 02:51:50  dark264sh
 *  A_ITCP 추가
 *
 *
 */
