/**		@file	http_func.c
 * 		- HTTP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: http_func.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:47 $
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

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"

// PROJECT
#include "procid.h"
#include "common_stg.h"

// TAF
#include "tools.h"
#include "http.h"

// .
#include "http_func.h"
#include "http_msgq.h"
#include "http_util.h"

extern S64		curSessCnt;		/* Transaction 개수 */
extern S64		sessCnt;
extern S64		rcvNodeCnt;		/* 받은 NODE 개수 */
extern S64		diffSeqCnt;		/* DIFF SEQ가 된 개수 */

extern int		gACALLCnt;

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

		//dSndQID = (pTCPINFO->usAppCode == S_MSGQ_A_VOD) ? dVODQID : dCALLQID[pHTTPTSESSKEY->uiCliIP % gACALLCnt];
		dSeqProcID = ( pTCPINFO->usAppCode == SEQ_PROC_A_VOD ) ? 
						SEQ_PROC_A_VOD : SEQ_PROC_A_CALL + (pHTTPTSESSKEY->uiCliIP % gACALLCnt);

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

		//dSndQID = (pTCPINFO->usAppCode == S_MSGQ_A_VOD) ? dVODQID : dCALLQID[pHTTPTSESSKEY->uiCliIP % gACALLCnt];
		dSeqProcID = ( pTCPINFO->usAppCode == SEQ_PROC_A_VOD ) ? 
						SEQ_PROC_A_VOD : SEQ_PROC_A_CALL + (pHTTPTSESSKEY->uiCliIP % gACALLCnt);

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
	LOG_HTTP_TRANS	*pLOGHTTP;
	struct timeval	stNowTime;
	OFFSET			offset;

	curSessCnt--;

	gettimeofday(&stNowTime, NULL);

	pLOGHTTP = (LOG_HTTP_TRANS *)nifo_ptr(pMEMSINFO, pHTTPTRANS->offset_LOG);

	STG_DiffTIME64(pLOGHTTP->uiLastPktTime, pLOGHTTP->uiLastPktMTime, pLOGHTTP->uiReqStartTime, pLOGHTTP->uiReqStartMTime, &pLOGHTTP->llTransGapTime);
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
			nifo_node_link_nont_prev(pMEMSINFO, pHead, pNode);
		}
	}		

	if((dRet = dSend_HTTP_Data(pMEMSINFO, pHTTPTRANS->dSndMsgQ, pHead)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dSendSignal dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
	}

//	LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS", pLOGHTTP);

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
	pHTTPTSESS->usL4Code = pTCPINFO->usL4Code;
	pHTTPTSESS->usL7Code = pTCPINFO->usL7Code;

	pHTTPTSESS->ucSSL = SSL_OFF;
	pHTTPTSESS->ucMethod = 0;
	pHTTPTSESS->usResCode = 0;
	pHTTPTSESS->hostNameLen = 0;
	pHTTPTSESS->szHostName[0] = 0x00;
	pHTTPTSESS->urlType = 0;
	pHTTPTSESS->usUrlSize = 0;
	pHTTPTSESS->szUrl[0] = 0x00;
	pHTTPTSESS->usContentsType = 0;

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
		pMSGINFO->ucIsBuffering = TSESS_BUFFERING_ON;
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
	LOG_HTTP_TRANS		*pLOGHTTP;
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

		if((pLOGHTTP = (LOG_HTTP_TRANS *)nifo_tlv_alloc(pMEMSINFO, pLOGNODE, LOG_HTTP_TRANS_DEF_NUM, LOG_HTTP_TRANS_SIZE, DEF_MEMSET_ON)) == NULL) {
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
		pLOGHTTP->uiTcpSynTime = pHTTPTSESS->uiTcpSynTime;
		pLOGHTTP->uiTcpSynMTime = pHTTPTSESS->uiTcpSynMTime;
		pLOGHTTP->usTransID = pHTTPTRANSKEY->usHttpTransID;
		pLOGHTTP->usSvcL4Type = pHTTPTSESS->usL4Code;
		pLOGHTTP->usSvcL7Type = pHTTPTSESS->usL7Code;
		pLOGHTTP->ucSubSysNo = 1;
		pLOGHTTP->usContentsType = pHTTPTSESS->usContentsType;
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

		pLOGHTTP->usLOGURLSize = GetURL(pHTTPTSESS->szUrl, pHTTPTSESS->usUrlSize, pHTTPTSESS->urlType, pHTTPTSESS->szHostName, pHTTPTSESS->hostNameLen, pLOGHTTP->szLOGURL, MAX_LOGURL_LEN);

		if(pLOGHTTP->usLOGURLSize > MAX_URL_LEN) {
			pLOGHTTP->usURLSize = MAX_URL_LEN;
		} else {
			pLOGHTTP->usURLSize = pLOGHTTP->usLOGURLSize;
		}

		memcpy(pLOGHTTP->szURL, pLOGHTTP->szLOGURL, pLOGHTTP->usURLSize);
		pLOGHTTP->szURL[pLOGHTTP->usURLSize] = 0x00;

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

		pHTTPTRANS->dSndMsgQ = dGetSeqProcID(pHTTPTSESS->usAppCode, pHTTPTRANSKEY->uiCliIP);
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

		pLOGHTTP = (LOG_HTTP_TRANS *)nifo_ptr(pMEMSINFO, pHTTPTRANS->offset_LOG);

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
 * $Log: http_func.c,v $
 * Revision 1.3  2011/09/07 06:30:47  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/04 11:12:11  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/10 09:57:43  uamyd
 * modified and block added
 *
 * Revision 1.2  2011/08/05 09:04:49  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.10  2011/05/10 07:05:45  jsyoon
 * *** empty log message ***
 *
 * Revision 1.9  2011/05/09 15:18:31  jsyoon
 * *** empty log message ***
 *
 * Revision 1.8  2011/01/11 04:09:06  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.7  2009/10/26 08:57:37  pkg
 * A_HTTP LimitLen 처리 추가
 *
 * Revision 1.6  2009/09/13 08:53:50  jsyoon
 * PI프로세스의 uiNASName 필드값 제거
 *
 * Revision 1.5  2009/08/19 12:24:38  pkg
 * LOGN_XXX_Prt 함수 주석 처리
 *
 * Revision 1.4  2009/08/18 14:51:57  pkg
 * A_HTTP node full 버그 수정
 *
 * Revision 1.3  2009/07/15 16:12:38  dqms
 * 멀티프로세스 수정
 *
 * Revision 1.2  2009/06/16 15:16:00  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:20  dqms
 * Init TAF_RPPI
 *
 * Revision 1.8  2009/03/02 09:11:12  dark264sh
 * DOWNLOAD VOD : 1x 단말에서 Content-Length, Transfer-Encoding Chunked, Multi Part 없이 Packet-Counter만 있는 경우 처리
 *
 * Revision 1.7  2008/11/17 11:13:36  dark264sh
 * *** empty log message ***
 *
 * Revision 1.6  2008/11/17 09:04:42  dark264sh
 * 64bits 작업
 *
 * Revision 1.5  2008/09/18 07:40:39  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.4  2008/07/02 06:35:06  jsyoon
 * *** empty log message ***
 *
 * Revision 1.3  2008/06/24 23:40:49  jsyoon
 * *** empty log message ***
 *
 * Revision 1.2  2008/06/22 10:20:42  dark264sh
 * A_FB chunked, multipart, gzip, deflate, min 처리
 *
 * Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.4  2007/09/05 06:13:07  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2007/08/29 07:41:01  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2007/08/27 13:58:03  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/08/21 12:53:00  dark264sh
 * no message
 *
 * Revision 1.42  2006/12/08 01:44:00  dark264sh
 * *** empty log message ***
 *
 * Revision 1.41  2006/12/06 09:08:24  dark264sh
 * *** empty log message ***
 *
 * Revision 1.40  2006/12/06 06:38:45  dark264sh
 * *** empty log message ***
 *
 * Revision 1.39  2006/12/06 03:36:25  dark264sh
 * *** empty log message ***
 *
 * Revision 1.38  2006/12/05 08:21:41  dark264sh
 * *** empty log message ***
 *
 * Revision 1.37  2006/12/04 08:06:03  dark264sh
 * *** empty log message ***
 *
 * Revision 1.36  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.35  2006/11/24 09:09:35  dark264sh
 * *** empty log message ***
 *
 * Revision 1.34  2006/11/22 08:05:24  dark264sh
 * *** empty log message ***
 *
 * Revision 1.33  2006/11/22 06:32:16  dark264sh
 * *** empty log message ***
 *
 * Revision 1.32  2006/11/16 06:14:29  dark264sh
 * *** empty log message ***
 *
 * Revision 1.31  2006/11/14 08:48:48  dark264sh
 * *** empty log message ***
 *
 * Revision 1.30  2006/11/14 03:04:09  dark264sh
 * *** empty log message ***
 *
 * Revision 1.29  2006/11/13 07:15:24  dark264sh
 * LOG_HTTP_TRANS memset 하도록 수정
 *
 * Revision 1.28  2006/11/10 15:48:30  dark264sh
 * *** empty log message ***
 *
 * Revision 1.27  2006/11/10 14:21:26  dark264sh
 * *** empty log message ***
 *
 * Revision 1.26  2006/11/10 13:46:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.25  2006/11/10 12:07:54  dark264sh
 * *** empty log message ***
 *
 * Revision 1.24  2006/11/10 09:32:59  dark264sh
 * *** empty log message ***
 *
 * Revision 1.23  2006/11/09 09:26:19  dark264sh
 * A_HTTP CRI LOG 변경
 *
 * Revision 1.22  2006/11/08 07:13:41  shlee
 * CONF관련 hasho -> hashg로 변경 및 CONF_CNT 101 CONF_PREA_CNT 811로 변경
 *
 * Revision 1.21  2006/11/07 09:24:22  dark264sh
 * A_HTTP 에러 코드 세팅 변경
 *
 * Revision 1.20  2006/11/06 07:36:54  dark264sh
 * nifo NODE size 4*1024 => 6*1024로 변경하기
 * nifo_tlv_alloc에서 argument로 memset할지 말지 결정하도록 수정
 * nifo_node_free에서 semaphore 삭제
 *
 * Revision 1.19  2006/11/02 07:19:42  dark264sh
 * REQ 메시지가 두개로 나누어 진 경우 URL 처리가 잘못되는 문제 해결
 *
 * Revision 1.18  2006/11/01 09:25:02  dark264sh
 * SESS, SEQ, NODE 개수 LOG추가
 *
 * Revision 1.17  2006/10/30 03:12:49  dark264sh
 * ssl browserinfo, model 값 세팅 추가
 *
 * Revision 1.16  2006/10/30 00:48:28  dark264sh
 * *** empty log message ***
 *
 * Revision 1.15  2006/10/27 12:36:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.14  2006/10/27 03:05:12  dark264sh
 * RSTP SSL 처리
 *
 * Revision 1.13  2006/10/26 04:07:31  dark264sh
 * *** empty log message ***
 *
 * Revision 1.12  2006/10/25 02:45:53  dark264sh
 * *** empty log message ***
 *
 * Revision 1.11  2006/10/24 11:08:10  dark264sh
 * *** empty log message ***
 *
 * Revision 1.10  2006/10/20 10:01:32  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2006/10/20 02:29:04  dark264sh
 * *** empty log message ***
 *
 * Revision 1.8  2006/10/19 05:52:44  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2006/10/18 08:53:31  dark264sh
 * nifo debug 코드 추가
 *
 * Revision 1.6  2006/10/17 03:50:55  dark264sh
 * nifo_tlv_alloc에 memset 추가로 인한 변경
 *
 * Revision 1.5  2006/10/13 04:58:40  dark264sh
 * LOG 초기화 방법 변경 (memset 이용)
 *
 * Revision 1.4  2006/10/12 12:56:00  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2006/10/11 12:29:27  dark264sh
 * URL에서 포트 번호를 빼던 것을 있으면 그냥 두는 것으로 수정
 *
 * Revision 1.2  2006/10/11 11:52:33  dark264sh
 * PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 * Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 * no message
 *
 * Revision 1.136  2006/10/10 07:00:30  dark264sh
 * A_CALL에 전송하는 부분 추가
 * nifo_node_alloc 함수 변경에 따른 변경
 * A_TCP에서 timerN_update의 리턴으로 timerNID 업데이트 하도록 변경
 *
 * Revision 1.135  2006/10/09 13:27:26  dark264sh
 * URL을 http://hostname/path 형태로 만드는 부분 추가 (Port 번호를 뺀 값)
 *
 * Revision 1.134  2006/10/09 02:40:14  dark264sh
 * CALL MSGQID 변경
 *
 * Revision 1.133  2006/10/04 01:53:47  dark264sh
 * FailCode, HTTP Status 값 할당 기능 추가
 *
 * Revision 1.132  2006/10/02 02:58:21  dark264sh
 * *** empty log message ***
 *
 * Revision 1.131  2006/10/02 00:19:21  dark264sh
 * nifo의 TLV 변경에 따른 변경
 *
 * Revision 1.130  2006/09/29 08:58:32  dark264sh
 * Content-type을 보고 버퍼일 결정 하도록 변경
 *
 * Revision 1.129  2006/09/28 08:26:00  dark264sh
 * 하나의 패킷에 REQ Header 끝나지 않는 경우 처리 변경
 *
 * Revision 1.128  2006/09/28 05:05:22  dark264sh
 * content-type으로 buffering 처리 부분 추가 하면서 header에도 동일한 조건이 적용돼서 header를 buffering 하지 않는 문제 처리
 *
 * Revision 1.127  2006/09/27 11:35:40  dark264sh
 * *** empty log message ***
 *
 * Revision 1.126  2006/09/27 07:27:13  dark264sh
 * content-type을 보고 버퍼링 결정을 위한 추가
 *
 * Revision 1.125  2006/09/27 05:07:28  dark264sh
 * 변수 이름 변경
 *
 * Revision 1.124  2006/09/26 10:23:24  dark264sh
 * RESCODE 잘못 들어간 부분 수정
 *
 * Revision 1.123  2006/09/26 09:28:35  dark264sh
 * HTTP_TRANS 멤버 변경
 * 필요없는 필드 삭제
 *
 * Revision 1.122  2006/09/26 09:03:00  dark264sh
 * HTTP로 전송 하기 위한 Data Cnt, Size 잘못 전달 하는 부분 수정
 *
 * Revision 1.121  2006/09/26 08:58:47  dark264sh
 * HTTP로 전송 하기 위한 Data Cnt, Size 잘못 전달 하는 부분 수정
 *
 * Revision 1.120  2006/09/26 08:55:31  dark264sh
 * HTTP로 전송 하기 위한 Data Cnt, Size 잘못 전달 하는 부분 수정
 *
 * Revision 1.119  2006/09/26 07:47:11  dark264sh
 * HTTP로 전송 하기 위한 Data Cnt, Size 잘못 전달 하는 부분 수정
 *
 * Revision 1.118  2006/09/26 06:11:50  dark264sh
 * HTTP LOG값 잘못 되는 부분 수정
 *
 * Revision 1.117  2006/09/26 06:03:08  dark264sh
 * HTTP LOG값 잘못 되는 부분 수정
 *
 * Revision 1.116  2006/09/26 04:39:24  dark264sh
 * HTTP LOG값 잘못 되는 부분 수정
 *
 * Revision 1.115  2006/09/25 09:12:07  dark264sh
 * no message
 *
 * Revision 1.114  2006/09/25 07:38:41  dark264sh
 * HTTP의 처리 상태값 잘못 처리된 부분 수정
 *
 * Revision 1.113  2006/09/25 07:15:05  dark264sh
 * HTTP의 처리 상태값 잘못 처리된 부분 수정
 *
 * Revision 1.112  2006/09/25 06:44:20  dark264sh
 * no message
 *
 * Revision 1.111  2006/09/25 06:28:32  dark264sh
 * *** empty log message ***
 *
 * Revision 1.110  2006/09/25 06:15:05  dark264sh
 * no message
 *
 * Revision 1.109  2006/09/25 02:58:47  dark264sh
 * *** empty log message ***
 *
 * Revision 1.108  2006/09/21 09:06:13  dark264sh
 * content-length 받아오는 부분 수정
 *
 * Revision 1.107  2006/09/21 09:05:23  dark264sh
 * content-length 받아오는 부분 수정
 *
 * Revision 1.106  2006/09/21 08:41:22  dark264sh
 * print 함수 추가
 *
 * Revision 1.105  2006/09/21 08:40:38  dark264sh
 * print 함수 추가
 *
 * Revision 1.104  2006/09/21 08:37:56  dark264sh
 * print 함수 추가
 *
 * Revision 1.103  2006/09/21 06:46:47  dark264sh
 * MAX SIZE를 두어 제한을 가하던 방식 삭제
 *
 * Revision 1.102  2006/09/21 06:46:17  dark264sh
 * MAX SIZE를 두어 제한을 가하던 방식 삭제
 *
 * Revision 1.101  2006/09/21 06:29:44  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.100  2006/09/21 05:39:02  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.99  2006/09/21 05:32:29  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.98  2006/09/19 07:23:40  dark264sh
 * no message
 *
 * Revision 1.97  2006/09/19 07:20:51  dark264sh
 * no message
 *
 * Revision 1.96  2006/09/19 01:39:33  dark264sh
 * no message
 *
 * Revision 1.95  2006/09/19 01:23:44  dark264sh
 * no message
 *
 * Revision 1.94  2006/09/19 00:59:21  dark264sh
 * no message
 *
 * Revision 1.93  2006/09/18 09:11:32  dark264sh
 * no message
 *
 * Revision 1.92  2006/09/18 08:57:28  dark264sh
 * no message
 *
 * Revision 1.91  2006/09/18 07:46:22  dark264sh
 * no message
 *
 * Revision 1.90  2006/09/18 07:42:10  dark264sh
 * no message
 *
 * Revision 1.89  2006/09/18 07:41:44  dark264sh
 * no message
 *
 * Revision 1.88  2006/09/18 07:28:18  dark264sh
 * no message
 *
 * Revision 1.87  2006/09/18 07:20:04  dark264sh
 * no message
 *
 * Revision 1.86  2006/09/18 07:17:52  dark264sh
 * no message
 *
 * Revision 1.85  2006/09/18 06:13:01  dark264sh
 * *** empty log message ***
 *
 * Revision 1.84  2006/09/18 06:10:03  dark264sh
 * no message
 *
 * Revision 1.83  2006/09/18 05:54:11  dark264sh
 * no message
 *
 * Revision 1.82  2006/09/18 04:56:51  dark264sh
 * no message
 *
 * Revision 1.81  2006/09/18 04:49:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.80  2006/09/18 03:15:13  dark264sh
 * no message
 *
 * Revision 1.79  2006/09/15 09:28:52  dark264sh
 * nifo_node_link_nont, nifo_node_link_cont API 변경
 *
 * Revision 1.78  2006/09/14 11:33:01  dark264sh
 * no message
 *
 * Revision 1.77  2006/09/14 09:14:32  dark264sh
 * no message
 *
 * Revision 1.76  2006/09/14 08:46:56  dark264sh
 * no message
 *
 * Revision 1.75  2006/09/14 08:42:58  dark264sh
 * no message
 *
 * Revision 1.74  2006/09/14 08:39:28  dark264sh
 * no message
 *
 * Revision 1.73  2006/09/14 08:38:12  dark264sh
 * no message
 *
 * Revision 1.72  2006/09/14 07:08:57  dark264sh
 * no message
 *
 * Revision 1.71  2006/09/14 07:07:21  dark264sh
 * no message
 *
 * Revision 1.70  2006/09/14 06:50:05  dark264sh
 * no message
 *
 * Revision 1.69  2006/09/14 06:49:03  dark264sh
 * no message
 *
 * Revision 1.68  2006/09/14 06:10:00  dark264sh
 * no message
 *
 * Revision 1.67  2006/09/14 02:59:20  dark264sh
 * no message
 *
 * Revision 1.66  2006/09/14 01:19:14  dark264sh
 * no message
 *
 * Revision 1.65  2006/09/13 11:47:46  dark264sh
 * *** empty log message ***
 *
 * Revision 1.64  2006/09/13 11:37:58  dark264sh
 * hasho_add, hasho_find를 잘못 사용한 부분 수정
 *
 * Revision 1.63  2006/09/13 04:30:25  dark264sh
 * strerror 잘못 찍는 부분 수정
 *
 * Revision 1.62  2006/09/11 03:05:30  dark264sh
 * nifo 변경에 따른 변경
 *
 * Revision 1.61  2006/09/11 02:30:28  dark264sh
 * nifo 변경에 따른 변경
 *
 * Revision 1.60  2006/09/06 11:55:30  dark264sh
 * *** empty log message ***
 *
 * Revision 1.59  2006/09/06 10:58:25  dark264sh
 * ../../INCOLD 변경
 *
 * Revision 1.58  2006/09/05 05:37:03  dark264sh
 * *** empty log message ***
 *
 * Revision 1.57  2006/09/05 04:57:19  dark264sh
 * 에러 핸들링, 에러 코드 부분 수정
 *
 * Revision 1.56  2006/09/04 05:31:45  dark264sh
 * 에러값 세팅 부분 수정
 *
 * Revision 1.55  2006/08/29 04:41:41  dark264sh
 * *** empty log message ***
 *
 * Revision 1.54  2006/08/29 04:40:56  dark264sh
 * no message
 *
 * Revision 1.53  2006/08/29 04:40:37  dark264sh
 * no message
 *
 * Revision 1.52  2006/08/29 04:38:59  dark264sh
 * no message
 *
 * Revision 1.51  2006/08/29 04:27:57  dark264sh
 * no message
 *
 * Revision 1.50  2006/08/29 01:46:18  dark264sh
 * no message
 *
 * Revision 1.49  2006/08/29 01:34:39  dark264sh
 * no message
 *
 * Revision 1.48  2006/08/29 01:30:42  dark264sh
 * no message
 *
 * Revision 1.47  2006/08/29 01:23:12  dark264sh
 * no message
 *
 * Revision 1.46  2006/08/28 12:22:30  dark264sh
 * no message
 *
 * Revision 1.45  2006/08/28 12:22:00  dark264sh
 * no message
 *
 * Revision 1.44  2006/08/28 12:21:35  dark264sh
 * no message
 *
 * Revision 1.43  2006/08/28 12:21:11  dark264sh
 * no message
 *
 * Revision 1.42  2006/08/28 12:18:45  dark264sh
 * DeleteBuffer 함수 추가
 *
 * Revision 1.41  2006/08/28 12:15:55  dark264sh
 * dProcStatus 함수 변경
 *
 * Revision 1.40  2006/08/28 04:09:51  dark264sh
 * no message
 *
 * Revision 1.39  2006/08/28 04:04:27  dark264sh
 * no message
 *
 * Revision 1.38  2006/08/28 03:55:10  dark264sh
 * no message
 *
 * Revision 1.37  2006/08/28 03:54:47  dark264sh
 * no message
 *
 * Revision 1.36  2006/08/28 03:53:12  dark264sh
 * no message
 *
 * Revision 1.35  2006/08/28 03:49:47  dark264sh
 * no message
 *
 * Revision 1.34  2006/08/28 03:47:32  dark264sh
 * no message
 *
 * Revision 1.33  2006/08/28 02:46:32  dark264sh
 * no message
 *
 * Revision 1.32  2006/08/28 02:44:31  dark264sh
 * no message
 *
 * Revision 1.31  2006/08/28 02:43:02  dark264sh
 * no message
 *
 * Revision 1.30  2006/08/28 02:41:47  dark264sh
 * no message
 *
 * Revision 1.29  2006/08/28 02:40:18  dark264sh
 * no message
 *
 * Revision 1.28  2006/08/28 02:38:34  dark264sh
 * no message
 *
 * Revision 1.27  2006/08/28 02:32:59  dark264sh
 * no message
 *
 * Revision 1.26  2006/08/28 02:28:49  dark264sh
 * no message
 *
 * Revision 1.25  2006/08/28 02:23:08  dark264sh
 * no message
 *
 * Revision 1.24  2006/08/28 02:21:06  dark264sh
 * no message
 *
 * Revision 1.23  2006/08/28 02:17:48  dark264sh
 * no message
 *
 * Revision 1.22  2006/08/28 02:16:05  dark264sh
 * no message
 *
 * Revision 1.21  2006/08/28 02:12:37  dark264sh
 * no message
 *
 * Revision 1.20  2006/08/28 02:06:13  dark264sh
 * no message
 *
 * Revision 1.19  2006/08/28 02:00:10  dark264sh
 * no message
 *
 * Revision 1.18  2006/08/28 01:57:05  dark264sh
 * no message
 *
 * Revision 1.17  2006/08/28 01:54:35  dark264sh
 * no message
 *
 * Revision 1.16  2006/08/28 01:46:59  dark264sh
 * msgqid를 global로 변경
 *
 * Revision 1.15  2006/08/25 07:15:08  dark264sh
 * no message
 *
 * Revision 1.14  2006/08/24 04:08:16  dark264sh
 * HTTP 기본 Flow 구성
 *
 * Revision 1.13  2006/07/28 12:23:25  dark264sh
 * no message
 *
 * Revision 1.12  2006/07/28 08:28:02  dark264sh
 * no message
 *
 * Revision 1.11  2006/07/28 08:01:11  dark264sh
 * TCP_DATA, TCP_DATAEND의 메시지 처리 pseudo 코드 추가
 *
 * Revision 1.10  2006/07/26 11:21:09  dark264sh
 * A_HTTP
 * TCP_DATA pseudo 코드 추가
 *
 * Revision 1.9  2006/07/26 05:55:10  dark264sh
 * TCP_END
 * 처리 추가
 *
 * Revision 1.8  2006/07/26 05:19:20  dark264sh
 * no message
 *
 * Revision 1.7  2006/07/26 03:23:09  dark264sh
 * no message
 *
 * Revision 1.6  2006/07/26 03:20:28  dark264sh
 * 오타 수정
 *
 * Revision 1.5  2006/07/26 03:05:57  dark264sh
 * TCP_START 처리 추가
 *
 */
