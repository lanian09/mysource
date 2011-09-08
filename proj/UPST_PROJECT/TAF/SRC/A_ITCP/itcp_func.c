/**		@file	tcp_func.c
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: itcp_func.c,v 1.2 2011/09/05 12:26:39 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 12:26:39 $
 * 		@ref		tcp_main.c
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
 * Include headers
 */
#include <sys/time.h>
#include <unistd.h>

// TOP
#include "commdef.h"
#include "common_stg.h"
#include "filter.h"
#include "func_time_check.h"	/* st_FuncTimeCheckList */

// LIB headers
#include "loglib.h"
#include "utillib.h"
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */

// TAF headers
#include "debug.h"

// .
#include "itcp_sess.h"
#include "itcp_func.h"

/**
 * Declare variables;
 */
CALL_KEY 					stCALLKEY;
SESSKEY_TBL 				*pstSESSKEYTBL;
SESSKEY_LIST 				*pstSESSKEYList;
SESSKEY_LIST 				*pstNEWSESSKEYList;
stHASHONODE					*pSESSKEYNODE;
				
int                 		guiTimerValue;
extern ATCP_SUBINFO			*pATCPSUBINFO;
extern stHASHOINFO			*pSESSKEYINFO;  	/* TCP SESSION 정리를 위한 HASH */
extern st_Flt_Info  		*flt_info;
extern st_FuncTimeCheckList	*pFUNC;

/**
 *	Declare extern func.
 */
extern int save_pkt(Capture_Header_Msg *header, INFO_ETH *info, TCP_SESS *tcpsess, int retrans);
extern void invoke_del(void *p);

