/**     @file   sipt_util.c
 *      - SIPT Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: sipt_util.c,v 1.3 2011/09/07 06:30:48 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.3 $
 *      @date       $Date: 2011/09/07 06:30:48 $
 *      @ref        sipt_util.c
 *      @todo       Nothing
 *
 *      @section    Intro(소개)
 *      - SIPT Service Processing
 *
 *      @section    Requirement
 *       @li Nothing
 *
 **/
/**

 * Include headers
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// LIB
#include "typedef.h"
#include "loglib.h"

// TAF
#include "sip.h"

// .
#include "sipt_util.h"

/**
 *	Impl func.
 */

/** dGetSIPTransKey function.
 *
 *  dGetSIPTransKey Function
 *
 *  @param  *data : 분석할 SIP Message Data
 *  @param  len : 분석할 SIP Message Data Length
 *  @param  *pSIPINFOKEY : 분석된 값을 받기 위한 구조체
 *  @param  *msgtype : SIP Message Type
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(Message 분석 실패)
 *  @see            sipt_msgq.c
 *
 **/
S32 dGetSIPTransKey(U8 *data, U32 len, SIP_INFO_KEY *pSIPINFOKEY, S32 *msgtype, U32 *ip)
{
	S32			dRet, dSeqType = 0;
//	struct in_addr	stAddr;
//	char		szIP[BUFSIZ];

	dRet = sip_trans_key((char*)data, len, (char*)&pSIPINFOKEY->CallID[0], (char*)&pSIPINFOKEY->FromTag[0], (int*)&pSIPINFOKEY->CSeq, msgtype, &dSeqType);
	if(dRet != 0) {
		log_print(LOGN_CRI, "PARSING ERROR [%d][%.*s]", len, len, data);
		return -1;
	}

	*ip = 0;
	pSIPINFOKEY->SeqType = dSeqType;

#if 0
	szIP[0] = 0x00;
	/* IMS 개발 당시 CSCF 구간에서 단말 IP를 알기 위한 코드 (aqau, watas, wntas는 불필요) */
	dRet = sip_clientip(pSIPINFOKEY->CallID, strlen(pSIPINFOKEY->CallID), szIP);
	if(dRet != 0) {
		log_print(LOGN_CRI, "PARSING CALLID ERROR [%s]", pSIPINFOKEY->CallID);
		return -2;
	}

	if(inet_pton(AF_INET, szIP, (void *)&stAddr) > 0) {
		*ip = htonl(stAddr.s_addr);
	}
#endif

	return 0;
}

