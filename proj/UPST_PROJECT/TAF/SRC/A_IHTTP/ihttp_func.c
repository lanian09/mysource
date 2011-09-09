/**		@file	ihttp_func.c
 * 		- HTTP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: ihttp_func.c,v 1.2 2011/09/04 11:40:36 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 11:40:36 $
 * 		@ref		http_func.c http_main.c http_init.c http_util.c l4.h a_http_api.h
 * 		@todo		
 *
 * 		@section	Intro(소개)
 * 		- HTTP Transaction을 관리 하는 함수들
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <sys/time.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"

// PROJECT
#include "procid.h"
#include "sshmid.h"

// TAF
#include "http.h"
#include "tools.h"


// .
#include "ihttp_func.h"
#include "ihttp_msgq.h"
#include "ihttp_util.h"


extern S64		curSessCnt;		/* Transaction 개수 */
extern S64		sessCnt;
extern S64		rcvNodeCnt;		/* 받은  NODE 개수  */
extern S64		diffSeqCnt;		/* DIFF SEQ 가 된 개수  */

/** dProcHttpTrans function.
 *
 *  TCP로 부터 받은 데이터를 Control하고 TCP Session, HTTP Transaction 관리
 *
 *  @param	*pMEMSINFO : New Interface 관리를 위한 구조체
 *  @param	*pTCPHASH : TCP Session 관리를 위한 HASH
 *  @param	*pHTTPHASH : HTTP Transaction 관리를 위한 HASH
 *  @param	*pTCPINFO : TCP로 부터 받은 데이터의 정보를 담고 있는 구조체
 *  @param	*pNode : TCP로 부터 New Interface Node
 *
 *  @return			S32
 *  @see			http_func.c http_msgq.c http_main.c http_init.c http_util.c http_api.h
 *
 *  @exception		nothing
 *  @note			nothing
 **/