/** dHttpInit function.
 *
 *  dHttpInit Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
S32 dProcTcpSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, stTIMERNINFO *pTIMER, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, U8 *pNode)
{
START_FUNC_TIME_CHECK(pFUNC, 10);
	U8					ucRtxType;		/* 현재 전송된 패킷의 방향 */
	U8					ucSubRtxType;
	U8					ucControl;
	S32					delFlag, retrans = 0;
	S32					dRet;
	
	TCP_SESS_KEY		TCPSESSKEY;
	TCP_SESS			*pTCPSESS;
	LOG_ITCP_SESS		*pTCPLOG;

	stHASHONODE			*pHASHNODE;

    U8              	szSIP[INET_ADDRSTRLEN];
    U8              	szDIP[INET_ADDRSTRLEN];


	/* TCP HASH KEY 생성 */
	MakeTCPHashKey(pCAPHEAD, pINFOETH, &TCPSESSKEY);

	/* TCP Control Check */
	ucControl = GetTCPControl(pINFOETH->stUDPTCP.nControlType);

	/* 방향 */
	ucRtxType = pCAPHEAD->bRtxType;

	pATCPSUBINFO->rcvNodeCnt++;
	pATCPSUBINFO->rcvSize += pINFOETH->stUDPTCP.wDataLen;
	
	log_print(LOGN_DEBUG, "### NEW MGS SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]CONTROL[%s]DATASIZE[%d]", 
					util_cvtipaddr(szSIP, TCPSESSKEY.uiSIP), TCPSESSKEY.usSPort, 
					util_cvtipaddr(szDIP, TCPSESSKEY.uiDIP), TCPSESSKEY.usDPort,
					pCAPHEAD->curtime, pCAPHEAD->ucurtime,
					PrintRtx(ucRtxType), PrintControl(ucControl), pINFOETH->stUDPTCP.wDataLen);

	/* 기존 세션이 존재 하지 않는 경우 */
	if((pHASHNODE = hasho_find(pTCPHASH, (U8 *)&TCPSESSKEY)) == NULL)
	{
		/* Control에 따라 세션을 생성 할지 패킷을 버릴지 판단 */
		log_print(LOGN_DEBUG, "NOT EXIST SESSION");

		switch (ucControl)
		{
		case TCP_SYN:
		case TCP_SYNACK:
		case TCP_ACK:
			log_print(LOGN_DEBUG, "NEW SESSION");
			/* 새로운 세션 생성 */
			if((pTCPSESS = pCreateSession(
							pMEMSINFO, pTCPHASH, pTIMER, &TCPSESSKEY, pCAPHEAD, pINFOETH, ucRtxType, ucControl)) == NULL) 
			{
				log_print(LOGN_CRI, 
						LH"pCreateSession NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]CONTROL[%s]", 
						LT,
						util_cvtipaddr(szSIP, TCPSESSKEY.uiSIP), TCPSESSKEY.usSPort, 
						util_cvtipaddr(szDIP, TCPSESSKEY.uiDIP), TCPSESSKEY.usDPort,
						pCAPHEAD->curtime, pCAPHEAD->ucurtime,
						PrintRtx(ucRtxType), PrintControl(ucControl));
				nifo_node_delete(pMEMSINFO, pNode);
				pATCPSUBINFO->delCnt++;
END_FUNC_TIME_CHECK(pFUNC, 10);
				return 0;
			}

			/* ADD SESSION KEY LIST */
			if ((dRet = dCheckSessKeyList(pMEMSINFO, &TCPSESSKEY, pTCPSESS)) < 0) {
				log_print(LOGN_CRI, LH"#### dCheckSessKeyList() dRet[%d]", LT, dRet);
				return -1;
			}

			pTCPLOG = (LOG_ITCP_SESS *)nifo_ptr(pMEMSINFO, pTCPSESS->offset_LOG);
			
#if 1
			if(ucControl == TCP_ACK)
			{
				if((dRet = dTCPStartMsg(pMEMSINFO, &TCPSESSKEY, pTCPSESS, pINFOETH)) < 0) {
					log_print(LOGN_CRI, LH"dTCPStartMsg dRet[%d]", LT, dRet);
					TCP_SESS_KEY_Prt((S8 *)__FUNCTION__, &TCPSESSKEY);
					TCP_SESS_Prt((S8 *)__FUNCTION__, pTCPSESS);
				}
			}
#endif
			break;
			
		default:
			/* Drop Packet */
			log_print(LOGN_DEBUG, "DROP PACKET");
			nifo_node_delete(pMEMSINFO, pNode);
			pATCPSUBINFO->delCnt++;
END_FUNC_TIME_CHECK(pFUNC, 10);
			return 0;
		}
	}
	/* 기존 세션이 존재 하는 경우 */
	else
	{
		pTCPSESS = (TCP_SESS *)nifo_ptr(pTCPHASH, pHASHNODE->offset_Data);

		log_print(LOGN_DEBUG, "EXIST SESSION SYN[%u]SYNACK[%u]SYNACKACK[%u]SEQ[%u]ACK[%u]", 
				pTCPSESS->uiSynSeq, pTCPSESS->uiSynAckSeq, pTCPSESS->uiSynAckAck,
				pINFOETH->stUDPTCP.seq, pINFOETH->stUDPTCP.ack);


		guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_TCP_TIMEOUT];
		/* DEF_TCP_SESSUPDATE = guiTimerValue - 10 */
		if(pCAPHEAD->curtime >= pTCPSESS->uiLastSessUpdateTime + guiTimerValue - 10) {
			dSessUpdataMsg(pMEMSINFO, &TCPSESSKEY, pTCPSESS);
			pTCPSESS->uiLastSessUpdateTime = pCAPHEAD->curtime;
			pATCPSUBINFO->updateCnt++;
		}

		pTCPSESS->timerNID = timerN_update(pTIMER, pTCPSESS->timerNID, time(NULL) + guiTimerValue);

		pTCPLOG = (LOG_ITCP_SESS *)nifo_ptr(pMEMSINFO, pTCPSESS->offset_LOG);

		switch(ucControl)
		{
		case TCP_SYN:
			/* SYN 재전송된 경우 */
//			if(pTCPSESS->uiSynSeq == pINFOETH->stUDPTCP.seq)
			if(pTCPSESS->ucRcvSyn == SYN_RCV_ON)
			{
				log_print(LOGN_DEBUG, "RETRANS SYN");
				/* 재전송 처리 */
				/* SYN COUNT 증가 */
				pTCPLOG->ucTcpSynCnt++;
				pTCPSESS->uiIPTotUpRetransCnt++;
				pTCPSESS->uiIPTotUpRetransSize += pINFOETH->stIP.wTotalLength;
				pTCPLOG->uiIPTotUpRetransCnt++;
				pTCPLOG->uiIPTotUpRetransSize += pINFOETH->stIP.wTotalLength;
				retrans = 1;
			}
			/* SYN과 SYNACK가 역전된 경우 */
//			else if((pTCPSESS->uiSynSeq == 0) && (pTCPSESS->uiSynAckAck == pINFOETH->stUDPTCP.seq + 1))
			else if((pTCPSESS->ucRcvSyn == SYN_RCV_OFF) && (pTCPSESS->uiSynAckAck == pINFOETH->stUDPTCP.seq + 1))
			{
				log_print(LOGN_DEBUG, "SYNACK->SYN");

				/* 역전 처리 */
				/* TCP SYN TIME Setting */
				pTCPSESS->ucRcvSyn = SYN_RCV_ON;
				pTCPSESS->ucTcpClientStatus = DEF_CLI_SYN;
				pTCPLOG->ucTcpClientStatus = DEF_CLI_SYN;
				pTCPLOG->uiITcpSynTime = pCAPHEAD->curtime;
				pTCPLOG->uiITcpSynMTime = pCAPHEAD->ucurtime;
				pTCPLOG->ucTcpSynCnt++;
				pTCPLOG->usTcpUpMSS = pINFOETH->stUDPTCP.mss;
				pTCPLOG->usTcpUpFirstWindowSize = pINFOETH->stUDPTCP.window;
			}
			/* 새로운 SYN이 온 경우 */
			else
			{
				log_print(LOGN_DEBUG, "NEW SYN");

				/* 기존에 존재하는 세션 종료 */
				pTCPSESS->ucEndStatus = DEF_END_ABNORMAL;
				dCloseSession(pMEMSINFO, pTCPHASH, &TCPSESSKEY, pTCPSESS);
				timerN_del(pTIMER, pTCPSESS->timerNID);
				/* Delete Call Session List */
				Delete_SessList(&TCPSESSKEY);

				log_print(LOGN_DEBUG, "CLOSE SESSION");

				/* 새로운 세션 생성 */
				if((pTCPSESS = pCreateSession(pMEMSINFO, pTCPHASH, pTIMER, &TCPSESSKEY, pCAPHEAD, pINFOETH, ucRtxType, ucControl)) == NULL) 
				{
					log_print(LOGN_CRI, 
					LH"pCreateSession NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]CONTROL[%s]", 
					LT,
					util_cvtipaddr(szSIP, TCPSESSKEY.uiSIP), TCPSESSKEY.usSPort, 
					util_cvtipaddr(szDIP, TCPSESSKEY.uiDIP), TCPSESSKEY.usDPort,
					pCAPHEAD->curtime, pCAPHEAD->ucurtime,
					PrintRtx(ucRtxType), PrintControl(ucControl));
					nifo_node_delete(pMEMSINFO, pNode);
					pATCPSUBINFO->delCnt++;
END_FUNC_TIME_CHECK(pFUNC, 10);
					return 0;
				}
				log_print(LOGN_DEBUG, "CREATE SESSION");

				/* ADD SESSION KEY LIST */
				if ((dRet = dCheckSessKeyList(pMEMSINFO, &TCPSESSKEY, pTCPSESS)) < 0) {
					log_print(LOGN_CRI, LH"#### dCheckSessKeyList() dRet[%d]", LT, dRet);
					return -1;
				}
				
				pTCPLOG = (LOG_ITCP_SESS *)nifo_ptr(pMEMSINFO, pTCPSESS->offset_LOG);

			}
			break;
		
		case TCP_SYNACK:
			/* SYN, SYNACK 순서가 정상적으로 들어온 경우 */
			if((pTCPSESS->uiSynAckSeq == 0) && (pTCPSESS->uiSynSeq + 1 == pINFOETH->stUDPTCP.ack))
			{
				log_print(LOGN_DEBUG, "SYN->SYNACK");
				/* SYNACK 해당 처리 및 값 할당 */
				/* TCP SYNACK TIME Setting */
				pTCPSESS->ucStatus = DEF_STATUS_SYNACK;
				pTCPSESS->uiSynAckSeq = pINFOETH->stUDPTCP.seq;
				pTCPSESS->uiSynAckAck = pINFOETH->stUDPTCP.ack;
				pTCPSESS->uiLastResSeq = pINFOETH->stUDPTCP.seq;
				pTCPSESS->uiNextResSeq = pINFOETH->stUDPTCP.seq + 1;
				pTCPSESS->ucTcpServerStatus = DEF_SVR_SYNACK;
				pTCPLOG->ucTcpServerStatus = DEF_SVR_SYNACK;
				pTCPLOG->uiTcpSynAckTime = pCAPHEAD->curtime;
				pTCPLOG->uiTcpSynAckMTime = pCAPHEAD->ucurtime;
				pTCPLOG->ucTcpSynAckCnt++;
				pTCPLOG->usTcpDnMSS = pINFOETH->stUDPTCP.mss;
				pTCPLOG->usTcpDnFirstWindowSize = pINFOETH->stUDPTCP.window;
			}
			/* SYNACK가 재전송 된 경우 */
			else if(pTCPSESS->uiSynAckSeq == pINFOETH->stUDPTCP.seq)
			{
				log_print(LOGN_DEBUG, "RETRANS SYNACK");
				/* 재전송 처리 */
				/* SYNACK COUNT 중가 */
				pTCPLOG->ucTcpSynAckCnt++;
				pTCPSESS->uiIPTotDnRetransCnt++;
				pTCPSESS->uiIPTotDnRetransSize += pINFOETH->stIP.wTotalLength;
				pTCPLOG->uiIPTotDnRetransCnt++;
				pTCPLOG->uiIPTotDnRetransSize += pINFOETH->stIP.wTotalLength;
				retrans = 1;
			}
			/* SYN, SYNACK가 역전 되면서 새로운 세션이 발생한 경우 */
			else
			{
				log_print(LOGN_DEBUG, "NEW SYNACK");

				/* 기존 세션 종료 */
				pTCPSESS->ucEndStatus = DEF_END_ABNORMAL;
				dCloseSession(pMEMSINFO, pTCPHASH, &TCPSESSKEY, pTCPSESS);
				timerN_del(pTIMER, pTCPSESS->timerNID);
				/* Delete Call Session List */
				Delete_SessList(&TCPSESSKEY);

				log_print(LOGN_DEBUG, "CLOSE SESSION");

				/* 새로운 세션 생성 */
				if((pTCPSESS = pCreateSession(pMEMSINFO, pTCPHASH, pTIMER, &TCPSESSKEY, pCAPHEAD, pINFOETH, ucRtxType, ucControl)) == NULL) 
				{
					log_print(LOGN_CRI, 
					LH"pCreateSession NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]CONTROL[%s]", 
					LT,
					util_cvtipaddr(szSIP, TCPSESSKEY.uiSIP), TCPSESSKEY.usSPort, 
					util_cvtipaddr(szDIP, TCPSESSKEY.uiDIP), TCPSESSKEY.usDPort,
					pCAPHEAD->curtime, pCAPHEAD->ucurtime,
					PrintRtx(ucRtxType), PrintControl(ucControl));
					nifo_node_delete(pMEMSINFO, pNode);
					pATCPSUBINFO->delCnt++;
END_FUNC_TIME_CHECK(pFUNC, 10);
					return 0;
				}
				log_print(LOGN_DEBUG, "CREATE SESSION");

				/* ADD SESSION KEY LIST */
				if ((dRet = dCheckSessKeyList(pMEMSINFO, &TCPSESSKEY, pTCPSESS)) < 0) {
					log_print(LOGN_CRI, LH"#### dCheckSessKeyList() dRet[%d]", LT, dRet);
					return -1;
				}
				
				pTCPLOG = (LOG_ITCP_SESS *)nifo_ptr(pMEMSINFO, pTCPSESS->offset_LOG);

			}
			break;
		
		case TCP_RST:
			log_print(LOGN_DEBUG, "RST");

			/* 세션 상태에 따른 에러 코드 생성 */
			pTCPSESS->ucEndStatus = DEF_END_RST;

			/* 세션 종료 */
			pTCPLOG->uiITcpFinTime = pCAPHEAD->curtime;
			pTCPLOG->uiITcpFinMTime = pCAPHEAD->ucurtime;
			if(ucRtxType == DEF_FROM_SERVER) {
				pTCPSESS->ucRstRtx = DEF_FROM_SERVER;
				pTCPLOG->ucTcpDnRstCnt++;
			} else {
				pTCPSESS->ucRstRtx = DEF_FROM_CLIENT;
				pTCPLOG->ucTcpUpRstCnt++;
			}

			if(pTCPSESS->ucSynRtx == ucRtxType) {
				pTCPSESS->ucTcpClientStatus = DEF_CLI_RST;
				pTCPLOG->ucTcpClientStatus = DEF_CLI_RST;
			} else {
				pTCPSESS->ucTcpServerStatus = DEF_SVR_RST;
				pTCPLOG->ucTcpServerStatus = DEF_SVR_RST;
			}

			pINFOETH->stUDPTCP.wDataLen = 0;

			break;
			
		case TCP_FIN:
			/* 첫번째 FIN */
			if(pTCPSESS->ucFinStatus == DEF_FIN_0)
			{
				log_print(LOGN_DEBUG, "FIN 1");

				/* 첫번째 FIN 시간 할당 */
				pTCPLOG->uiITcpFinTime = pCAPHEAD->curtime;
				pTCPLOG->uiITcpFinMTime = pCAPHEAD->ucurtime;

				pTCPSESS->ucFinStatus = DEF_FIN_1;
				pTCPSESS->uiFinChkAck[ucRtxType] = pINFOETH->stUDPTCP.seq + pINFOETH->stUDPTCP.wDataLen + 1;

				if(ucRtxType == DEF_FROM_SERVER) {
					pTCPSESS->ucFinRtx = DEF_FROM_SERVER;
					pTCPLOG->ucTcpDnFinCnt++;
				} else {
					pTCPSESS->ucFinRtx = DEF_FROM_CLIENT;
					pTCPLOG->ucTcpUpFinCnt++;
				}

				if(pTCPSESS->ucSynRtx == ucRtxType) {
					pTCPSESS->ucTcpClientStatus = DEF_CLI_FIN;
					pTCPLOG->ucTcpClientStatus = DEF_CLI_FIN;
				} else {
					pTCPSESS->ucTcpServerStatus = DEF_SVR_FIN;
					pTCPLOG->ucTcpServerStatus = DEF_SVR_FIN;
				}
			}
			/* 두번째 FIN */
			else if(pTCPSESS->ucFinStatus == DEF_FIN_1)
			{

				/* FIN 재전송 된 경우 */
				if(pTCPSESS->ucFinRtx == ucRtxType) {
					if(pTCPSESS->uiFinChkAck[ucRtxType] != pINFOETH->stUDPTCP.seq + pINFOETH->stUDPTCP.wDataLen + 1) {
						pTCPSESS->uiFinChkAck[ucRtxType] = pINFOETH->stUDPTCP.seq + pINFOETH->stUDPTCP.wDataLen + 1;
						pTCPSESS->uiFinChkSeq[ucRtxType] = 0;
					}

					if(ucRtxType == DEF_FROM_SERVER) {
						pTCPLOG->ucTcpDnFinCnt++;
					} else {
						pTCPLOG->ucTcpUpFinCnt++;
					}
				} else {
					log_print(LOGN_DEBUG, "FIN 2"); 

					pTCPSESS->ucFinStatus = DEF_FIN_2;
					pTCPSESS->uiFinChkAck[ucRtxType] = pINFOETH->stUDPTCP.seq + pINFOETH->stUDPTCP.wDataLen + 1;

					ucSubRtxType = (ucRtxType == DEF_FROM_SERVER) ? DEF_FROM_CLIENT : DEF_FROM_SERVER;
					if((pTCPSESS->uiFinChkAck[ucSubRtxType] > 0) && (pTCPSESS->uiFinChkAck[ucSubRtxType] <= pINFOETH->stUDPTCP.ack)) {
						pTCPSESS->uiFinChkSeq[ucSubRtxType] = pINFOETH->stUDPTCP.seq;

						if(pTCPSESS->ucSynRtx == ucSubRtxType) {
							pTCPSESS->ucTcpClientStatus = DEF_CLI_FINACK;
							pTCPLOG->ucTcpClientStatus = DEF_CLI_FINACK;
						} else {
							pTCPSESS->ucTcpServerStatus = DEF_SVR_FINACK;
							pTCPLOG->ucTcpServerStatus = DEF_SVR_FINACK;
						}
					}

					if(ucRtxType == DEF_FROM_SERVER) {
						pTCPLOG->ucTcpDnFinCnt++;
					} else {
						pTCPLOG->ucTcpUpFinCnt++;
					}

					if(pTCPSESS->ucSynRtx == ucRtxType) {
						pTCPSESS->ucTcpClientStatus = DEF_CLI_FIN;
						pTCPLOG->ucTcpClientStatus = DEF_CLI_FIN;
					} else {
						pTCPSESS->ucTcpServerStatus = DEF_SVR_FIN;
						pTCPLOG->ucTcpServerStatus = DEF_SVR_FIN;
					}
				}
			} 
			else if(pTCPSESS->ucFinStatus == DEF_FIN_2) 
			{
//				if(pTCPSESS->ucFinRtx == ucRtxType) {
					if(pTCPSESS->uiFinChkAck[ucRtxType] != pINFOETH->stUDPTCP.seq + pINFOETH->stUDPTCP.wDataLen + 1) {
						pTCPSESS->uiFinChkAck[ucRtxType] = pINFOETH->stUDPTCP.seq + pINFOETH->stUDPTCP.wDataLen + 1;
						pTCPSESS->uiFinChkSeq[ucRtxType] = 0;
					}
//				}

				if(ucRtxType == DEF_FROM_SERVER) {
					pTCPLOG->ucTcpDnFinCnt++;
				} else {
					pTCPLOG->ucTcpUpFinCnt++;
				}
			}

			break;
			
		case TCP_ACK:
			/* SYN에 대한 ACK인지 판단 */
//			if(((pTCPSESS->ucStatus == DEF_STATUS_SYN) || (pTCPSESS->ucStatus == DEF_STATUS_SYNACK)) && (pTCPSESS->uiSynAckAck <= pINFOETH->stUDPTCP.seq))

			if((pTCPSESS->ucStatus == DEF_STATUS_SYN) || (pTCPSESS->ucStatus == DEF_STATUS_SYNACK))
			{
				log_print(LOGN_DEBUG, "SYN->SYNACK->ACK");
				
				/* 세션 생성 완료 필드 값 세팅 */
				pTCPSESS->ucStatus = DEF_STATUS_ACK;
				pTCPSESS->ucTcpClientStatus = DEF_CLI_ACK;
				pTCPLOG->ucTcpClientStatus = DEF_CLI_ACK;
				pTCPLOG->uiTcpSynAckAckTime = pCAPHEAD->curtime;
				pTCPLOG->uiTcpSynAckAckMTime = pCAPHEAD->ucurtime;

				if(pTCPSESS->uiSynAckSeq == 0) {
#if 0
					if(ucRtxType == DEF_FROM_SERVER) {
						pTCPSESS->uiSynAckSeq = pINFOETH->stUDPTCP.seq;
						pTCPSESS->uiLastResSeq = pINFOETH->stUDPTCP.seq;
						pTCPSESS->uiNextResSeq = pINFOETH->stUDPTCP.seq + pINFOETH->stUDPTCP.wDataLen;
					} else {
						pTCPSESS->uiSynAckSeq = pINFOETH->stUDPTCP.ack - 1;
						pTCPSESS->uiLastResSeq = pINFOETH->stUDPTCP.ack - 1;
						pTCPSESS->uiNextResSeq = pINFOETH->stUDPTCP.ack;
					}
#endif

					if(pTCPSESS->ucSynRtx == ucRtxType) {
						pTCPSESS->uiSynAckSeq = pINFOETH->stUDPTCP.ack - 1;
						pTCPSESS->uiLastResSeq = pINFOETH->stUDPTCP.ack - 1;
						pTCPSESS->uiNextResSeq = pINFOETH->stUDPTCP.ack;
					} else {
#if 0
						pTCPSESS->uiSynAckSeq = pINFOETH->stUDPTCP.seq;
						pTCPSESS->uiLastResSeq = pINFOETH->stUDPTCP.seq;
						pTCPSESS->uiNextResSeq = pINFOETH->stUDPTCP.seq + pINFOETH->stUDPTCP.wDataLen;
#endif

						pTCPSESS->uiSynAckSeq = pINFOETH->stUDPTCP.seq - pINFOETH->stUDPTCP.wDataLen - 1;;
						pTCPSESS->uiLastResSeq = pINFOETH->stUDPTCP.seq - pINFOETH->stUDPTCP.wDataLen - 1;;
						pTCPSESS->uiNextResSeq = pINFOETH->stUDPTCP.seq;
					}

				}

				if((dRet = dTCPStartMsg(pMEMSINFO, &TCPSESSKEY, pTCPSESS, pINFOETH)) < 0) {
					log_print(LOGN_CRI, LH"dTCPStartMsg dRet[%d]", LT, dRet);
					TCP_SESS_KEY_Prt((S8 *)__FUNCTION__, &TCPSESSKEY);
					TCP_SESS_Prt((S8 *)__FUNCTION__, pTCPSESS);
				}
			}
			else if((pTCPSESS->ucStatus == DEF_STATUS_DATA) && (pTCPSESS->ucSynRtx != ucRtxType))
			{
				log_print(LOGN_DEBUG, "DATA->DATA");

				/* 세션 생성 완료 필드 값 세팅 */
				pTCPSESS->ucStatus = DEF_STATUS_ACK;
//				pTCPSESS->ucTcpClientStatus = DEF_CLI_ACK;
//				pTCPLOG->ucTcpClientStatus = DEF_CLI_ACK;
				pTCPLOG->uiTcpSynAckAckTime = pCAPHEAD->curtime;
				pTCPLOG->uiTcpSynAckAckMTime = pCAPHEAD->ucurtime;

#if 0
				if((dRet = dTCPStartMsg(pMEMSINFO, &TCPSESSKEY, pTCPSESS, pINFOETH)) < 0) {
					log_print(LOGN_CRI, LH"dTCPStartMsg dRet[%d]", LT, dRet);
					TCP_SESS_KEY_Prt((S8 *)__FUNCTION__, &TCPSESSKEY);
					TCP_SESS_Prt((S8 *)__FUNCTION__, pTCPSESS);
				}
#endif
			}
			/* FIN에 대한 ACK인지 판단 */
			if(pTCPSESS->ucFinStatus > DEF_FIN_0)
			{
				ucSubRtxType = (ucRtxType == DEF_FROM_SERVER) ? DEF_FROM_CLIENT : DEF_FROM_SERVER;
				if((pTCPSESS->uiFinChkAck[ucSubRtxType] > 0) && (pTCPSESS->uiFinChkAck[ucSubRtxType] <= pINFOETH->stUDPTCP.ack)) {
					pTCPSESS->uiFinChkSeq[ucSubRtxType] = pINFOETH->stUDPTCP.seq;

					if(pTCPSESS->ucSynRtx == ucSubRtxType) {
						pTCPSESS->ucTcpClientStatus = DEF_CLI_FINACK;
						pTCPLOG->ucTcpClientStatus = DEF_CLI_FINACK;
					} else {
						pTCPSESS->ucTcpServerStatus = DEF_SVR_FINACK;
						pTCPLOG->ucTcpServerStatus = DEF_SVR_FINACK;
					}

					if((pTCPSESS->uiFinChkSeq[DEF_FROM_CLIENT] > 0) && (pTCPSESS->uiFinChkSeq[DEF_FROM_SERVER] > 0))
					{
						log_print(LOGN_DEBUG, "FIN->FINACK->ACK");
						/* FIN 상태값 변경 */
						pTCPSESS->ucEndStatus = DEF_END_NORMAL;
						pTCPSESS->ucFinStatus = DEF_FIN_3;
						pTCPLOG->uiITcpFinAckTime = pCAPHEAD->curtime;
						pTCPLOG->uiITcpFinAckMTime = pCAPHEAD->ucurtime;
					}
				}
			}

			break;
			
		default:
			log_print(LOGN_CRI, 
			LH"STRANGE CONTROL[%d] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]RTX[%s]", 
				LT, ucControl,
				util_cvtipaddr(szSIP, TCPSESSKEY.uiSIP), TCPSESSKEY.usSPort, 
				util_cvtipaddr(szDIP, TCPSESSKEY.uiDIP), TCPSESSKEY.usDPort,
				pCAPHEAD->curtime, pCAPHEAD->ucurtime,
				PrintRtx(ucRtxType));
			nifo_node_delete(pMEMSINFO, pNode);
			pATCPSUBINFO->delCnt++;
END_FUNC_TIME_CHECK(pFUNC, 10);
			return 0;
		}


	}

	/* 공통 처리 */
	pTCPSESS->uiLastUpdateTime = pCAPHEAD->curtime;
	pTCPSESS->uiLastUpdateMTime = pCAPHEAD->ucurtime;

	if(ucRtxType == DEF_FROM_SERVER) {
		if(pTCPSESS->ucStatus == DEF_STATUS_ACK) {
			pTCPSESS->uiIPDataDnPktCnt++;
			pTCPSESS->uiIPDataDnPktSize += pINFOETH->stIP.wTotalLength;
			pTCPLOG->uiIPDataDnPktCnt++;
			pTCPLOG->uiIPDataDnPktSize += pINFOETH->stIP.wTotalLength;
		}
		pTCPSESS->uiIPTotDnPktCnt++;
		pTCPSESS->uiIPTotDnPktSize += pINFOETH->stIP.wTotalLength;
		pTCPLOG->uiIPTotDnPktCnt++;
		pTCPLOG->uiIPTotDnPktSize += pINFOETH->stIP.wTotalLength;

		pTCPLOG->uiITcpDnLastPktTime = pCAPHEAD->curtime;
		pTCPLOG->uiITcpDnLastPktMTime = pCAPHEAD->ucurtime;
	} else {
		if(pTCPSESS->ucStatus == DEF_STATUS_ACK) {
			pTCPSESS->uiIPDataUpPktCnt++;
			pTCPSESS->uiIPDataUpPktSize += pINFOETH->stIP.wTotalLength;
			pTCPLOG->uiIPDataUpPktCnt++;
			pTCPLOG->uiIPDataUpPktSize += pINFOETH->stIP.wTotalLength;
		}
		pTCPSESS->uiIPTotUpPktCnt++;
		pTCPSESS->uiIPTotUpPktSize += pINFOETH->stIP.wTotalLength;
		pTCPLOG->uiIPTotUpPktCnt++;
		pTCPLOG->uiIPTotUpPktSize += pINFOETH->stIP.wTotalLength;

		pTCPLOG->uiITcpUpLastPktTime = pCAPHEAD->curtime;
		pTCPLOG->uiITcpUpLastPktMTime = pCAPHEAD->ucurtime;
	}

	pTCPLOG->uiTcpLastPktTime = pCAPHEAD->curtime;
	pTCPLOG->uiTcpLastPktMTime = pCAPHEAD->ucurtime;

	log_print(LOGN_DEBUG, "ACK CHECK");
	/* Ack를 확인하여 Ack를 받은 데이터 전송 */
	dCheckAck(pMEMSINFO, &TCPSESSKEY, pTCPSESS, pCAPHEAD, pINFOETH, ucRtxType);

	if(pINFOETH->stUDPTCP.wDataLen > 0) 
	{
		log_print(LOGN_DEBUG, "INSERT DATA");
		/* Ack를 기다리는 데이터를 Linked List로 구성 */
		delFlag = 0;

		if(ucRtxType == DEF_FROM_SERVER) {
			switch(pTCPSESS->ucTcpServerStatus)
			{
			case DEF_SVR_SUCCESS:
			case DEF_SVR_ETC:
			case DEF_SVR_SYNACK:
				pTCPSESS->ucTcpServerStatus = DEF_SVR_DATA;
				pTCPLOG->ucTcpServerStatus = DEF_SVR_DATA;
				break;
			case DEF_SVR_DATA:
			case DEF_SVR_RST:
			case DEF_SVR_FIN:
			case DEF_SVR_FINACK:
			default:
				break;
			}
			pTCPLOG->uiTcpDnBodySize += pINFOETH->stUDPTCP.wDataLen;
		} else {
			switch(pTCPSESS->ucTcpClientStatus)
			{
			case DEF_CLI_SUCCESS:
			case DEF_CLI_ETC:
			case DEF_CLI_SYN:
			case DEF_CLI_ACK:
				pTCPSESS->ucTcpClientStatus = DEF_CLI_DATA;
				pTCPLOG->ucTcpClientStatus = DEF_CLI_DATA;
				break;
			case DEF_CLI_DATA:
			case DEF_CLI_RST:
			case DEF_CLI_FIN:
			case DEF_CLI_FINACK:
			default:
				break;
			}
			pTCPLOG->uiTcpUpBodySize += pINFOETH->stUDPTCP.wDataLen;
		}

#ifdef DEBUG
	U8					*pData;
	pData = (ucRtxType == DEF_FROM_SERVER) ? nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData) : nifo_ptr(pMEMSINFO, pTCPSESS->offset_ReqData);
    if(pData != NULL) {
        log_print(LOGN_INFO, LH"BEFORE PRINT SORTSEQ", LT);
        nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
    } else {
        log_print(LOGN_INFO, LH"BEFORE PRINT SORTSEQ NO HAVE DATA", LT);
    }
