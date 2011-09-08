/**		@file	sipm_util.c
 * 		- SIP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: sipm_util.c,v 1.3 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// LIB
#include "loglib.h"

// .
#include "sipm_util.h"

/**
 *	Implement func.
 */
void MakeHashKey(TCP_INFO *pstTCPINFO, st_SIPM_TSESS_KEY *pstSIPMTSESSKEY)
{
	pstSIPMTSESSKEY->uiCliIP = pstTCPINFO->uiCliIP;
	pstSIPMTSESSKEY->uiSrvIP = pstTCPINFO->uiSrvIP;
	pstSIPMTSESSKEY->usCliPort = pstTCPINFO->usCliPort;
	pstSIPMTSESSKEY->usSrvPort = pstTCPINFO->usSrvPort;
	pstSIPMTSESSKEY->dReserved = 0;
}

char *PrintRtx(S32 dRtxType)
{
	switch(dRtxType)
	{
	case DEF_FROM_SERVER:			return "FROM_SERVER";
	case DEF_FROM_CLIENT:			return "FROM_CLIENT";
	case DEF_FROM_NONE:				return "FROM_NONE";
	default:						return "UNKNOWN";
	}
}

char *PrintEndStatus(S32 dEndStatus)
{
	switch(dEndStatus)
	{
	case SIPM_ENDSTATUS_EMPTY:		return "EMPTY";
	case SIPM_ENDSTATUS_0D:			return "0D";
	case SIPM_ENDSTATUS_0D0A:		return "0D0A";
	case SIPM_ENDSTATUS_0D0A0D:		return "0D0A0D";
	case SIPM_ENDSTATUS_0D0A0D0A:	return "0D0A0D0A";
	case SIPM_ENDSTATUS_BODY:		return "BODY";
	case SIPM_ENDSTATUS_END:		return "END";
	default:						return "UNKNOWN";
	}
}

S32 dCheckStart(U8 *pData, U32 uiLen)
{
	S32			isYes = 0;
	S32			dRet;

	if((dRet = sipheader(pData, uiLen, &isYes)) != 0) {
		log_print(LOGN_CRI, LH"FAIL PARSING dRet=%d", 
					LT, dRet);
		return 0;	
	}

	return isYes;
}

S32 dGetHDRINFO(U8 *pData, U32 uiLen)
{
	S32			contentLen = 0;
	S32			dRet;

	if((dRet = sip_contentlen(pData, uiLen, &contentLen)) != 0) {
		log_print(LOGN_CRI, LH"FAIL PARSING dRet=%d", 
					LT, dRet);
		return 0;	
	}

	return contentLen;
}

void InitCount(st_SIPM_TSESS *pstSIPMTSESS, U8 ucRtx)
{
	if(ucRtx == DEF_FROM_CLIENT) {
		pstSIPMTSESS->uiIPDataUpPktCnt = 0;
		pstSIPMTSESS->uiIPTotUpPktCnt = 0;
		pstSIPMTSESS->uiIPDataUpRetransCnt = 0;
		pstSIPMTSESS->uiIPTotUpRetransCnt = 0;
		pstSIPMTSESS->uiIPDataUpPktSize = 0;
		pstSIPMTSESS->uiIPTotUpPktSize = 0;
		pstSIPMTSESS->uiIPDataUpRetransSize = 0;
		pstSIPMTSESS->uiIPTotUpRetransSize = 0;
	} else {
		pstSIPMTSESS->uiIPDataDnPktCnt = 0;
		pstSIPMTSESS->uiIPTotDnPktCnt = 0;
		pstSIPMTSESS->uiIPDataDnRetransCnt = 0;
		pstSIPMTSESS->uiIPTotDnRetransCnt = 0;
		pstSIPMTSESS->uiIPDataDnPktSize = 0;
		pstSIPMTSESS->uiIPTotDnPktSize = 0;
		pstSIPMTSESS->uiIPDataDnRetransSize = 0;
		pstSIPMTSESS->uiIPTotDnRetransSize = 0;
	}
}

