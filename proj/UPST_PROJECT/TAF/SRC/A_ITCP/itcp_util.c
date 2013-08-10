/**		@file	tcp_util.c
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: itcp_util.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:47 $
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

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"
#include "capdef.h"

// LIB
#include "loglib.h"
#include "Analyze_Ext_Abs.h"

// .
#include "itcp_sess.h"
#include "itcp_util.h"

/**
 * Declare variables
 */
extern int 					gAHTTPCnt;
extern int 					gACALLCnt;
extern Capture_Header_Msg	*pCAPHEAD;	/* itcp_msgq.h */

/**
 *	Implement func.
 */

/** dHttpInit function.
 *
 *  dHttpInit Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
void MakeTCPHashKey(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, TCP_SESS_KEY *pTCPSESSKEY)
{
	if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
		pTCPSESSKEY->uiSIP = pINFOETH->stIP.dwSrcIP;
		pTCPSESSKEY->uiDIP = pINFOETH->stIP.dwDestIP;
		pTCPSESSKEY->usSPort = pINFOETH->stUDPTCP.wSrcPort;
		pTCPSESSKEY->usDPort = pINFOETH->stUDPTCP.wDestPort;
	} else {
		pTCPSESSKEY->uiSIP = pINFOETH->stIP.dwDestIP;
		pTCPSESSKEY->uiDIP = pINFOETH->stIP.dwSrcIP;
		pTCPSESSKEY->usSPort = pINFOETH->stUDPTCP.wDestPort;
		pTCPSESSKEY->usDPort = pINFOETH->stUDPTCP.wSrcPort;
	}
}

/** dHttpInit function.
 *
 *  dHttpInit Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
U8 GetTCPControl(U8 control)
{
	if((control & TCP_SYNACK) == TCP_SYNACK) return TCP_SYNACK;
	else if((control & TCP_SYN) == TCP_SYN) return TCP_SYN;
	else if((control & TCP_RST) == TCP_RST) return TCP_RST;
	else if((control & TCP_FIN) == TCP_FIN) return TCP_FIN;
	else if((control & TCP_ACK) == TCP_ACK) return TCP_ACK;
	else return 0;
}

U16 GetFailCode(TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS)
{
	U16					usRet = 0;
	U16					ucEndStatus;

	U8					szSIP[INET_ADDRSTRLEN];
	U8					szDIP[INET_ADDRSTRLEN];

	ucEndStatus = pTCPSESS->ucEndStatus;

	log_print(LOGN_DEBUG, 
		"FAILCODE SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]STATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
		util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
		util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
		pTCPSESS->uiSessCreateTime, pTCPSESS->uiSessCreateTime,
		PrintFinStatus(pTCPSESS->ucFinStatus), PrintEndStatus(pTCPSESS->ucEndStatus),
		PrintRtx(pTCPSESS->ucFinRtx), PrintRtx(pTCPSESS->ucEndStatus));

	switch(ucEndStatus) 
	{
	case DEF_END_NORMAL:
		switch(pTCPSESS->ucFinStatus)
		{
		case DEF_FIN_3:
			/* TCP_NOERR_FIN_E1, TCP_NOERR_FIN_E2 */
