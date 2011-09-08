/**		@file	http_util.c
 * 		- Util 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: ihttp_util.c,v 1.2 2011/09/04 11:40:36 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 11:40:36 $
 * 		@ref		http_util.c http_func.c http_main.c http_init.c l4.h a_http_api.h
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- Util 함수들
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <ctype.h>

// LIB
#include "typedef.h"
#include "loglib.h"

// PROJECT
#include "procid.h"
#include "common_stg.h"

// TAF
#include "http.h"

// .
#include "ihttp_util.h"


/**
 * Declare variables
 */
extern S32		gACALLCnt;

/** MakeTcpHashKey function.
 *
 *  MakeTcpHashKey Function
 *
 *  @param	*pstTcpInfo : TCP Session 정보 (const st_TcpInfo *)
 *  @param	*pstTcpHashKey : TCP Hash Key (st_TcpHashKey *)
 *
 *  @return			void
 *  @see			http_util.c http_func.c http_main.c http_init.c l4.h a_http_api.h
 *
 *  @note			TCP로 부터 읽는 함수 변경될 경우 변경 될수 있음.
 **/
void MakeTcpHashKey(TCP_INFO *pTCPINFO, HTTP_TSESS_KEY *pHTTPTSESSKEY)
{
	pHTTPTSESSKEY->uiCliIP = pTCPINFO->uiCliIP;
	pHTTPTSESSKEY->usCliPort = pTCPINFO->usCliPort;
	pHTTPTSESSKEY->usReserved = 0;	
}

/** MakeHttpHashKey function.
 *
 *  MakeHttpHashKey Function
 *
 *  @param	*pstTcpInfo : TCP Session 정보 (const st_TcpInfo *)
 *  @param	*pstHttpHashKey : TCP Hash Key (st_TcpHashKey *)
 *
 *  @return			void
 *  @see			http_util.c http_func.c http_main.c http_init.c l4.h a_http_api.h
 *
 *  @note			TCP로 부터 읽는 함수 변경될 경우 변경 될수 있음.
 **/
void MakeHttpHashKey(HTTP_TSESS_KEY *pHTTPTSESSKEY, U16 usTransID, HTTP_TRANS_KEY *pHTTPTRANSKEY)
{
	pHTTPTRANSKEY->uiCliIP = pHTTPTSESSKEY->uiCliIP;
	pHTTPTRANSKEY->usCliPort = pHTTPTSESSKEY->usCliPort;
	pHTTPTRANSKEY->usHttpTransID = usTransID;	
}

/** dProcessMSG function.
 *
 *  dProcessMSG Function
 *
 *  @param	*pstTcpInfo : TCP Session 정보 (const st_TcpInfo *)
 *  @param	*pszTcpPayLoad : Tcp Payload Data (const S8 *)
 *  @param	*pstBufInfo : Buffer 관리 구조체 (st_BufInfo *)
 *	@param	*pdLen : 처리된 사이즈 (int *)
 *
 *  @return			S32 -1: Invalid State, 0: Not Find 0D0A0D0A, 1: Find 0D0A0D0A
 *  @see			http_util.c http_func.c http_main.c http_init.c l4.h a_http_api.h
 *
 **/
