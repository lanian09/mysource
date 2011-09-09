/**     @file   xcap_func.c
 *      - XCAP Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: xcap_func.c,v 1.2 2011/09/06 12:46:42 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:42 $
 *      @ref        xcap_util.c
 *      @todo       Nothing
 *
 *      @section    Intro(소개)
 *      - XCAP Service Processing
 *
 *      @section    Requirement
 *       @li Nothing
 *
 **/

/**
 *	Include headers
 */
#include "xcap_func.h"

/**
 *	Implement func.
 */

/** dProcXCAPrans function.
 *
 *  ddProcXCAPrans Function
 *
 *  @param  *data : 분석할 SIP Message Data
 *  @param  len : 분석할 SIP Message Data Length
 *  @param  *pSIPINFOKEY : 분석된 값을 받기 위한 구조체
 *  @param  *msgtype : SIP Message Type
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(Message 분석 실패)
 *  @see            xcap_msgq.c
 *
 **/
S32 dProcXCAPCF(stMEMSINFO *pMEMSINFO, LOG_HTTP_TRANS *pLOG, TEXT_INFO *pTEXTINFO, S32 type)
{
	U8				*pNODE;
	CF_XCAP_MSG		*pCF;

	if((pNODE = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, LH"nifo_node_alloc", LT);
		return -1;
	}

	if((pCF = (CF_XCAP_MSG *)nifo_tlv_alloc(pMEMSINFO, pNODE, CF_XCAP_MSG_DEF_NUM, CF_XCAP_MSG_SIZE, DEF_MEMSET_ON)) == NULL) 
	{
		log_print(LOGN_CRI, LH"nifo_tlv_alloc", LT);
		return -2;
	}

	pCF->uiCallTime = pLOG->uiCallTime;
	pCF->uiCallMTime = pLOG->uiCallMTime;
	pCF->uiClientIP = pLOG->uiClientIP;
	/*
	pCF->CallFirstRawFileIndex = pLOG->CallFirstRawFileIndex;
	pCF->CallFirstRawPacketIndex = pLOG->CallFirstRawPacketIndex;
	pCF->SessTime = pLOG->SessTime;
	pCF->SessMTime = pLOG->SessMTime;
	pCF->TransTime = pLOG->CreateTime;
	pCF->TransMTime = pLOG->CreateMTime;
	pCF->RawFileIndex = pTEXTINFO->rawFileIndex;
	pCF->RawPacketIndex = pTEXTINFO->rawPacketIndex;
	pCF->range = pTEXTINFO->range;
	pCF->network = pTEXTINFO->network;
	pCF->SrcSystemID = pLOG->ClientIP;
	pCF->DestSystemID = pLOG->ServerIP;	
	pCF->ProtocolType = CF_XCAP_MSG_DEF_NUM;
	*/

	switch(type)
	{
	case HTTP_REQ_HDR_NUM:
	case HTTP_REQ_BODY_NUM:
		pCF->uiCallTime = pLOG->uiReqStartTime;
		pCF->uiCallMTime = pLOG->uiReqStartMTime;
		/*
		pCF->updown = DEF_FROM_CLIENT;
		*/

		pCF->HeaderSize = pLOG->uiUpHeaderSize;
		pCF->BodySize = pLOG->uiUpBodySize;
		pCF->ContentLength = pLOG->uiContentLength;

		/*
		pCF->MessageID = XCAP_MSG_REQUEST;
		sprintf(pCF->MessageString, "XCAP_REQUEST (%s) %.*s", PrintMsgType(pLOG->Method), 150, pLOG->LOGURL);
		*/
		break;
	case HTTP_RES_HDR_NUM:
	case HTTP_RES_BODY_NUM:
		pCF->uiCallTime = pLOG->uiResStartTime;
		pCF->uiCallMTime = pLOG->uiResStartMTime;
		/*
		pCF->updown = DEF_FROM_SERVER;
		*/
		pCF->HeaderSize = pLOG->uiDnHeaderSize;
		pCF->BodySize = pLOG->uiDnBodySize;
		pCF->ContentLength = pLOG->uiContentLength;
		/*
		pCF->MessageID = XCAP_MSG_RESPONSE;
		sprintf(pCF->MessageString, "XCAP_RESPONSE %d (%s) %.*s", 
					pLOG->ResCode, PrintResStr(pLOG->ResCode), 150, pLOG->LOGURL);
		*/
		break;
	default:
		log_print(LOGN_CRI, LH"UNKNOWN TYPE[%d]", LT, type);
		break;
	}

	memcpy(pCF->LOGURL, pLOG->szLOGURL, pLOG->usLOGURLSize);
	pCF->LOGURL[pLOG->usLOGURLSize] = 0x00;

//	CF_XCAP_MSG_Prt("PRINT CF_XCAP_MSG", pCF);

	dSend_XCAP_Data(pMEMSINFO, SEQ_PROC_A_CALL, pNODE);
	
	return 0;
}

/**
 *  $Log: xcap_func.c,v $
 *  Revision 1.2  2011/09/06 12:46:42  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 07:28:12  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:44  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.3  2011/01/11 04:09:11  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.2  2009/08/19 12:32:18  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:30  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 07:24:37  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.8  2007/06/21 11:43:45  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2007/06/20 07:17:48  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.6  2007/03/29 08:26:39  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2007/03/28 02:59:55  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2007/03/25 13:35:33  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2007/03/24 12:43:14  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2007/03/14 09:27:46  dark264sh
 *  remove prefix
 *
 *  Revision 1.1  2007/03/07 10:33:41  dark264sh
 *  *** empty log message ***
 *
 */
          