#endif
		dInsertTCPData(pMEMSINFO, &TCPSESSKEY, pTCPSESS, pCAPHEAD, pINFOETH, pNode, &delFlag);
#ifdef DEBUG
	pData = (ucRtxType == DEF_FROM_SERVER) ? nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData) : nifo_ptr(pMEMSINFO, pTCPSESS->offset_ReqData);
    if(pData != NULL) {
        log_print(LOGN_INFO, LH"AFTER PRINT SORTSEQ", LT);
        nifo_print_nont(pMEMSINFO, pData, PrintSEQ, (U8 *)__FUNCTION__);
    } else {
        log_print(LOGN_INFO, LH"AFTER PRINT SORTSEQ NO HAVE DATA", LT);
    }
#endif
		if(delFlag > 0) {
			/* 재전송 데이터 */
			if(ucRtxType == DEF_FROM_SERVER) {
				pTCPLOG->uiTcpDnRetransBodySize += delFlag;

				pTCPSESS->uiIPDataDnRetransCnt++;
				pTCPSESS->uiIPTotDnRetransCnt++;
				pTCPSESS->uiIPDataDnRetransSize += delFlag;
				pTCPSESS->uiIPTotDnRetransSize += delFlag;
				pTCPLOG->uiIPDataDnRetransCnt++;
				pTCPLOG->uiIPTotDnRetransCnt++;
				pTCPLOG->uiIPDataDnRetransSize += delFlag;
				pTCPLOG->uiIPTotDnRetransSize += delFlag;
			} else {
				pTCPLOG->uiTcpUpRetransBodySize += delFlag;

				pTCPSESS->uiIPDataUpRetransCnt++;
				pTCPSESS->uiIPTotUpRetransCnt++;
				pTCPSESS->uiIPDataUpRetransSize += delFlag;
				pTCPSESS->uiIPTotUpRetransSize += delFlag;
				pTCPLOG->uiIPDataUpRetransCnt++;
				pTCPLOG->uiIPTotUpRetransCnt++;
				pTCPLOG->uiIPDataUpRetransSize += delFlag;
				pTCPLOG->uiIPTotUpRetransSize += delFlag;
			}
			retrans = 1;
		}

