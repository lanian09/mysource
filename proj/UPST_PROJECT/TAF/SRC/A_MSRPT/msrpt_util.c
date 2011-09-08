/**		@file	msrpt_util.c
 * 		- MSRP Transaction을 관리 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpt_util.c,v 1.3 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
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

/**
 *	Include headers
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// LIB
#include "loglib.h"

// TAF
#include "msrp.h"

// .
#include "msrpt_util.h"

/**
 *	Implement func.
 */
char *PrintType(S32 type)
{
	switch(type)
	{
		case ETH_DATA_NUM:					return "ETH_DATA";
		case TEXT_INFO_DEF_NUM:				return "TEXT_INFO";
		default:							return "UNKNOWN";
	}
}

S32 dGetMSRPTINFO(U8 *pDATA, U32 uiLen, st_MSRPT_INFO *pstMSRPTINFO) 
{
	S32			dRet;
	U8			suri[MSRP_SURI_SIZE];
	U8			min[MAX_MIN_SIZE];
	S32			vendor = IM_VENDOR_LGT;
	S32			len = 0;

	suri[0] = 0x00;
	min[0] = 0;

	if((dRet = msrp((char*)pDATA, uiLen, &pstMSRPTINFO->usMethod, (char*)pstMSRPTINFO->szContentType, &pstMSRPTINFO->usSuccessReport, &pstMSRPTINFO->usFailureReport, (char*)pstMSRPTINFO->szToPath, (char*)pstMSRPTINFO->szFromPath, (char*)pstMSRPTINFO->szMSGID, &pstMSRPTINFO->usResCode, &pstMSRPTINFO->usEndFlag, (char*)pstMSRPTINFO->szTID,(char*) suri)) != 0)
	{
		log_print(LOGN_CRI, LH"FAIL PARSING dRet=%d",
					LT, dRet);
		return -1;
	}

	pstMSRPTINFO->usVendor = IM_VENDOR_LGT;
	if(suri[0] != 0x00) {
		len = strlen((char*)suri);
		if(msrp_min((char*)suri, len, (char*)min, &vendor) != 0) {
			log_print(LOGN_CRI, LH"PARSING FAIL len=%d SURI=%.*s", LT, len, len, suri);
			return -2;
		}

		len = strlen((char*)min);
		memcpy(pstMSRPTINFO->szMIN, min, len);
		pstMSRPTINFO->szMIN[len] = 0x00;

		pstMSRPTINFO->usVendor = vendor;
	}

	return 0;
}

S32 dGetMsgStatus(S32 dMethod, S32 dEndFlag)
{
	S32		dStatus = 0;

	switch(dMethod)
	{
	case MSRP_METHOD_SEND:
	case MSRP_METHOD_AUTH:
		switch(dEndFlag)
		{
		case MSRP_ENDFLAG_END:
			dStatus = MSRPT_MSG_REQ_END;
			break;
		case MSRP_ENDFLAG_CONTINUE:
			dStatus = MSRPT_MSG_REQ_CONTINUE;
			break;
		case MSRP_ENDFLAG_ABORT:
			dStatus = MSRPT_MSG_REQ_ABORT;
			break;
		default:
			dStatus = MSRPT_MSG_REQ_NOTHING;
			break;
		}
		break;

	case MSRP_METHOD_REPORT:
		dStatus = MSRPT_MSG_REPORT;
		break;

	case MSRP_METHOD_RESPONSE:
		dStatus = MSRPT_MSG_RES;
		break;

	default:
		log_print(LOGN_CRI, LH"UNKNOWN Method=%d", LT, dMethod);
		break;
	}

	log_print(LOGN_INFO, "%s Method=%d Msg=%d", __FUNCTION__, dMethod, dStatus);

	return dStatus;
}

char *PrintStatus(S32 dStatus)
{
	switch(dStatus)
	{
		case MSRPT_STATUS_WAIT:				return "MSRPT_STATUS_WAIT";
		case MSRPT_STATUS_START:			return "MSRPT_STATUS_START";
		default:							return "UNKNOWN";
	}
}

