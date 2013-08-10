/**		@file	tcp_util.c
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: tcp_util.c,v 1.3 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
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

/* SYS HEADER */
#include <arpa/inet.h>
/* LIB HEADER */
#include "loglib.h"
#include "utillib.h"
#include "Analyze_Ext_Abs.h"
/* PRO HEADER */
#include "procid.h"
#include "capdef.h"
#include "sockio.h"
#include "common_stg.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "tcp_util.h"

extern int 	gAHTTPCnt;
extern int 	gACALLCnt;

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
			usRet = (pTCPSESS->ucFinRtx == DEF_FROM_SERVER) ? TCP_NOERR_FIN_E2 : TCP_NOERR_FIN_E1;
			break;
		default:
			log_print(LOGN_CRI, 
			"FAILCODE [NORMAL] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
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
				"FAILCODE [LONGLAST][FIN_0] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
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
			usRet = (pTCPSESS->ucFinRtx == DEF_FROM_SERVER) ? LONGLAST_FIN_E2 : LONGLAST_FIN_E1;
			break;

		default:
			log_print(LOGN_CRI, 
			"FAILCODE [LONGLAST] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
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
				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_SYN : TCP_ERR_RST_E1_SYN;
				break;
			case DEF_STATUS_SYNACK:
				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_SYNACK : TCP_ERR_RST_E1_SYNACK;
				break;
			case DEF_STATUS_ACK:
				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_NOFIN : TCP_ERR_RST_E1_NOFIN;
				break;
			default:
				log_print(LOGN_CRI, 
				"FAILCODE [RST][FIN_0] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
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
			if(pTCPSESS->ucFinRtx == DEF_FROM_SERVER) {
				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_FIN_E2 : TCP_ERR_RST_E1_FIN_E2;
			} else {
				usRet = (pTCPSESS->ucRstRtx == DEF_FROM_SERVER) ? TCP_ERR_RST_E2_FIN_E1 : TCP_ERR_RST_E1_FIN_E1;
			}
			break;

		default:
			log_print(LOGN_CRI, 
			"FAILCODE [RST] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
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
		"FAILCODE [UNKNOWN] SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]STIME[%u]MTIME[%d]FINSTATUS[%s]ENDSTATUS[%s]FINRTX[%s]RSTRTX[%s]", 
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

U8 *PrintRtx(U8 ucRtxType)
{
	return (U8*)((ucRtxType == DEF_FROM_SERVER) ? "FROM_SERVER" : "FROM_CLIENT");
}

U8 *PrintControl(U8 ucControl)
{
	switch(ucControl)
	{
	case TCP_SYNACK:	return (U8*)"SYNACK";
	case TCP_SYN:		return (U8*)"SYN";
	case TCP_RST:		return (U8*)"RST";
	case TCP_FIN:		return (U8*)"FIN";
	case TCP_ACK:		return (U8*)"ACK";
	default:			return (U8*)"UNKNOWN";
	}
}

U8 *PrintFinStatus(U8 ucFinStatus)
{
	switch(ucFinStatus)
	{
	case DEF_FIN_0:		return (U8*)"NO FIN";
	case DEF_FIN_1:		return (U8*)"FIN 1";
	case DEF_FIN_2:		return (U8*)"FIN 2";
	case DEF_FIN_3:		return (U8*)"FINACK";
	default:			return (U8*)"UNKNOWN";
	}
}

U8 *PrintEndStatus(U8 ucEndStatus)
{
	switch(ucEndStatus)
	{
	case DEF_END_NORMAL:	return (U8*)"SUCC";
	case DEF_END_ABNORMAL:	return (U8*)"ABNORMAL";
	case DEF_END_LONGLAST:	return (U8*)"LONGLAST";
	case DEF_END_RST:		return (U8*)"RST";
	default:				return (U8*)"UNKNOWN";
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

S32 dGetCALLProcID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

S32	dGetProcID(U16 usAppCode, UINT uiClientIP)
{
	S32		seqProcID = 0;
	
	switch(usAppCode)
	{
		case SEQ_PROC_A_TCP0:
		case SEQ_PROC_A_JNC:
		case SEQ_PROC_A_IV:
		case SEQ_PROC_A_ONLINE:
		case SEQ_PROC_A_DNS:
		case SEQ_PROC_A_FTP:
			seqProcID = usAppCode;

			break;
		case SEQ_PROC_A_WAP20:
		case SEQ_PROC_A_WIPI:
		case SEQ_PROC_A_2G:
		case SEQ_PROC_A_JAVA:
		case SEQ_PROC_A_VOD:
		case SEQ_PROC_A_MMS:
		case SEQ_PROC_A_FV:
		case SEQ_PROC_A_EMS:
		case SEQ_PROC_A_FB:
		case SEQ_PROC_A_XCAP:
		case SEQ_PROC_A_WIDGET:

			seqProcID = SEQ_PROC_A_HTTP + ( uiClientIP % gAHTTPCnt );
			break;
		case SEQ_PROC_A_MSRPM:
		case SEQ_PROC_A_MSRPT:
			seqProcID = SEQ_PROC_A_MSRPM;
			break;
		case SEQ_PROC_A_SIPM:
		case SEQ_PROC_A_SIPT:
			seqProcID = SEQ_PROC_A_SIPM;
			break;

		case SEQ_PROC_A_CALL0:
			seqProcID = dGetCALLProcID(uiClientIP);
			break;

		default:
			log_print(LOGN_CRI, "[%s][%s.%d] UNKNOWN APPCODE[%d]", __FILE__, __FUNCTION__, __LINE__, usAppCode);
			seqProcID = 0;
			break;
	}
	return seqProcID;
}

void PrintSEQ(stMEMSINFO *pMEMSINFO, U8 *pNode, U8 *PrefixStr)
{
	TCP_INFO 	*pTCPINFO;

	if((pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, nifo_offset(pMEMSINFO, pNode))) != NULL) {
		log_print(LOGN_INFO, LH"[%s]+==+ OFFSET[%ld][%s] SEQ[%u]DATASIZE[%u]" ,
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
	S32		startGAP, endGAP;

	S32		result;


	*pRetransSize = 0;
	if(oldEndSEQ < newStartSEQ) {			/* Normal Case */
		return DEF_NORETRANS_OVER;
	} else if(oldStartSEQ > newEndSEQ) {	/* Next Step is to check PreNode for Retransmission */
		return DEF_NORETRANS_UNDER;
	} 

	/* This Node is Retransmission and Old Node is associated */ 
	startGAP = newStartSEQ - oldStartSEQ;
	endGAP = newEndSEQ - oldEndSEQ;

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
	S32		startGAP, endGAP;

	S32		result;

	*pRetransSize = 0;
	if(oldEndSEQ < newStartSEQ) {			/* Normal Case */
		return DEF_NORETRANS_OVER;
	} else if(oldStartSEQ > newEndSEQ) {	/* Next Step is to check PreNode for Retransmission */
		*pRetransSize = pNEW->uiDataSize;
		return DEF_RETRANS_UNDER;
	} 

	/* This Node is Retransmission and Old Node is associated */ 
	startGAP = newStartSEQ - oldStartSEQ;
	endGAP = newEndSEQ - oldEndSEQ;
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
 *  $Log: tcp_util.c,v $
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
 *  Revision 1.12  2011/05/09 15:03:35  dark264sh
 *  A_TCP: A_CALL multi 처리
 *
 *  Revision 1.11  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.10  2009/08/23 13:24:46  jsyoon
 *  ADD S_MSGQ_ETC
 *
 *  Revision 1.9  2009/08/01 08:46:11  dqms
 *  *** empty log message ***
 *
 *  Revision 1.8  2009/07/29 07:02:34  dqms
 *  *** empty log message ***
 *
 *  Revision 1.7  2009/07/26 09:13:13  dqms
 *  *** empty log message ***
 *
 *  Revision 1.6  2009/07/22 06:25:21  dqms
 *  *** empty log message ***
 *
 *  Revision 1.5  2009/07/15 16:11:13  dqms
 *  멀티프로세스 수정
 *
 *  Revision 1.4  2009/06/16 15:16:00  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.3  2009/06/16 11:58:36  dqms
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/06/15 08:45:42  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:34  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.16  2009/01/28 07:10:07  dark264sh
 *  A_TCP 서버 SYN 처리 | FIN 처리 버그 수정
 *
 *  Revision 1.15  2008/12/19 11:11:09  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.14  2008/12/17 09:48:48  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.13  2008/12/17 06:26:48  dark264sh
 *  LOG_MMS L7Code L4_MMS_NEW 추가
 *
 *  Revision 1.12  2008/12/17 02:41:41  dark264sh
 *  Internet 구간 관련하여 L4Code 변경
 *
 *  Revision 1.11  2008/11/25 12:50:40  dark264sh
 *  WIDGET 처리
 *
 *  Revision 1.10  2008/11/24 12:47:11  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.9  2008/11/24 07:05:47  dark264sh
 *  WIPI ONLINE 처리
 *
 *  Revision 1.8  2008/10/30 04:49:10  dark264sh
 *  no message
 *
 *  Revision 1.7  2008/09/18 07:47:39  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.6  2008/07/14 07:21:10  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2008/06/19 11:49:05  dark264sh
 *  오늘은 서비스 code 변경에 따른 변경
 *
 *  Revision 1.4  2008/06/18 12:27:18  dark264sh
 *  A_FB 추가
 *
 *  Revision 1.3  2008/06/18 10:39:34  dark264sh
 *  A_TCP IV관련 routing 처리
 *
 *  Revision 1.2  2008/06/17 12:24:21  dark264sh
 *  A_FV, A_EMS 추가
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.4  2007/10/08 04:53:31  dark264sh
 *  no message
 *
 *  Revision 1.3  2007/09/03 05:29:18  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2007/08/27 13:58:23  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/08/21 12:54:17  dark264sh
 *  no message
 *
 *  Revision 1.9  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.8  2006/11/22 06:32:01  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2006/11/20 14:33:21  dark264sh
 *  *** empty log message ***
 *
 **/
