/**		@file	msrpm_util.c
 * 		- MSRP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpm_util.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "loglib.h"

#include "msrp.h"
#include "msrpm_util.h"

void MakeHashKey(TCP_INFO *pstTCPINFO, st_MSRPM_TSESS_KEY *pstMSRPMTSESSKEY)
{
	pstMSRPMTSESSKEY->uiCliIP = pstTCPINFO->uiCliIP;
	pstMSRPMTSESSKEY->uiSrvIP = pstTCPINFO->uiSrvIP;
	pstMSRPMTSESSKEY->usCliPort = pstTCPINFO->usCliPort;
	pstMSRPMTSESSKEY->usSrvPort = pstTCPINFO->usSrvPort;
	pstMSRPMTSESSKEY->dReserved = 0;
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

char *PrintMethod(S32 dMethod)
{
	switch(dMethod)
	{
	case MSRP_METHOD_RESPONSE:		return "RESPONSE";
	case MSRP_METHOD_SEND:			return "SEND";
	case MSRP_METHOD_AUTH:			return "AUTH";
	case MSRP_METHOD_REPORT:		return "REPORT";
	default:						return "UNKNOWN";
	}
}

S32 dCheckStart(U8 *pData, U32 uiLen, U8 *szTID)
{
	U16			usMethod = 0;
	S32			dRet;

	if((dRet = msrpheader((char*)pData, uiLen, &usMethod, (char*)szTID)) != 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d FAIL PARSING dRet=%d", 
					__FILE__, __FUNCTION__, __LINE__, dRet);
		return 0;	
	}

	if(szTID[0] == 0x00) {
		log_print(LOGN_INFO, "%s msrpheader NO START TID IS NULL", __FUNCTION__);
		return 0;
	}

	log_print(LOGN_INFO, "PARSING Method=%d:%s TID=%s", usMethod, PrintMethod(usMethod), szTID);
	return 1;
}

void InitCount(st_MSRPM_TSESS *pstMSRPMTSESS, U8 ucRtx)
{
	if(ucRtx == DEF_FROM_CLIENT) {
		pstMSRPMTSESS->uiIPDataUpPktCnt = 0;
		pstMSRPMTSESS->uiIPTotUpPktCnt = 0;
		pstMSRPMTSESS->uiIPDataUpRetransCnt = 0;
		pstMSRPMTSESS->uiIPTotUpRetransCnt = 0;
		pstMSRPMTSESS->uiIPDataUpPktSize = 0;
		pstMSRPMTSESS->uiIPTotUpPktSize = 0;
		pstMSRPMTSESS->uiIPDataUpRetransSize = 0;
		pstMSRPMTSESS->uiIPTotUpRetransSize = 0;
	} else {
		pstMSRPMTSESS->uiIPDataDnPktCnt = 0;
		pstMSRPMTSESS->uiIPTotDnPktCnt = 0;
		pstMSRPMTSESS->uiIPDataDnRetransCnt = 0;
		pstMSRPMTSESS->uiIPTotDnRetransCnt = 0;
		pstMSRPMTSESS->uiIPDataDnPktSize = 0;
		pstMSRPMTSESS->uiIPTotDnPktSize = 0;
		pstMSRPMTSESS->uiIPDataDnRetransSize = 0;
		pstMSRPMTSESS->uiIPTotDnRetransSize = 0;
	}
}

void UpCount(TCP_INFO *pstTCPINFO, st_MSRPM_TSESS *pstMSRPMTSESS)
{
    pstMSRPMTSESS->uiIPDataUpPktCnt += pstTCPINFO->uiIPDataUpPktCnt;
    pstMSRPMTSESS->uiIPDataDnPktCnt += pstTCPINFO->uiIPDataDnPktCnt;
    pstMSRPMTSESS->uiIPTotUpPktCnt += pstTCPINFO->uiIPTotUpPktCnt;
    pstMSRPMTSESS->uiIPTotDnPktCnt += pstTCPINFO->uiIPTotDnPktCnt;
    pstMSRPMTSESS->uiIPDataUpRetransCnt += pstTCPINFO->uiIPDataUpRetransCnt;
    pstMSRPMTSESS->uiIPDataDnRetransCnt += pstTCPINFO->uiIPDataDnRetransCnt;
    pstMSRPMTSESS->uiIPTotUpRetransCnt += pstTCPINFO->uiIPTotUpRetransCnt;
    pstMSRPMTSESS->uiIPTotDnRetransCnt += pstTCPINFO->uiIPTotDnRetransCnt;
    pstMSRPMTSESS->uiIPDataUpPktSize += pstTCPINFO->uiIPDataUpPktSize;
    pstMSRPMTSESS->uiIPDataDnPktSize += pstTCPINFO->uiIPDataDnPktSize;
    pstMSRPMTSESS->uiIPTotUpPktSize += pstTCPINFO->uiIPTotUpPktSize;
    pstMSRPMTSESS->uiIPTotDnPktSize += pstTCPINFO->uiIPTotDnPktSize;
    pstMSRPMTSESS->uiIPDataUpRetransSize += pstTCPINFO->uiIPDataUpRetransSize; 
    pstMSRPMTSESS->uiIPDataDnRetransSize += pstTCPINFO->uiIPDataDnRetransSize;
    pstMSRPMTSESS->uiIPTotUpRetransSize += pstTCPINFO->uiIPTotUpRetransSize;
    pstMSRPMTSESS->uiIPTotDnRetransSize += pstTCPINFO->uiIPTotDnRetransSize;
}

/*
 * $Log: msrpm_util.c,v $
 * Revision 1.3  2011/09/07 06:30:47  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 05:43:38  uamyd
 * MSRPM modified
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/17 12:12:18  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/08 11:05:43  uamyd
 * modified block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.2  2011/01/11 04:09:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.1.1.1  2009/05/26 02:14:40  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1  2008/09/18 06:35:03  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:42  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.1  2007/05/07 01:46:17  dark264sh
 * INIT
 *
 */