S32 dGetData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen)
{
	S32				i, limit = 0;
	TLV				*pTLV;
	U8				*pTmp;
	S64				gap;

	gap = GetGapTime(pMSGINFO->uiLastUpdateTime, pMSGINFO->uiLastUpdateMTime, pMSGINFO->uiStartTime, pMSGINFO->uiStartMTime);
	if(gap >= 0 && gap <= DEF_LIMIT_10) limit = 1;

	pTLV = (TLV *)nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR + NIFO_SIZE);

	for(i = 0; i < uiDataLen; i++)
	{
		switch(pMSGINFO->ucEndStatus)
		{
		case END_STATE_EMPTY:
			switch(szInData[i])
			{
			case 0x0D:
				pMSGINFO->ucEndStatus = END_STATE_0D;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D:
			switch(szInData[i])
			{
			case 0x0A:
				pMSGINFO->ucEndStatus = END_STATE_0D0A;
				break;
			case 0x0D:
				pMSGINFO->ucEndStatus = END_STATE_0D;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D0A:
			switch(szInData[i])
			{
			case 0x0D:
				pMSGINFO->ucEndStatus = END_STATE_0D0A0D;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D0A0D:
			switch(szInData[i])
			{
			case 0x0A:
				pMSGINFO->ucEndStatus = END_STATE_0D0A0D0A;
				break;
			case 0x0D:
				pMSGINFO->ucEndStatus = END_STATE_0D;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		default:
			log_print(LOGN_CRI, "[%s][%s.%d] INVALID STATUS [%d]", 
					__FILE__, __FUNCTION__, __LINE__, pMSGINFO->ucEndStatus);
			return -1;
		}

		pTmp = nifo_ptr(pMEMSINFO, pMSGINFO->offset_CurHDR);
		if(((NIFO *)pTmp)->maxoffset == ((NIFO *)pTmp)->lastoffset)
		{
			/* 새로운 cont Node 할당 */
			if((pTmp = nifo_node_alloc(pMEMSINFO)) == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
				return -2;
			}

			nifo_node_link_cont_prev(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR), pTmp);
			pMSGINFO->offset_CurHDR = nifo_offset(pMEMSINFO, pTmp);
		}

		pTmp = nifo_ptr(pMEMSINFO, pMSGINFO->offset_CurHDR);

		TOLOWER(pTmp[((NIFO *)pTmp)->lastoffset], szInData[i]);
		((NIFO *)pTmp)->lastoffset++;
		pTLV->len++;

		(*pdLen)++;
		pMSGINFO->uiHdrLen++;
		if(limit) pMSGINFO->uiLimitLen++;

		if(pMSGINFO->ucEndStatus == END_STATE_0D0A0D0A) {
			return END_STATE_0D0A0D0A;
		}
	}

	return 0;
}

S32 dGetLengthData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen)
{
	S32				i, limit = 0;
	S32				tmpLen;
	TLV				*pTLV;
	U8				*pTmp;
	S64				gap;

	gap = GetGapTime(pMSGINFO->uiLastUpdateTime, pMSGINFO->uiLastUpdateMTime, pMSGINFO->uiStartTime, pMSGINFO->uiStartMTime);
	if(gap >= 0 && gap <= DEF_LIMIT_10) limit = 1;

	pTLV = (TLV *)nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY + NIFO_SIZE);

	if(pMSGINFO->ucIsBuffering == TSESS_BUFFERING_ON)
	{
		for(i = 0; i < uiDataLen; i++)
		{
			pTmp = nifo_ptr(pMEMSINFO, pMSGINFO->offset_CurBODY);
			if(((NIFO *)pTmp)->maxoffset == ((NIFO *)pTmp)->lastoffset)
			{
				/* 새로운 cont Node 할당 */
				if((pTmp = nifo_node_alloc(pMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -2;
				}

				nifo_node_link_cont_prev(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY), pTmp);
				pMSGINFO->offset_CurBODY = nifo_offset(pMEMSINFO, pTmp);
			}

			pTmp = nifo_ptr(pMEMSINFO, pMSGINFO->offset_CurBODY);

			if(pMSGINFO->dZip > 0) pTmp[((NIFO *)pTmp)->lastoffset] = szInData[i];
			else TOLOWER(pTmp[((NIFO *)pTmp)->lastoffset], szInData[i]);
			((NIFO *)pTmp)->lastoffset++;
			pTLV->len++;

			(*pdLen)++;
			pMSGINFO->uiBodyLen++;
			if(limit) pMSGINFO->uiLimitLen++;

			if(pMSGINFO->uiLen == pMSGINFO->uiBodyLen) {
				pMSGINFO->ucEndStatus = END_STATE_0D0A0D0A;
				return END_STATE_0D0A0D0A;
			}

			if((pMSGINFO->uiMaxLen > 0) && (pMSGINFO->uiMaxLen == pMSGINFO->uiBodyLen)) {
				pMSGINFO->ucIsBuffering = TSESS_BUFFERING_OFF;
			}
		}
	}
	else
	{
		tmpLen = pMSGINFO->uiBodyLen + uiDataLen;

		if(pMSGINFO->uiLen == tmpLen) {
			*pdLen += uiDataLen;
			pMSGINFO->uiBodyLen += uiDataLen;
			if(limit) pMSGINFO->uiLimitLen += uiDataLen;
			pMSGINFO->ucEndStatus = END_STATE_0D0A0D0A;
			return END_STATE_0D0A0D0A;
		} else if(pMSGINFO->uiLen < tmpLen) {
			tmpLen = tmpLen - pMSGINFO->uiLen;
			tmpLen = uiDataLen - tmpLen;
			*pdLen += tmpLen;
			pMSGINFO->uiBodyLen += tmpLen;
			if(limit) pMSGINFO->uiLimitLen += tmpLen;
			pMSGINFO->ucEndStatus = END_STATE_0D0A0D0A;
			return END_STATE_0D0A0D0A;
		} else {
			*pdLen += uiDataLen;
			pMSGINFO->uiBodyLen += uiDataLen;
			if(limit) pMSGINFO->uiLimitLen += uiDataLen;
		}
	}

	return 0;
}

S32 dGetChunkedData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen)
{
	S32				i, limit = 0;
	TLV				*pTLV;
	U8				*pTmp;
	S64				gap;

	gap = GetGapTime(pMSGINFO->uiLastUpdateTime, pMSGINFO->uiLastUpdateMTime, pMSGINFO->uiStartTime, pMSGINFO->uiStartMTime);
	if(gap >= 0 && gap <= DEF_LIMIT_10) limit = 1;

	pTLV = (TLV *)nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY + NIFO_SIZE);

	for(i = 0; i < uiDataLen; i++)
	{
		switch(pMSGINFO->ucEndStatus)
		{
		case END_STATE_EMPTY:
			switch(szInData[i])
			{
			case 0x30:
				pMSGINFO->ucEndStatus = END_STATE_30;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_30:
			switch(szInData[i])
			{
			case 0x0D:
				pMSGINFO->ucEndStatus = END_STATE_0D;
				break;
			case 0x30:
				pMSGINFO->ucEndStatus = END_STATE_30;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D:
			switch(szInData[i])
			{
			case 0x0A:
				pMSGINFO->ucEndStatus = END_STATE_0D0A;
				break;
			case 0x30:
				pMSGINFO->ucEndStatus = END_STATE_30;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D0A:
			switch(szInData[i])
			{
			case 0x0D:
				pMSGINFO->ucEndStatus = END_STATE_0D0A0D;
				break;
			case 0x30:
				pMSGINFO->ucEndStatus = END_STATE_30;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D0A0D:
			switch(szInData[i])
			{
			case 0x0A:
				pMSGINFO->ucEndStatus = END_STATE_0D0A0D0A;
				break;
			case 0x30:
				pMSGINFO->ucEndStatus = END_STATE_30;
				break;
			default:
				pMSGINFO->ucEndStatus = END_STATE_EMPTY;
				break;
			}
			break;
		default:
			log_print(LOGN_CRI, "[%s][%s.%d] INVALID STATUS [%d]", 
					__FILE__, __FUNCTION__, __LINE__, pMSGINFO->ucEndStatus);
			return -1;
		}

		if(pMSGINFO->ucIsBuffering == TSESS_BUFFERING_ON)
		{
			pTmp = nifo_ptr(pMEMSINFO, pMSGINFO->offset_CurBODY);
			if(((NIFO *)pTmp)->maxoffset == ((NIFO *)pTmp)->lastoffset)
			{
				/* 새로운 cont Node 할당 */
				if((pTmp = nifo_node_alloc(pMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -2;
				}

				nifo_node_link_cont_prev(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY), pTmp);
				pMSGINFO->offset_CurBODY = nifo_offset(pMEMSINFO, pTmp);
			}

			pTmp = nifo_ptr(pMEMSINFO, pMSGINFO->offset_CurBODY);

			if(pMSGINFO->dZip > 0) pTmp[((NIFO *)pTmp)->lastoffset] = szInData[i];
			else TOLOWER(pTmp[((NIFO *)pTmp)->lastoffset], szInData[i]);
			((NIFO *)pTmp)->lastoffset++;
			pTLV->len++;
		}

		(*pdLen)++;
		pMSGINFO->uiBodyLen++;
		if(limit) pMSGINFO->uiLimitLen++;

		if(pMSGINFO->ucEndStatus == END_STATE_0D0A0D0A) {
			return END_STATE_0D0A0D0A;
		}

		if((pMSGINFO->uiMaxLen > 0) && (pMSGINFO->uiMaxLen == pMSGINFO->uiBodyLen)) {
			pMSGINFO->ucIsBuffering = TSESS_BUFFERING_OFF;
		}
	}

	return 0;
}

S32 dGetMultiData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen)
{
	S32				i, limit = 0;
	TLV				*pTLV;
	U8				*pTmp;
	MULTI_KEY		*pMULTIKEY = &pMSGINFO->MULTIKEY;
	S64				gap;

	gap = GetGapTime(pMSGINFO->uiLastUpdateTime, pMSGINFO->uiLastUpdateMTime, pMSGINFO->uiStartTime, pMSGINFO->uiStartMTime);
	if(gap >= 0 && gap <= DEF_LIMIT_10) limit = 1; 

	pTLV = (TLV *)nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY + NIFO_SIZE);

	for(i = 0; i < uiDataLen; i++)
	{
		if(pMULTIKEY->state >= pMULTIKEY->len - 1) {
			pMSGINFO->ucEndStatus = END_STATE_0D0A0D0A;
			return END_STATE_0D0A0D0A;
		}

		if(pMULTIKEY->key[pMULTIKEY->state] == tolower(szInData[i])) pMULTIKEY->state++;
		else pMULTIKEY->state = 0;

		if(pMSGINFO->ucIsBuffering == TSESS_BUFFERING_ON)
		{
			pTmp = nifo_ptr(pMEMSINFO, pMSGINFO->offset_CurBODY);
			if(((NIFO *)pTmp)->maxoffset == ((NIFO *)pTmp)->lastoffset)
			{
				/* 새로운 cont Node 할당 */
				if((pTmp = nifo_node_alloc(pMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
					return -2;
				}

				nifo_node_link_cont_prev(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY), pTmp);
				pMSGINFO->offset_CurBODY = nifo_offset(pMEMSINFO, pTmp);
			}

			pTmp = nifo_ptr(pMEMSINFO, pMSGINFO->offset_CurBODY);

//			TOLOWER(pTmp[((NIFO *)pTmp)->lastoffset], szInData[i]);
			pTmp[((NIFO *)pTmp)->lastoffset] = szInData[i];
			((NIFO *)pTmp)->lastoffset++;
			pTLV->len++;
		}

		(*pdLen)++;
		pMSGINFO->uiBodyLen++;
		if(limit) pMSGINFO->uiLimitLen++;
	}

	return 0;
}

void InitBuffer(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, S32 isNotREQ)
{
	U8 *pNode;

	if(pMSGINFO->offset_HDR != 0) {
		pNode = nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR);
		nifo_node_delete(pMEMSINFO, pNode);
		pMSGINFO->offset_HDR = 0;	
		pMSGINFO->offset_CurHDR = 0;
	}
	if(pMSGINFO->offset_BODY != 0) {
		pNode = nifo_ptr(pMEMSINFO, pMSGINFO->offset_BODY);
		nifo_node_delete(pMEMSINFO, pNode);
		pMSGINFO->offset_BODY = 0;
		pMSGINFO->offset_CurBODY = 0;
	}

	if((pNode = pAllocDataNode(pMEMSINFO, ((isNotREQ) ? HTTP_RES_HDR_NUM : HTTP_REQ_HDR_NUM))) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] pAllocDataNode NULL", __FILE__, __FUNCTION__, __LINE__);
		return;
	}

	pMSGINFO->offset_HDR = nifo_offset(pMEMSINFO, pNode);	
	pMSGINFO->offset_CurHDR = pMSGINFO->offset_HDR;

	if((pNode = pAllocDataNode(pMEMSINFO, ((isNotREQ) ? HTTP_RES_BODY_NUM : HTTP_REQ_BODY_NUM))) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] pAllocDataNode NULL", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pMEMSINFO, nifo_ptr(pMEMSINFO, pMSGINFO->offset_HDR));
		pMSGINFO->offset_HDR = 0;	
		pMSGINFO->offset_CurHDR = 0;
		return;
	}

	pMSGINFO->offset_BODY = nifo_offset(pMEMSINFO, pNode);
	pMSGINFO->offset_CurBODY = pMSGINFO->offset_BODY;

	pMSGINFO->ucStatus = TSESS_STATUS_HDRWAIT;
	pMSGINFO->ucEndStatus = END_STATE_EMPTY;
	pMSGINFO->uiStartTime = 0;
	pMSGINFO->uiStartMTime = 0;
	pMSGINFO->uiLastUpdateTime = 0;
	pMSGINFO->uiLastUpdateMTime = 0;
	pMSGINFO->uiAckTime = 0;
	pMSGINFO->uiAckMTime = 0;

	pMSGINFO->MULTIKEY.len = 0;
	pMSGINFO->MULTIKEY.state = 0;

	pMSGINFO->dZip = 0;
	pMSGINFO->dChunked = 0;
}