S32 dProcHttpTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, stHASHOINFO *pHTTPHASH, TCP_INFO *pTCPINFO, U8 *pNode, U8 *pDATA)
{
   	S32					dLen = 0;	/**< 처리된 버퍼의 사이즈 */
	S32					dSeqProcID;
	S32					dRet;		/**< 함수 Return 값 */
	U32					isNotREQ, isHDR;
	U32					uiNextSeq;	/**< 받을 데이터의 Seq번호, 데이터가 손실없이 도착했는지 비교를 위해서*/
	U16					failCode;
	U8					*pTMPINPUT;
	U8					szSIP[INET_ADDRSTRLEN];
	U8					szDIP[INET_ADDRSTRLEN];
	U8					*pTCPDATA;
	U8					*pHEAD;
	HTTP_TSESS_KEY		HTTPTSESSKEY;
	HTTP_TSESS_KEY		*pHTTPTSESSKEY;
	HTTP_TSESS			*pHTTPTSESS;
	HTTP_TRANS_KEY		HTTPTRANSKEY;
	HTTP_TRANS_KEY		*pHTTPTRANSKEY;

	stHASHONODE			*pHASHNODE;
	st_MSG_INFO			*pMSGINFO;

	pHTTPTSESSKEY = &HTTPTSESSKEY;
	pHTTPTRANSKEY = &HTTPTRANSKEY;

	rcvNodeCnt++;

	/* TCP 상태에 따른 처리 */
	switch(pTCPINFO->cTcpFlag)
	{
	case DEF_TCP_START:
		/** 
		 *  A_TCP에서 TCP Session이 생성된 경우 
		 *  새로운 TCP Node 생성
		 *  A_CALL에 TCP Session 생성 통보 
		 **/

		log_print(LOGN_DEBUG, "@@@ START SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
				util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
					
		/* Make Tcp Hash Key */
		MakeTcpHashKey(pTCPINFO, pHTTPTSESSKEY);

		/* Create Tcp Session */
		if((pHTTPTSESS = pCreateTcpSess(pMEMSINFO, pTCPHASH, pHTTPHASH, pHTTPTSESSKEY, pTCPINFO)) == NULL) {
			log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] pCreateTcpSess NULL", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
			return -11;
		}

		dSeqProcID = dGetCALLProcID(pTCPINFO->uiCliIP);

		/* A_CALL에 TCP Session 생성 통보 */
		if((dRet = dSend_HTTP_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dSendSignal dRet[%d]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, dRet);
			return -12;
		}

		break;
	
	case DEF_TCP_END:
		/**
		 *  A_TCP에서 TCP Session이 끝난 경우
		 *  TCP Node 삭제
		 *  남아 있는 HTTP Node 종료
		 *  A_CALL에 TCP Session이 종료 사실 통보
		 *  Ack 체크
		 **/
		
		log_print(LOGN_DEBUG, "*** END SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
				util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);

		/* Make Tcp Hash Key */
		MakeTcpHashKey(pTCPINFO, pHTTPTSESSKEY);

		/* Find Tcp Hash */
		if((pHASHNODE = hasho_find(pTCPHASH, (U8 *)pHTTPTSESSKEY)) == NULL)
		{
			/* Tcp Hash를 찾지 못함 */
			log_print(LOGN_CRI, "[%s][%s.%d] HASH NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
			return -21;
		}

		pHTTPTSESS = (HTTP_TSESS *)nifo_ptr(pTCPHASH, pHASHNODE->offset_Data);
		UpdateHttpTsess(pTCPINFO, pHTTPTSESS);

		/*
		 * 에러 값 세팅, 연산 값 
		 */

		/* 해당 Http Transaction 찾기 */
		if((dRet = dCloseTcpSess(pMEMSINFO, pTCPHASH, pHTTPHASH, pHTTPTSESSKEY, pHTTPTSESS)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dCloseTcpSess dRet[%d]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, dRet);
			return -22;
		}

		dSeqProcID = dGetCALLProcID(pTCPINFO->uiCliIP);

		/* A_CALL에 Tcp Session 종료 통보 */
	 	if((dRet = dSend_HTTP_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dSendSignal dRet[%d]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, dRet);
			return -23;
		}
		 
		break;

	case DEF_TCP_DATA:
		/**
		 *  TCP Node가 존재하는지 체크
		 *  Header 시작인지 체크
		 *  데이터 버퍼링
		 **/
		pTCPDATA = &pDATA[pTCPINFO->uiSOffset];

		log_print(LOGN_DEBUG, 
				"### DATA SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]SIZE[%u]SEQ[%u]ACK[%u]L4[%d]FAIL[%ld]RTX[%s]STIME[%u]MTIME[%u]", 
				util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
				util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort,
				pTCPINFO->uiDataSize, pTCPINFO->uiSeqNum, pTCPINFO->uiAckNum,
				pTCPINFO->usL4Code, pTCPINFO->usL4FailCode, 
				PrintRtx(pTCPINFO->ucRtx), pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime);

		/* Make Tcp Hash Key */
		MakeTcpHashKey(pTCPINFO, pHTTPTSESSKEY);

		/* Find Tcp Hash */
		if((pHASHNODE = hasho_find(pTCPHASH, (U8 *)pHTTPTSESSKEY)) == NULL)
		{
			/* Tcp Hash를 찾지 못함 */
			log_print(LOGN_CRI, "[%s][%s.%d] HASH NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
			return -31;
		}

		pHTTPTSESS = (HTTP_TSESS *)nifo_ptr(pTCPHASH, pHASHNODE->offset_Data);

		if(pHTTPTSESS->ucSynRtx == DEF_FROM_NONE)
		{
			log_print(LOGN_DEBUG, "CHECK RTX SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s]",
					util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
					util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
			if((dRet = dCheckRtx(pTCPDATA, pTCPINFO->uiDataSize, pTCPINFO->ucRtx)) > 0)
			{
				pHTTPTSESS->ucSynRtx = dRet;
			}
			else
			{
				log_print(LOGN_WARN, "[%s][%s.%d] NO START NO HEADER NO RTX SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s]", 
						__FILE__, __FUNCTION__, __LINE__,
						util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
						util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
				return -110;
			}	
		}

		/* 기본적인 값 UPDATE */
		UpdateHttpTsess(pTCPINFO, pHTTPTSESS);

		/**
		 *	SEQ 검사 
		 *	일치하지 않는 경우 연속적인 데이터가 아니라고 판단
		 *	모든 세션 종료
		 */
		isNotREQ = (pTCPINFO->ucRtx == pHTTPTSESS->ucSynRtx) ? 0 : 1;
		pMSGINFO = &pHTTPTSESS->MSGINFO[isNotREQ];

		if(pMSGINFO->offset_HDR == 0 || pMSGINFO->offset_BODY == 0) {
			log_print(LOGN_CRI, "MSGINFO HDR BODY NULL");
			return -100;
		}

		uiNextSeq = pMSGINFO->uiNextSeq;
		if((uiNextSeq != 0) && (uiNextSeq != pTCPINFO->uiSeqNum)) {
			/* 모든 HTTP Trans 정리 */
			log_print(LOGN_CRI, 
				"DIFF SEQ NSEQ[%u]RSEQ[%u] SIP[%s:%u]DIP[%s:%u]STIME[%u.%d]RTX[%s]", 
					uiNextSeq, pTCPINFO->uiSeqNum,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, 
					pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime, PrintRtx(pTCPINFO->ucRtx));

if(pMSGINFO->uiLastSeq == pTCPINFO->uiSeqNum)
log_print(LOGN_CRI, "SAME =====================================");

			diffSeqCnt++;

#if 0
 			if((dRet = dCloseAllHttpTrans(pMEMSINFO, pHTTPHASH, pHTTPTSESSKEY, pHTTPTSESS, HTTP_UERR_970)) < 0) {
				log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s] dCloseAllHttpTrans dRet[%d]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, 
					PrintRtx(pTCPINFO->ucRtx), dRet);
			}	
#endif


#if 0
			pMSGINFO->uiNextSeq = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize;
			pMSGINFO->uiLastSeq = pTCPINFO->uiSeqNum;
			return -33;
#endif
		}

		pMSGINFO->uiNextSeq = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize;
		pMSGINFO->uiLastSeq = pTCPINFO->uiSeqNum;

		isHDR = (isNotREQ) ? dCheckResHeader(pHTTPTSESS, pTCPDATA, pTCPINFO->uiDataSize) : dCheckReqHeader(pHTTPTSESS, pTCPDATA, pTCPINFO->uiDataSize);

		if((!isHDR) && (pMSGINFO->ucStatus == TSESS_STATUS_HDRWAIT)) {
			/* Drop */
			log_print(LOGN_WARN, "[%s][%s.%d] NO START NO HEADER HDR[%s] TSESS STATUS[%s] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s]", 
					__FILE__, __FUNCTION__, __LINE__,
					PrintIsHDR(isHDR), PrintTSESSStatus(pMSGINFO->ucStatus),
					util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
					util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
if(!isNotREQ) {
log_print(LOGN_CRI, "NO START NO HEADER SIP[%s:%u]DIP[%s:%u]RTX[%s]",
util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
if(isalnum(pTCPDATA[0])) log_print(LOGN_CRI, "NO PARSING[%.*s]", pTCPINFO->uiDataSize, pTCPDATA);
}
			return -51;
		} else if((isHDR) && (pMSGINFO->ucStatus != TSESS_STATUS_HDRWAIT)) {
			log_print(LOGN_CRI, 
					"[%s][%s.%d] ABNORMAL MAKE TRANS HDR[%s] TSESS STATUS[%s] LENTYPE[%s] LEN[%d] HDRLEN[%d] BODYLEN[%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s]",
					__FILE__, __FUNCTION__, __LINE__,
					PrintIsHDR(isHDR), PrintTSESSStatus(pMSGINFO->ucStatus),
					PrintLenType(pMSGINFO->ucLenType), pMSGINFO->uiLen, pMSGINFO->uiHdrLen, pMSGINFO->uiBodyLen,
					util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
					util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
//log_print(LOGN_CRI, "* DATA=\n%.*s", 100, pTCPDATA);
			failCode = (isNotREQ) ? HTTP_UERR_941 : HTTP_UERR_911;
			if((dRet = dProcStatus(pMEMSINFO, pHTTPHASH, pHTTPTSESSKEY, pHTTPTSESS, isNotREQ, failCode)) < 0)
			{
				log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s] dProcStatus dRet[%d]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, PrintRtx(pTCPINFO->ucRtx), dRet);
			}
			pMSGINFO->ucStatus = TSESS_STATUS_HDRWAIT;
		}

		log_print(LOGN_INFO, "TCP DATA BUFF INFO STIME[%u]MTIME[%u]ISBUF[%s]TSESS_STS[%s]ESTS[%s]RTX[%s]", 
				pMSGINFO->uiStartTime, pMSGINFO->uiStartMTime, PrintBuffering(pMSGINFO->ucIsBuffering), 
				PrintTSESSStatus(pMSGINFO->ucStatus), PrintEndStatus(pMSGINFO->ucEndStatus), 
				PrintRtx(pTCPINFO->ucRtx));

		while(dLen < pTCPINFO->uiDataSize) 
		{
			/* 시간 초기화 및 UPDATE를 위함 */
			switch(pMSGINFO->ucStatus)
			{
			case TSESS_STATUS_HDRWAIT:
				pMSGINFO->uiStartTime = pTCPINFO->uiCapTime;
				pMSGINFO->uiStartMTime = pTCPINFO->uiCapMTime;
			default:
				pMSGINFO->uiLastUpdateTime = pTCPINFO->uiCapTime;
				pMSGINFO->uiLastUpdateMTime = pTCPINFO->uiCapMTime;
				pMSGINFO->uiAckTime = pTCPINFO->uiAckTime;
				pMSGINFO->uiAckMTime = pTCPINFO->uiAckMTime;
				break;
			}

			pTMPINPUT = pTCPDATA + dLen;

			switch(pMSGINFO->ucStatus)
			{
			case TSESS_STATUS_HDRWAIT:
			case TSESS_STATUS_HDRDOING:
				dRet = dGetData(pMEMSINFO, pMSGINFO, pTMPINPUT, pTCPINFO->uiDataSize - dLen, &dLen);
				break;
			case TSESS_STATUS_BODYWAIT:
			case TSESS_STATUS_BODYDOING:
				switch(pMSGINFO->ucLenType)
				{
				case LEN_TYPE_CONTENTLENGTH:
				case LEN_TYPE_PACKETCOUNTER:
					dRet = dGetLengthData(pMEMSINFO, pMSGINFO, pTMPINPUT, pTCPINFO->uiDataSize - dLen, &dLen);
					break;
				case LEN_TYPE_CHUNKED:
					pMSGINFO->dChunked = 1;
					dRet = dGetChunkedData(pMEMSINFO, pMSGINFO, pTMPINPUT, pTCPINFO->uiDataSize - dLen, &dLen);
					break;
				case LEN_TYPE_MULTIPART:
					dRet = dGetMultiData(pMEMSINFO, pMSGINFO, pTMPINPUT, pTCPINFO->uiDataSize - dLen, &dLen);
					break;
				default:
					log_print(LOGN_CRI, "F=%s:%s.%d STRANGE CONTENT-TYPE=%d:%s SIP=%s:%u DIP=%s:%u RTX=%s",
						__FILE__, __FUNCTION__, __LINE__,
						pMSGINFO->ucLenType, PrintLenType(pMSGINFO->ucLenType),
						util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
						util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
					dRet = -1;	
					break;
				}
				break;
			default:
				log_print(LOGN_CRI, "[%s][%s.%d] STRANGE TSESS STATUS [%d][%s] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s]",
					__FILE__, __FUNCTION__, __LINE__, 
					pMSGINFO->ucStatus, PrintTSESSStatus(pMSGINFO->ucStatus),
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
				dRet = -1;
				break;
			}

			if(dRet == END_STATE_0D0A0D0A) {
				/* 0D0A0D0A를 찾은 경우 */
				log_print(LOGN_DEBUG, "TCP DATA FIND 0D0A0D0A STATUS[%s] LEN[%d] BUFFERING[%s] OLEN[%u] RLEN[%d]", 
					PrintTSESSStatus(pMSGINFO->ucStatus), dLen, PrintBuffering(pMSGINFO->ucIsBuffering),
					pTCPINFO->uiDataSize, dLen);
				switch(pMSGINFO->ucStatus)
				{
				case TSESS_STATUS_HDRWAIT:
				case TSESS_STATUS_HDRDOING:

if(pMSGINFO->uiLen > DEF_MEMNODEBODY_SIZE)
log_print(LOGN_CRI, "[%s][%s.%d] ==== MAX HEADER SIZE MAX[%d] CUR[%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s]", 
__FILE__, __FUNCTION__, __LINE__,
DEF_MEMNODEBODY_SIZE, pMSGINFO->uiLen,
util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, PrintRtx(pTCPINFO->ucRtx));

					pHEAD = nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR);
					pMSGINFO->ucLenType = (isNotREQ) ? 
							dGetResHeaderInfo(pHTTPTSESS, pHEAD, (S32*)&pMSGINFO->uiLen, &pMSGINFO->ucIsBuffering, &pMSGINFO->uiMaxLen, &pMSGINFO->MULTIKEY.len, pMSGINFO->MULTIKEY.key, &pMSGINFO->dZip) : 
							dGetReqHeaderInfo(pHTTPTSESS, pHEAD, (S32*)&pMSGINFO->uiLen, &pMSGINFO->ucIsBuffering, &pMSGINFO->uiMaxLen, &pMSGINFO->MULTIKEY.len, pMSGINFO->MULTIKEY.key, &pMSGINFO->dZip);
					if(pMSGINFO->ucLenType) {
						log_print(LOGN_DEBUG, "HAVE BODY LENTYPE[%s]", PrintLenType(pMSGINFO->ucLenType));
						pMSGINFO->ucStatus = TSESS_STATUS_BODYWAIT;
						pMSGINFO->ucEndStatus = END_STATE_EMPTY;
					} else {
						log_print(LOGN_DEBUG, "NO HAVE BODY");
						if((dRet = dProcStatus(pMEMSINFO, pHTTPHASH, pHTTPTSESSKEY, pHTTPTSESS, isNotREQ, HTTP_UERR_EMPTY)) < 0) {
							log_print(LOGN_CRI, 
								"[%s][%s.%d] TCP DATA FIND 0D0A0D0A SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s] dProcStatus dRet[%d]", 
								__FILE__, __FUNCTION__, __LINE__,
								util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
								util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, PrintRtx(pTCPINFO->ucRtx), dRet);
							return -41;
						}
						pMSGINFO->ucStatus = TSESS_STATUS_HDRWAIT;
						pMSGINFO->ucEndStatus = END_STATE_EMPTY;
						dLen = pTCPINFO->uiDataSize;
					}
					break;
				case TSESS_STATUS_BODYWAIT:
				case TSESS_STATUS_BODYDOING:

					log_print(LOGN_DEBUG, "END BODY");
					if((dRet = dProcStatus(pMEMSINFO, pHTTPHASH, pHTTPTSESSKEY, pHTTPTSESS, isNotREQ, HTTP_UERR_EMPTY)) < 0) {
						log_print(LOGN_CRI, 
							"[%s][%s.%d] FIND 0D0A0D0A SIP[%s:%u]DIP[%s:%u]RTX[%s] dProcStatus dRet[%d]", 
							__FILE__, __FUNCTION__, __LINE__,
							util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
							util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, PrintRtx(pTCPINFO->ucRtx), dRet);
						return -41;
					}
					pMSGINFO->ucStatus = TSESS_STATUS_HDRWAIT;
					pMSGINFO->ucEndStatus = END_STATE_EMPTY;
					dLen = pTCPINFO->uiDataSize;
					break;
				default:
					log_print(LOGN_CRI, "[%s][%s.%d] INVALID STATUS[%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s]", 
						__FILE__, __FUNCTION__, __LINE__, 
						pMSGINFO->ucStatus,
						util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
						util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, PrintRtx(pTCPINFO->ucRtx));
					dLen = pTCPINFO->uiDataSize;
					break;
				}
			} else if(dRet == 0) {
				switch(pMSGINFO->ucStatus)
				{
				case TSESS_STATUS_HDRWAIT:
					pMSGINFO->ucStatus = TSESS_STATUS_HDRDOING;
					break;
				case TSESS_STATUS_BODYWAIT:
					pMSGINFO->ucStatus = TSESS_STATUS_BODYDOING;
					break;
				default:
					break;
				}
			} else {
				/* 에러 발생 */
				log_print(LOGN_CRI, 
					"[%s][%s.%d] TCP DATA STRANGE END STATUS INIT BUFFER SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s] dRet[%d]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, 
					PrintRtx(pTCPINFO->ucRtx), dRet);
				InitBuffer(pMEMSINFO, pMSGINFO, isNotREQ);
				dLen = pTCPINFO->uiDataSize;
			}	
		}

		nifo_node_delete(pMEMSINFO, pNode);
		break;

	default:
		/**
		 *  버그 발생
		 **/
		log_print(LOGN_CRI, 
			"[%s][%s.%d] STRANGE TCP FLAG SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s] FLAG[%d]", 
			__FILE__, __FUNCTION__, __LINE__,
			util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, 
			PrintRtx(pTCPINFO->ucRtx), pTCPINFO->cTcpFlag);
		return -100;
	}

	return 0;
}

/** dCloseHttpTrnas function.
 *
 *  TCP로 부터 받은 데이터를 Control하고 TCP Session, HTTP Transaction 관리
 *
 *  @param	*pMEMSINFO : New Interface 관리를 위한 구조체
 *  @param	*pHTTPHASH : Http Transaction 관리를 위한 HASH
 *  @param	*pHTTPTRANSKEY : HTTP Transaction HASH KEY 값
 *  @param	*pHTTPTRANS : HTTP Transaction HASH Data
 *
 *  @return			S32
 *  @see			http_func.c http_msgq.c http_main.c http_init.c http_util.c http_api.h
 *
 *  @exception		nothing
 *  @note			nothing
 **/
S32 dCloseHttpTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHTTPHASH, HTTP_TSESS *pHTTPTSESS, HTTP_TRANS_KEY *pHTTPTRANSKEY, HTTP_TRANS *pHTTPTRANS)
{
	S32				i, dRet;
	U8 				*pHead = NULL;
	U8				*pNode;
	LOG_IHTTP_TRANS	*pLOGHTTP;
	struct timeval	stNowTime;
	OFFSET			offset;

	curSessCnt--;

	gettimeofday(&stNowTime, NULL);

	pLOGHTTP = (LOG_IHTTP_TRANS *)nifo_ptr(pMEMSINFO, pHTTPTRANS->offset_LOG);

	STG_DiffTIME64(pLOGHTTP->uiLastPktTime, pLOGHTTP->uiLastPktMTime, pLOGHTTP->uiReqStartTime, pLOGHTTP->uiReqStartMTime, &pLOGHTTP->llTransGapTime);

	pLOGHTTP->uiReqEndGapTime = GetGap32Time(pLOGHTTP->uiReqEndTime, pLOGHTTP->uiReqEndMTime, pLOGHTTP->uiReqStartTime, pLOGHTTP->uiReqStartMTime);

	pLOGHTTP->uiReqAckGapTime = GetGap32Time(pLOGHTTP->uiReqAckTime, pLOGHTTP->uiReqAckMTime, pLOGHTTP->uiReqStartTime, pLOGHTTP->uiReqStartMTime);

	pLOGHTTP->uiResStartGapTime = GetGap32Time(pLOGHTTP->uiResStartTime, pLOGHTTP->uiResStartMTime, pLOGHTTP->uiReqStartTime, pLOGHTTP->uiReqStartMTime);

	pLOGHTTP->uiResEndGapTime = GetGap32Time(pLOGHTTP->uiResEndTime, pLOGHTTP->uiResEndMTime, pLOGHTTP->uiReqStartTime, pLOGHTTP->uiReqStartMTime);

	pLOGHTTP->uiMNAckGapTime = GetGap32Time(pLOGHTTP->uiMNAckTime, pLOGHTTP->uiMNAckMTime, pLOGHTTP->uiReqStartTime, pLOGHTTP->uiReqStartMTime);

	if(pHTTPTRANS->usL7FailCode == HTTP_UERR_EMPTY)
		pLOGHTTP->usL7FailCode = GetFailCode(pLOGHTTP, pHTTPTRANS->ucStatus);
	else
		pLOGHTTP->usL7FailCode = pHTTPTRANS->usL7FailCode;

	pLOGHTTP->usUserErrorCode = pLOGHTTP->usL7FailCode;

#if 0
	switch(pLOGHTTP->usL7FailCode)
	{
	case HTTP_UERR_EMPTY:
	case HTTP_UERR_900:
	case HTTP_UERR_910:
	case HTTP_UERR_911:
	case HTTP_UERR_920:
	case HTTP_UERR_930:
	case HTTP_UERR_940:
	case HTTP_UERR_941:
	case HTTP_UERR_950:
	case HTTP_UERR_960:
		pLOGHTTP->usUserErrorCode = pHTTPTRANS->usL7FailCode;
		break;
	default:
		break;
	}
#endif
	UpCount(pLOGHTTP, pHTTPTSESS);
		
	if((pLOGHTTP->usL4FailCode == 0) && (pHTTPTSESS->usL4FailCode > 0)) {
		pLOGHTTP->usL4FailCode = pHTTPTSESS->usL4FailCode;
	}
	pLOGHTTP->ucTcpClientStatus = pHTTPTSESS->ucTcpClientStatus;
	pLOGHTTP->ucTcpServerStatus = pHTTPTSESS->ucTcpServerStatus;
	pLOGHTTP->uiOpEndTime = stNowTime.tv_sec;
	pLOGHTTP->uiOpEndMTime = stNowTime.tv_usec;

	offset = nifo_get_offset_node(pMEMSINFO, (U8 *)pLOGHTTP);
	pHead = nifo_ptr(pMEMSINFO, offset);
	
	for(i = 0; i < HTTP_MSGTYPE_CNT; i++) {
		pNode = nifo_ptr(pMEMSINFO, pHTTPTRANS->offset_Node[i]);

		if(pNode != NULL) {
//			nifo_node_link_nont_prev(pMEMSINFO, pHead, pNode);
			nifo_node_delete(pMEMSINFO, pNode);
			pHTTPTRANS->offset_Node[i] = 0;
		}
	}		

	if((dRet = dSend_HTTP_Data(pMEMSINFO, pHTTPTRANS->dSndMsgQ, pHead)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dSendSignal dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
	}

//	LOG_IHTTP_TRANS_Prt("PRINT LOG_IHTTP_TRANS", pLOGHTTP);

	hasho_del(pHTTPHASH, (U8 *)pHTTPTRANSKEY);

	pHTTPTSESS->usCurTransCnt--;
	if(pHTTPTSESS->usCurTransCnt == 0) {
		pHTTPTSESS->usFirstTransID = 0;
		pHTTPTSESS->usLastTransID = 0;
	} else {
		pHTTPTSESS->usFirstTransID++;
	}

	return 0;
}

/** dCloseAllHttpTrans function.
 *
 *  해당 TCP Session의 HTTP Transaction를 전송 및 정리 
 *
 *  @param	*pMEMSINFO : New Interface 관리를 위한 구조체
 *  @param	*pHTTPHASH : Http Transaction 관리를 위한 HASH
 *  @param	*pHTTPTSESSKEY : Tcp Session HASH KEY 값
 *  @param	*pHTTPTSESS : Tcp Session HASH DATA 값
 *  @param	usFailCode : HTTP Fail Reason
 *
 *  @return			S32		SUCC: 0, FAIL: < 0
 *  @see			http_func.c http_msgq.c http_main.c http_init.c http_util.c http_api.h
 *
 *  @exception		nothing
 *  @note			nothing
 **/
S32 dCloseAllHttpTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHTTPHASH, HTTP_TSESS_KEY *pHTTPTSESSKEY, HTTP_TSESS *pHTTPTSESS, U16 usFailCode)
{
	S32				i, dRet;
	S32				lasttid;

	HTTP_TRANS_KEY	HTTPTRANSKEY;
	HTTP_TRANS_KEY	*pHTTPTRANSKEY;
	HTTP_TRANS		*pHTTPTRANS;
	st_MSG_INFO		*pMSGINFO;
	stHASHONODE		*pHASHNODE;

	pHTTPTRANSKEY = &HTTPTRANSKEY;

	for(i = 0; i < MSG_INFO_CNT; i++) {
		pMSGINFO = &pHTTPTSESS->MSGINFO[i];

		log_print(LOGN_INFO, "CLOSE ALL TSESS_STS[%s]ESTS[%s]BUFF[%s]LEN[%u]HDRLEN[%u]BODYLEN[%u]",
			PrintTSESSStatus(pMSGINFO->ucStatus), PrintEndStatus(pMSGINFO->ucEndStatus), 
			PrintBuffering(pMSGINFO->ucIsBuffering), pMSGINFO->uiLen, pMSGINFO->uiHdrLen, pMSGINFO->uiBodyLen);

		switch(pMSGINFO->ucStatus)
		{
		case TSESS_STATUS_HDRDOING:
		case TSESS_STATUS_BODYWAIT:
		case TSESS_STATUS_BODYDOING:
			if((dRet = dProcStatus(pMEMSINFO, pHTTPHASH, pHTTPTSESSKEY, pHTTPTSESS, i, usFailCode)) < 0) 
			{
				log_print(LOGN_CRI, "[%s][%s.%d] dProcStatus[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			}
			break;
		default:
			break;
		}

		pMSGINFO->ucStatus = TSESS_STATUS_HDRWAIT;
	}

	log_print(LOGN_INFO, "CLOSE ALL TRANS FIRST_TID[%d] LAST_TID[%d] CUR_TID[%d]", 
		pHTTPTSESS->usFirstTransID, pHTTPTSESS->usLastTransID, pHTTPTSESS->usCurTransCnt);

	lasttid = pHTTPTSESS->usFirstTransID + pHTTPTSESS->usCurTransCnt;

	for(i = pHTTPTSESS->usFirstTransID; i < lasttid; i++) {
		MakeHttpHashKey(pHTTPTSESSKEY, i, pHTTPTRANSKEY);

		if((pHASHNODE = hasho_find(pHTTPHASH, (U8 *)pHTTPTRANSKEY)) == NULL) {
			log_print(LOGN_CRI, "[%s][%s.%d] hasho_find NULL", __FILE__, __FUNCTION__, __LINE__);
		}

		pHTTPTRANS = (HTTP_TRANS *)nifo_ptr(pHTTPHASH, pHASHNODE->offset_Data);

		pHTTPTRANS->usL7FailCode = usFailCode;

		if((dRet = dCloseHttpTrans(pMEMSINFO, pHTTPHASH, pHTTPTSESS, pHTTPTRANSKEY, pHTTPTRANS)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] dCloseHttpTrans[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		}
		hasho_del(pHTTPHASH, (U8 *)pHTTPTRANSKEY);

		pHTTPTSESS->usTotTransCnt++;
	}

	pHTTPTSESS->usFirstTransID = 0;
	pHTTPTSESS->usLastTransID = 0;
	pHTTPTSESS->usCurTransCnt = 0;

	return 0;
}

/** pCreateTcpSess function.
 *
 *  TCP HASH를 생성하고 TCP Session 정보를 초기화
 *		데이터 버퍼링을 위한 nifo Node 할당
 *
 *  @param	*pMEMSINFO : New Interface 관리를 위한 구조체
 *  @param	*pTCPHASH : TCP Session 관리를 위한 HASH
 *  @param	*pHTTPHASH : Http Transaction 관리를 위한 HASH
 *  @param	*pHTTPTSESSKEY : TCP Session HASH KEY 값
 *  @param	*pTCPINFO : TCP로 부터 받은 데이터의 정보를 담고 있는 구조체
 *
 *  @return			S32		SUCC: 0, FAIL: < 0
 *  @see			http_func.c http_msgq.c http_main.c http_init.c http_util.c http_api.h
 *
 *  @exception		nothing
 *  @note			nothing
 **/
HTTP_TSESS *pCreateTcpSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, stHASHOINFO *pHTTPHASH, HTTP_TSESS_KEY *pHTTPTSESSKEY, TCP_INFO *pTCPINFO)
{
	S32		dRet, i;
	U8		*pNode;
	HTTP_TSESS	HTTPTSESS;
	HTTP_TSESS	*pHTTPTSESS = &HTTPTSESS;

	st_MSG_INFO	*pMSGINFO;

	stHASHONODE	*pHASHNODE;

	/* Make Tcp Hash Data */
	pHTTPTSESS->uiSrvIP = pTCPINFO->uiSrvIP;
	pHTTPTSESS->usSrvPort = pTCPINFO->usSrvPort;
	pHTTPTSESS->uiTcpSynTime = pTCPINFO->uiCapTime;
	pHTTPTSESS->uiTcpSynMTime = pTCPINFO->uiCapMTime;
	pHTTPTSESS->usNextTransID = 1;
	pHTTPTSESS->usFirstTransID = 0;
	pHTTPTSESS->usLastTransID = 0;
	pHTTPTSESS->usCurTransCnt = 0;
	pHTTPTSESS->usTotTransCnt = 0;
	pHTTPTSESS->ucSynRtx = pTCPINFO->ucRtx;

	pHTTPTSESS->usL4FailCode = pTCPINFO->usL4FailCode;
	pHTTPTSESS->usAppCode = pTCPINFO->usAppCode;
//	pHTTPTSESS->usL4Code = pTCPINFO->usL4Code;
//	pHTTPTSESS->usL7Code = pTCPINFO->usL7Code;
	pHTTPTSESS->usL4Code = L4_INET_HTTP;
	pHTTPTSESS->usL7Code = APP_UNKNOWN;

	pHTTPTSESS->ucSSL = SSL_OFF;
	pHTTPTSESS->ucMethod = 0;
	pHTTPTSESS->usResCode = 0;
	pHTTPTSESS->hostNameLen = 0;
	pHTTPTSESS->szHostName[0] = 0x00;
	pHTTPTSESS->urlType = 0;
	pHTTPTSESS->usUrlSize = 0;
	pHTTPTSESS->szUrl[0] = 0x00;
	pHTTPTSESS->usContentsType = 0;
	pHTTPTSESS->usContentsTypeSize = 0;
	pHTTPTSESS->szContentsType[0] = 0;

	for(i = 0; i < MSG_INFO_CNT; i++) {
		pMSGINFO = &pHTTPTSESS->MSGINFO[i];
	
		pMSGINFO->uiLastSeq = 0;
		pMSGINFO->uiNextSeq = 0;
		pMSGINFO->uiStartTime = 0;
		pMSGINFO->uiStartMTime = 0;
		pMSGINFO->uiLastUpdateTime = 0;
		pMSGINFO->uiLastUpdateMTime = 0;
		pMSGINFO->uiAckTime = 0;
		pMSGINFO->uiAckMTime = 0;
		pMSGINFO->uiHdrLen = 0;
		pMSGINFO->uiBodyLen = 0;
		pMSGINFO->uiLimitLen = 0;
		pMSGINFO->ucLenType = 0;
		pMSGINFO->uiLen = 0;
		pMSGINFO->uiMaxLen = 0;
		pMSGINFO->ucIsBuffering = TSESS_BUFFERING_OFF;
		pMSGINFO->ucStatus = TSESS_STATUS_HDRWAIT;
		pMSGINFO->ucEndStatus = END_STATE_EMPTY;
		pMSGINFO->MULTIKEY.len = 0;
		pMSGINFO->MULTIKEY.state = 0;
		pMSGINFO->dZip = 0;
		pMSGINFO->dChunked = 0;

		if((pNode = pAllocDataNode(pMEMSINFO, ((i == 0) ? HTTP_REQ_HDR_NUM : HTTP_RES_HDR_NUM))) == NULL) {
			log_print(LOGN_CRI, "[%s][%s.%d] pAllocDataNode NULL", __FILE__, __FUNCTION__, __LINE__);
			return NULL;
		}

		pMSGINFO->offset_HDR = nifo_offset(pMEMSINFO, pNode);
		pMSGINFO->offset_CurHDR = pMSGINFO->offset_HDR;

		if((pNode = pAllocDataNode(pMEMSINFO, ((i == 0) ? HTTP_REQ_BODY_NUM : HTTP_RES_BODY_NUM))) == NULL) {
			log_print(LOGN_CRI, "[%s][%s.%d] pAllocDataNode NULL", __FILE__, __FUNCTION__, __LINE__);
			nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR));
			return NULL;
		}

		pMSGINFO->offset_BODY = nifo_offset(pMEMSINFO, pNode);
		pMSGINFO->offset_CurBODY = pMSGINFO->offset_BODY;
	}

	pHTTPTSESS->MSGINFO[0].uiNextSeq = pTCPINFO->uiSeqNum;
	pHTTPTSESS->MSGINFO[1].uiNextSeq = pTCPINFO->uiAckNum;

	pHTTPTSESS->ucTcpClientStatus = pTCPINFO->ucTcpClientStatus;
	pHTTPTSESS->ucTcpServerStatus = pTCPINFO->ucTcpServerStatus;

	pHTTPTSESS->uiIPDataUpPktCnt = pTCPINFO->uiIPDataUpPktCnt;
	pHTTPTSESS->uiIPDataDnPktCnt = pTCPINFO->uiIPDataDnPktCnt;
	pHTTPTSESS->uiIPTotUpPktCnt = pTCPINFO->uiIPTotUpPktCnt;
	pHTTPTSESS->uiIPTotDnPktCnt = pTCPINFO->uiIPTotDnPktCnt;
	pHTTPTSESS->uiIPDataUpRetransCnt = pTCPINFO->uiIPDataUpRetransCnt;
	pHTTPTSESS->uiIPDataDnRetransCnt = pTCPINFO->uiIPDataDnRetransCnt;
	pHTTPTSESS->uiIPTotUpRetransCnt = pTCPINFO->uiIPTotUpRetransCnt;
	pHTTPTSESS->uiIPTotDnRetransCnt = pTCPINFO->uiIPTotDnRetransCnt;
	pHTTPTSESS->uiIPDataUpPktSize = pTCPINFO->uiIPDataUpPktSize;
	pHTTPTSESS->uiIPDataDnPktSize = pTCPINFO->uiIPDataDnPktSize;
	pHTTPTSESS->uiIPTotUpPktSize = pTCPINFO->uiIPTotUpPktSize;
	pHTTPTSESS->uiIPTotDnPktSize = pTCPINFO->uiIPTotDnPktSize;
	pHTTPTSESS->uiIPDataUpRetransSize = pTCPINFO->uiIPDataUpRetransSize;
	pHTTPTSESS->uiIPDataDnRetransSize = pTCPINFO->uiIPDataDnRetransSize;
	pHTTPTSESS->uiIPTotUpRetransSize = pTCPINFO->uiIPTotUpRetransSize;
	pHTTPTSESS->uiIPTotDnRetransSize = pTCPINFO->uiIPTotDnRetransSize;

	/* Tcp Session 생성 */
	if((pHASHNODE = hasho_add(pTCPHASH, (U8 *)pHTTPTSESSKEY, (U8 *)pHTTPTSESS)) == NULL)
	{
		/* 동일한 TCP Session 존재 */
		/* 기존 세션 정리 */
        if((pHASHNODE = hasho_find(pTCPHASH, (U8 *)pHTTPTSESSKEY)) == NULL)
        {
            log_print(LOGN_CRI, "SAME BUT NOT FINE ????? EXIT EXIT");
            exit(0);
        }

        pHTTPTSESS = (HTTP_TSESS *)nifo_ptr(pTCPHASH, pHASHNODE->offset_Data);

		if((dRet = dCloseTcpSess(pMEMSINFO, pTCPHASH, pHTTPHASH, pHTTPTSESSKEY, pHTTPTSESS)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] dCloseTcpSess dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			for(i = 0; i < MSG_INFO_CNT; i++) {
				pMSGINFO = &pHTTPTSESS->MSGINFO[i];
				nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR));
				nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY));
			}
			return NULL;
		}

		pHTTPTSESS = &HTTPTSESS;

		if((pHASHNODE = hasho_add(pTCPHASH, (U8 *)pHTTPTSESSKEY, (U8 *)pHTTPTSESS)) == NULL) {
			log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL", __FILE__, __FUNCTION__, __LINE__);
			for(i = 0; i < MSG_INFO_CNT; i++) {
				pMSGINFO = &pHTTPTSESS->MSGINFO[i];
				nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR));
				nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY));
			}
			return NULL;
		}
	}

	pHTTPTSESS = (HTTP_TSESS *)nifo_ptr(pTCPHASH, pHASHNODE->offset_Data);

	return pHTTPTSESS;
}