//		save_pkt(pCAPHEAD, pINFOETH, pTCPSESS, retrans);

	}
	else
	{
		log_print(LOGN_DEBUG, "NO PAYLOAD");

//		save_pkt(pCAPHEAD, pINFOETH, pTCPSESS, retrans);
		/*
		 * Input Node Free 
		 */
		nifo_node_delete(pMEMSINFO, pNode);
		pATCPSUBINFO->delCnt++;
	}

	dSendPayLoad(pMEMSINFO, &TCPSESSKEY, pTCPSESS);

	/* 세션 종료 조건 검사 */
	if(pTCPSESS->ucEndStatus == DEF_END_NORMAL)
	{
		log_print(LOGN_DEBUG, 
			"FIN CLOSE SESSION FINSTATUS[%s]", PrintFinStatus(pTCPSESS->ucFinStatus));
		/* 세션 종료 */
		dCloseSession(pMEMSINFO, pTCPHASH, &TCPSESSKEY, pTCPSESS);
		timerN_del(pTIMER, pTCPSESS->timerNID);
		/* Delete Call Session List */
		Delete_SessList(&TCPSESSKEY);
	}
	else if(pTCPSESS->ucEndStatus == DEF_END_RST)
	{
		log_print(LOGN_DEBUG, "RST CLOSE SESSION FINSTATUS[%s]", PrintFinStatus(pTCPSESS->ucFinStatus));

		/* 특정 시간(1초)를 기다려 세션을 종료 하기 위함. */
		guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_TCP_RSTWAIT];
		pTCPSESS->timerNID = timerN_update(pTIMER, pTCPSESS->timerNID, time(NULL) + guiTimerValue);
	}