//			usRet = (pTCPSESS->ucFinRtx == DEF_FROM_SERVER) ? TCP_NOERR_FIN_E2 : TCP_NOERR_FIN_E1;
			usRet = (pTCPSESS->ucFinRtx == pTCPSESS->ucSynRtx) ? TCP_NOERR_FIN_E1 : TCP_NOERR_FIN_E2;
			break;
		default:
			log_print(LOGN_CRI, 
			"FAILCODE [NORMAL] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]STATUS[%s]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
			util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
			util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
			pTCPSESS->uiSessCreateTime, pTCPSESS->uiSessCreateTime,
			PrintFinStatus(pTCPSESS->ucFinStatus), PrintEndStatus(pTCPSESS->ucEndStatus),
			PrintRtx(pTCPSESS->ucFinRtx), PrintRtx(pTCPSESS->ucEndStatus));
			break;
		}
			
		break;

	case DEF_END_ABNORMAL:
		usRet = ABNORMAL_TRANS;
		break;

	case DEF_END_LONGLAST:
		switch(pTCPSESS->ucFinStatus)
		{
		case DEF_FIN_0:
			switch(pTCPSESS->ucStatus)
			{
			case DEF_STATUS_SYN:
			case DEF_STATUS_DATA:
				usRet = LONGLAST_SYN_TRANS;
				break;
			case DEF_STATUS_SYNACK:
				usRet = LONGLAST_SYNACK_TRANS;
				break;
			case DEF_STATUS_ACK:
				usRet = LONGLAST_NOFIN_TRANS;
				break;
			default:
				log_print(LOGN_CRI, 
				"FAILCODE [LONGLAST][FIN_0] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]STATUS[%s]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
				util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
				util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
				pTCPSESS->uiSessCreateTime, pTCPSESS->uiSessCreateTime,
				PrintFinStatus(pTCPSESS->ucFinStatus), PrintEndStatus(pTCPSESS->ucEndStatus),
				PrintRtx(pTCPSESS->ucFinRtx), PrintRtx(pTCPSESS->ucEndStatus));
				break;
			}
			break;
		case DEF_FIN_1:
		case DEF_FIN_2:
			/* LONGLAST_FIN_E1, LONGLAST_FIN_E2 */
//			usRet = (pTCPSESS->ucFinRtx == DEF_FROM_SERVER) ? LONGLAST_FIN_E2 : LONGLAST_FIN_E1;
			usRet = (pTCPSESS->ucFinRtx == pTCPSESS->ucSynRtx) ? LONGLAST_FIN_E1 : LONGLAST_FIN_E2;
			break;

		default:
			log_print(LOGN_CRI, 
			"FAILCODE [LONGLAST] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]STATUS[%s]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
			util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
			util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
			pTCPSESS->uiSessCreateTime, pTCPSESS->uiSessCreateTime,
			PrintFinStatus(pTCPSESS->ucFinStatus), PrintEndStatus(pTCPSESS->ucEndStatus),
			PrintRtx(pTCPSESS->ucFinRtx), PrintRtx(pTCPSESS->ucEndStatus));
			break;
		}
		break;
	
	case DEF_END_RST:
		switch(pTCPSESS->ucFinStatus)
		{
		case DEF_FIN_0:
			switch(pTCPSESS->ucStatus)
			{
			case DEF_STATUS_SYN:
			case DEF_STATUS_DATA:
//				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_SYN : TCP_ERR_RST_E1_SYN;
				usRet = (pTCPSESS->ucRstRtx == pTCPSESS->ucSynRtx) ? TCP_ERR_RST_E1_SYN : TCP_ERR_RST_E2_SYN;
				break;
			case DEF_STATUS_SYNACK:
//				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_SYNACK : TCP_ERR_RST_E1_SYNACK;
				usRet = (pTCPSESS->ucRstRtx == pTCPSESS->ucSynRtx) ? TCP_ERR_RST_E1_SYNACK : TCP_ERR_RST_E2_SYNACK;
				break;
			case DEF_STATUS_ACK:
//				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_NOFIN : TCP_ERR_RST_E1_NOFIN;
				usRet = (pTCPSESS->ucRstRtx == pTCPSESS->ucSynRtx) ? TCP_ERR_RST_E1_NOFIN : TCP_ERR_RST_E2_NOFIN;
				break;
			default:
				log_print(LOGN_CRI, 
				"FAILCODE [RST][FIN_0] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]STATUS[%s]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
				util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
				util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
				pTCPSESS->uiSessCreateTime, pTCPSESS->uiSessCreateTime,
				PrintFinStatus(pTCPSESS->ucFinStatus), PrintEndStatus(pTCPSESS->ucEndStatus),
				PrintRtx(pTCPSESS->ucFinRtx), PrintRtx(pTCPSESS->ucEndStatus));
				break;
			}
			break;
		case DEF_FIN_1:
		case DEF_FIN_2:
#if 0
			if(pTCPSESS->ucFinRtx == DEF_FROM_SERVER) {
				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_FIN_E2 : TCP_ERR_RST_E1_FIN_E2;
			} else {
				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_FIN_E1 : TCP_ERR_RST_E1_FIN_E1;
			}
#endif
			if(pTCPSESS->ucFinRtx == pTCPSESS->ucSynRtx) {
				usRet = (pTCPSESS->ucRstRtx == pTCPSESS->ucSynRtx) ? TCP_ERR_RST_E1_FIN_E1 : TCP_ERR_RST_E2_FIN_E1;
			} else {
				usRet = (pTCPSESS->ucRstRtx == pTCPSESS->ucSynRtx) ? TCP_ERR_RST_E1_FIN_E2 : TCP_ERR_RST_E2_FIN_E2;
			}
			break;

		default:
			log_print(LOGN_CRI, 
			"FAILCODE [RST] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]STATUS[%s]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
			util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
			util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
			pTCPSESS->uiSessCreateTime, pTCPSESS->uiSessCreateTime,
			PrintFinStatus(pTCPSESS->ucFinStatus), PrintEndStatus(pTCPSESS->ucEndStatus),
			PrintRtx(pTCPSESS->ucFinRtx), PrintRtx(pTCPSESS->ucEndStatus));
			break;
		}		
		break;

	default:
		log_print(LOGN_CRI, 
		"FAILCODE [UNKNOWN] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]STATUS[%s]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
		util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
		util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
		pTCPSESS->uiSessCreateTime, pTCPSESS->uiSessCreateTime,
		PrintFinStatus(pTCPSESS->ucFinStatus), PrintEndStatus(pTCPSESS->ucEndStatus),
		PrintRtx(pTCPSESS->ucFinRtx), PrintRtx(pTCPSESS->ucEndStatus));
		break;

	}

	return usRet;
}