/** dCloseTcpSess function.
 *
 *  TCP HASH를 삭제하고 TCP Session 정리
 *		모든 HTTP Transaction 데이터를 전송 및 정리
 *
 *  @param	*pMEMSINFO : New Interface 관리를 위한 구조체
 *  @param	*pTCPHASH : TCP Session 관리를 위한 HASH
 *  @param	*pHTTPHASH : Http Transaction 관리를 위한 HASH
 *  @param	*pHTTPTSESSKEY : Tcp Session HASH KEY 값
 *  @param	*pHTTPTRANSKEY : HTTP Transaction HASH KEY 값
 *
 *  @return			S32		SUCC: 0, FAIL: < 0
 *  @see			http_func.c http_msgq.c http_main.c http_init.c http_util.c http_api.h
 *
 *  @exception		nothing
 *  @note			nothing
 **/
S32 dCloseTcpSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, stHASHOINFO *pHTTPHASH, HTTP_TSESS_KEY *pHTTPTSESSKEY, HTTP_TSESS *pHTTPTSESS)
{
	S32		dRet;
	S32		i;
	st_MSG_INFO	*pMSGINFO;

	if((dRet = dCloseAllHttpTrans(pMEMSINFO, pHTTPHASH, pHTTPTSESSKEY, pHTTPTSESS, HTTP_UERR_EMPTY)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dCloseAllHttpTrans dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
	}

	for(i = 0; i < MSG_INFO_CNT; i++) {
		pMSGINFO = &pHTTPTSESS->MSGINFO[i];

		if(pMSGINFO->offset_HDR != 0) {
			nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR));
			pMSGINFO->offset_HDR = 0;
			pMSGINFO->offset_CurHDR = 0;
		}
		if(pMSGINFO->offset_BODY != 0) {
			nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY));
			pMSGINFO->offset_BODY = 0;
			pMSGINFO->offset_CurBODY = 0;
		}
	}

	hasho_del(pTCPHASH, (U8 *)pHTTPTSESSKEY);

	return 0;
}