END_FUNC_TIME_CHECK(pFUNC, 10);
	return 0;
}


/** pCreateSession function.
 *
 *  pCreateSession Function
 *
 *  @return			S32  SUCC:TCP_SESS Pointer, FAIL: NULL
 *  @see			tcp_init.c tcp_msgq.c tcp_util.c tcp_func.c tcp_main.c tcp_api.h
 *
 *  @exception		nothing
 *  @note			nothing
 **/
TCP_SESS *pCreateSession(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, stTIMERNINFO *pTIMER, TCP_SESS_KEY *pTCPSESSKEY, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, U8 ucRtxType, U8 ucControl)
{
START_FUNC_TIME_CHECK(pFUNC, 11);
	S32				i;
	TCP_SESS		*pTCPSESS;
	TCP_SESS		TCPSESS;
	LOG_ITCP_SESS	*pTCPLOG;	
	TCP_COMMON		TCPCOMMON;
	struct timeval	stNowTime;
	U8				*pLOGNODE;

	stHASHONODE		*pHASHNODE;

	pATCPSUBINFO->curSessCnt++;
	pATCPSUBINFO->sessCnt++;

	memcpy(&TCPCOMMON.TCPSESSKEY, pTCPSESSKEY, TCP_SESS_KEY_SIZE);
	
	gettimeofday(&stNowTime, NULL);

	pTCPSESS = &TCPSESS;

	/* Last Update Time Setting */
	pTCPSESS->uiLastUpdateTime = pCAPHEAD->curtime;
	pTCPSESS->uiLastUpdateMTime = pCAPHEAD->ucurtime;

	/* 세션 생성 시간 */
	pTCPSESS->uiSessCreateTime = pCAPHEAD->curtime;
	pTCPSESS->uiSessCreateMTime= pCAPHEAD->ucurtime;

	pTCPSESS->uiLastSessUpdateTime = stNowTime.tv_sec;

	/* 데이터 관리 정보 */
	pTCPSESS->uiReqCount = 0;
	pTCPSESS->offset_ReqData = 0;
	pTCPSESS->uiResCount = 0;
	pTCPSESS->offset_ResData = 0;

	/* 서비스 타입에 따른 전송 블록 세팅 */
	if(ucRtxType == DEF_FROM_CLIENT)
		pTCPSESS->dSndMsgQ = dGetProcID(pINFOETH->stIP.dwSrcIP, pINFOETH->stUDPTCP.wDestPort);
	else
		pTCPSESS->dSndMsgQ = dGetProcID(pINFOETH->stIP.dwDestIP, pINFOETH->stUDPTCP.wSrcPort);

	/* FIN 관리 정보 */
	pTCPSESS->ucFinStatus = DEF_FIN_0;
	pTCPSESS->ucEndStatus = 0;

	pTCPSESS->ucFinRtx = 0;
	pTCPSESS->ucRstRtx = 0;

	for(i = 0; i < DEF_ARRAY_RTX; i++) {
		pTCPSESS->uiFinChkSeq[i] = 0;
		pTCPSESS->uiFinChkAck[i] = 0;
	}

	pTCPSESS->uiRcvReqAck = 0;
	pTCPSESS->uiRcvResAck = 0;
	pTCPSESS->uiRcvReqAckTime = 0;
	pTCPSESS->uiRcvReqAckMTime = 0;
	pTCPSESS->uiRcvResAckTime = 0;
	pTCPSESS->uiRcvResAckMTime = 0;
	
	/* 서비스 정보 */
//	pTCPSESS->usL4Code = pINFOETH->usL4Code;
//	pTCPSESS->usL7Code = pINFOETH->usL7Code;
	pTCPSESS->usL4Code = L4_INET_TCP;
	pTCPSESS->usL7Code = APP_UNKNOWN;
	pTCPSESS->usAppCode = pINFOETH->usAppCode;
	pTCPSESS->usL4FailCode = 0;

    pTCPSESS->uiIPDataUpPktCnt = 0;
    pTCPSESS->uiIPDataDnPktCnt = 0;
    pTCPSESS->uiIPTotUpPktCnt = 0;
    pTCPSESS->uiIPTotDnPktCnt = 0;
    pTCPSESS->uiIPDataUpRetransCnt = 0;
    pTCPSESS->uiIPDataDnRetransCnt = 0;
    pTCPSESS->uiIPTotUpRetransCnt = 0;
    pTCPSESS->uiIPTotDnRetransCnt = 0;
    pTCPSESS->uiIPDataUpPktSize = 0;
    pTCPSESS->uiIPDataDnPktSize = 0;
    pTCPSESS->uiIPTotUpPktSize = 0;
    pTCPSESS->uiIPTotDnPktSize = 0; 
    pTCPSESS->uiIPDataUpRetransSize = 0;
    pTCPSESS->uiIPDataDnRetransSize = 0;
    pTCPSESS->uiIPTotUpRetransSize = 0;
    pTCPSESS->uiIPTotDnRetransSize = 0;

	/* TCP LOG Node 생성 */
	if((pLOGNODE = nifo_node_alloc(pMEMSINFO)) == NULL) 
	{
		log_print(LOGN_CRI, LH"nifo_node_alloc NULL", LT);
		TCP_SESS_KEY_Prt((S8 *)__FUNCTION__, pTCPSESSKEY);
		TCP_SESS_Prt((S8 *)__FUNCTION__, pTCPSESS);
END_FUNC_TIME_CHECK(pFUNC, 11);
		return NULL;
	}

	if((pTCPLOG = (LOG_ITCP_SESS *)nifo_tlv_alloc(pMEMSINFO, pLOGNODE, LOG_ITCP_SESS_DEF_NUM, LOG_ITCP_SESS_SIZE, DEF_MEMSET_ON)) == NULL) {
		log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL", LT);
		nifo_node_delete(pMEMSINFO, pLOGNODE);
END_FUNC_TIME_CHECK(pFUNC, 11);
		return NULL;
	}

	pTCPSESS->offset_LOG = nifo_offset(pMEMSINFO, pTCPLOG);

	pTCPLOG->uiCallTime = pCAPHEAD->curtime;
	pTCPLOG->uiCallMTime = pCAPHEAD->ucurtime;
	pTCPLOG->uiClientIP = pTCPSESSKEY->uiSIP;
//	pTCPLOG->uiNASName = pTCPSESSKEY->uiSIP;
	pTCPLOG->uiNASName = 0;
	
	pTCPLOG->usPlatformType = dGetPlatformType(pINFOETH->usL4Code, pINFOETH->usL7Code);
//	pTCPLOG->usPlatformType = (pINFOETH->usL4Code / 1000) * 1000;
	pTCPLOG->usSvcL4Type = pINFOETH->usL4Code;
	pTCPLOG->ucSubSysNo = 1;	/* 행후 변경 요망 */
	pTCPLOG->usClientPort = pTCPSESSKEY->usSPort;
	pTCPLOG->uiServerIP = pTCPSESSKEY->uiDIP;
	pTCPLOG->usServerPort = pTCPSESSKEY->usDPort;

	pTCPLOG->uiTcpLastPktTime = pCAPHEAD->curtime;
	pTCPLOG->uiTcpLastPktMTime = pCAPHEAD->ucurtime;

	pTCPLOG->uiOpStartTime = stNowTime.tv_sec;
	pTCPLOG->uiOpStartMTime = stNowTime.tv_usec;
	
	if(ucRtxType == DEF_FROM_SERVER) {
		pTCPLOG->uiITcpDnLastPktTime = pCAPHEAD->curtime;
		pTCPLOG->uiITcpDnLastPktMTime = pCAPHEAD->ucurtime;
		pTCPLOG->usTcpDnMSS = pINFOETH->stUDPTCP.mss;
		pTCPLOG->usTcpDnFirstWindowSize = pINFOETH->stUDPTCP.window;
	} else {
		pTCPLOG->uiITcpUpLastPktTime = pCAPHEAD->curtime;
		pTCPLOG->uiITcpUpLastPktMTime = pCAPHEAD->ucurtime;
		pTCPLOG->usTcpUpMSS = pINFOETH->stUDPTCP.mss;
		pTCPLOG->usTcpUpFirstWindowSize = pINFOETH->stUDPTCP.window;
	}

	pTCPLOG->uiTcpSessStartTime = pCAPHEAD->curtime;
	pTCPLOG->uiTcpSessStartMTime = pCAPHEAD->ucurtime;
	
	switch(ucControl)
	{
	case TCP_SYN:
		pTCPSESS->ucTcpClientStatus = DEF_CLI_SYN;
		pTCPSESS->ucTcpServerStatus = DEF_SVR_ETC;

		pTCPSESS->ucStatus = DEF_STATUS_SYN;	
		pTCPSESS->ucSynRtx = ucRtxType;
		pTCPSESS->ucRcvSyn = SYN_RCV_ON;
		pTCPSESS->uiSynSeq = pINFOETH->stUDPTCP.seq;
		pTCPSESS->uiSynAckSeq = 0;
		pTCPSESS->uiSynAckAck = 0;
		pTCPSESS->uiLastResSeq = 0;
		pTCPSESS->uiNextResSeq = 0;
		pTCPSESS->uiLastReqSeq = pINFOETH->stUDPTCP.seq;
		pTCPSESS->uiNextReqSeq = pINFOETH->stUDPTCP.seq + 1;
		
		pTCPLOG->uiITcpSynTime = pCAPHEAD->curtime;
		pTCPLOG->uiITcpSynMTime = pCAPHEAD->ucurtime;
		
		pTCPLOG->ucTcpClientStatus = DEF_CLI_SYN;
		pTCPLOG->ucTcpServerStatus = DEF_SVR_ETC;

		pTCPLOG->ucTcpSynCnt = 1;

		pTCPLOG->ucRtx = pTCPSESS->ucSynRtx;
		break;
	case TCP_SYNACK:
		pTCPSESS->ucTcpClientStatus = DEF_CLI_ETC;
		pTCPSESS->ucTcpServerStatus = DEF_SVR_SYNACK;

		pTCPSESS->ucStatus = DEF_STATUS_SYNACK;
		pTCPSESS->ucSynRtx = (ucRtxType == DEF_FROM_SERVER) ? DEF_FROM_CLIENT : DEF_FROM_SERVER;
		pTCPSESS->ucRcvSyn = SYN_RCV_OFF;
		pTCPSESS->uiSynSeq = pINFOETH->stUDPTCP.ack - 1;
		pTCPSESS->uiSynAckSeq = pINFOETH->stUDPTCP.seq;
		pTCPSESS->uiSynAckAck = pINFOETH->stUDPTCP.ack;
		pTCPSESS->uiLastResSeq = pINFOETH->stUDPTCP.seq;
		pTCPSESS->uiNextResSeq = pINFOETH->stUDPTCP.seq + 1;
		pTCPSESS->uiLastReqSeq = pINFOETH->stUDPTCP.ack - 1;
		pTCPSESS->uiNextReqSeq = pINFOETH->stUDPTCP.ack;

		pTCPLOG->uiTcpSynAckTime = pCAPHEAD->curtime;
		pTCPLOG->uiTcpSynAckMTime = pCAPHEAD->ucurtime;

		pTCPLOG->ucTcpClientStatus = DEF_CLI_ETC;
		pTCPLOG->ucTcpServerStatus = DEF_SVR_SYNACK;

		pTCPLOG->ucTcpSynAckCnt = 1;

		pTCPLOG->ucRtx = pTCPSESS->ucSynRtx;
		break;
	case TCP_ACK:
		pTCPSESS->ucTcpClientStatus = DEF_CLI_DATA;
		pTCPSESS->ucTcpServerStatus = DEF_SVR_DATA;

		pTCPSESS->ucStatus = DEF_STATUS_DATA;
		pTCPSESS->ucSynRtx = ucRtxType;
		pTCPSESS->ucRcvSyn = SYN_RCV_OFF;
		pTCPSESS->uiSynSeq = pINFOETH->stUDPTCP.seq - pINFOETH->stUDPTCP.wDataLen - 1;
		pTCPSESS->uiSynAckSeq = pINFOETH->stUDPTCP.ack - 1;
		pTCPSESS->uiSynAckAck = 0;
		pTCPSESS->uiLastResSeq = pINFOETH->stUDPTCP.ack - 1;
		pTCPSESS->uiNextResSeq = pINFOETH->stUDPTCP.ack;
		pTCPSESS->uiLastReqSeq = pINFOETH->stUDPTCP.seq - pINFOETH->stUDPTCP.wDataLen - 1;
		pTCPSESS->uiNextReqSeq = pINFOETH->stUDPTCP.seq;

		pTCPLOG->ucTcpClientStatus = DEF_CLI_DATA;
		pTCPLOG->ucTcpServerStatus = DEF_SVR_DATA;

		pTCPLOG->ucRtx = pTCPSESS->ucSynRtx;
		break;
	}

	if((pHASHNODE = hasho_add(pTCPHASH, (U8 *)pTCPSESSKEY, (U8 *)pTCPSESS)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_add NULL", LT);
		TCP_SESS_KEY_Prt((S8 *)__FUNCTION__, pTCPSESSKEY);
		TCP_SESS_Prt((S8 *)__FUNCTION__, pTCPSESS);
		nifo_node_delete(pMEMSINFO, pLOGNODE);
END_FUNC_TIME_CHECK(pFUNC, 11);
		return NULL;
	} else {
		pTCPSESS = (TCP_SESS *)nifo_ptr(pTCPHASH, pHASHNODE->offset_Data);
		guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_TCP_TIMEOUT];
		pTCPSESS->timerNID = timerN_add(pTIMER, invoke_del, (U8 *)&TCPCOMMON, sizeof(TCP_COMMON), time(NULL) + guiTimerValue);
	}

END_FUNC_TIME_CHECK(pFUNC, 11);
	return pTCPSESS;
}