char *PrintMsgType(S32 msgtype)
{

	if(msgtype >= 1000)			return "RESP";

	switch(msgtype)
	{
	case SIP_MSG_ACK:					return "ACK";
	case SIP_MSG_BYE:					return "BYE";
	case SIP_MSG_CANCEL:				return "CANCEL";
	case SIP_MSG_INFO:					return "INFO";
	case SIP_MSG_INVITE:
	case SIP_MSG_INVITE_ADHOC:
	case SIP_MSG_INVITE_MTOCSCF:
	case SIP_MSG_INVITE_MTOIM:
	case SIP_MSG_INVITE_TERMUE:			return "INVITE";
	case SIP_MSG_MESSAGE:				return "MESSAGE";
	case SIP_MSG_NOTIFY:
	case SIP_MSG_NOTIFY_PRES:
	case SIP_MSG_NOTIFY_CONF:			return "NOTIFY";
	case SIP_MSG_OPTIONS:				return "OPTIONS";
	case SIP_MSG_PRACK:					return "PRACK";
	case SIP_MSG_PUBLISH:				return "PUBLISH";
	case SIP_MSG_REFER:					return "REFER";
	case SIP_MSG_REGISTER:				return "REGISTER";
	case SIP_MSG_SUBSCRIBE:
	case SIP_MSG_SUBSCRIBE_PRES:
	case SIP_MSG_SUBSCRIBE_CONF:		return "SUBSCRIBE";
	case SIP_MSG_UPDATE:				return "UPDATE";
	default:							return "UNKNOWN";
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

S32 dGetMsgType(S32 msgtype)
{
	S32		type = 0;

	if(msgtype >= 2000) {
		type = TSIP_RES_NORMAL;
	} else if(msgtype >= 1000) {
		type = TSIP_RES_SKIP;
	} else {
		switch(msgtype)
		{
		case SIP_MSG_ACK:
			type = TSIP_REQ_ACK;
			break;
		case SIP_MSG_BYE:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_CANCEL:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_INFO:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_INVITE:
			type = TSIP_REQ_INVITE;
			break;
		case SIP_MSG_MESSAGE:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_NOTIFY:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_OPTIONS:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_PRACK:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_PUBLISH:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_REFER:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_REGISTER:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_SUBSCRIBE:
			type = TSIP_REQ_NORMAL;
			break;
		case SIP_MSG_UPDATE:
			type = TSIP_REQ_NORMAL;
			break;
		default:
			type = 0;
			break;
		}
	}

	return type;
}

char *PrintStatus(S32 type)
{
	switch(type)
	{
	case TSIP_RES_SKIP:			return "TSIP_RES_SKIP";
	case TSIP_RES_NORMAL:		return "TSIP_RES_NORMAL";
	case TSIP_REQ_NORMAL:		return "TSIP_REQ_NORAML";
	case TSIP_REQ_INVITE:		return "TSIP_REQ_INVITE";
	case TSIP_REQ_ACK:			return "TSIP_REQ_ACK";
	default:					return "UNKNOWN";
	}
}

char *PrintTYPE(S32 type)
{
	switch(type)
	{
	case CAP_HEADER_NUM:		return "CAP_HEADER";
	case INFO_ETH_NUM:			return "INFO_ETH";
	case TCP_INFO_DEF_NUM:		return "TCP_INFO";
	case TEXT_INFO_DEF_NUM:		return "TEXT_INFO";
	case ETH_DATA_NUM:			return "ETH_DATA";
	default:					return "UNKNOWN";
	}
}

S32 dGetL7TYPE(S32 dRtx)
{
	switch(dRtx)
	{
	case DEF_FROM_CLIENT:	return APP_IM_UP;
	case DEF_FROM_SERVER:	return APP_IM_DN;
	default:				return APP_ETC;
	}
}

#if 0
void nifo_splice(stMEMSINFO *pMEMSINFO, U8 *pLIST, U8 *pHEAD)
{
	U8		*pLAST, *pAT;

	pLAST = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pLIST)->nont.offset_prev), NIFO, nont);
	pAT = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pHEAD)->nont.offset_prev), NIFO, nont);

	((NIFO *)pHEAD)->nont.offset_prev = nifo_offset(pMEMSINFO, pLAST);
	((NIFO *)pLAST)->nont.offset_next = nifo_offset(pMEMSINFO, pHEAD);
	
	((NIFO *)pLIST)->nont.offset_prev = nifo_offset(pMEMSINFO, pAT);
	((NIFO *)pAT)->nont.offset_next = nifo_offset(pMEMSINFO, pLIST);
}
#endif

void AddMsg(void *p1, void *p2)
{
	SIP_INFO	*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO	*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO	*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	U8			*pHEAD, *pNODE;

	pHEAD = nifo_ptr(pMEMSINFO, pSIPINFO->offset_NODE);
	pNODE = nifo_ptr(pMEMSINFO, pTSIPINFO->offset_DATA);
    
	/* Transaction 생성 */
//	nifo_node_link_nont_next(pMEMSINFO, pHEAD, pNODE);

	nifo_splice_nont(pMEMSINFO, pNODE, pHEAD);
}

void InitLog(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	pLOG->method = pTSIPINFO->msgtype;
	sprintf((char*)pLOG->MethodString, "%.*s", SIP_METHOD_LEN, PrintMsgType(pLOG->method));
	pLOG->uiCallTime = pTEXTINFO->uiStartTime;
	pLOG->uiCallMTime = pTEXTINFO->uiStartMTime;
//	pLOG->uiNASName = pTEXTINFO->clientIP;
	pLOG->uiNASName = 0;
	
	pLOG->SrcIP = pTEXTINFO->clientIP;
	pLOG->DestIP = pTEXTINFO->serverIP;
	pLOG->SrcPort = pTEXTINFO->clientPort;
	pLOG->DestPort = pTEXTINFO->serverPort;		

	pLOG->TransStartTime = pTEXTINFO->uiStartTime;
	pLOG->TransStartMTime = pTEXTINFO->uiStartMTime;
	pLOG->OpStartTime = pTEXTINFO->uiStartTime;
	pLOG->OpStartMTime = pTEXTINFO->uiStartMTime;
	pLOG->OpEndTime = pTEXTINFO->uiStartTime;
	pLOG->OpEndMTime = pTEXTINFO->uiStartMTime;
	if(pTEXTINFO->uiAckTime == 0) {
		pLOG->TransEndTime = pTEXTINFO->uiStartTime;
		pLOG->TransEndMTime = pTEXTINFO->uiStartMTime;
	} else {
		pLOG->TransEndTime = pTEXTINFO->uiAckTime;
		pLOG->TransEndMTime = pTEXTINFO->uiAckMTime;
	}
}

