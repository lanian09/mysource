/**		@file	online_func.c
 * 		- ONLINE HTTP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: online_func.c,v 1.2 2011/09/05 08:20:23 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 08:20:23 $
 * 		@ref		online_func.c online_main.c online_init.c online_util.c l4.h online_api.h
 * 		@todo		
 *
 * 		@section	Intro(소개)
 * 		- ONLINE HTTP Transaction을 관리 하는 함수들
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

// LIB
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"
#include "mems.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"

// PROJECT
#include "common_stg.h"

// TAF
#include "tools.h"

// .
#include "online_func.h"
#include "online_msgq.h"
#include "online_util.h"

/**
 * Declare variables
 */
extern S64      curSessCnt;
extern S64      sessCnt;
extern S64      rcvNodeCnt;
extern S64      diffSeqCnt;
extern S32      __loop_cnt;



/** dProcOnlineTrans function.
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
 *  @see			online_func.c online_msgq.c online_main.c online_init.c online_util.c online_api.h
 *
 *  @exception		nothing
 *  @note			nothing
 **/
S32 dProcOnlineTrans(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASH, TCP_INFO *pTCPINFO, U8 *pNode, U8 *pDATA)
{
	S32					dRet;		/**< 함수 Return 값 */
	U32					uiNextSeq;	/**< 받을 데이터의 Seq번호, 데이터가 손실없이 도착했는지 비교를 위해서*/
	U8					szSIP[INET_ADDRSTRLEN];
	U8					szDIP[INET_ADDRSTRLEN];
	U8					*pTCPDATA;
	U8					*pLOGNODE;
	struct timeval		stTime;
	ONLINE_TSESS_KEY	TSESSKEY;
	ONLINE_TSESS_KEY	*pTSESSKEY;
	ONLINE_TSESS		TSESS;
	ONLINE_TSESS		*pTSESS;
	LOG_ONLINE_TRANS	*pLOG;
	NODE_INFO			NODEINFO;
	NODE_INFO			*pNODEINFO = &NODEINFO;

	stHASHONODE			*pHASHNODE;

	pTSESSKEY = &TSESSKEY;
	pTSESS = &TSESS;

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

		curSessCnt++;
		sessCnt++;

		log_print(LOGN_DEBUG, "@@@ START SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
				util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
				util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
					
		/* Make Tcp Hash Key */
		MakeHashKey(pTCPINFO, pTSESSKEY);

    	if((pLOGNODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
        	log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
    	}   

    	if((pLOG = (LOG_ONLINE_TRANS *)nifo_tlv_alloc(pMEMSINFO, pLOGNODE, LOG_ONLINE_TRANS_DEF_NUM, LOG_ONLINE_TRANS_SIZE, DEF_MEMSET_ON)) == NULL) {
        	log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc NULL", __FUNCTION__, __LINE__);
        	nifo_node_delete(pMEMSINFO, pLOGNODE);
    	}   
		
		pTSESS->ONLINE_STATE = 0;
		pTSESS->ONLINE_OLD_STATE = 0;
		pTSESS->offset_LOG = nifo_offset(pMEMSINFO, pLOG);
		pTSESS->type = GetOnlineType(pTCPINFO->usL4Code);
		if(pTSESS->type == ONLINE_WICGS) {
			pTSESS->ONLINEINFO[ONLINE_UP].checkHdrSize = st_WicgsBillCom_SIZE;
			pTSESS->ONLINEINFO[ONLINE_DOWN].checkHdrSize = st_Wicgs_SIZE;
		} else {
			pTSESS->ONLINEINFO[ONLINE_UP].checkHdrSize = st_MacsBillCom_SIZE;
			pTSESS->ONLINEINFO[ONLINE_DOWN].checkHdrSize = st_Macs_SIZE;
		}
		
		pTSESS->ONLINEINFO[ONLINE_UP].isState = ONLINE_HDR_WAIT;	
		pTSESS->ONLINEINFO[ONLINE_DOWN].isState = ONLINE_HDR_WAIT;	
		pTSESS->ONLINEINFO[ONLINE_UP].checkDataSize = 0;	
		pTSESS->ONLINEINFO[ONLINE_DOWN].checkDataSize = 0;	
		pTSESS->ONLINEINFO[ONLINE_UP].oldDataSize = 0;	
		pTSESS->ONLINEINFO[ONLINE_DOWN].oldDataSize = 0;	
		pTSESS->ONLINEINFO[ONLINE_UP].remainHDRSize = 0;	
		pTSESS->ONLINEINFO[ONLINE_DOWN].remainHDRSize = 0;	
		pTSESS->ONLINEINFO[ONLINE_UP].szHDR[0] = 0x00;	
		pTSESS->ONLINEINFO[ONLINE_DOWN].szHDR[0] = 0x00;	

		pTSESS->ucSynRtx = pTCPINFO->ucRtx;
		pTSESS->ucBroken = ONLINE_NO;
		pTSESS->uiUpNextSeq = pTCPINFO->uiSeqNum;
		pTSESS->uiDnNextSeq = pTCPINFO->uiAckNum;

		pLOG->uiCallTime = pTCPINFO->uiCapTime;
		pLOG->uiCallMTime = pTCPINFO->uiCapMTime;
		pLOG->uiClientIP = pTCPINFO->uiCliIP;
//		pLOG->uiNASName = pTCPINFO->uiCliIP;
		pLOG->uiNASName = 0;

		pLOG->usClientPort = pTCPINFO->usCliPort;
		pLOG->uiServerIP = pTCPINFO->uiSrvIP;
		pLOG->usServerPort = pTCPINFO->usSrvPort;
		pLOG->uiTcpSynTime = pTCPINFO->uiCapTime;
		pLOG->uiTcpSynMTime = pTCPINFO->uiCapMTime;

//		pLOG->usPlatformType = pTCPINFO->usL4Code / 1000 * 1000;
		pLOG->usSvcL4Type = pTCPINFO->usL4Code;
		pLOG->usSvcL7Type = pTCPINFO->usL7Code;
		pLOG->usPlatformType = dGetPlatformType(pLOG->usSvcL4Type, pLOG->usSvcL7Type);

		pLOG->ucSubSysNo = 1;

		pLOG->usUserErrorCode = GetFailCode(pLOG, pTCPINFO->usL4FailCode);
		pLOG->usL4FailCode = pTCPINFO->usL4FailCode;

		UpCount(pTCPINFO, pLOG);

		gettimeofday(&stTime, NULL);

		pLOG->uiOpStartTime = stTime.tv_sec;
		pLOG->uiOpStartMTime = stTime.tv_usec;


		/* Tcp Session 생성 */
		if((pHASHNODE = hasho_add(pHASH, (U8 *)pTSESSKEY, (U8 *)pTSESS)) == NULL) {
			/* 동일한 TCP Session 존재 */
			/* 기존 세션 정리 */
			if((pHASHNODE = hasho_find(pHASH, (U8 *)pTSESSKEY)) == NULL) 
			{   
				log_print(LOGN_CRI, "SAME BUT NOT FINE ????? EXIT EXIT");
				exit(0);
			}
            
			pTSESS = (ONLINE_TSESS *)nifo_ptr(pHASH, pHASHNODE->offset_Data);

			if((dRet = dCloseSess(pMEMSINFO, pHASH, pTSESSKEY, pTSESS)) < 0) {
				log_print(LOGN_CRI, "[%s][%s.%d] dCloseTcpSess dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			}

			pTSESS = &TSESS;

			if((pHASHNODE = hasho_add(pHASH, (U8 *)pTSESSKEY, (U8 *)pTSESS)) == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL", __FILE__, __FUNCTION__, __LINE__);
			}
		}

		pTSESS = (ONLINE_TSESS *)nifo_ptr(pHASH, pHASHNODE->offset_Data);

		pTSESS->ONLINE_OLD_STATE = 0;
		pTSESS->ONLINE_STATE = STS_DATA_WAIT;

		/* A_CALL에 TCP Session 생성 통보 */
		if((dRet = dSend_ONLINE_Data(pMEMSINFO, dGetCALLProcID(pLOG->uiClientIP), pNode)) < 0) {
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
		MakeHashKey(pTCPINFO, pTSESSKEY);

		/* Find Tcp Hash */
		if((pHASHNODE = hasho_find(pHASH, (U8 *)pTSESSKEY)) == NULL)
		{
			/* Tcp Hash를 찾지 못함 */
			log_print(LOGN_WARN, "[%s][%s.%d] HASH NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
			return -21;
		}

		pTSESS = (ONLINE_TSESS *)nifo_ptr(pHASH, pHASHNODE->offset_Data);

		pLOG = (LOG_ONLINE_TRANS *)nifo_ptr(pMEMSINFO, pTSESS->offset_LOG);

		/*
		 * 에러 값 세팅, 연산 값 
		 */
		pLOG->usUserErrorCode = GetFailCode(pLOG, pTCPINFO->usL4FailCode);
		pLOG->usL4FailCode = pTCPINFO->usL4FailCode;
		UpCount(pTCPINFO, pLOG);

		/* 해당 Http Transaction 찾기 */
		if((dRet = dCloseSess(pMEMSINFO, pHASH, pTSESSKEY, pTSESS)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u] dCloseTcpSess dRet[%d]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, dRet);
			return -22;
		}

		/* A_CALL에 Tcp Session 종료 통보 */
	 	if((dRet = dSend_ONLINE_Data(pMEMSINFO, dGetCALLProcID(pLOG->uiClientIP), pNode)) < 0) {
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
		MakeHashKey(pTCPINFO, pTSESSKEY);

		/* Find Tcp Hash */
		if((pHASHNODE = hasho_find(pHASH, (U8 *)pTSESSKEY)) == NULL)
		{
			/* Tcp Hash를 찾지 못함 */
			log_print(LOGN_WARN, "[%s][%s.%d] HASH NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
					__FILE__, __FUNCTION__, __LINE__,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort);
			return -31;
		}

		pTSESS = (ONLINE_TSESS *)nifo_ptr(pHASH, pHASHNODE->offset_Data);

		pLOG = (LOG_ONLINE_TRANS *)nifo_ptr(pMEMSINFO, pTSESS->offset_LOG);

		/* 기본적인 값 UPDATE */
		pLOG->usUserErrorCode = GetFailCode(pLOG, pTCPINFO->usL4FailCode);
		pLOG->usL4FailCode = pTCPINFO->usL4FailCode;
		UpCount(pTCPINFO, pLOG);

		/**
		 *	SEQ 검사 
		 *	일치하지 않는 경우 연속적인 데이터가 아니라고 판단
		 *	모든 세션 종료
		 */
		if(pTSESS->ucBroken == ONLINE_YES) {
			if(pTCPINFO->ucRtx == pTSESS->ucSynRtx) {
				pLOG->uiTcpUpBodySize += pTCPINFO->uiDataSize;
			} else {
				pLOG->uiTcpDnBodySize += pTCPINFO->uiDataSize;
			}
			return -12;
		}

		if(pTCPINFO->ucRtx == pTSESS->ucSynRtx) {
			uiNextSeq = pTSESS->uiUpNextSeq;
		} else {
			uiNextSeq = pTSESS->uiDnNextSeq;
		}

		if(uiNextSeq != pTCPINFO->uiSeqNum) {
			/* 모든 HTTP Trans 정리 */
			log_print(LOGN_CRI, "TCP DATA DIFF SEQ NSEQ[%u]RSEQ[%u] SIP[%s:%u]DIP[%s:%u]STIME[%u.%d]", 
					uiNextSeq, pTCPINFO->uiSeqNum,
					util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
					util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, 
					pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime);

			diffSeqCnt++;

			pTSESS->ucBroken = ONLINE_YES;
			return -33;
		}

		if(pTCPINFO->ucRtx == pTSESS->ucSynRtx) {
			pTSESS->uiUpNextSeq = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize;
			pNODEINFO->updown = ONLINE_UP;
		} else {
			pTSESS->uiDnNextSeq = pTCPINFO->uiSeqNum + pTCPINFO->uiDataSize;
			pNODEINFO->updown = ONLINE_DOWN;
		}
		
		pLOG->uiLastPktTime = pTCPINFO->uiCapTime;
		pLOG->uiLastPktMTime = pTCPINFO->uiCapMTime;

		/* CALL FLOW */
		pNODEINFO->magic = 0;
		pNODEINFO->dataSize = pTCPINFO->uiDataSize;
		pNODEINFO->svcType = GetSvcType(pTSESS, pTCPINFO);
		pNODEINFO->offset_DATA = nifo_offset(pMEMSINFO, pTCPDATA);
		pNODEINFO->offset_TCPINFO = nifo_offset(pMEMSINFO, pTCPINFO);
		pNODEINFO->offset_NODE = nifo_offset(pMEMSINFO, pNode);
		pNODEINFO->pMEMSINFO = pMEMSINFO;
		pNODEINFO->func1 = CheckWicgsBillComHeader;
		pNODEINFO->func2 = CheckWicgsServerHeader;
		pNODEINFO->func3 = CheckMacsBillComHeader;
		pNODEINFO->func4 = CheckMacsServerHeader;


		flow_ONLINE_state_go(pTSESS, NODE_INFO_DEF_NUM, NODE_INFO_SIZE, pNODEINFO, pTSESS->ONLINE_STATE, YES);

		if((__loop_cnt < 0) || (pNODEINFO->magic < 0)) {
			log_print(LOGN_CRI, "OVERFLOW FLOW CNT DELETE SESSION");
			pTSESS->ucBroken = ONLINE_YES;
		}

		nifo_node_delete(pMEMSINFO, pNode);
		break;

	default:
		/**
		 *  버그 발생
		 **/
		log_print(LOGN_CRI, 
			"STRANGE TCP FLAG SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]RTX[%s] FLAG[%d]", 
			util_cvtipaddr(szSIP, pTCPINFO->uiCliIP), pTCPINFO->usCliPort,
			util_cvtipaddr(szDIP, pTCPINFO->uiSrvIP), pTCPINFO->usSrvPort, 
			PrintRtx(pTCPINFO->ucRtx), pTCPINFO->cTcpFlag);
		return -100;
	}

	return 0;
}

/** dCloseSess function.
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
 *  @see			online_func.c online_msgq.c online_main.c online_init.c online_util.c online_api.h
 *
 *  @exception		nothing
 *  @note			nothing
 **/
S32 dCloseSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASH, ONLINE_TSESS_KEY *pTSESSKEY, ONLINE_TSESS *pTSESS)
{
	S32		dRet, i;
	struct timeval		stTime;
	U8					*pNode;
	LOG_ONLINE_TRANS	*pLOG;
	ONLINE_INFO			*pINFO;

	curSessCnt--;

	pLOG = (LOG_ONLINE_TRANS *)nifo_ptr(pMEMSINFO, pTSESS->offset_LOG);

	gettimeofday(&stTime, NULL);

	pLOG->uiOpEndTime = stTime.tv_sec;
	pLOG->uiOpEndMTime = stTime.tv_usec;
	STG_DiffTIME64(pLOG->uiLastPktTime, pLOG->uiLastPktMTime, pLOG->uiFirstReqStartTime, pLOG->uiFirstReqStartMTime, &pLOG->llUseGapTime);

	for(i = 0; i < 2; i++)
	{
		pINFO = &pTSESS->ONLINEINFO[i];
		if((pINFO->oldDataSize != 0) || (pINFO->remainHDRSize != 0))
		{
			pLOG->usUserErrorCode = (pLOG->usUserErrorCode == ONLINE_UERR_EMPTY) ? ONLINE_UERR_105 : pLOG->usUserErrorCode;
//			pLOG->usL4FailCode = pLOG->usUserErrorCode;
		}
	}

//	LOG_ONLINE_TRANS_Prt("PRINT LOG_ONLINE_TRANS", pLOG);

	pNode = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *)pLOG));

	if((dRet = dSend_ONLINE_Data(pMEMSINFO, dGetCALLProcID(pLOG->uiClientIP), pNode)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dSendSignal dRet[%d]", 
				__FILE__, __FUNCTION__, __LINE__, dRet);
		return -12;
	}

	hasho_del(pHASH, (U8 *)pTSESSKEY);

	return 0;
}


/*
 * $Log: online_func.c,v $
 * Revision 1.2  2011/09/05 08:20:23  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/17 13:06:40  hhbaek
 * A_ONLINE
 *
 * Revision 1.2  2011/08/09 08:17:40  uamyd
 * add blocks
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.6  2011/05/09 13:59:17  dark264sh
 * A_ONLINE: A_CALL multi 처리
 *
 * Revision 1.5  2011/01/11 04:09:09  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.4  2009/09/13 08:53:50  jsyoon
 * PI프로세스의 uiNASName 필드값 제거
 *
 * Revision 1.3  2009/08/19 12:27:43  pkg
 * LOG_XXX_Prt 함수 주석 처리
 *
 * Revision 1.2  2009/07/15 16:44:34  dqms
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:42  dqms
 * Init TAF_RPPI
 *
 * Revision 1.4  2008/12/10 06:01:36  jsyoon
 * bug
 *
 * Revision 1.3  2008/11/24 12:44:46  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2008/09/18 07:46:02  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.1  2007/08/21 12:53:54  dark264sh
 * no message
 *
 * Revision 1.15  2006/12/12 08:54:37  dark264sh
 * *** empty log message ***
 *
 * Revision 1.14  2006/12/04 08:03:52  dark264sh
 * *** empty log message ***
 *
 * Revision 1.13  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.12  2006/11/28 02:25:33  dark264sh
 * *** empty log message ***
 *
 * Revision 1.11  2006/11/22 06:32:30  dark264sh
 * *** empty log message ***
 *
 * Revision 1.10  2006/11/21 08:23:45  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2006/11/13 07:10:00  dark264sh
 * LOG_ONLINE_TRANS memset 하도록 수정
 *
 * Revision 1.8  2006/11/10 14:38:19  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2006/11/10 14:21:08  dark264sh
 * *** empty log message ***
 *
 * Revision 1.6  2006/11/06 07:38:29  dark264sh
 * nifo NODE size 4*1024 => 6*1024로 변경하기
 * nifo_tlv_alloc에서 argument로 memset할지 말지 결정하도록 수정
 * nifo_node_free에서 semaphore 삭제
 *
 * Revision 1.5  2006/11/01 09:25:20  dark264sh
 * SESS, SEQ, NODE 개수 LOG추가
 *
 * Revision 1.4  2006/10/31 05:11:59  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2006/10/30 08:56:32  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2006/10/30 05:05:05  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2006/10/27 12:35:51  dark264sh
 * *** empty log message ***
 *
 */