/** dCheckAck function.
 *
 *  dCheckAck Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/

S32 dCloseSession(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS)
{
START_FUNC_TIME_CHECK(pFUNC, 12);
	struct timeval	stNowTime;
	LOG_ITCP_SESS	*pTCPLOG;

	gettimeofday(&stNowTime, NULL);

	pATCPSUBINFO->curSessCnt--;

	pTCPSESS->usL4FailCode = GetFailCode(pTCPSESSKEY, pTCPSESS);
	pTCPLOG = (LOG_ITCP_SESS *)nifo_ptr(pMEMSINFO, pTCPSESS->offset_LOG);

	STG_DiffTIME64(pTCPLOG->uiTcpSynAckAckTime, pTCPLOG->uiTcpSynAckAckMTime, pTCPLOG->uiCallTime, pTCPLOG->uiCallMTime, &pTCPLOG->llConnSetupGapTime);
	STG_DiffTIME64(pTCPLOG->uiTcpLastPktTime, pTCPLOG->uiTcpLastPktMTime, pTCPLOG->uiCallTime, pTCPLOG->uiCallMTime, &pTCPLOG->llTcpSessGapTime);
	pTCPLOG->uiTcpSynAckGapTime = GetGap32Time(pTCPLOG->uiTcpSynAckTime, pTCPLOG->uiTcpSynAckMTime, pTCPLOG->uiITcpSynTime, pTCPLOG->uiITcpSynMTime);

#if 0
	if(pTCPLOG->uiITcpSynTime == 0) {
		pTCPLOG->uiITcpSynTime = pTCPLOG->uiTcpSynAckTime;
		pTCPLOG->uiITcpSynMTime = pTCPLOG->uiTcpSynAckMTime;
	}
#endif

	pTCPLOG->usL4FailCode = pTCPSESS->usL4FailCode;
	pTCPLOG->uiOpEndTime = stNowTime.tv_sec;
	pTCPLOG->uiOpEndMTime = stNowTime.tv_usec;
	
	dSendAllData(pMEMSINFO, pTCPSESSKEY, pTCPSESS);
	switch(pTCPSESS->ucStatus)
	{
		case DEF_STATUS_DATA:
		case DEF_STATUS_ACK:
			dTCPStopMsg(pMEMSINFO, pTCPSESSKEY, pTCPSESS);
			break;
	}
	hasho_del(pTCPHASH, (U8 *)pTCPSESSKEY);

END_FUNC_TIME_CHECK(pFUNC, 12);
	return 0;
}

/** dCheckSessKeyList function.
 *
 *  dCheckSessKeyList Function
 *
 *  @return			S32
 *  @see			itcp_api.h
 *
 **/