S32 dProcStatus(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHTTPHASH, HTTP_TSESS_KEY *pHTTPTSESSKEY, HTTP_TSESS *pHTTPTSESS, S32 isNotREQ, U16 failCode)
{
	S32					dRet, i;
	U8					*pNode;
	U8					*pLOGNODE;
	U16					logStatus;
	HTTP_TRANS_KEY		HTTPTRANSKEY;
	HTTP_TRANS_KEY		*pHTTPTRANSKEY;
	HTTP_TRANS			HTTPTRANS;
	HTTP_TRANS			*pHTTPTRANS;
	LOG_IHTTP_TRANS		*pLOGHTTP;
	struct timeval		stNowTime;
	U8					szSIP[INET_ADDRSTRLEN];
	U8					szDIP[INET_ADDRSTRLEN];

	stHASHONODE			*pHASHNODE;
	st_MSG_INFO			*pMSGINFO;


	pHTTPTRANSKEY = &HTTPTRANSKEY;
	pHTTPTRANS = &HTTPTRANS;

	log_print(LOGN_DEBUG, "PROC STATUS NEXTID[%d]FID[%d]LID[%d]CURCNT[%d]TOTCNT[%d]", 
			pHTTPTSESS->usNextTransID, pHTTPTSESS->usFirstTransID, 
			pHTTPTSESS->usLastTransID, pHTTPTSESS->usCurTransCnt, pHTTPTSESS->usTotTransCnt);

	pMSGINFO = &pHTTPTSESS->MSGINFO[isNotREQ];

	logStatus = GetLogHttpStatus(pMSGINFO, isNotREQ);

	if(!isNotREQ) {
		/* REQ */
		/* 새로운 HTTP Trans 생성 */

		curSessCnt++;
		sessCnt++;

		log_print(LOGN_DEBUG, 
			"PROC STATUS REQ SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
			util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
			util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort);

		/* Make Http Hash Key */
		MakeHttpHashKey(pHTTPTSESSKEY, pHTTPTSESS->usNextTransID, pHTTPTRANSKEY);

		gettimeofday(&stNowTime, NULL);
		pHTTPTRANS->usL7FailCode = failCode;

		pHTTPTRANS->usL4FailCode = pHTTPTSESS->usL4FailCode;
		pHTTPTRANS->usL4Code = pHTTPTSESS->usL4Code;
		pHTTPTRANS->usL7Code = pHTTPTSESS->usL7Code;

		pHTTPTRANS->offset_Node[HTTP_MSGTYPE_REQHDR] = pMSGINFO->offset_HDR;
		pHTTPTRANS->offset_Node[HTTP_MSGTYPE_REQBODY] = pMSGINFO->offset_BODY;

		pMSGINFO->offset_HDR = 0;
		pMSGINFO->offset_CurHDR = 0;
		pMSGINFO->offset_BODY = 0;
		pMSGINFO->offset_CurBODY = 0;	
	
		if((pNode = pAllocDataNode(pMEMSINFO, HTTP_REQ_HDR_NUM)) == NULL) {
			log_print(LOGN_CRI, "PROC STATUS REQ HEADER ALLOC NODE NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
				util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort);
			return -43;
		}

		pMSGINFO->offset_HDR = nifo_offset(pMEMSINFO, pNode);
		pMSGINFO->offset_CurHDR = pMSGINFO->offset_HDR;

		if((pNode = pAllocDataNode(pMEMSINFO, HTTP_REQ_BODY_NUM)) == NULL) {
			log_print(LOGN_CRI, "PROC STATUS REQ BODY ALLOC NODE NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
				util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort);
			return -53;
		}

		pMSGINFO->offset_BODY = nifo_offset(pMEMSINFO, pNode);
		pMSGINFO->offset_CurBODY = pMSGINFO->offset_BODY;


		for(i = HTTP_MSGTYPE_RESHDR; i < HTTP_MSGTYPE_CNT; i++) {
			pHTTPTRANS->offset_Node[i] = 0;
		}

		if((pLOGNODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
			log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
			return -111;
		}

		if((pLOGHTTP = (LOG_IHTTP_TRANS *)nifo_tlv_alloc(pMEMSINFO, pLOGNODE, LOG_IHTTP_TRANS_DEF_NUM, LOG_IHTTP_TRANS_SIZE, DEF_MEMSET_ON)) == NULL) {
			log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc NULL", __FUNCTION__, __LINE__);
			nifo_node_delete(pMEMSINFO, pLOGNODE);
			return -112;
		}

		pHTTPTRANS->offset_LOG = nifo_offset(pMEMSINFO, pLOGHTTP);

		pLOGHTTP->uiCallTime = pHTTPTSESS->uiTcpSynTime;
		pLOGHTTP->uiCallMTime = pHTTPTSESS->uiTcpSynMTime;
		pLOGHTTP->uiClientIP = pHTTPTRANSKEY->uiCliIP;
//		pLOGHTTP->uiNASName = pHTTPTRANSKEY->uiCliIP;
		pLOGHTTP->uiNASName = 0;

		pLOGHTTP->usClientPort = pHTTPTRANSKEY->usCliPort;
		pLOGHTTP->uiServerIP = pHTTPTSESS->uiSrvIP;
		pLOGHTTP->usServerPort = pHTTPTSESS->usSrvPort;
		pLOGHTTP->uiITcpSynTime = pHTTPTSESS->uiTcpSynTime;
		pLOGHTTP->uiITcpSynMTime = pHTTPTSESS->uiTcpSynMTime;
		pLOGHTTP->usTransID = pHTTPTRANSKEY->usHttpTransID;
		pLOGHTTP->usSvcL4Type = pHTTPTSESS->usL4Code;
		pLOGHTTP->usSvcL7Type = pHTTPTSESS->usL7Code;
		pLOGHTTP->ucSubSysNo = 1;
//		pLOGHTTP->usContentsType = pHTTPTSESS->usContentsType;
		pLOGHTTP->ucMethod = pHTTPTSESS->ucMethod;
		pLOGHTTP->uiReqStartTime = pMSGINFO->uiStartTime;
		pLOGHTTP->uiReqStartMTime = pMSGINFO->uiStartMTime;
		pLOGHTTP->uiReqEndTime = pMSGINFO->uiLastUpdateTime;
		pLOGHTTP->uiReqEndMTime = pMSGINFO->uiLastUpdateMTime;
		pLOGHTTP->uiReqAckTime = pMSGINFO->uiAckTime;
		pLOGHTTP->uiReqAckMTime = pMSGINFO->uiAckMTime;
		if(pMSGINFO->uiAckMTime == 0) {
			pLOGHTTP->uiLastPktTime = pMSGINFO->uiLastUpdateTime;
			pLOGHTTP->uiLastPktMTime = pMSGINFO->uiLastUpdateMTime;
		} else {
			pLOGHTTP->uiLastPktTime = pMSGINFO->uiAckTime;
			pLOGHTTP->uiLastPktMTime = pMSGINFO->uiAckMTime;
		}
		pLOGHTTP->ucTcpClientStatus = pHTTPTSESS->ucTcpClientStatus;
		pLOGHTTP->ucTcpServerStatus = pHTTPTSESS->ucTcpServerStatus;
		pLOGHTTP->ucStatus = logStatus;
		pLOGHTTP->usUserErrorCode = pHTTPTRANS->usL7FailCode;
		if((pLOGHTTP->usL4FailCode == 0) && (pHTTPTRANS->usL4FailCode > 0)) {
			pLOGHTTP->usL4FailCode = pHTTPTRANS->usL4FailCode;
		}
		pLOGHTTP->usL7FailCode = pHTTPTRANS->usL7FailCode;

#if 0
		pLOGHTTP->usLOGURLSize = GetURL(pHTTPTSESS->szUrl, pHTTPTSESS->usUrlSize, pHTTPTSESS->urlType, pHTTPTSESS->szHostName, pHTTPTSESS->hostNameLen, pLOGHTTP->szLOGURL, MAX_LOGURL_LEN);

		if(pLOGHTTP->usLOGURLSize > MAX_URL_LEN) {
			pLOGHTTP->usURLSize = MAX_URL_LEN;
		} else {
			pLOGHTTP->usURLSize = pLOGHTTP->usLOGURLSize;
		}

		memcpy(pLOGHTTP->szURL, pLOGHTTP->szLOGURL, pLOGHTTP->usURLSize);
		pLOGHTTP->szURL[pLOGHTTP->usURLSize] = 0x00;
#endif

		pLOGHTTP->usLOGURLSize = GetURL(pHTTPTSESS->szUrl, pHTTPTSESS->usUrlSize, pHTTPTSESS->urlType, pHTTPTSESS->szHostName, pHTTPTSESS->hostNameLen, pLOGHTTP->szLOGURL, MAX_LOGURL_LEN);

		if(pLOGHTTP->usLOGURLSize > MAX_IURL_LEN) {
			pLOGHTTP->usIURLSize = MAX_IURL_LEN;
		} else {
			pLOGHTTP->usIURLSize = pLOGHTTP->usLOGURLSize;
		}

		memcpy(pLOGHTTP->szIURL, pLOGHTTP->szLOGURL, pLOGHTTP->usIURLSize);
		pLOGHTTP->szIURL[pLOGHTTP->usIURLSize] = 0x00;

		UpCount(pLOGHTTP, pHTTPTSESS);

		pLOGHTTP->uiTcpUpBodySize = pMSGINFO->uiHdrLen + pMSGINFO->uiBodyLen;
		pLOGHTTP->uiUpHeaderSize = pMSGINFO->uiHdrLen;
		pLOGHTTP->uiUpBodySize = pMSGINFO->uiBodyLen;
		pLOGHTTP->uiContentLength = pMSGINFO->uiLen;
		memcpy(pLOGHTTP->szHostName, pHTTPTSESS->szHostName, pHTTPTSESS->hostNameLen);
		pLOGHTTP->szHostName[pHTTPTSESS->hostNameLen] = 0x00;

		pLOGHTTP->dReqZip = pMSGINFO->dZip;
		pLOGHTTP->dReqMultiLen = pMSGINFO->MULTIKEY.len;
		memcpy(pLOGHTTP->szReqMulti, pMSGINFO->MULTIKEY.key, pLOGHTTP->dReqMultiLen);
		pLOGHTTP->szReqMulti[pLOGHTTP->dReqMultiLen] = 0x00;

		pLOGHTTP->uiOpStartTime = stNowTime.tv_sec;
		pLOGHTTP->uiOpStartMTime = stNowTime.tv_usec;

		/* Create Http Transaction */
		if((pHASHNODE = hasho_add(pHTTPHASH, (U8 *)pHTTPTRANSKEY, (U8 *)pHTTPTRANS)) == NULL)
		{
			/* 동일한 Http Transaction 존재 */
			log_print(LOGN_CRI, "[%s.%d] hasho_add EXIST NODE", __FUNCTION__, __LINE__);
			nifo_node_delete(pMEMSINFO, pLOGNODE);
			return -114;
		}

		pHTTPTRANS = (HTTP_TRANS *)nifo_ptr(pHTTPHASH, pHASHNODE->offset_Data);

		if(pHTTPTSESS->usFirstTransID == 0) {
			pHTTPTSESS->usFirstTransID = pHTTPTSESS->usNextTransID;
		}

		pHTTPTSESS->usLastTransID = pHTTPTSESS->usNextTransID;
		pHTTPTSESS->usNextTransID++;
		pHTTPTSESS->usCurTransCnt++;
		pHTTPTSESS->usTotTransCnt++;

		pHTTPTRANS->dSndMsgQ = dGetProcID(pHTTPTSESS->usAppCode, pLOGHTTP->uiClientIP);
		pLOGHTTP->usPlatformType = dGetPlatformType(pLOGHTTP->usSvcL4Type, pLOGHTTP->usSvcL7Type);
//		pLOGHTTP->usPlatformType = (pLOGHTTP->usSvcL4Type / 1000) * 1000;

//		pHTTPTRANS->usL7FailCode = failCode;
		pHTTPTRANS->ucStatus = GetHtttpTransStatus(pMSGINFO, isNotREQ);

	} else {
		/* RES */
	
		/*
		 * HTTP HASH에서 첫번째 NODE를 찾아서 비교
		 */
	
		/* Make Http Hash Key */
		MakeHttpHashKey(pHTTPTSESSKEY, pHTTPTSESS->usFirstTransID, pHTTPTRANSKEY);
	
		/* Find Http Hash */
		if((pHASHNODE = hasho_find(pHTTPHASH, (U8 *)pHTTPTRANSKEY)) == NULL)
		{
			/* HTTP Hash를 찾지 못함 */
			log_print(LOGN_CRI, 
				"PROC STATUS RES HTTP TRANS NODE NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
				util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort);
			InitBuffer(pMEMSINFO, pMSGINFO, isNotREQ);
			return -21;
		}
		
		pHTTPTRANS = (HTTP_TRANS *)nifo_ptr(pHTTPHASH, pHASHNODE->offset_Data);
		
		if(pHTTPTRANS->usL7FailCode == HTTP_UERR_EMPTY) {
			pHTTPTRANS->usL7FailCode = failCode;
		}
		pHTTPTRANS->ucStatus = GetHtttpTransStatus(pMSGINFO, isNotREQ);

		log_print(LOGN_DEBUG, "PROC STATUS RES SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
			util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
			util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort);

		pLOGHTTP = (LOG_IHTTP_TRANS *)nifo_ptr(pMEMSINFO, pHTTPTRANS->offset_LOG);

		pHTTPTRANS->offset_Node[HTTP_MSGTYPE_RESHDR] = pMSGINFO->offset_HDR;
		pHTTPTRANS->offset_Node[HTTP_MSGTYPE_RESBODY] = pMSGINFO->offset_BODY;

		pMSGINFO->offset_HDR = 0;
		pMSGINFO->offset_CurHDR = 0;
		pMSGINFO->offset_BODY = 0;
		pMSGINFO->offset_CurBODY = 0;

		if((pNode = pAllocDataNode(pMEMSINFO, HTTP_RES_HDR_NUM)) == NULL) {
			log_print(LOGN_CRI, "PROC STATUS RES HEADER ALLOC NODE NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
				util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort);
			return -83;
		}

		pMSGINFO->offset_HDR = nifo_offset(pMEMSINFO, pNode);
		pMSGINFO->offset_CurHDR = pMSGINFO->offset_HDR;

		if((pNode = pAllocDataNode(pMEMSINFO, HTTP_RES_BODY_NUM)) == NULL) {
			log_print(LOGN_CRI, "PROC STATUS RES BODY ALLOC NODE NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
				util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort);
			return -84;
		}

		pMSGINFO->offset_BODY = nifo_offset(pMEMSINFO, pNode);
		pMSGINFO->offset_CurBODY = pMSGINFO->offset_BODY;

		pLOGHTTP->uiResStartTime = pMSGINFO->uiStartTime;
		pLOGHTTP->uiResStartMTime = pMSGINFO->uiStartMTime;
		pLOGHTTP->uiResEndTime = pMSGINFO->uiLastUpdateTime;
		pLOGHTTP->uiResEndMTime = pMSGINFO->uiLastUpdateMTime;
		pLOGHTTP->uiMNAckTime = pMSGINFO->uiAckTime;
		pLOGHTTP->uiMNAckMTime = pMSGINFO->uiAckMTime;
		if(pMSGINFO->uiAckTime == 0) {
			pLOGHTTP->uiLastPktTime = pMSGINFO->uiLastUpdateTime;
			pLOGHTTP->uiLastPktMTime = pMSGINFO->uiLastUpdateMTime;
		} else {
			pLOGHTTP->uiLastPktTime = pMSGINFO->uiAckTime;
			pLOGHTTP->uiLastPktMTime = pMSGINFO->uiAckMTime;
		}
		pLOGHTTP->usResCode = pHTTPTSESS->usResCode;
		pLOGHTTP->ucStatus = logStatus;
		pLOGHTTP->uiTcpDnBodySize = pMSGINFO->uiHdrLen + pMSGINFO->uiBodyLen;
		pLOGHTTP->uiLimitDataSize = pMSGINFO->uiLimitLen;
		pLOGHTTP->uiDnHeaderSize = pMSGINFO->uiHdrLen;
		pLOGHTTP->uiDnBodySize = pMSGINFO->uiBodyLen;
		pLOGHTTP->uiContentLength = pMSGINFO->uiLen;

		pLOGHTTP->dChunked = pMSGINFO->dChunked;
		pLOGHTTP->dResZip = pMSGINFO->dZip;
		pLOGHTTP->dResMultiLen = pMSGINFO->MULTIKEY.len;
		memcpy(pLOGHTTP->szResMulti, pMSGINFO->MULTIKEY.key, pLOGHTTP->dResMultiLen);
		pLOGHTTP->szResMulti[pLOGHTTP->dResMultiLen] = 0x00;

		memcpy(pLOGHTTP->szContentsType, pHTTPTSESS->szContentsType, pHTTPTSESS->usContentsTypeSize);
		pLOGHTTP->szContentsType[pHTTPTSESS->usContentsTypeSize] = 0x00;

		pLOGHTTP->ucTcpClientStatus = pHTTPTSESS->ucTcpClientStatus;
		pLOGHTTP->ucTcpServerStatus = pHTTPTSESS->ucTcpServerStatus;
		pLOGHTTP->usUserErrorCode = pHTTPTRANS->usL7FailCode;
		if((pLOGHTTP->usL4FailCode == 0) && (pHTTPTRANS->usL4FailCode > 0)) {
			pLOGHTTP->usL4FailCode = pHTTPTRANS->usL4FailCode;
		}
		pLOGHTTP->usL7FailCode = pHTTPTRANS->usL7FailCode;

		UpCount(pLOGHTTP, pHTTPTSESS);

		if((dRet = dCloseHttpTrans(pMEMSINFO, pHTTPHASH, pHTTPTSESS, pHTTPTRANSKEY, pHTTPTRANS)) < 0) {
			log_print(LOGN_CRI, 
				"[%s][%s.%d] dCloseHttpTrans dRet[%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				__FILE__, __FUNCTION__, __LINE__, dRet,
				util_cvtipaddr(szSIP, pHTTPTSESSKEY->uiCliIP), pHTTPTSESSKEY->usCliPort,
				util_cvtipaddr(szDIP, pHTTPTSESS->uiSrvIP), pHTTPTSESS->usSrvPort);
			return -96;
		}

	}

	pMSGINFO->uiStartTime = 0;
	pMSGINFO->uiStartMTime = 0;
	pMSGINFO->uiLastUpdateTime = 0;
	pMSGINFO->uiLastUpdateMTime = 0;
	pMSGINFO->uiAckTime = 0;
	pMSGINFO->uiAckMTime = 0;
	pMSGINFO->uiHdrLen = 0;
	pMSGINFO->uiBodyLen = 0;
	pMSGINFO->uiLimitLen = 0;
	pMSGINFO->ucLenType = 0;
	pMSGINFO->uiLen = 0;
	pMSGINFO->uiMaxLen = 0;
	pMSGINFO->ucEndStatus = END_STATE_EMPTY;
	pMSGINFO->MULTIKEY.len = 0;
	pMSGINFO->MULTIKEY.state = 0;
	pMSGINFO->dZip = 0;
	pMSGINFO->dChunked = 0;

	return 0;
}

/*
 * $Log: ihttp_func.c,v $
 * Revision 1.2  2011/09/04 11:40:36  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/17 12:57:49  hhbaek
 * A_IHTTP
 *
 * Revision 1.2  2011/08/10 09:57:43  uamyd
 * modified and block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.9  2011/05/09 10:28:43  dark264sh
 * A_IHTTP: A_CALL multi 처리
 *
 * Revision 1.8  2011/04/18 07:16:12  dark264sh
 * L4_INET 변경에 따른 수정
 *
 * Revision 1.7  2011/04/17 15:05:37  dark264sh
 * A_ITCP, A_IHTTP: log_print 수정
 *
 * Revision 1.6  2011/04/16 11:50:27  dark264sh
 * A_IHTTP: RTX를 모르는 경우 http header를 보고 판단
 *
 * Revision 1.5  2011/04/14 14:54:48  dark264sh
 * A_IHTTP: Content-Type 처리 추가
 *
 * Revision 1.4  2011/04/14 13:11:29  dark264sh
 * A_IHTTP: GapTime 처리 추가
 *
 * Revision 1.3  2011/04/14 12:07:59  dark264sh
 * A_IHTTP: Body Buffering off, Log만 전송 하도록 변경
 *
 * Revision 1.2  2011/04/14 11:20:41  dark264sh
 * A_IHTTP: LOG_HTTP_TRANS => LOG_IHTTP_TRANS 변경
 *
 * Revision 1.1  2011/04/11 12:06:33  dark264sh
 * A_IHTTP 추가
 *
 */