void UpCount(TCP_INFO *pstTCPINFO, st_SIPM_TSESS *pstSIPMTSESS)
{
    pstSIPMTSESS->uiIPDataUpPktCnt += pstTCPINFO->uiIPDataUpPktCnt;
    pstSIPMTSESS->uiIPDataDnPktCnt += pstTCPINFO->uiIPDataDnPktCnt;
    pstSIPMTSESS->uiIPTotUpPktCnt += pstTCPINFO->uiIPTotUpPktCnt;
    pstSIPMTSESS->uiIPTotDnPktCnt += pstTCPINFO->uiIPTotDnPktCnt;
    pstSIPMTSESS->uiIPDataUpRetransCnt += pstTCPINFO->uiIPDataUpRetransCnt;
    pstSIPMTSESS->uiIPDataDnRetransCnt += pstTCPINFO->uiIPDataDnRetransCnt;
    pstSIPMTSESS->uiIPTotUpRetransCnt += pstTCPINFO->uiIPTotUpRetransCnt;
    pstSIPMTSESS->uiIPTotDnRetransCnt += pstTCPINFO->uiIPTotDnRetransCnt;
    pstSIPMTSESS->uiIPDataUpPktSize += pstTCPINFO->uiIPDataUpPktSize;
    pstSIPMTSESS->uiIPDataDnPktSize += pstTCPINFO->uiIPDataDnPktSize;
    pstSIPMTSESS->uiIPTotUpPktSize += pstTCPINFO->uiIPTotUpPktSize;
    pstSIPMTSESS->uiIPTotDnPktSize += pstTCPINFO->uiIPTotDnPktSize;
    pstSIPMTSESS->uiIPDataUpRetransSize += pstTCPINFO->uiIPDataUpRetransSize; 
    pstSIPMTSESS->uiIPDataDnRetransSize += pstTCPINFO->uiIPDataDnRetransSize;
    pstSIPMTSESS->uiIPTotUpRetransSize += pstTCPINFO->uiIPTotUpRetransSize;
    pstSIPMTSESS->uiIPTotDnRetransSize += pstTCPINFO->uiIPTotDnRetransSize;
}

void SetTextInfo(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, TEXT_INFO *pTEXTINFO)
{
    pTEXTINFO->offset               = pINFOETH->offset;
    pTEXTINFO->len                  = pINFOETH->stUDPTCP.wDataLen;
    pTEXTINFO->uiStartTime          = pCAPHEAD->curtime;
    pTEXTINFO->uiStartMTime         = pCAPHEAD->ucurtime;
    pTEXTINFO->uiLastUpdateTime     = pCAPHEAD->curtime;
    pTEXTINFO->uiLastUpdateMTime    = pCAPHEAD->ucurtime;
    pTEXTINFO->uiAckTime            = pCAPHEAD->curtime;
    pTEXTINFO->uiAckMTime           = pCAPHEAD->ucurtime;
    if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
        pTEXTINFO->clientIP             = pINFOETH->stIP.dwSrcIP;
        pTEXTINFO->serverIP             = pINFOETH->stIP.dwDestIP;
        pTEXTINFO->clientPort           = pINFOETH->stUDPTCP.wSrcPort;
        pTEXTINFO->serverPort           = pINFOETH->stUDPTCP.wDestPort;
    } else {
        pTEXTINFO->clientIP             = pINFOETH->stIP.dwDestIP;
        pTEXTINFO->serverIP             = pINFOETH->stIP.dwSrcIP;
        pTEXTINFO->clientPort           = pINFOETH->stUDPTCP.wDestPort;
        pTEXTINFO->serverPort           = pINFOETH->stUDPTCP.wSrcPort;
    }
    pTEXTINFO->protocol             = pINFOETH->stIP.ucProtocol;
    pTEXTINFO->IPDataSize           = pINFOETH->stIP.wTotalLength;
    /*
    pTEXTINFO->range                = pstPKTINFO->stPKTCAP.dRange;
    pTEXTINFO->network              = pstPKTINFO->stPKTCAP.dNetwork;
    pTEXTINFO->rawFileIndex         = pstPKTINFO->stPKTCAP.uiFileIdx;
    pTEXTINFO->rawPacketIndex       = pstPKTINFO->stPKTCAP.uiRecordIdx;
    */
    pTEXTINFO->rtx                  = pCAPHEAD->bRtxType;
    pTEXTINFO->usL4Code             = pINFOETH->usL4Code;
}

/*
 * $Log: sipm_util.c,v $
 * Revision 1.3  2011/09/07 06:30:48  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 12:26:42  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/18 01:38:46  hhbaek
 * A_SIPM
 *
 * Revision 1.2  2011/08/09 05:31:09  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.2  2011/01/11 04:09:10  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.1.1.1  2009/05/26 02:14:35  dqms
 * Init TAF_RPPI
 *
 * Revision 1.2  2009/01/28 16:36:42  dark264sh
 * A_SIPT TCP/UDP 역전 처리
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
 * Revision 1.1  2007/05/10 02:57:30  dark264sh
 * A_SIPM (TCP Merge) 추가
 *
 */