S32 dCheckSessKeyList(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS)
{
	S32		dRet;

	/* ADD SESSION KEY LIST */
	stCALLKEY.uiSrcIP = pTCPSESSKEY->uiSIP;
	stCALLKEY.uiReserved = 0;

	if( (pSESSKEYNODE = hasho_find(pSESSKEYINFO, (U8 *)&stCALLKEY)) == NULL ) 
	{
		/* ADD TCP_SESS_KEY LIST */
		dRet = dGetSessKeyList(&pstNEWSESSKEYList);
		if(dRet < 0) {
			log_print(LOGN_CRI, LH"#### dGetStackOnly() dRet[%d]", LT, dRet);
			return -1;
		}
		memcpy(&pstNEWSESSKEYList->stSessKey, pTCPSESSKEY, TCP_SESS_KEY_SIZE);
		pstNEWSESSKEYList->SessStartTime = pTCPSESS->uiSessCreateTime;
		if((pSESSKEYNODE = hasho_add(pSESSKEYINFO, (U8 *)&stCALLKEY, (U8 *)pstNEWSESSKEYList)) == NULL) {
			log_print(LOGN_CRI, LH"#### hasho_add NULL", LT);
		} else {
			log_print(LOGN_DEBUG, "#### ADD HASH NEW CALL CIP:%d.%d.%d.%d START:%u", 
					HIPADDR(stCALLKEY.uiSrcIP), pstNEWSESSKEYList->SessStartTime);
		}

		if(pTCPSESS->ucSynRtx == DEF_FROM_SERVER)
		{
			dTCPReCallStartMsg(pMEMSINFO, pTCPSESSKEY, pTCPSESS);
		}
	} else {
		pstSESSKEYList = (SESSKEY_LIST *)nifo_ptr(pSESSKEYINFO, pSESSKEYNODE->offset_Data);

		dRet = dGetSessKeyList(&pstNEWSESSKEYList);
		if(dRet < 0) {
			log_print(LOGN_CRI, "#### ERROR dGetStackOnly() dRet[%d]", dRet);
			return -2;
		}
		memcpy(&pstNEWSESSKEYList->stSessKey, pTCPSESSKEY, TCP_SESS_KEY_SIZE);
		pstNEWSESSKEYList->SessStartTime = pTCPSESS->uiSessCreateTime;
		dRet = dAddSessKeyNext(pstSESSKEYList, pstNEWSESSKEYList);
		if(dRet < 0) {
			log_print(LOGN_CRI, "#### SESSKEY INSERT ERROR][RET]:%d", dRet);
			FreeSessKeyList(pstNEWSESSKEYList);
			return -3;
		} else {
			log_print(LOGN_DEBUG, "#### ADD HASH EXIST CALL CIP:%d.%d.%d.%d START:%u", 
					HIPADDR(stCALLKEY.uiSrcIP), pstNEWSESSKEYList->SessStartTime);
		}
	}

	return 0;
}

/** dCheckAck function.
 *
 *  dCheckAck Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
S32 dCheckAck(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, U8 ucRtxType)
{
START_FUNC_TIME_CHECK(pFUNC, 13);
	U32				uiBase;
	OFFSET			offset;
	U8				*pData, *pNext, *pHEAD;
	INFO_ETH		*pInfoEth;
	TCP_INFO		*pTcpInfo;
	Capture_Header_Msg	*pCapHead;

//	if(ucRtxType == DEF_FROM_SERVER) {
	if(pTCPSESS->ucSynRtx != ucRtxType) {
		pData = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ReqData);
		pHEAD = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ReqData);
		uiBase = pTCPSESS->uiSynSeq;
		pTCPSESS->uiRcvReqAck = pINFOETH->stUDPTCP.ack;
		pTCPSESS->uiRcvReqAckTime = pCAPHEAD->curtime;
		pTCPSESS->uiRcvReqAckMTime = pCAPHEAD->ucurtime;
	} else {
		pData = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData);
		pHEAD = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData);
		uiBase = pTCPSESS->uiSynAckSeq;
		pTCPSESS->uiRcvResAck = pINFOETH->stUDPTCP.ack;
		pTCPSESS->uiRcvResAckTime = pCAPHEAD->curtime;
		pTCPSESS->uiRcvResAckMTime = pCAPHEAD->ucurtime;
	}

	while(pData != NULL) {

		offset = nifo_offset(pMEMSINFO, pData);

		pCapHead = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, offset);
		pInfoEth = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
		pTcpInfo = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
		
		/**
		 *  SEQ가 MAX값을 넘어서 할당된 경우 처리 방안이 필요
		 * 	다른 변수를 두어 처리 하는 방안 고려
		 */
		if(OFFSET_SEQ(uiBase, pInfoEth->stUDPTCP.seq) + pInfoEth->stUDPTCP.wDataLen <= OFFSET_SEQ(uiBase, pINFOETH->stUDPTCP.ack)) {
			if(pTcpInfo->uiAckTime == 0) {
				pTcpInfo->uiAckTime = pCAPHEAD->curtime;
				pTcpInfo->uiAckMTime = pCAPHEAD->ucurtime;

				SetData(pTCPSESS, pTcpInfo);
			}

			pNext = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pData)->nont.offset_next), NIFO, nont);
			if(pHEAD == pNext)
				pNext = NULL;	

			pData = pNext;
		} else {
			break;
		}
	}

END_FUNC_TIME_CHECK(pFUNC, 13);
	return 0;
}


/** dCheckAck function.
 *
 *  dCheckAck Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
S32 dInsertTCPData(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, U8 *pNode, S32 *delFlag)
{
	TCP_INFO			*pTCPINFO, *pPrevTCP;
	U8					*pHead, *pPrev, *pTmp;
	U32					uiLastSeq, uiLastSeqData;
	U32					uiBase, uiRcvAck;
	STIME				uiRcvAckTime;
	MTIME				uiRcvAckMTime;
	OFFSET				offset;
	S32					dRet, dSubRet, retransSize;

	/* TCP DATA가 존재하면 STACK에 집어넣는다. */
	pTCPINFO = (TCP_INFO *)nifo_tlv_alloc(pMEMSINFO, pNode, TCP_INFO_DEF_NUM, TCP_INFO_SIZE, DEF_MEMSET_OFF);

	pTCPINFO->uiDataSize = pINFOETH->stUDPTCP.wDataLen;
	pTCPINFO->uiSrvIP = pTCPSESSKEY->uiDIP;
	pTCPINFO->uiCliIP = pTCPSESSKEY->uiSIP;
	pTCPINFO->usSrvPort = pTCPSESSKEY->usDPort;
	pTCPINFO->usCliPort = pTCPSESSKEY->usSPort;
	pTCPINFO->uiSeqNum = pINFOETH->stUDPTCP.seq;
	pTCPINFO->uiAckNum = pINFOETH->stUDPTCP.ack;
	pTCPINFO->uiSOffset = pINFOETH->offset;
	pTCPINFO->uiCapTime = pCAPHEAD->curtime;
	pTCPINFO->uiCapMTime = pCAPHEAD->ucurtime;
	pTCPINFO->uiAckTime = 0;
	pTCPINFO->uiAckMTime = 0;
	pTCPINFO->usAppCode = pTCPSESS->usAppCode;
	pTCPINFO->usL4Code = pTCPSESS->usL4Code;
	pTCPINFO->usL7Code = pTCPSESS->usL7Code;
	pTCPINFO->usL4FailCode = 0;
	pTCPINFO->ucRtx = pCAPHEAD->bRtxType;
	pTCPINFO->cTcpFlag = DEF_TCP_DATA;
	pTCPINFO->cRetrans = DEF_RETRANS_OFF;
	pTCPINFO->ucProtocol = DEF_PROTOCOL_TCP;

	pTCPINFO->uiIPDataUpPktCnt = 0;
	pTCPINFO->uiIPDataDnPktCnt = 0;
	pTCPINFO->uiIPTotUpPktCnt = 0;
	pTCPINFO->uiIPTotDnPktCnt = 0;
	pTCPINFO->uiIPDataUpRetransCnt = 0;
	pTCPINFO->uiIPDataDnRetransCnt = 0;
	pTCPINFO->uiIPTotUpRetransCnt = 0;
	pTCPINFO->uiIPTotDnRetransCnt = 0;
	pTCPINFO->uiIPDataUpPktSize = 0;
	pTCPINFO->uiIPDataDnPktSize = 0;
	pTCPINFO->uiIPTotUpPktSize = 0;
	pTCPINFO->uiIPTotDnPktSize = 0;
	pTCPINFO->uiIPDataUpRetransSize = 0;
	pTCPINFO->uiIPDataDnRetransSize = 0;
	pTCPINFO->uiIPTotUpRetransSize = 0;
	pTCPINFO->uiIPTotDnRetransSize = 0;

