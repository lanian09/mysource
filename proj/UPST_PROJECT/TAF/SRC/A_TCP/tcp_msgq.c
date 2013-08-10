/**		@file	tcp_msgq.c
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: tcp_msgq.c,v 1.3 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
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

/* SYS HEADER */
/* LIB HEADER */
#include "cifo.h"
#include "gifo.h"
#include "loglib.h"
#include "utillib.h"
#include "Analyze_Ext_Abs.h"
/* PRO HEADER */
#include "procid.h"
#include "capdef.h"
/* TAM HEADER */
/* TAF HEADER */
#include "debug.h"
#include "func_time_check.h"
/* OAM HEADER */
/* LOC HEADER */
#include "tcp_util.h"
#include "tcp_msgq.h"

extern st_FuncTimeCheckList	*pFUNC;
extern ATCP_SUBINFO			*pATCPSUBINFO;
extern stCIFO				*gpCIFO;

extern int              	gAHTTPCnt;
extern UINT					guiSeqProcID;

extern S32 					dGetCALLProcID(U32 uiClientIP);

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
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
END_FUNC_TIME_CHECK(pFUNC, 20);
		return -1;
	}

	if((pTCPINFO = (TCP_INFO *)nifo_tlv_alloc(pMEMSINFO, pNode, TCP_INFO_DEF_NUM, TCP_INFO_SIZE, DEF_MEMSET_OFF)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
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
	pTCPINFO->ucRtx = pTCPSESS->ucSynRtx;
	pTCPINFO->cTcpFlag = DEF_TCP_START;
	pTCPINFO->cRetrans = DEF_RETRANS_OFF;
	pTCPINFO->ucProtocol = DEF_PROTOCOL_TCP;

	SetData(pTCPSESS, pTCPINFO);

	dSeqProcID = (pTCPSESS->dSndMsgQ > 0) ? pTCPSESS->dSndMsgQ : dGetCALLProcID(pTCPSESSKEY->uiSIP);

	if((dRet = dSend_TCP_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dSendData dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
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
		if(dSeqProcID == ( SEQ_PROC_A_HTTP + i ) ){
			pATCPSUBINFO->httpSndCnt++;
			pATCPSUBINFO->allSndCnt++;
		}
	}	
	if(dSeqProcID == SEQ_PROC_A_JNC) {
		pATCPSUBINFO->delCnt++;
		pATCPSUBINFO->allSndCnt++;
	}

	if(dSeqProcID > 0) {
		log_print(LOGN_DEBUG, "SEND DATA OFFSET[%ld] dSeqProcID[%d]", nifo_offset(pMEMSINFO, pData), dSeqProcID);

		if((dRet = gifo_write(pMEMSINFO, gpCIFO, guiSeqProcID, dSeqProcID, nifo_offset(pMEMSINFO, pData))) < 0) {
			log_print(LOGN_CRI, "[%s.%d] gifo_write. to =%d dRet[%d][%s]", 
				__FUNCTION__, __LINE__, dSeqProcID, dRet, strerror(-dRet));
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
		log_print(LOGN_INFO, "[%s][%s.%d] BEFORE REQ PRINT SEQ", __FILE__, __FUNCTION__, __LINE__);
		nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
	} else {
		log_print(LOGN_INFO, "[%s][%s.%d] BEFORE REQ PRINT SEQ NO HAVE DATA", __FILE__, __FUNCTION__, __LINE__);
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
		log_print(LOGN_INFO, "[%s][%s.%d] REQ BASE[%u] NEXTREQSEQ[%u] RCVACK[%u] TCPINFO_SEQNUM[%u] TCPINFO_DATA[%d] TCPINFO_ACKTIME[%d] RCVACKTIME[%d]",
			__FILE__, __FUNCTION__, __LINE__, uiBase, pTCPSESS->uiNextReqSeq, pTCPSESS->uiRcvReqAck, pTCPINFO->uiSeqNum, pTCPINFO->uiDataSize, pTCPINFO->uiAckTime, pTCPSESS->uiRcvReqAckTime);
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
				LH" FAIL SEND PAYLOAD SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%ld]MTIME[%ld]RTX[%s]SEQ[%d]",LT	
					,util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
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
		log_print(LOGN_INFO, "[%s][%s.%d] AFTER REQ PRINT SEQ", __FILE__, __FUNCTION__, __LINE__);
		nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
	} else {
		log_print(LOGN_INFO, "[%s][%s.%d] AFTER REQ PRINT SEQ NO HAVE DATA", __FILE__, __FUNCTION__, __LINE__);
	}
#endif

	pData = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData);
	uiBase = pTCPSESS->uiSynAckSeq;
#ifdef DEBUG1
	if(pData != NULL) {
		log_print(LOGN_INFO, "[%s][%s.%d] BEFORE RES PRINT SEQ", __FILE__, __FUNCTION__, __LINE__);
		nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
	} else {
		log_print(LOGN_INFO, "[%s][%s.%d] BEFORE RES PRINT SEQ NO HAVE DATA", __FILE__, __FUNCTION__, __LINE__);
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
		log_print(LOGN_INFO, "[%s][%s.%d] RES BASE[%u] NEXTRESSEQ[%u] RCVACK[%u] TCPINFO_SEQNUM[%u] TCPINFO_DATA[%d] TCPINFO_ACKTIME[%d] RCVACKTIME[%d]",
			__FILE__, __FUNCTION__, __LINE__, uiBase, pTCPSESS->uiNextResSeq, pTCPSESS->uiRcvResAck, pTCPINFO->uiSeqNum, pTCPINFO->uiDataSize, pTCPINFO->uiAckTime, pTCPSESS->uiRcvResAckTime);
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
				LH"  FAIL SEND PAYLOAD SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%ld]MTIME[%ld]RTX[%s]SEQ[%u]",LT,
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
		log_print(LOGN_INFO, "[%s][%s.%d] AFTER RES PRINT SEQ", __FILE__, __FUNCTION__, __LINE__);
		nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
	} else {
		log_print(LOGN_INFO, "[%s][%s.%d] AFTER RES PRINT SEQ NO HAVE DATA", __FILE__, __FUNCTION__, __LINE__);
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
			"FAIL SEND LOGDATA SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]SEQ[%d]RCV_ACK[%d]", 
			util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
			util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
			pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime, PrintRtx(pTCPINFO->ucRtx), pTCPINFO->uiSeqNum, pTCPINFO->uiAckNum);
		nifo_node_free(pMEMSINFO, pNode);
	}

//	LOG_TCP_SESS_Prt("PRINT LOG_TCP_SESS", (LOG_TCP_SESS *)nifo_ptr(pMEMSINFO, pTCPSESS->offset_LOG));

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
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
END_FUNC_TIME_CHECK(pFUNC, 24);
		return -1;
	}

	if((pTCPINFO = (TCP_INFO *)nifo_tlv_alloc(pMEMSINFO, pNode, TCP_INFO_DEF_NUM, TCP_INFO_SIZE, DEF_MEMSET_OFF)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
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

	if((dRet = dSend_TCP_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dSendData dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
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
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
END_FUNC_TIME_CHECK(pFUNC, 25);
		return -1;
	}

	if((pTCPINFO = (TCP_INFO *)nifo_tlv_alloc(pMEMSINFO, pNode, TCP_INFO_DEF_NUM, TCP_INFO_SIZE, DEF_MEMSET_OFF)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
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
		log_print(LOGN_CRI, "[%s][%s.%d] dSendData dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		nifo_node_delete(pMEMSINFO, pNode);
END_FUNC_TIME_CHECK(pFUNC, 25);
		return -3;
	}
END_FUNC_TIME_CHECK(pFUNC, 25);
	return 0;
}

/**
 *  $Log: tcp_msgq.c,v $
 *  Revision 1.3  2011/09/07 06:30:48  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/09/06 06:44:52  dcham
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 07:25:47  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/10 09:57:45  uamyd
 *  modified and block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.9  2011/05/12 01:52:29  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.8  2011/05/09 15:03:35  dark264sh
 *  A_TCP: A_CALL multi 처리
 *
 *  Revision 1.7  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.6  2009/08/19 12:29:41  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.5  2009/08/10 19:24:39  dqms
 *  패킷이 빠졌을 때 연산에서 MICRO 시간까지 비교
 *
 *  Revision 1.4  2009/07/29 12:42:11  jsyoon
 *  TCP_STOP pTCPINFO->uiCapTime = pTCPSESS->uiLastUpdateTime
 *
 *  Revision 1.3  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.2  2009/06/25 17:41:12  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:33  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.2  2009/01/28 07:10:07  dark264sh
 *  A_TCP 서버 SYN 처리 | FIN 처리 버그 수정
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.3  2007/09/03 05:29:18  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2007/08/29 07:41:13  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/08/21 12:54:17  dark264sh
 *  no message
 *
 *  Revision 1.26  2006/12/01 09:37:34  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.25  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.24  2006/11/28 02:21:41  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.23  2006/11/21 09:16:24  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.22  2006/11/21 08:24:06  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.21  2006/11/20 14:33:21  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.20  2006/11/13 07:55:00  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.19  2006/11/10 11:47:13  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.18  2006/11/07 10:56:54  dark264sh
 *  A_TCP SEQ가 맞지 않아도 그 다음 ACK를 받은 경우 전송하도록 변경
 *
 *  Revision 1.17  2006/11/07 07:14:15  dark264sh
 *  A_TCP에 받은 SIZE, A_HTTP, A_ONLINE으로 보낸 개수 Debug용 코드 추가
 *
 *  Revision 1.16  2006/11/06 07:53:18  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.15  2006/11/06 07:35:29  dark264sh
 *  nifo NODE size 4*1024 => 6*1024로 변경하기
 *  nifo_tlv_alloc에서 argument로 memset할지 말지 결정하도록 수정
 *  nifo_node_free에서 semaphore 삭제
 *
 *  Revision 1.14  2006/11/03 08:30:02  dark264sh
 *  A_TCP에 func_time_check 추가
 *
 *  Revision 1.13  2006/11/01 09:24:44  dark264sh
 *  SESS, SEQ, NODE 개수 LOG추가
 *
 *  Revision 1.12  2006/11/01 05:51:21  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.11  2006/10/31 13:10:33  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.10  2006/10/31 05:11:31  dark264sh
 *  TCP START MSG를 synack의 ack를 받은 이후로 수정
 *
 *  Revision 1.9  2006/10/31 02:47:13  dark264sh
 *  Sess Update Msg 추가
 *
 *  Revision 1.8  2006/10/30 00:50:47  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2006/10/23 11:35:01  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.6  2006/10/20 10:02:43  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2006/10/18 12:15:36  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2006/10/18 08:53:31  dark264sh
 *  nifo debug 코드 추가
 *
 *  Revision 1.3  2006/10/16 14:39:47  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2006/10/11 11:52:33  dark264sh
 *  PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 *  Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 *  no message
 *
 *  Revision 1.23  2006/10/10 08:38:13  dark264sh
 *  에러 핸들링 추가
 *
 *  Revision 1.22  2006/10/10 07:00:30  dark264sh
 *  A_CALL에 전송하는 부분 추가
 *  nifo_node_alloc 함수 변경에 따른 변경
 *  A_TCP에서 timerN_update의 리턴으로 timerNID 업데이트 하도록 변경
 *
 *  Revision 1.21  2006/09/26 07:39:58  dark264sh
 *  HTTP로 전송 하기 위한 Data Cnt, Size 잘못 전달 하는 부분 수정
 *
 *  Revision 1.20  2006/09/26 07:29:03  dark264sh
 *  오타 수정
 *
 *  Revision 1.19  2006/09/26 07:26:27  dark264sh
 *  HTTP로 전송 하기 위한 Data Cnt, Size 잘못 전달 하는 부분 수정
 *
 *  Revision 1.18  2006/09/25 09:12:14  dark264sh
 *  no message
 *
 *  Revision 1.17  2006/09/25 05:51:30  dark264sh
 *  디버깅 목적으로 LOG_TCP_SESS Print 추가
 *
 *  Revision 1.16  2006/09/22 07:24:16  dark264sh
 *  no message
 *
 *  Revision 1.15  2006/09/18 03:15:06  dark264sh
 *  no message
 *
 *  Revision 1.14  2006/09/15 02:53:44  dark264sh
 *  LOG 데이터를 HTTP가 아닌 PAGE로 보내도록 수정
 *  nifo_msg_read에서 현재는 자연스러운 처리가 돼지 않음.
 *  향후 nifo_msg_read를 수정해서 처리해야 할꺼 같음.
 *
 *  Revision 1.13  2006/09/14 12:03:29  dark264sh
 *  no message
 *
 *  Revision 1.12  2006/09/14 04:48:50  dark264sh
 *  no message
 *
 *  Revision 1.11  2006/09/13 04:30:25  dark264sh
 *  strerror 잘못 찍는 부분 수정
 *
 *  Revision 1.10  2006/09/11 02:22:57  dark264sh
 *  nifo 변경에 따른 변경
 *
 *  Revision 1.9  2006/09/06 11:43:28  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.8  2006/09/04 06:38:51  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2006/09/04 05:53:33  dark264sh
 *  READ_VAL_LIST들을 global로 처리
 *
 *  Revision 1.6  2006/09/04 05:30:58  dark264sh
 *  Data Cnt, Size를 HTTP에 전송하기 위한 부분 수정
 *
 *  Revision 1.5  2006/08/21 09:30:37  dark264sh
 *  L4FailCode 설정 함수 추가
 *
 *  Revision 1.4  2006/08/21 07:40:30  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2006/08/21 07:29:51  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2006/08/21 03:07:38  dark264sh
 *  no message
 *
 */