/** dHttpInit function.
 *
 *  dHttpInit Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime)
{
	S64		gapTime;

	gapTime = (((S64)endtime * 1000000 + (S64)endmtime) - ((S64)starttime * 1000000 + (S64)startmtime));

	if(gapTime < 0)
		gapTime = 0;

	return gapTime;
}

U32 GetGap32Time(U32 endtime, U32 endmtime, U32 starttime, U32 startmtime)
{               
	S64		gapTime;
	U32		gap = 0;
        
	if((starttime > 0) && (endtime > 0))
	{
		gapTime = (((S64)endtime * 1000000 + (S64)endmtime) - ((S64)starttime * 1000000 + (S64)startmtime));
    
		if(gapTime < 0)
			gapTime = 0;
            
		gap = gapTime;
    }
    
	return gap;
} 

U8 *PrintRtx(U8 ucRtxType)
{
	return ((ucRtxType == DEF_FROM_SERVER) ? "FROM_SERVER" : "FROM_CLIENT");
}

U8 *PrintControl(U8 ucControl)
{
	switch(ucControl)
	{
	case TCP_SYNACK:	return "SYNACK";
	case TCP_SYN:		return "SYN";
	case TCP_RST:		return "RST";
	case TCP_FIN:		return "FIN";
	case TCP_ACK:		return "ACK";
	default:			return "UNKNOWN";
	}
}

U8 *PrintFinStatus(U8 ucFinStatus)
{
	switch(ucFinStatus)
	{
	case DEF_FIN_0:		return "NO FIN";
	case DEF_FIN_1:		return "FIN 1";
	case DEF_FIN_2:		return "FIN 2";
	case DEF_FIN_3:		return "FINACK";
	default:			return "UNKNOWN";
	}
}

U8 *PrintEndStatus(U8 ucEndStatus)
{
	switch(ucEndStatus)
	{
	case DEF_END_NORMAL:	return "SUCC";
	case DEF_END_ABNORMAL:	return "ABNORMAL";
	case DEF_END_LONGLAST:	return "LONGLAST";
	case DEF_END_RST:		return "RST";
	default:				return "UNKNOWN";
	}
}

void SetData(TCP_SESS *pTCPSESS, TCP_INFO *pTCPINFO)
{
	pTCPINFO->ucTcpClientStatus = pTCPSESS->ucTcpClientStatus;
	pTCPINFO->ucTcpServerStatus = pTCPSESS->ucTcpServerStatus;

	pTCPINFO->uiIPDataUpPktCnt = pTCPSESS->uiIPDataUpPktCnt;
	pTCPINFO->uiIPDataDnPktCnt = pTCPSESS->uiIPDataDnPktCnt;
	pTCPINFO->uiIPTotUpPktCnt = pTCPSESS->uiIPTotUpPktCnt;
	pTCPINFO->uiIPTotDnPktCnt = pTCPSESS->uiIPTotDnPktCnt;
	pTCPINFO->uiIPDataUpRetransCnt = pTCPSESS->uiIPDataUpRetransCnt;
	pTCPINFO->uiIPDataDnRetransCnt = pTCPSESS->uiIPDataDnRetransCnt;
	pTCPINFO->uiIPTotUpRetransCnt = pTCPSESS->uiIPTotUpRetransCnt;
	pTCPINFO->uiIPTotDnRetransCnt = pTCPSESS->uiIPTotDnRetransCnt;
	pTCPINFO->uiIPDataUpPktSize = pTCPSESS->uiIPDataUpPktSize;
	pTCPINFO->uiIPDataDnPktSize = pTCPSESS->uiIPDataDnPktSize;
	pTCPINFO->uiIPTotUpPktSize = pTCPSESS->uiIPTotUpPktSize;
	pTCPINFO->uiIPTotDnPktSize = pTCPSESS->uiIPTotDnPktSize;
	pTCPINFO->uiIPDataUpRetransSize = pTCPSESS->uiIPDataUpRetransSize;
	pTCPINFO->uiIPDataDnRetransSize = pTCPSESS->uiIPDataDnRetransSize;
	pTCPINFO->uiIPTotUpRetransSize = pTCPSESS->uiIPTotUpRetransSize;
	pTCPINFO->uiIPTotDnRetransSize = pTCPSESS->uiIPTotDnRetransSize;

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
}

S32	dGetProcID(U32 uiClientIP, U16 usServerPort)
{
	switch(usServerPort)
	{
		case 80:
		case 8080:
			return SEQ_PROC_A_IHTTP + ( uiClientIP % gAHTTPCnt );
	}
	return 0;
}

S32 dGetCALLProcID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

void PrintSEQ(stMEMSINFO *pMEMSINFO, U8 *pNode, U8 *PrefixStr)
{
	TCP_INFO 	*pTCPINFO;

	if((pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, nifo_offset(pMEMSINFO, pNode))) != NULL) {
		log_print(LOGN_INFO, LH"[%s]+==+ OFFSET[%d][%p] SEQ[%u]DATASIZE[%u]", 
			LT, PrefixStr,
			nifo_offset(pMEMSINFO, pNode), pNode, pTCPINFO->uiSeqNum, pTCPINFO->uiDataSize);
	}
}

S32 CalcRetrans(TCP_INFO *pNEW, U32 baseSEQ, U32 oldSEQ, U32 oldDataSize, S32 *pRetransSize)
{
	U32		newStartSEQ = OFFSET_SEQ(baseSEQ, pNEW->uiSeqNum);
	U32		newEndSEQ = OFFSET_SEQ(baseSEQ, pNEW->uiSeqNum) + pNEW->uiDataSize - 1;
	U32		oldStartSEQ = OFFSET_SEQ(baseSEQ, oldSEQ);
	U32		oldEndSEQ = OFFSET_SEQ(baseSEQ, oldSEQ) + oldDataSize - 1;
//	S32		startGAP, endGAP;
	S64		startGAP, endGAP;

	S32		result;


	*pRetransSize = 0;
	if(oldEndSEQ < newStartSEQ) {			/* Normal Case */
		return DEF_NORETRANS_OVER;
	} else if(oldStartSEQ > newEndSEQ) {	/* Next Step is to check PreNode for Retransmission */
		return DEF_NORETRANS_UNDER;
	} 

	/* This Node is Retransmission and Old Node is associated */ 