//	if(pTCPINFO->ucRtx == DEF_FROM_SERVER) {
	if(pTCPSESS->ucSynRtx != pTCPINFO->ucRtx) {
		uiBase = pTCPSESS->uiSynAckSeq;
		uiRcvAck = pTCPSESS->uiRcvResAck;
		uiRcvAckTime = pTCPSESS->uiRcvResAckTime;
		uiRcvAckMTime = pTCPSESS->uiRcvResAckMTime;
		uiLastSeq = pTCPSESS->uiLastResSeq;
		uiLastSeqData = pTCPSESS->uiNextResSeq - pTCPSESS->uiLastResSeq;
		pHead = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ResData);
	} else {
		uiBase = pTCPSESS->uiSynSeq;
		uiRcvAck = pTCPSESS->uiRcvReqAck;
		uiRcvAckTime = pTCPSESS->uiRcvReqAckTime;
		uiRcvAckMTime = pTCPSESS->uiRcvReqAckMTime;
		uiLastSeq = pTCPSESS->uiLastReqSeq;
		uiLastSeqData = pTCPSESS->uiNextReqSeq - pTCPSESS->uiLastReqSeq;
		pHead = nifo_ptr(pMEMSINFO, pTCPSESS->offset_ReqData);
	}

	if(pHead == NULL) {
		dSubRet = CalcBase(pTCPINFO, uiBase, uiLastSeq, uiLastSeqData, &retransSize);
		if((dSubRet == DEF_NORETRANS_OVER) || (dSubRet == DEF_RETRANS_OVER)) {
//        	if(pTCPINFO->ucRtx == DEF_FROM_SERVER) {
			if(pTCPSESS->ucSynRtx != pTCPINFO->ucRtx) {
           		pTCPSESS->offset_ResData = nifo_offset(pMEMSINFO, pNode);
        	} else {
           		pTCPSESS->offset_ReqData = nifo_offset(pMEMSINFO, pNode);
        	}
			if((uiRcvAckTime > 0) &&
					((OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum) + pTCPINFO->uiDataSize) <= OFFSET_SEQ(uiBase, uiRcvAck))) 
			{
				pTCPINFO->uiAckTime = uiRcvAckTime;
				pTCPINFO->uiAckMTime = uiRcvAckMTime;
			}
		} else {
			nifo_node_delete(pMEMSINFO, pNode);
			pATCPSUBINFO->delCnt++;
		}
		*delFlag += retransSize;
		return 0;
	}

	pPrev = pHead;
	do
	{
		// First Get Last Node && Next Time is Prev Node
		pPrev = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pPrev)->nont.offset_prev), NIFO, nont);
		offset = nifo_offset(pMEMSINFO, pPrev);

		pPrevTCP = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);

		dRet = CalcRetrans(pTCPINFO, uiBase, pPrevTCP->uiSeqNum, pPrevTCP->uiDataSize, &retransSize);
		*delFlag += retransSize;
		switch(dRet)
		{
		case DEF_NORETRANS_OVER:	/* Normal Case */
		case DEF_RETRANS_OVER:		/* This Node have new data */
									/* so New Data is Added Next of PreNode */ 
			/*  clist_add_head(pMEMSINFO, &((NIFO *)pNode)->nont, &((NIFO *)pPrev)->nont); */
			if((uiRcvAckTime > 0) &&
					((OFFSET_SEQ(uiBase, pTCPINFO->uiSeqNum) + pTCPINFO->uiDataSize) <= OFFSET_SEQ(uiBase, uiRcvAck))) 
			{
				pTCPINFO->uiAckTime = uiRcvAckTime;
				pTCPINFO->uiAckMTime = uiRcvAckMTime;
			}
			nifo_node_link_nont_next(pMEMSINFO, pPrev, pNode);
			return 0;

		case DEF_RETRANS_SUBSET:	/* This Node is subset of Old Node */
			nifo_node_delete(pMEMSINFO, pNode);
			pATCPSUBINFO->delCnt++;
			return 0;

		case DEF_NORETRANS_UNDER:	/* Next Step is to check PreNode for Retransmission */
		case DEF_RETRANS_UNDER:		/* This Node's Seq + data size is between Old Node's Seq and Seq + data size OR */
									/* This Node's Seq + data size is Old Node Seq + data size, 
                                   		for reducing time to remake linked-list */
			/* Set Ack Time */
			if((pPrevTCP->uiAckTime > 0) && (pTCPINFO->uiAckTime == 0)) {
				pTCPINFO->uiAckTime = pPrevTCP->uiAckTime;
				pTCPINFO->uiAckMTime = pPrevTCP->uiAckMTime;
			}

			if(pHead == pPrev) {	
				dSubRet = CalcBase(pTCPINFO, uiBase, uiLastSeq, uiLastSeqData, &retransSize);
				if((dSubRet == DEF_NORETRANS_OVER) || (dSubRet == DEF_RETRANS_OVER)) {
            		nifo_node_link_nont_prev(pMEMSINFO, pHead, pNode);
					/* Header Setting */
//        			if(pTCPINFO->ucRtx == DEF_FROM_SERVER) {
					if(pTCPSESS->ucSynRtx != pTCPINFO->ucRtx) {
            			pTCPSESS->offset_ResData = nifo_offset(pMEMSINFO, pNode);
        			} else {
            			pTCPSESS->offset_ReqData = nifo_offset(pMEMSINFO, pNode);
        			}
				} else {
					nifo_node_delete(pMEMSINFO, pNode);
					pATCPSUBINFO->delCnt++;
				}
				pPrev = NULL;
			} 

			break;

		case DEF_RETRANS_RSUBSET:	/* Old Node is subset of This Node */
                                	/* except this Node's Seq + data size is Old Node Seq + data size */

			/* Set Ack Time */
			if((pPrevTCP->uiAckTime > 0) && (pTCPINFO->uiAckTime == 0)) {
				pTCPINFO->uiAckTime = pPrevTCP->uiAckTime;
				pTCPINFO->uiAckMTime = pPrevTCP->uiAckMTime;
			}

			if(pHead == pPrev) {
				dSubRet = CalcBase(pTCPINFO, uiBase, uiLastSeq, uiLastSeqData, &retransSize);

            	nifo_node_link_nont_prev(pMEMSINFO, pHead, pNode);

				nifo_node_unlink_nont(pMEMSINFO, pPrev);	
				nifo_node_delete(pMEMSINFO, pPrev);
				pATCPSUBINFO->delCnt++;

				/* Header Setting */
//        		if(pTCPINFO->ucRtx == DEF_FROM_SERVER) {
				if(pTCPSESS->ucSynRtx != pTCPINFO->ucRtx) {
            		pTCPSESS->offset_ResData = nifo_offset(pMEMSINFO, pNode);
        		} else {
            		pTCPSESS->offset_ReqData = nifo_offset(pMEMSINFO, pNode);
        		}

				pPrev = NULL;
			} else {
				pTmp = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pPrev)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pPrev);
				nifo_node_delete(pMEMSINFO, pPrev);
				pATCPSUBINFO->delCnt++;
				pPrev = pTmp;
			}
			break;
		default:
			log_print(LOGN_CRI, LH"???? dRet[%d]", LT, dRet);
			return -1;
		}		
		*delFlag += retransSize;
	} while(pPrev != NULL);

	return 0;
}
/**
 * 	$Log: itcp_func.c,v $
 * 	Revision 1.2  2011/09/05 12:26:39  hhbaek
 * 	*** empty log message ***
 * 	
 * 	Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * 	NEW OAM SYSTEM
 * 	
 * 	Revision 1.3  2011/08/17 13:02:39  hhbaek
 * 	A_ITCP
 * 	
 * 	Revision 1.2  2011/08/10 09:57:44  uamyd
 * 	modified and block added
 * 	
 * 	Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * 	init DQMS2
 * 	
 * 	Revision 1.21  2011/05/12 01:47:06  dark264sh
 * 	*** empty log message ***
 * 	
 * 	Revision 1.20  2011/05/12 00:42:09  dark264sh
 * 	*** empty log message ***
 * 	
 * 	Revision 1.19  2011/05/06 10:19:53  dark264sh
 * 	*** empty log message ***
 * 	
 * 	Revision 1.18  2011/04/29 05:15:21  sbkim
 * 	*** empty log message ***
 * 	
 * 	Revision 1.17  2011/04/29 02:50:22  sbkim
 * 	*** empty log message ***
 * 	
 * 	Revision 1.16  2011/04/24 17:18:26  dark264sh
 * 	*** empty log message ***
 * 	
 * 	Revision 1.15  2011/04/24 17:09:38  dark264sh
 * 	A_ITCP: DATA 세션의 경우 양방향으로 패킷이 있는 경우만 START 전송
 * 	
 * 	Revision 1.14  2011/04/19 07:42:36  dark264sh
 * 	A_ITCP: 역방향 세션의 Client, Server 상태값 잘못 세팅하는 버그 수정
 * 	
 * 	Revision 1.13  2011/04/18 08:25:18  dark264sh
 * 	A_ITCP: data 세션의 경우 Start 보내는 시점 변경
 * 	
 * 	Revision 1.12  2011/04/18 07:16:50  dark264sh
 * 	L4_INET 변경에 따른 수정
 * 	
 * 	Revision 1.11  2011/04/17 15:21:59  dark264sh
 * 	A_ITCP: data호에서 rtx값을 잘못 전달하는 버그 수정
 * 	
 * 	Revision 1.10  2011/04/17 14:47:38  dark264sh
 * 	*** empty log message ***
 * 	
 * 	Revision 1.9  2011/04/17 14:34:08  dark264sh
 * 	A_ITCP: data호의 seq 처리 버그 수정
 * 	
 * 	Revision 1.8  2011/04/16 10:47:07  dark264sh
 * 	A_ITCP: Server Port가 80, 8080인 경우만 A_IHTTP로 전송
 * 	
 * 	Revision 1.7  2011/04/16 10:28:38  dark264sh
 * 	A_ITCP: 서버방향에 의해서 세션 생성시 RecallStart 전송
 * 	
 * 	Revision 1.6  2011/04/16 09:12:47  dark264sh
 * 	A_ITCP: 에러 처리
 * 	
 * 	Revision 1.5  2011/04/16 08:43:34  dark264sh
 * 	A_ITCP: uiTcpSynAckGapTime 처리
 * 	
 * 	Revision 1.4  2011/04/16 08:27:18  dark264sh
 * 	A_ITCP: 세션 생성 방향성 필드 세팅
 * 	
 * 	Revision 1.3  2011/04/16 08:23:18  dark264sh
 * 	A_ITCP: SYN, SYNACK 없이 DATA 있는 경우 세션 생성
 * 	
 * 	Revision 1.2  2011/04/14 11:21:55  dark264sh
 * 	A_ITCP: LOG_TCP_SESS => LOG_ITCP_SESS 변경
 * 	
 * 	Revision 1.1  2011/04/12 02:51:50  dark264sh
 * 	A_ITCP 추가
 * 	
 */