char *PrintMsgStatus(S32 dMsgStatus)
{
	switch(dMsgStatus)
	{
		case MSRPT_MSG_REQ_END:				return "MSRPT_MSG_REQ_END";
		case MSRPT_MSG_REQ_CONTINUE:		return "MSRPT_MSG_REQ_CONTINUE";
		case MSRPT_MSG_REQ_ABORT:			return "MSRPT_MSG_REQ_ABORT";
		case MSRPT_MSG_REQ_NOTHING:			return "MSRPT_MSG_REQ_NOTHING";
		case MSRPT_MSG_RES:					return "MSRPT_MSG_RES";
		case MSRPT_MSG_REPORT:				return "MSRPT_MSG_REPORT";
		default:							return "UNKNOWN";
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

char *PrintReport(S32 dFlag)
{
	switch(dFlag)
	{
		case MSRP_FLAG_NOTHING:			return "NOTHING";
		case MSRP_FLAG_YES:				return "YES";
		case MSRP_FLAG_NO:				return "NO";
		default:						return "UNKNOWN";
	}
}

char *PrintEndFlag(S32 dEndFlag)
{
	switch(dEndFlag)
	{
		case MSRP_ENDFLAG_NOTHING:		return "NOTHING";
		case MSRP_ENDFLAG_END:			return "END:$";
		case MSRP_ENDFLAG_CONTINUE:		return "CONTINUE:+";
		case MSRP_ENDFLAG_ABORT:		return "ABORT:#";
		default:						return "UNKNOWN";
	}
}

char *PrintFinish(S32 dFinish)
{
	switch(dFinish)
	{
		case MSRPT_ACT_CONTINUE:		return "CONTINUE";
		case MSRPT_ACT_FINISH:			return "FINISH";
		case MSRPT_ACT_DELETE:			return "DELETE";
		default:						return "UNKNOWN";
	}
}

char *PrintResStr(S32 msgtype)
{
	switch(msgtype)
	{
		case 100: return "Trying";
		case 180: return "Ringing";
		case 181: return "Call is being forwarded";
		case 182: return "Queued";
		case 183: return "Session progress";
		case 200: return "OK";
		case 202: return "Accepted";
		case 300: return "Multiple choices";
		case 301: return "Moved permanently";
		case 302: return "Moved temporarily";
		case 305: return "Use proxy";
		case 380: return "Alternative service";
		case 400: return "Bad request";
		case 401: return "Unauthorized";
		case 402: return "Payment required";
		case 403: return "Forbidden";
		case 404: return "Not found";
		case 405: return "Method not allowed";
		case 406: return "Not acceptable";
		case 407: return "Proxy authentication required";
		case 408: return "Request timeout";
		case 410: return "Gone";
		case 412: return "Conditional request failed";
		case 413: return "Request entity too large";
		case 414: return "Request-URI too long";
		case 415: return "Unsupported media type";
		case 416: return "Unsupported URI scheme";
		case 417: return "Unknown Resource-Priority";
		case 420: return "Bad extension";
		case 421: return "Extension required";
		case 422: return "Session Interval too small";
		case 423: return "Interval too brief";
		case 429: return "Provide referrer identity";
		case 480: return "Temporarily unavailable";
		case 481: return "Call/Transaction does not exist";
		case 482: return "Loop detected";
		case 483: return "Too many hops";
		case 484: return "Address incompletex";
		case 485: return "Ambiguous"; 
		case 486: return "Busy here";
		case 487: return "Request terminated";
		case 488: return "Not acceptable here";
		case 489: return "Bad event";
		case 491: return "Request pending";
		case 493: return "Undecipherable";
		case 494: return "Security agreement required";
		case 500: return "Server internal error";
		case 501: return "Not implemented";
		case 502: return "Bad gateway"; 
		case 503: return "Service unavailable";
		case 504: return "Server timeout";
		case 505: return "Version not supported";
		case 513: return "Message too large";
		case 580: return "Precondition Failure";
		case 600: return "Busy everywhere";
		case 603: return "Decline";
		case 604: return "Does not exist anywhere";
		case 606: return "Not acceptable";
		default: return "UNKNOWN";
	}
}

S32 dGetL7TYPE(S32 dRtx)
{
	switch(dRtx)
	{
	case DEF_FROM_CLIENT:   return APP_IM_UP;
	case DEF_FROM_SERVER:   return APP_IM_DN;
	default:                return APP_ETC;
	}
}

int dGetLen(int dDataLen, char *data)
{
	int	i, status = MSRPEND_STATE_EMPTY;

	for(i = 0; i < dDataLen; i++)
	{
		switch(status)
		{
		case MSRPEND_STATE_EMPTY:
			switch(data[i])
			{
			case 0x0D:
				status = MSRPEND_STATE_0D;
				break;
			default:
				status = MSRPEND_STATE_EMPTY;
				break;
			}
			break;
		case MSRPEND_STATE_0D:
			switch(data[i])
			{
			case 0x0A:
				status = MSRPEND_STATE_0D0A;
				break;
			case 0x0D:
				status = MSRPEND_STATE_0D;
				break;
			default:
				status = MSRPEND_STATE_EMPTY;
				break;
			}
			break;
		case MSRPEND_STATE_0D0A:
			switch(data[i])
			{
			case 0x0D:
				status = MSRPEND_STATE_0D0A0D;
				break;
			default:
				status = MSRPEND_STATE_EMPTY;
				break;
			}
			break;
		case MSRPEND_STATE_0D0A0D:
			switch(data[i])
			{
			case 0x0A:
				status = MSRPEND_STATE_0D0A0D0A;
				break;
			case 0x0D:
				status = MSRPEND_STATE_0D;
				break;
			default:
				status = MSRPEND_STATE_EMPTY;
				break;
			}
			break;
		default:
			log_print(LOGN_CRI, LH"INVALID STATUS %d", LT, status);
			return dDataLen;
		}

		if(status == MSRPEND_STATE_0D0A0D0A) 
		{
			log_print(LOGN_INFO, "### FINISH %d\n", i+1);
			return i+1;
		}
	}

	return dDataLen;
}

void InitLog(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, TEXT_INFO *pstTEXTINFO, LOG_MSRP_TRANS *pLOG)
{
	/* Call Common */
	pLOG->uiCallTime = pstTEXTINFO->uiStartTime;
	pLOG->uiCallMTime = pstTEXTINFO->uiStartMTime;
//	pLOG->uiNASName = pstTEXTINFO->clientIP;
	pLOG->uiNASName = 0;

	if(pLOG->usSvcL7Type == APP_IM_UP) {
		memcpy(pLOG->szMIN, pstMSRPTINFO->szMIN, MAX_MIN_LEN);
		pLOG->szMIN[MAX_MIN_LEN] = 0x00;
	}

	pLOG->Vendor = pstMSRPTINFO->usVendor;

	/* LOG */
    pLOG->TransStartTime = pstTEXTINFO->uiStartTime;
    pLOG->TransStartMTime = pstTEXTINFO->uiStartMTime;
	memcpy(pLOG->MSGID, pstMSRPTINFO->szMSGID, MSRP_MSGID_LEN);
	pLOG->MSGID[MSRP_MSGID_LEN] = 0x00;
	pLOG->method = pstMSRPTINFO->usMethod;
	sprintf((char*)pLOG->MethodString, "%s", PrintMethod(pstMSRPTINFO->usMethod));
	memcpy(pLOG->ToPath, pstMSRPTINFO->szToPath, MSRP_PATH_LEN);
	pLOG->ToPath[MSRP_PATH_LEN] = 0x00;
	memcpy(pLOG->FromPath, pstMSRPTINFO->szFromPath, MSRP_PATH_LEN);
	pLOG->FromPath[MSRP_PATH_LEN] = 0x00;
	pLOG->SuccessReport = pstMSRPTINFO->usSuccessReport;
	pLOG->FailureReport = pstMSRPTINFO->usFailureReport;
	memcpy(pLOG->ContentsType, pstMSRPTINFO->szContentType, MSRP_CONTENTTYPE_LEN);
	pLOG->ContentsType[MSRP_CONTENTTYPE_LEN] = 0x00;
    pLOG->SrcIP = pstTEXTINFO->clientIP;
    pLOG->DestIP = pstTEXTINFO->serverIP;
    pLOG->SrcPort = pstTEXTINFO->clientPort;
    pLOG->DestPort = pstTEXTINFO->serverPort;
	if(pstTEXTINFO->uiAckTime == 0) {
    	pLOG->TransEndTime = pstTEXTINFO->uiStartTime;
    	pLOG->TransEndMTime = pstTEXTINFO->uiStartMTime;
	} else {
    	pLOG->TransEndTime = pstTEXTINFO->uiAckTime;
    	pLOG->TransEndMTime = pstTEXTINFO->uiAckMTime;
	}
}

void SetNormalReq(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, TEXT_INFO *pstTEXTINFO, LOG_MSRP_TRANS *pLOG)
{
	/* Transaction Common 
	pLOG->EndTime = pstTEXTINFO->uiStartTime;
	pLOG->EndMTime = pstTEXTINFO->uiStartMTime;
	*/
	/* LOG */
	if(pstTEXTINFO->uiAckTime == 0) {
    	pLOG->TransEndTime = pstTEXTINFO->uiStartTime;
    	pLOG->TransEndMTime = pstTEXTINFO->uiStartMTime;
	} else {
    	pLOG->TransEndTime = pstTEXTINFO->uiAckTime;
    	pLOG->TransEndMTime = pstTEXTINFO->uiAckMTime;
	}
	pLOG->TotalReqCnt++;
	pLOG->ReqBodySize += pstMSRPTINFO->usBodyLen;
	pLOG->ReqDataSize += pstTEXTINFO->len;
	pLOG->ReqIPDataSize += pstTEXTINFO->IPDataSize;
	pLOG->EndStatus = MSRP_ENDSTATUS_REQ;
}

void SetNormalRes(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, TEXT_INFO *pstTEXTINFO, LOG_MSRP_TRANS *pLOG)
{
	/* Transaction Common 
	pLOG->EndTime = pstTEXTINFO->uiStartTime;
	pLOG->EndMTime = pstTEXTINFO->uiStartMTime;
	*/

	/* LOG */
	if(pstTEXTINFO->uiAckTime == 0) {
    	pLOG->TransEndTime = pstTEXTINFO->uiStartTime;
    	pLOG->TransEndMTime = pstTEXTINFO->uiStartMTime;
	} else {
    	pLOG->TransEndTime = pstTEXTINFO->uiAckTime;
    	pLOG->TransEndMTime = pstTEXTINFO->uiAckMTime;
	}
	if(pLOG->ResTime == 0) {
		pLOG->ResTime = pstTEXTINFO->uiStartTime;
		pLOG->ResMTime = pstTEXTINFO->uiStartMTime;
	}
	pLOG->ResCode = pstMSRPTINFO->usResCode;
	pLOG->TotalResCnt++;
	pLOG->ResBodySize += pstMSRPTINFO->usBodyLen;
	pLOG->ResDataSize += pstTEXTINFO->len;
	pLOG->ResIPDataSize += pstTEXTINFO->IPDataSize;
	pLOG->EndStatus = MSRP_ENDSTATUS_RES;
}

void SetNormalReport(stMEMSINFO *pstMEMSINFO, st_MSRPT_INFO *pstMSRPTINFO, TEXT_INFO *pstTEXTINFO, LOG_MSRP_TRANS *pLOG)
{
	/* Transaction Common 
	pLOG->EndTime = pstTEXTINFO->uiStartTime;
	pLOG->EndMTime = pstTEXTINFO->uiStartMTime;
	*/

	/* LOG */
	if(pstTEXTINFO->uiAckTime == 0) {
    	pLOG->TransEndTime = pstTEXTINFO->uiStartTime;
    	pLOG->TransEndMTime = pstTEXTINFO->uiStartMTime;
	} else {
    	pLOG->TransEndTime = pstTEXTINFO->uiAckTime;
    	pLOG->TransEndMTime = pstTEXTINFO->uiAckMTime;
	}
	pLOG->ReportTime = pstTEXTINFO->uiStartTime;
	pLOG->ReportMTime = pstTEXTINFO->uiStartMTime;
	pLOG->ReportCode = pstMSRPTINFO->usResCode;
	pLOG->TotalReportCnt++;
	pLOG->ReportBodySize += pstMSRPTINFO->usBodyLen;
	pLOG->ReportDataSize += pstTEXTINFO->len;
	pLOG->ReportIPDataSize += pstTEXTINFO->IPDataSize;
	pLOG->EndStatus = MSRP_ENDSTATUS_REPORT;
}

/*
 * $Log: msrpt_util.c,v $
 * Revision 1.3  2011/09/07 06:30:48  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 12:26:40  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.2  2011/08/09 05:31:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.4  2011/01/11 04:09:09  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:00  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.3  2009/10/08 06:48:20  pkg
 * A_MSRPT log_print argument 문제 수정
 *
 * Revision 1.2  2009/09/13 08:53:50  jsyoon
 * PI프로세스의 uiNASName 필드값 제거
 *
 * Revision 1.1.1.1  2009/05/26 02:14:13  dqms
 * Init TAF_RPPI
 *
 * Revision 1.8  2009/02/11 08:04:42  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2009/02/11 06:32:29  dark264sh
 * MSRP 3사 연동 처리
 *
 * Revision 1.6  2008/12/29 12:06:14  dark264sh
 * SIP MSRP TransEndTime 변경
 *
 * Revision 1.5  2008/12/14 10:08:00  dark264sh
 * LOG_MSRP MIN 정보 parsing 규칙 변경
 *
 * Revision 1.4  2008/12/12 10:36:31  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2008/12/12 10:24:18  dark264sh
 * SIP MSRP nasname, port, calltime bug 수정
 *
 * Revision 1.2  2008/10/30 04:47:36  dark264sh
 * no message
 *
 * Revision 1.1  2008/09/18 06:48:10  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:15:16  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.5  2007/07/03 07:59:57  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2007/07/03 07:09:46  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2007/06/25 14:10:09  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2007/06/20 05:46:53  dark264sh
 * CF_MSRP_MSG MessageString 형식 변경
 *
 * Revision 1.1  2007/05/07 01:48:09  dark264sh
 * INIT
 *
 */