#if 0
	startGAP = newStartSEQ - oldStartSEQ;
	endGAP = newEndSEQ - oldEndSEQ;
#endif
	startGAP = ((S64)newStartSEQ - (S64)oldStartSEQ);
	endGAP = ((S64)newEndSEQ - (S64)oldEndSEQ);

log_print(LOGN_INFO, "NEW S[%u]E[%u] END S[%u]E[%u] GAP S[%d]E[%d]", newStartSEQ, newEndSEQ, oldStartSEQ, oldEndSEQ, startGAP, endGAP);
log_print(LOGN_INFO, "ORIGIN NEW S[%u]E[%u] END S[%u]E[%u]", 
pNEW->uiSeqNum, pNEW->uiSeqNum + pNEW->uiDataSize - 1, oldSEQ, oldSEQ + oldDataSize - 1);

	if(startGAP == 0) {		/* This Node is same start sequence with Old Node */		
		if(endGAP <= 0) {	/* This Node is subset of Old Node */
			*pRetransSize += pNEW->uiDataSize;
			result = DEF_RETRANS_SUBSET;
		} else {			/* Old Node is subset of This Node */
			*pRetransSize = oldDataSize;
			pNEW->uiSOffset += *pRetransSize;
			pNEW->uiDataSize -= *pRetransSize;
			pNEW->uiSeqNum += *pRetransSize;
			result = DEF_RETRANS_OVER;
		}
	} else if(startGAP < 0) {	/* This Node is under start sequence with Old Node */
		if(endGAP <= 0) {		/* This Node's Seq + data size is between Old Node's Seq and Seq + data size OR */	
								/* This Node's Seq + data size is Old Node Seq + data size, 
								   for reducing time to remake linked-list */
			*pRetransSize = newEndSEQ - oldStartSEQ + 1;
			pNEW->uiDataSize -= *pRetransSize;
			result = DEF_RETRANS_UNDER;
		} else {				/* Old Node is subset of This Node */
								/* except this Node's Seq + data size is Old Node Seq + data size */
			*pRetransSize = oldDataSize;
			result = DEF_RETRANS_RSUBSET;
		}
	} else { /* This Node's Seq is between start Seq and start seq + data size of Old Node */
		if(endGAP <= 0) { 	/* This Node is subset of Old Node */
			*pRetransSize = pNEW->uiDataSize;
			result = DEF_RETRANS_SUBSET;
		} else {			/* This Node's start Seq + data size is over with Old Node's Seq + data size */ 
			*pRetransSize = oldEndSEQ - newStartSEQ + 1;
			pNEW->uiSOffset += *pRetransSize;
			pNEW->uiDataSize -= *pRetransSize;
			pNEW->uiSeqNum += *pRetransSize;
			result = DEF_RETRANS_OVER;
		}
	}

	return result;
}