void SetResSkip(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	if(pLOG->FirstResTime == 0) {
		pLOG->FirstResTime = pTEXTINFO->uiStartTime;
		pLOG->FirstResMTime = pTEXTINFO->uiStartMTime;
	}
	pLOG->ResTime = pTEXTINFO->uiStartTime;
	pLOG->ResMTime = pTEXTINFO->uiStartMTime;
	pLOG->ResCode = pTSIPINFO->msgtype / 10;

	if(pTEXTINFO->uiAckTime == 0) {
		pLOG->TransEndTime = pTEXTINFO->uiStartTime;
		pLOG->TransEndMTime = pTEXTINFO->uiStartMTime;
	} else {
		pLOG->TransEndTime = pTEXTINFO->uiAckTime;
		pLOG->TransEndMTime = pTEXTINFO->uiAckMTime;
	}
	pLOG->OpEndTime = pTEXTINFO->uiStartTime;
	pLOG->OpEndMTime = pTEXTINFO->uiStartMTime;
}

void SetResNormal(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	if(pLOG->FirstResTime == 0) {
		pLOG->FirstResTime = pTEXTINFO->uiStartTime;
		pLOG->FirstResMTime = pTEXTINFO->uiStartMTime;
	}
	pLOG->ResTime = pTEXTINFO->uiStartTime;
	pLOG->ResMTime = pTEXTINFO->uiStartMTime;
	pLOG->ResCode = pTSIPINFO->msgtype / 10;

	if(pTEXTINFO->uiAckTime == 0) {
		pLOG->TransEndTime = pTEXTINFO->uiStartTime;
		pLOG->TransEndMTime = pTEXTINFO->uiStartMTime;
	} else {
		pLOG->TransEndTime = pTEXTINFO->uiAckTime;
		pLOG->TransEndMTime = pTEXTINFO->uiAckMTime;
	}
	pLOG->OpEndTime = pTEXTINFO->uiStartTime;
	pLOG->OpEndMTime = pTEXTINFO->uiStartMTime;
}

void SetRetransReq(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	pLOG->RetransReqCnt++;

	if(pTEXTINFO->uiAckTime == 0) {
		pLOG->TransEndTime = pTEXTINFO->uiStartTime;
		pLOG->TransEndMTime = pTEXTINFO->uiStartMTime;
	} else {
		pLOG->TransEndTime = pTEXTINFO->uiAckTime;
		pLOG->TransEndMTime = pTEXTINFO->uiAckMTime;
	}
	pLOG->OpEndTime = pTEXTINFO->uiStartTime;
	pLOG->OpEndMTime = pTEXTINFO->uiStartMTime;
}

void SetRetransRes(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	pLOG->RetransResCnt++;

	if(pTEXTINFO->uiAckTime == 0) {
		pLOG->TransEndTime = pTEXTINFO->uiStartTime;
		pLOG->TransEndMTime = pTEXTINFO->uiStartMTime;
	} else {
		pLOG->TransEndTime = pTEXTINFO->uiAckTime;
		pLOG->TransEndMTime = pTEXTINFO->uiAckMTime;
	}
	pLOG->OpEndTime = pTEXTINFO->uiStartTime;
	pLOG->OpEndMTime = pTEXTINFO->uiStartMTime;
}