void InitCount(HTTP_TSESS *pHTTPTSESS)
{
	pHTTPTSESS->uiIPDataUpPktCnt = 0;
	pHTTPTSESS->uiIPDataDnPktCnt = 0;
	pHTTPTSESS->uiIPTotUpPktCnt = 0;
	pHTTPTSESS->uiIPTotDnPktCnt = 0;
	pHTTPTSESS->uiIPDataUpRetransCnt = 0;
	pHTTPTSESS->uiIPDataDnRetransCnt = 0;
	pHTTPTSESS->uiIPTotUpRetransCnt = 0;
	pHTTPTSESS->uiIPTotDnRetransCnt = 0;
	pHTTPTSESS->uiIPDataUpPktSize = 0;
	pHTTPTSESS->uiIPDataDnPktSize = 0;
	pHTTPTSESS->uiIPTotUpPktSize = 0;
	pHTTPTSESS->uiIPTotDnPktSize = 0;
	pHTTPTSESS->uiIPDataUpRetransSize = 0;
	pHTTPTSESS->uiIPDataDnRetransSize = 0;
	pHTTPTSESS->uiIPTotUpRetransSize = 0;
	pHTTPTSESS->uiIPTotDnRetransSize = 0;	
}

void UpCount(LOG_IHTTP_TRANS *pLOGHTTP, HTTP_TSESS *pHTTPTSESS)
{
	pLOGHTTP->uiIPDataUpPktCnt += pHTTPTSESS->uiIPDataUpPktCnt;
	pLOGHTTP->uiIPDataDnPktCnt += pHTTPTSESS->uiIPDataDnPktCnt;
	pLOGHTTP->uiIPTotUpPktCnt += pHTTPTSESS->uiIPTotUpPktCnt;
	pLOGHTTP->uiIPTotDnPktCnt += pHTTPTSESS->uiIPTotDnPktCnt;
	pLOGHTTP->uiIPDataUpRetransCnt += pHTTPTSESS->uiIPDataUpRetransCnt;
	pLOGHTTP->uiIPDataDnRetransCnt += pHTTPTSESS->uiIPDataDnRetransCnt;
	pLOGHTTP->uiIPTotUpRetransCnt += pHTTPTSESS->uiIPTotUpRetransCnt;
	pLOGHTTP->uiIPTotDnRetransCnt += pHTTPTSESS->uiIPTotDnRetransCnt;
	pLOGHTTP->uiIPDataUpPktSize += pHTTPTSESS->uiIPDataUpPktSize;
	pLOGHTTP->uiIPDataDnPktSize += pHTTPTSESS->uiIPDataDnPktSize;
	pLOGHTTP->uiIPTotUpPktSize += pHTTPTSESS->uiIPTotUpPktSize;
	pLOGHTTP->uiIPTotDnPktSize += pHTTPTSESS->uiIPTotDnPktSize;
	pLOGHTTP->uiIPDataUpRetransSize += pHTTPTSESS->uiIPDataUpRetransSize;
	pLOGHTTP->uiIPDataDnRetransSize += pHTTPTSESS->uiIPDataDnRetransSize;
	pLOGHTTP->uiIPTotUpRetransSize += pHTTPTSESS->uiIPTotUpRetransSize;
	pLOGHTTP->uiIPTotDnRetransSize += pHTTPTSESS->uiIPTotDnRetransSize;

	InitCount(pHTTPTSESS);
}

