/**     @file   sip_util.c
 *      - SIP Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: sip_util.c,v 1.2 2011/09/05 12:26:41 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/05 12:26:41 $
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
// LIB
#include "common_stg.h"
#include "loglib.h"
#include "typedef.h"

// .
#include "sip_util.h"

/**
 *	Implement func.
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
 *  @see            sip_msgq.c
 *
 **/
S32 dGetSIP(U8 *data, U32 len, LOG_SIP_TRANS *pLOG)
{
	S32			dRet;
	S32			useragent_len = 0;
	S32			length = 0;
	S32			accept_length = 0;
	S32			msgtype = 0;
	S32			event = 0;
	S32			range = 0;
	S32			ctype = 0;
	S32			from_vendor = 0, to_vendor = 0;
	U8			contact[SIP_CONTACT_SIZE];
	U8			accept[SIP_CONTACT_SIZE];
	U8			from_min[MAX_MIN_SIZE], to_min[MAX_MIN_SIZE];
	USHORT 		msgport;

	contact[0] = 0x00;
	accept[0] = 0x00;

	dRet = sip(data, len, &pLOG->URI[0], &pLOG->SIPFrom[0], &pLOG->SIPTo[0], &pLOG->SessID, &pLOG->AudioPort, &pLOG->VideoPort, &pLOG->UserAgent[0], &pLOG->Auth_nonce[0], &pLOG->UserName[0], &pLOG->AudioProto[0], &pLOG->VideoProto[0], &msgtype, &event, contact, accept, &msgport);
	if(dRet != 0) {
		log_print(LOGN_CRI, "PARSING ERROR [%d][%.*s]", len, len, data);
		return -1;
	}

	if((pLOG->usSvcL7Type == APP_IM_UP) && (pLOG->UserAgent[0] != 0x00)) {
		useragent_len = strlen(pLOG->UserAgent);
		dRet = sip_model(&pLOG->UserAgent[0], useragent_len, &pLOG->szModel[0]);
		if(dRet != 0) {
			log_print(LOGN_CRI, "PARSING ERROR [%d][%.*s]", useragent_len, SIP_USERAGENT_LEN, pLOG->UserAgent);
			return -2;
		}
	}

	from_min[0] = 0x00;
	from_vendor = IM_VENDOR_LGT;
	if(pLOG->SIPFrom[0] != 0x00) {
		length = strlen(pLOG->SIPFrom);
		dRet = sip_min(&pLOG->SIPFrom[0], length, from_min, &from_vendor);
		if(dRet != 0) {
			log_print(LOGN_CRI, "PARSING FROM ERROR [%d][%.*s]", length, SIP_FROM_LEN, pLOG->SIPFrom);
			return -3;
		}
	}

	to_min[0] = 0x00;
	to_vendor = IM_VENDOR_LGT;
	if(pLOG->SIPTo[0] != 0x00) {
		length = strlen(pLOG->SIPTo);
		dRet = sip_min(&pLOG->SIPTo[0], length, to_min, &to_vendor);
		if(dRet != 0) {
			log_print(LOGN_CRI, "PARSING TO ERROR [%d][%.*s]", length, SIP_TO_LEN, pLOG->SIPTo);
			return -4;
		}
	}

	if(pLOG->szMIN[0] == 0x00) {
		if(pLOG->usSvcL7Type == APP_IM_UP) {
			length = strlen(from_min);
			memcpy(pLOG->szMIN, from_min, length);
			pLOG->szMIN[length] = 0x00;
		} else {
			length = strlen(to_min);
			memcpy(pLOG->szMIN, to_min, length);
			pLOG->szMIN[length] = 0x00;
		}
	}

	if(pLOG->Vendor == 0) {
		if(from_vendor == to_vendor) {
			pLOG->Vendor = IM_VENDOR_LGT;
		} else if(from_vendor == IM_VENDOR_LGT) {
			pLOG->Vendor = to_vendor;
		} else {
			pLOG->Vendor = from_vendor;
		}
	}

	ctype = 0;
	accept_length = strlen(accept);
	if(sip_ctype(accept, accept_length, &ctype) != 0) {
		log_print(LOGN_CRI, "PARSING ACCEPT-CONTACT ERROR [%d][%.*s]", accept_length, SIP_CONTACT_LEN, accept);
		return -5;
	}

	if(ctype == 0) {
		accept_length = strlen(pLOG->URI);	
		if(sip_ctype(&pLOG->URI[0], accept_length, &ctype) != 0) {
			log_print(LOGN_CRI, "PARSING ACCEPT-CONTACT ERROR [%d][%.*s]", accept_length, SIP_URI_LEN, pLOG->URI);
			return -6;
		}

		if(ctype == 0) ctype = SIP_CTYPE_CHAT;
	}
	pLOG->CType = ctype;

	switch(msgtype)
	{
	case SIP_MSG_SUBSCRIBE:
		switch(event)
		{
		case SIP_EVENT_PRES:	
			pLOG->method = SIP_MSG_SUBSCRIBE_PRES;
			break;
		case SIP_EVENT_CONF:	
			pLOG->method = SIP_MSG_SUBSCRIBE_CONF;
			break;
		default:
			break;
		}
		break;
	case SIP_MSG_NOTIFY:
		switch(event)
		{
		case SIP_EVENT_PRES:	
			pLOG->method = SIP_MSG_NOTIFY_PRES;
			break;
		case SIP_EVENT_CONF:	
			pLOG->method = SIP_MSG_NOTIFY_CONF;
			break;
		default:
			break;
		}
		break;
	case SIP_MSG_INVITE:
		switch(pLOG->usSvcL4Type)
		{
		case L4_SIP_MS:
			switch(pLOG->usSvcL7Type)
			{
			case APP_IM_UP:
				if(sip_invite(pLOG->URI, strlen(pLOG->URI), &range) == 0) {
					if(range == SIP_RANGE_ADHOC) {
						pLOG->method = SIP_MSG_INVITE_ADHOC;
					} else {
						pLOG->method = SIP_MSG_INVITE_MTOIM;
					}
				}
				break;
			case APP_IM_DN:
				if(sip_invite(contact, strlen(contact), &range) == 0) {
					if(range == SIP_RANGE_MTOCSCF) {
						pLOG->method = SIP_MSG_INVITE_MTOCSCF;
					} else {
						pLOG->method = SIP_MSG_INVITE_TERMUE;
					}
				}
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return 0;
}

U8 *PrintTYPE(S32 type)
{
	switch(type)
	{
	case CAP_HEADER_NUM:			return "CAP_HEADER";
	case INFO_ETH_NUM:				return "INFO_ETH";
	case TCP_INFO_DEF_NUM:			return "TCP_INFO";
	case TEXT_INFO_DEF_NUM:			return "TEXT_INFO";
	case ETH_DATA_NUM:				return "ETH_DATA";
	case LOG_SIP_TRANS_DEF_NUM:		return "LOG_SIP_TRANS";
	default:						return "UNKNOWN";
	}
}

U8 *dGetQStr(USHORT usPlatformType)
{
	switch(usPlatformType)
	{
		case DEF_PLATFORM_IM: 	return "A_IM";
		case DEF_PLATFORM_VT: 	return "A_VT";
		default: 			return "UNKNOWN";
	}
}
/**
 *  $Log: sip_util.c,v $
 *  Revision 1.2  2011/09/05 12:26:41  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 13:12:03  hhbaek
 *  A_SIPD
 *
 *  Revision 1.2  2011/08/09 08:17:41  uamyd
 *  add blocks
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.5  2011/01/11 04:09:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.4  2009/07/15 17:10:56  dqms
 *  set_version 위치 및 Plastform Type 변경
 *
 *  Revision 1.3  2009/06/28 15:02:16  dqms
 *  ADD PLATFORM TYPE
 *
 *  Revision 1.2  2009/06/14 12:59:19  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:37  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.5  2009/02/11 08:03:36  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2009/02/11 06:17:49  dark264sh
 *  SIP Key 변경 (SeqType 추가) | 구간 세분화 | 486,487,603 성공 처리
 *
 *  Revision 1.3  2008/12/12 15:25:45  dark264sh
 *  LOG_SIP MIN 정보 parsing 규칙 변경
 *
 *  Revision 1.2  2008/09/21 11:30:54  dark264sh
 *  SIP 단말 모델 Parsing 추가
 *
 *  Revision 1.1  2008/09/18 06:50:03  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.3  2007/04/25 11:08:21  dark264sh
 *  NFO_ETH, Capture_Header_Msg 삭제, st_PKT_INFO 추가에 따른 변경
 *
 *  Revision 1.2  2007/03/22 12:20:47  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/03/07 01:15:19  dark264sh
 *  *** empty log message ***
 *
 */
          