void SetAck(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	pLOG->AckTime = pTEXTINFO->uiStartTime;
	pLOG->AckMTime = pTEXTINFO->uiStartMTime;

	if(pTEXTINFO->uiAckTime == 0) {
		pLOG->TransEndTime = pTEXTINFO->uiStartTime;
		pLOG->TransEndMTime = pTEXTINFO->uiStartMTime;
	} else {
		pLOG->TransEndTime = pTEXTINFO->uiAckTime;
		pLOG->TransEndMTime = pTEXTINFO->uiAckMTime;
	}
	pLOG->OpEndTime = pTEXTINFO->uiStartTime;
	pLOG->OpEndMTime = pTEXTINFO->uiStartMTime;
}

void SetRetransAck(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	pLOG->RetransReqCnt++;

	if(pTEXTINFO->uiAckTime == 0) {
		pLOG->TransEndTime = pTEXTINFO->uiStartTime;
		pLOG->TransEndMTime = pTEXTINFO->uiStartMTime;
	} else {
		pLOG->TransEndTime = pTEXTINFO->uiAckTime;
		pLOG->TransEndMTime = pTEXTINFO->uiAckMTime;
	}
	pLOG->OpEndTime = pTEXTINFO->uiStartTime;
	pLOG->OpEndMTime = pTEXTINFO->uiStartMTime;
}

void SetReqDataSize(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	pLOG->ReqDataSize += pTEXTINFO->len;
	pLOG->ReqIPDataSize += pTEXTINFO->IPDataSize;
	pLOG->TotalReqCnt++;
}

void SetResDataSize(void *p1, void *p2)
{
	SIP_INFO		*pSIPINFO = (SIP_INFO *)p1;
	TSIP_INFO		*pTSIPINFO = (TSIP_INFO *)p2;
	stMEMSINFO		*pMEMSINFO = (stMEMSINFO *)pTSIPINFO->pMEMSINFO;
	LOG_SIP_TRANS	*pLOG = (LOG_SIP_TRANS *)nifo_ptr(pMEMSINFO, pSIPINFO->offset_LOG);
	TEXT_INFO		*pTEXTINFO = (TEXT_INFO *)pTSIPINFO->pTEXTINFO;

	pLOG->ResDataSize += pTEXTINFO->len;
	pLOG->ResIPDataSize += pTEXTINFO->IPDataSize;
	pLOG->TotalResCnt++;

	if(pTSIPINFO->status == TSIP_RES_SKIP) {
		pLOG->SkipResCnt++;
	}
}

/**
 *  $Log: sipt_util.c,v $
 *  Revision 1.3  2011/09/07 06:30:48  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/09/06 12:46:39  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/17 13:13:56  hhbaek
 *  A_SIPT
 *
 *  Revision 1.3  2011/08/10 09:57:44  uamyd
 *  modified and block added
 *
 *  Revision 1.2  2011/08/09 05:31:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.3  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.2  2009/09/13 08:53:50  jsyoon
 *  PI프로세스의 uiNASName 필드값 제거
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:34  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.7  2009/02/11 06:20:24  dark264sh
 *  SIP Key 변경 (SeqType 추가) | 구간 세분화 | 486,487,603 성공 처리
 *
 *  Revision 1.6  2008/12/29 12:10:25  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2008/12/29 12:05:48  dark264sh
 *  SIP MSRP TransEndTime 변경
 *
 *  Revision 1.4  2008/12/12 10:23:20  dark264sh
 *  SIP MSRP nasname, port, calltime bug 수정
 *
 *  Revision 1.3  2008/10/30 04:48:37  dark264sh
 *  no message
 *
 *  Revision 1.2  2008/09/21 10:51:07  dark264sh
 *  A_SIPT 단말 IP 뽑는 불필요한 코드 삭제
 *
 *  Revision 1.1  2008/09/18 07:21:33  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.9  2007/06/20 07:18:23  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.8  2007/04/25 11:03:17  dark264sh
 *  NFO_ETH, Capture_Header_Msg 삭제, st_PKT_INFO 추가에 따른 변경
 *
 *  Revision 1.7  2007/03/29 06:59:28  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.6  2007/03/28 15:47:11  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2007/03/28 07:01:30  dark264sh
 *  A_SIPT Timeout 처리 되는 경우 EndTime 처리 못하는 문제 해결 log_print 수정
 *
 *  Revision 1.4  2007/03/24 12:47:02  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2007/03/14 09:29:30  dark264sh
 *  remove prefix
 *
 *  Revision 1.2  2007/03/07 01:12:06  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/03/05 00:37:21  dark264sh
 *  *** empty log message ***
 *
 */
          