void UpdateHttpTsess(TCP_INFO *pTCPINFO, HTTP_TSESS *pHTTPTSESS)
{
	pHTTPTSESS->usL4FailCode = pTCPINFO->usL4FailCode;
	pHTTPTSESS->ucTcpClientStatus = pTCPINFO->ucTcpClientStatus;
	pHTTPTSESS->ucTcpServerStatus = pTCPINFO->ucTcpServerStatus;

	pHTTPTSESS->uiIPDataUpPktCnt += pTCPINFO->uiIPDataUpPktCnt;
	pHTTPTSESS->uiIPDataDnPktCnt += pTCPINFO->uiIPDataDnPktCnt;
	pHTTPTSESS->uiIPTotUpPktCnt += pTCPINFO->uiIPTotUpPktCnt;
	pHTTPTSESS->uiIPTotDnPktCnt += pTCPINFO->uiIPTotDnPktCnt;
	pHTTPTSESS->uiIPDataUpRetransCnt += pTCPINFO->uiIPDataUpRetransCnt;
	pHTTPTSESS->uiIPDataDnRetransCnt += pTCPINFO->uiIPDataDnRetransCnt;
	pHTTPTSESS->uiIPTotUpRetransCnt += pTCPINFO->uiIPTotUpRetransCnt;
	pHTTPTSESS->uiIPTotDnRetransCnt += pTCPINFO->uiIPTotDnRetransCnt;
	pHTTPTSESS->uiIPDataUpPktSize += pTCPINFO->uiIPDataUpPktSize;
	pHTTPTSESS->uiIPDataDnPktSize += pTCPINFO->uiIPDataDnPktSize;
	pHTTPTSESS->uiIPTotUpPktSize += pTCPINFO->uiIPTotUpPktSize;
	pHTTPTSESS->uiIPTotDnPktSize += pTCPINFO->uiIPTotDnPktSize;
	pHTTPTSESS->uiIPDataUpRetransSize += pTCPINFO->uiIPDataUpRetransSize;
	pHTTPTSESS->uiIPDataDnRetransSize += pTCPINFO->uiIPDataDnRetransSize;
	pHTTPTSESS->uiIPTotUpRetransSize += pTCPINFO->uiIPTotUpRetransSize;
	pHTTPTSESS->uiIPTotDnRetransSize += pTCPINFO->uiIPTotDnRetransSize;
}