S32 CalcBase(TCP_INFO *pNEW, U32 baseSEQ, U32 oldSEQ, U32 oldDataSize, S32 *pRetransSize)
{
	U32		newStartSEQ = OFFSET_SEQ(baseSEQ, pNEW->uiSeqNum);
	U32		newEndSEQ = OFFSET_SEQ(baseSEQ, pNEW->uiSeqNum) + pNEW->uiDataSize - 1;
	U32		oldStartSEQ = OFFSET_SEQ(baseSEQ, oldSEQ);
	U32		oldEndSEQ = OFFSET_SEQ(baseSEQ, oldSEQ) + oldDataSize - 1;
//	S32		startGAP, endGAP;
	S64		startGAP, endGAP;

	S32		result;

	*pRetransSize = 0;
	if(oldEndSEQ < newStartSEQ) {			/* Normal Case */
		return DEF_NORETRANS_OVER;
	} else if(oldStartSEQ > newEndSEQ) {	/* Next Step is to check PreNode for Retransmission */
		*pRetransSize = pNEW->uiDataSize;
		return DEF_RETRANS_UNDER;
	} 

	/* This Node is Retransmission and Old Node is associated */ 
#if 0
	startGAP = newStartSEQ - oldStartSEQ;
	endGAP = newEndSEQ - oldEndSEQ;
#endif
	startGAP = ((S64)newStartSEQ - (S64)oldStartSEQ);
	endGAP = ((S64)newEndSEQ - (S64)oldEndSEQ);
	if(startGAP == 0) {		/* This Node is same start sequence with Old Node */		
		if(endGAP <= 0) {	/* This Node is subset of Old Node */
			*pRetransSize = pNEW->uiDataSize;
			result = DEF_RETRANS_SUBSET;
		} else {			/* Old Node is subset of This Node */
			*pRetransSize = oldDataSize;
			pNEW->uiSOffset += *pRetransSize;
			pNEW->uiDataSize -= *pRetransSize;
			pNEW->uiSeqNum += *pRetransSize;
			result = DEF_RETRANS_OVER;
		}
	} else if(startGAP < 0) {	/* This Node is under start sequence with Old Node */
		if(endGAP <= 0) {		/* This Node's Seq + data size is between Old Node's Seq and Seq + data size OR */	
								/* This Node's Seq + data size is Old Node Seq + data size, 
								   for reducing time to remake linked-list */
			*pRetransSize = pNEW->uiDataSize;
			result = DEF_RETRANS_UNDER;
		} else {				/* Old Node is subset of This Node */
								/* except this Node's Seq + data size is Old Node Seq + data size */
			*pRetransSize = oldEndSEQ - newStartSEQ + 1;
			pNEW->uiSOffset += *pRetransSize;
			pNEW->uiDataSize -= *pRetransSize;
			pNEW->uiSeqNum += *pRetransSize;
			result = DEF_RETRANS_OVER;
		}
	} else { /* This Node's Seq is between start Seq and start seq + data size of Old Node */
		if(endGAP <= 0) { 	/* This Node is subset of Old Node */
			*pRetransSize = pNEW->uiDataSize;
			result = DEF_RETRANS_SUBSET;
		} else {			/* This Node's start Seq + data size is over with Old Node's Seq + data size */ 
			*pRetransSize = oldEndSEQ - newStartSEQ + 1;
			pNEW->uiSOffset += *pRetransSize;
			pNEW->uiDataSize -= *pRetransSize;
			pNEW->uiSeqNum += *pRetransSize;
			result = DEF_RETRANS_OVER;
		}
	}

	return result;
}

/**
 *  $Log: itcp_util.c,v $
 *  Revision 1.3  2011/09/07 06:30:47  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/09/05 12:26:40  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 13:02:40  hhbaek
 *  A_ITCP
 *
 *  Revision 1.2  2011/08/10 09:57:44  uamyd
 *  modified and block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.7  2011/05/09 10:00:30  dark264sh
 *  A_ITCP: A_CALL multi 처리
 *
 *  Revision 1.6  2011/05/06 10:19:53  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2011/04/19 07:04:26  dark264sh
 *  A_ITCP: 에러코드 세팅시 SynRtx 고려 하도록 변경
 *
 *  Revision 1.4  2011/04/16 10:47:07  dark264sh
 *  A_ITCP: Server Port가 80, 8080인 경우만 A_IHTTP로 전송
 *
 *  Revision 1.3  2011/04/16 08:44:01  dark264sh
 *  A_ITCP: uiTcpSynAckGapTime 처리
 *
 *  Revision 1.2  2011/04/16 08:23:18  dark264sh
 *  A_ITCP: SYN, SYNACK 없이 DATA 있는 경우 세션 생성
 *
 *  Revision 1.1  2011/04/12 02:51:50  dark264sh
 *  A_ITCP 추가
 *
 **/