S32 dCheckReqHeader(HTTP_TSESS *pHTTPTSESS, U8 *pData, U16 usLen)
{
	S32			dRet;
	S32			dResCode, dMethod, dLogLen, urlType;
	U8			szLogUrl[MAX_LOGURL_SIZE];

	dLogLen = 0;
	dMethod = 0;
	urlType = 0;
	szLogUrl[0] = 0x00;

	if((dRet = httpheader((char*)pData, usLen, (char*)szLogUrl, &dLogLen, &urlType, &dResCode, &dMethod)) != 0) {
		/* 분석 실패 */
		log_print(LOGN_INFO, "[%s][%s.%d] httpheader dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return 0;
	}	
	
	switch(dMethod)
	{
	case METHOD_GET:
	case METHOD_POST:
	case METHOD_HEAD:
	case METHOD_OPTIONS:
	case METHOD_PUT:
	case METHOD_DELETE:
	case METHOD_TRACE:
	case METHOD_CONNECT:
	case METHOD_RESULT:
	case METHOD_DESCRIBE:
	case METHOD_SETUP:
	case METHOD_PLAY:
	case METHOD_PAUSE:
	case METHOD_ANNOUNCE:
	case METHOD_GET_PARAMETER:
	case METHOD_RECORD:
	case METHOD_REDIRECT:
	case METHOD_SET_PARAMETER:
	case METHOD_TEARDOWN:
		break;
	default:
		log_print(LOGN_INFO, "[%s][%s.%d] dMethod[%d]", __FILE__, __FUNCTION__, __LINE__, dMethod);
		return 0;
	}

	pHTTPTSESS->urlType = urlType;
	pHTTPTSESS->usUrlSize = dLogLen;
	pHTTPTSESS->ucMethod = dMethod;
	memcpy(pHTTPTSESS->szUrl, szLogUrl, dLogLen);
	pHTTPTSESS->szUrl[dLogLen] = 0x00;

	return 1;
}

S32 dGetReqHeaderInfo(HTTP_TSESS *pHTTPTSESS, U8 *pNode, S32 *pdCLen, U8 *isbuf, U32 *maxLen, S32 *multilen, U8 *multi, S32 *zip)
{
	/* HOST, ContentsType */
	TLV			*pTLV;
	U8			*pData;
	S32			dChunked;
	S32			dRet = 0;
	U8			szCType[MAX_TEMP_CONTENT_SIZE];
	S32			ctype = 0;
	S32			ctypelen = 0;
	S32			hostlen = 0;
	S32			dMultiPart = 0;
	S32			dPktCnt = 0;

	*pdCLen = 0;
	dChunked = 0;
	szCType[0] = 0x00;
	*isbuf = TSESS_BUFFERING_OFF;
	*maxLen = 0;

	pTLV = (TLV *)(pNode + NIFO_SIZE);
	pData = (pNode + NIFO_SIZE + TLV_SIZE);

	if(httphdrinfo((char*)pData, pTLV->len, (char*)pHTTPTSESS->szHostName, &hostlen, (char*)szCType, &ctypelen, pdCLen, &dChunked, zip, &dPktCnt) != 0) {
		/* 분석 실패 */
		return dRet;
	}	

	if(httpctype((char*)szCType, ctypelen, &ctype, multilen, (char*)multi) == 0) {
		if(ctype != 0) {
			switch(ctype)
			{
			case 2:
				*maxLen = MAX_K3G_SIZE;
				break;
			case 3:
				dMultiPart = 1;
				break;
			case 4:
				*maxLen = MAX_STREAM_SIZE;
				break;
			case 5:
				*maxLen = MAX_EMS_SIZE;
				break;
			}	
//			*isbuf = TSESS_BUFFERING_ON;

		} else {
			log_print(LOGN_INFO, "[%s][%s.%d] CTYPE[%s]", __FILE__, __FUNCTION__, __LINE__, szCType);
		}
	}

	if(*pdCLen > 0) dRet = LEN_TYPE_CONTENTLENGTH;
	else if(dChunked == 1) dRet = LEN_TYPE_CHUNKED;
	else if(dMultiPart == 1) dRet = LEN_TYPE_MULTIPART;
	else if(dPktCnt > 0) {
		dRet = LEN_TYPE_PACKETCOUNTER;
		*pdCLen = dPktCnt;
	}

	if(hostlen == 0) {
		httphost((char*)pHTTPTSESS->szUrl, pHTTPTSESS->usUrlSize, (char*)pHTTPTSESS->szHostName, &hostlen);
	}

	/* host, url host is null */
	if(hostlen == 0) {
		sprintf((char*)pHTTPTSESS->szHostName, "%d.%d.%d.%d", HIPADDR(pHTTPTSESS->uiSrvIP));	
		hostlen = strlen((char*)pHTTPTSESS->szHostName);
	}

	pHTTPTSESS->hostNameLen = hostlen;
	
	return dRet;
}

S32 dCheckResHeader(HTTP_TSESS *pHTTPTSESS, U8 *pData, U16 usLen)
{
	S32			dRet;
	S32			dResCode, dMethod, dLogLen, urlType;
	U8			szLogUrl[MAX_LOGURL_SIZE];

	dLogLen = 0;
	dMethod = 0;
	dResCode = 0;
	urlType = 0;

	if((dRet = httpheader((char*)pData, usLen, (char*)szLogUrl, &dLogLen, &urlType, &dResCode, &dMethod)) != 0) {
		/* 분석 실패 */
		log_print(LOGN_INFO, "[%s][%s.%d] httpheader dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return 0;
	}	
	
	if(dMethod != METHOD_RESPONSE) {
		log_print(LOGN_INFO, "[%s][%s.%d] dMethod[%d]", __FILE__, __FUNCTION__, __LINE__, dMethod);
		return 0;
	}

	pHTTPTSESS->usResCode = dResCode;

	return 1;
}

S32 dGetResHeaderInfo(HTTP_TSESS *pHTTPTSESS, U8 *pNode, S32 *pdCLen, U8 *isbuf, U32 *maxLen, S32 *multilen, U8 *multi, S32 *zip)
{
	/* HOST, ContentsType */
	TLV			*pTLV;
	U8			*pData;
	S32			dChunked;
	S32			dRet = 0;
	U8			szCType[MAX_TEMP_CONTENT_SIZE];
	U8			szHostName[MAX_HOSTNAME_SIZE];
	S32			ctype = 0;
	S32			ctypelen = 0;
	S32			hostlen;
	S32			dMultiPart = 0;
	S32			dPktCnt = 0;
	S32			ctypeinfolen = 0;

	*pdCLen = 0;
	dChunked = 0;
	szCType[0] = 0x00;
	*isbuf = TSESS_BUFFERING_OFF;
	*maxLen = 0;

	pTLV = (TLV *)(pNode + NIFO_SIZE);
	pData = (pNode + NIFO_SIZE + TLV_SIZE);

	if(httphdrinfo((char*)pData, pTLV->len, (char*)szHostName, &hostlen, (char*)szCType, &ctypelen, pdCLen, &dChunked, zip, &dPktCnt) != 0) {
		/* 분석 실패 */
		return dRet;
	}	

	if(httpctype((char*)szCType, ctypelen, &ctype, multilen, (char*)multi) == 0) {
		if(ctype != 0) {
			switch(ctype)
			{
			case 2:
				*maxLen = MAX_K3G_SIZE;
				break;
			case 3:
				dMultiPart = 1;
				break;
			case 4:
				*maxLen = MAX_STREAM_SIZE;
				break;
			case 5:
				*maxLen = MAX_EMS_SIZE;
				break;
			}	
//			*isbuf = TSESS_BUFFERING_ON;

		} else {
			log_print(LOGN_INFO, "[%s][%s.%d] CTYPE[%s]", __FILE__, __FUNCTION__, __LINE__, szCType);
		}
	}

	if(*pdCLen > 0) dRet = LEN_TYPE_CONTENTLENGTH;
	else if(dChunked == 1) dRet = LEN_TYPE_CHUNKED;
	else if(dMultiPart == 1) dRet = LEN_TYPE_MULTIPART;
	else if(dPktCnt > 0) {
		dRet = LEN_TYPE_PACKETCOUNTER;
		*pdCLen = dPktCnt;
	}

	pHTTPTSESS->usContentsTypeSize = 0;
	pHTTPTSESS->szContentsType[0] = 0x00;

	if(ctypelen > 0)
	{
		httpctypeinfo((char*)szCType, ctypelen, (char*)pHTTPTSESS->szContentsType, &ctypeinfolen);
		pHTTPTSESS->usContentsTypeSize = ctypeinfolen;
	}

	return dRet;
}

S32 GetURL(U8 *inUrl, S32 urlLen, S32 urlType, U8 *hostName, S32 hostNameLen, U8 *outUrl, S32 maxLen)
{
	int		i;
	int		newUrlLen = 0;
	int		tmpLen = 0;
	int		copyLen = 0;

	log_print(LOGN_INFO, "GetURL BEFORE URLTYPE[%d]HOSTNAME[%s]HOSTNAMELEN[%d]", urlType, hostName, hostNameLen);
	log_print(LOGN_INFO, "GetURL BEFORE SIZE[%d]URL[\n%s]", urlLen, inUrl);

	switch(urlType)
	{
	case URL_TYPE_HOST_PORT:
	case URL_TYPE_HOST_NOPORT:
//		memcpy(outUrl, inUrl, urlLen);
		for(i = 0; i < urlLen; i++) 
			TOLOWER(outUrl[i], inUrl[i]);

		newUrlLen = urlLen;
		/* http://를 소문자로 바꾼다. */
		/* URI 비교를 위해서 HostName도 소문자로 바꿔야 할지도 모름 현재는 HostName은 바꾸지 않는다. */
		/* RFC 2616 	3.2.3 URI Comparison 부분 참조 */
//		TOLOWER(outUrl, 4);
		outUrl[newUrlLen] = 0x00;	
		break;
	case URL_TYPE_NOHOST_NOPORT:
		if(hostNameLen >= 0) {
			tmpLen = 7;
			memcpy(outUrl, "http://", tmpLen);
			memcpy(&outUrl[tmpLen], hostName, hostNameLen);
			tmpLen = hostNameLen + 7;
			if(urlLen + tmpLen > maxLen) {
				copyLen = maxLen - tmpLen;
				newUrlLen = maxLen;
			} else {
				copyLen = urlLen;
				newUrlLen = urlLen + tmpLen;
			}
			
//			memcpy(&outUrl[tmpLen], inUrl, copyLen);
			for(i = 0; i < copyLen; i++) 
				TOLOWER(outUrl[tmpLen + i], inUrl[i]);

			outUrl[newUrlLen] = 0x00;
		}
		break;
	case URL_TYPE_CONNECT:
		tmpLen = 7;
		if(urlLen + tmpLen > maxLen) {
			copyLen = maxLen - tmpLen;	
			newUrlLen = maxLen;
		} else {
			copyLen = urlLen;
			newUrlLen = urlLen + tmpLen;	
		}

		memcpy(outUrl, "http://", tmpLen);
//		memcpy(&outUrl[tmpLen], inUrl, copyLen);
		for(i = 0; i < copyLen; i++) 
			TOLOWER(outUrl[tmpLen + i], inUrl[i]);

		outUrl[newUrlLen] = 0x00;
		break;
	default:
		log_print(LOGN_CRI, "[%s][%s.%d] UNKNOWN URLTYPE[%d]", __FILE__, __FUNCTION__, __LINE__, urlType);
		newUrlLen = 0;
		break;
	}

	log_print(LOGN_INFO, "GetURL AFTER SIZE[%d]URL[\n%.*s]", newUrlLen, newUrlLen, outUrl);

	return newUrlLen;
}

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

U16 GetFailCode(LOG_IHTTP_TRANS *pLOGHTTP, S32 httpTransStatus)
{
	/*
	 * HTTP_UERR_911, HTTP_UERR_941, HTTP_UERR_970은 http_func.c의 dProcHttpTrans DEF_TCP_DATA에서 처리
	 * HTTP_UERR_980은 http_func.c의 pCreateHttpTrans 함수에서 결정
	 */
	switch(httpTransStatus)
	{
	case HTTP_TRANS_STATUS_REQHDRING:
		/* 
		 * 체크 대상 에러코드
		 * HTTP_UERR_910 
		 */
		return HTTP_UERR_910;
	case HTTP_TRANS_STATUS_REQHDR:
		/* 
		 * 체크 대상 에러코드
		 * HTTP_UERR_920, HTTP_UERR_930 
		 */
		if(pLOGHTTP->uiReqAckTime == 0) {
			return HTTP_UERR_920;
		} else {
			return HTTP_UERR_930;
		}		
	case HTTP_TRANS_STATUS_REQBODYING:
		/* 
		 * 체크 대상 에러코드
		 * HTTP_UERR_910 
		 */
		return HTTP_UERR_910;
	case HTTP_TRANS_STATUS_REQBODY:
		/* 
		 * 체크 대상 에러코드
		 * HTTP_UERR_920, HTTP_UERR_930 
		 */
		if(pLOGHTTP->uiReqAckTime == 0) {
			return HTTP_UERR_920;
		} else {
			return HTTP_UERR_930;
		}
	case HTTP_TRANS_STATUS_RESHDRING:
		/* 
		 * 체크 대상 에러코드
		 * HTTP_UERR_940 
		 */
		return HTTP_UERR_940;
	case HTTP_TRANS_STATUS_RESHDR:
		/* 
		 * 체크 대상 에러코드
		 * HTTP_UERR_940, HTTP_UERR_950
		 */
		if(pLOGHTTP->uiContentLength > 0) {
			return HTTP_UERR_940;
		} else if(pLOGHTTP->uiMNAckTime == 0) {
			return HTTP_UERR_950;
		}
		break;
	case HTTP_TRANS_STATUS_RESBODYING:
		/* 
		 * 체크 대상 에러코드
		 * HTTP_UERR_940 
		 */
		return HTTP_UERR_940;
	case HTTP_TRANS_STATUS_RESBODY:
		/* 
		 * 체크 대상 에러코드
		 * HTTP_UERR_950
		 */
		if(pLOGHTTP->uiMNAckTime == 0) {
			return HTTP_UERR_950;
		}
		break;
	case HTTP_TRANS_STATUS_ACKHDRING:
	case HTTP_TRANS_STATUS_ACKHDR:
	case HTTP_TRANS_STATUS_ACKBODYING:
	case HTTP_TRANS_STATUS_ACKBODY:
	default:
		log_print(LOGN_CRI, "[%s][%s.%d] STRANGE STATUS[%d]", __FILE__, __FUNCTION__, __LINE__, httpTransStatus);
		break;
	}

	if(pLOGHTTP->ucMethod == 0) {
		return HTTP_UERR_960;
	}

	if(pLOGHTTP->usResCode >= 400) {
		return HTTP_UERR_900;
	}

	if(pLOGHTTP->usL7FailCode > 0) {
		return pLOGHTTP->usL7FailCode;
	}

	return HTTP_UERR_EMPTY;
}

U16 GetHtttpTransStatus(st_MSG_INFO *pMSGINFO, S32 isNotREQ)
{
	U16		ret = 0;

	switch(pMSGINFO->ucStatus)
	{
	case TSESS_STATUS_HDRWAIT:
	case TSESS_STATUS_HDRDOING:
		if(pMSGINFO->ucEndStatus == END_STATE_0D0A0D0A) {
			ret = (isNotREQ) ? HTTP_TRANS_STATUS_RESHDR : HTTP_TRANS_STATUS_REQHDR;			
		} else {
			ret = (isNotREQ) ? HTTP_TRANS_STATUS_RESHDRING : HTTP_TRANS_STATUS_REQHDRING;			
		}
		break;
	case TSESS_STATUS_BODYWAIT:
	case TSESS_STATUS_BODYDOING:
		if(pMSGINFO->ucEndStatus == END_STATE_0D0A0D0A) {
			ret = (isNotREQ) ? HTTP_TRANS_STATUS_RESBODY : HTTP_TRANS_STATUS_REQBODY;			
		} else {
			ret = (isNotREQ) ? HTTP_TRANS_STATUS_RESBODYING : HTTP_TRANS_STATUS_REQBODYING;			
		}
		break;
	default:
		log_print(LOGN_CRI, "[%s][%s.%d] STANGE TSESS STATUS[%d][%s]ENDSTATUS[%d][%s]ISHEADER[%d][%s]",
			__FILE__, __FUNCTION__, __LINE__, pMSGINFO->ucStatus, PrintTSESSStatus(pMSGINFO->ucStatus),
			pMSGINFO->ucEndStatus, PrintEndStatus(pMSGINFO->ucEndStatus),
			isNotREQ, PrintIsHDR(isNotREQ));	
		break;
	}
	return ret;
}

U16 GetLogHttpStatus(st_MSG_INFO *pMSGINFO, S32 isNotREQ)
{
	U16		ret = LOG_HTTP_STATUS_UNKNOWN;

	if(!isNotREQ) {
		switch(pMSGINFO->ucStatus)
		{
		case TSESS_STATUS_HDRWAIT:
		case TSESS_STATUS_HDRDOING:
			if(pMSGINFO->uiLen > 0) {
				ret = LOG_HTTP_REQ_DOING;
			} else {
				if(pMSGINFO->ucEndStatus == END_STATE_0D0A0D0A) {
					if(pMSGINFO->uiAckTime > 0) {
						ret = LOG_HTTP_REQ_ACK;
					} else {
						ret = LOG_HTTP_REQ_DONE;
					}
				} else {
					ret = LOG_HTTP_REQ_DOING;
				}
			}
			break;
		case TSESS_STATUS_BODYWAIT:
		case TSESS_STATUS_BODYDOING:
			if(pMSGINFO->ucEndStatus == END_STATE_0D0A0D0A) {
				if(pMSGINFO->uiAckTime > 0) {
					ret = LOG_HTTP_REQ_ACK;
				} else {
					ret = LOG_HTTP_REQ_DONE;
				}
			} else {
				ret = LOG_HTTP_REQ_DOING;
			}
			break;
		default:
			log_print(LOGN_CRI, "[%s][%s.%d] REQ STRANGE TSESS STATUS[%d]",
				__FILE__, __FUNCTION__, __LINE__, pMSGINFO->ucStatus);
			break;
		}
	} else {
		switch(pMSGINFO->ucStatus)
		{
		case TSESS_STATUS_HDRWAIT:
		case TSESS_STATUS_HDRDOING:
			if(pMSGINFO->uiLen > 0) {
				ret = LOG_HTTP_RES_DOING;
			} else {
				if(pMSGINFO->ucEndStatus == END_STATE_0D0A0D0A) {
					if(pMSGINFO->uiAckTime > 0) {
						ret = LOG_HTTP_RES_ACK;
					} else {
						ret = LOG_HTTP_RES_DONE;
					}
				} else {
					ret = LOG_HTTP_RES_DOING;
				}
			}
			break;
		case TSESS_STATUS_BODYWAIT:
		case TSESS_STATUS_BODYDOING:
			if(pMSGINFO->ucEndStatus == END_STATE_0D0A0D0A) {
				if(pMSGINFO->uiAckTime > 0) {
					ret = LOG_HTTP_RES_ACK;
				} else {
					ret = LOG_HTTP_RES_DONE;
				}
			} else {
				ret = LOG_HTTP_RES_DOING;
			}
			break;
		default:
			log_print(LOGN_CRI, "[%s][%s.%d] RES STRANGE TSESS STATUS[%d]",
				__FILE__, __FUNCTION__, __LINE__, pMSGINFO->ucStatus);
			break;
		}
	}

	return ret;
}

char *PrintRtx(U8 ucRtxType)
{
	switch(ucRtxType)
	{
	case DEF_FROM_NONE:				return "FROM_NONE";
	case DEF_FROM_CLIENT:			return "FROM_CLIENT";
	case DEF_FROM_SERVER:			return "FROM_SERVER";
	default:						return "UNKNOWN";
	}
}

char *PrintIsHDR(S32 isHDR)
{
	return ((isHDR) ? "IS HEADER" : "IS NOT HEADER");
}

char *PrintTSESSStatus(S32 status)
{
	switch(status)
	{
	case TSESS_STATUS_HDRWAIT: 		return "HDRWAIT";
	case TSESS_STATUS_HDRDOING: 	return "HDRDOING";
	case TSESS_STATUS_BODYWAIT: 	return "BODYWAIT";
	case TSESS_STATUS_BODYDOING: 	return "BODYDOING";
	default: 						return "UNKNOWN";
	}
}

char *PrintLenType(S32 lentype)
{
	switch(lentype)
	{
	case LEN_TYPE_CONTENTLENGTH: 	return "CONTENT-LENGTH";
	case LEN_TYPE_CHUNKED: 			return "CHUNKED";
	case LEN_TYPE_MULTIPART: 		return "MULTI-PART";
	case LEN_TYPE_PACKETCOUNTER: 	return "PACKET-COUNTER";
	default: 						return "UNKNOWN";
	}
}

char *PrintBuffering(S32 isbuffer)
{
	return ((isbuffer == TSESS_BUFFERING_ON) ? "BUFFERING ON" : "BUFFERING OFF");
}

char *PrintEndStatus(S32 status)
{
	switch(status)
	{
	case END_STATE_EMPTY: 		return "EMPTY";
	case END_STATE_0D: 			return "0D";
	case END_STATE_0D0A: 		return "0D0A";
	case END_STATE_0D0A0D: 		return "0D0A0D";
	case END_STATE_0D0A0D0A: 	return "0D0A0D0A";
	case END_STATE_30: 			return "30";
	default: 					return "UNKNOWN";
	}
}

S32	dGetProcID(S32 appCode, U32 uiClientIP)
{
	switch(appCode)
	{
	default:					return dGetCALLProcID(uiClientIP);
    }	
}

S32 dGetCALLProcID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

#if 0
S32 dGetPlatformTypeOld(S32 l4Code)
{
	S32		platformType = 0;

	switch(l4Code)
	{
	case L4_WAP20:
	case L4_TODAY:
		platformType = DEF_PLATFORM_WAP20;
		break;
	case L4_WIPI:
		platformType = DEF_PLATFORM_WIPI;
		break;
	case L4_DN_2G:
	case L4_DN_JAVA:
	case L4_DN_VOD:
		platformType = DEF_PLATFORM_DN;
		break;
	case L4_VOD_STREAM:
		platformType = DEF_PLATFORM_VOD;
		break;
	case L4_MMS_UP:
	case L4_MMS_DN:
		platformType = DEF_PLATFORM_MMS;
		break;
	case L4_JNC:
		platformType = DEF_PLATFORM_JNC;
		break;
	case L4_FV_FB:
	case L4_FV_EMS:
	case L4_FV_IV:
		platformType = DEF_PLATFORM_FV;
		break;
	case L4_EMS:
		platformType = DEF_PLATFORM_EMS;
		break;
	case L4_FB:
		platformType = DEF_PLATFORM_FB;
		break;
	case L4_XCAP:
		platformType = DEF_PLATFORM_IMS;
		break;
	case L4_WIDGET:
		platformType = DEF_PLATFORM_WIDGET;
		break;
	case L4_MBOX:
		platformType = DEF_PLATFORM_INET;
		break;
	default:
		log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN L4CODE[%d]", __FILE__, __FUNCTION__, __LINE__, l4Code);
		platformType = 0;
		break;
	}
	return platformType;
}
#endif

U8 *pAllocDataNode(stMEMSINFO *pMEMSINFO, S32 type)
{
	U8 *pNode;

	if((pNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	if(nifo_tlv_alloc(pMEMSINFO, pNode, type, 0, DEF_MEMSET_OFF) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d nifo_tlv_alloc HTTP_DATA_NUM NULL", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pMEMSINFO, pNode);
		return NULL;
	}

	return pNode;
}

S32 dCheckRtx(U8 *pData, U16 usLen, U8 ucRtx)
{
	S32			dRet, rtx = -1;
	S32			dResCode, dMethod, dLogLen, urlType;
	U8			szLogUrl[MAX_LOGURL_SIZE];

    dMethod = 0;

	if((dRet = httpheader((char*)pData, usLen, (char*)szLogUrl, &dLogLen, &urlType, &dResCode, &dMethod)) != 0) {
		return -1;
	}

	switch(dMethod)
	{
    	case METHOD_GET:
    	case METHOD_POST:
    	case METHOD_HEAD:
    	case METHOD_OPTIONS:
    	case METHOD_PUT:
    	case METHOD_DELETE:
    	case METHOD_TRACE:
    	case METHOD_CONNECT:
    	case METHOD_RESULT:
    	case METHOD_DESCRIBE:
    	case METHOD_SETUP:
    	case METHOD_PLAY:
    	case METHOD_PAUSE:
    	case METHOD_ANNOUNCE:
    	case METHOD_GET_PARAMETER:
    	case METHOD_RECORD:
    	case METHOD_REDIRECT:
    	case METHOD_SET_PARAMETER:
    	case METHOD_TEARDOWN:
        	rtx = ucRtx;
			break;
		case METHOD_RESPONSE:
			rtx = ((ucRtx == DEF_FROM_SERVER) ? DEF_FROM_CLIENT : DEF_FROM_SERVER);
			break;
	}	

	return rtx;
}

/*
 * $Log: ihttp_util.c,v $
 * Revision 1.2  2011/09/04 11:40:36  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/17 12:57:50  hhbaek
 * A_IHTTP
 *
 * Revision 1.2  2011/08/10 09:57:43  uamyd
 * modified and block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.10  2011/05/09 10:30:10  dark264sh
 * A_IHTTP: A_CALL multi 처리
 *
 * Revision 1.9  2011/04/19 10:46:52  dark264sh
 * A_IHTTP: PrintRtx 함수 수정
 *
 * Revision 1.8  2011/04/16 13:18:49  dark264sh
 * A_IHTTP: dGetQID 변경
 *
 * Revision 1.7  2011/04/16 11:50:27  dark264sh
 * A_IHTTP: RTX를 모르는 경우 http header를 보고 판단
 *
 * Revision 1.6  2011/04/16 10:54:17  dark264sh
 * A_IHTTP: GetGap32Time 함수에서 시간이 0보다 큰 경우만 계산
 *
 * Revision 1.5  2011/04/14 14:54:48  dark264sh
 * A_IHTTP: Content-Type 처리 추가
 *
 * Revision 1.4  2011/04/14 13:11:29  dark264sh
 * A_IHTTP: GapTime 처리 추가
 *
 * Revision 1.3  2011/04/14 12:07:59  dark264sh
 * A_IHTTP: Body Buffering off, Log만 전송 하도록 변경
 *
 * Revision 1.2  2011/04/14 11:20:41  dark264sh
 * A_IHTTP: LOG_HTTP_TRANS => LOG_IHTTP_TRANS 변경
 *
 * Revision 1.1  2011/04/11 12:06:34  dark264sh
 * A_IHTTP 추가
 *
 */
