/**		@file	http_util.c
 * 		- Util 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: http_util.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:47 $
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

// LIB
#include "typedef.h"
#include "commdef.h"
#include "loglib.h"

// PROJECT
#include "procid.h"
#include "common_stg.h"

// TAF
#include "http.h"

// .
#include "http_util.h"

extern int		gACALLCnt;

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

//		TOLOWER(pTmp[((NIFO *)pTmp)->lastoffset], szInData[i]);
		pTmp[((NIFO *)pTmp)->lastoffset] = szInData[i];
		((NIFO *)pTmp)->lastoffset++;
		pTLV->len++;

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

void UpCount(LOG_HTTP_TRANS *pLOGHTTP, HTTP_TSESS *pHTTPTSESS)
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
			*isbuf = TSESS_BUFFERING_ON;

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
			*isbuf = TSESS_BUFFERING_ON;

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

U16 GetFailCode(LOG_HTTP_TRANS *pLOGHTTP, S32 httpTransStatus)
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

U8 *PrintRtx(U8 ucRtxType)
{
	return (U8*)((ucRtxType == DEF_FROM_SERVER) ? "FROM_SERVER" : "FROM_CLIENT");
}

U8 *PrintIsHDR(S32 isHDR)
{
	return (U8*)((isHDR) ? "IS HEADER" : "IS NOT HEADER");
}

U8 *PrintTSESSStatus(S32 status)
{
	switch(status)
	{
	case TSESS_STATUS_HDRWAIT: 		return (U8*)"HDRWAIT";
	case TSESS_STATUS_HDRDOING: 	return (U8*)"HDRDOING";
	case TSESS_STATUS_BODYWAIT: 	return (U8*)"BODYWAIT";
	case TSESS_STATUS_BODYDOING: 	return (U8*)"BODYDOING";
	default: 						return (U8*)"UNKNOWN";
	}
}

U8 *PrintLenType(S32 lentype)
{
	switch(lentype)
	{
	case LEN_TYPE_CONTENTLENGTH: 	return (U8*)"CONTENT-LENGTH";
	case LEN_TYPE_CHUNKED: 			return (U8*)"CHUNKED";
	case LEN_TYPE_MULTIPART: 		return (U8*)"MULTI-PART";
	case LEN_TYPE_PACKETCOUNTER: 	return (U8*)"PACKET-COUNTER";
	default: 						return (U8*)"UNKNOWN";
	}
}

U8 *PrintBuffering(S32 isbuffer)
{
	return (U8*)((isbuffer == TSESS_BUFFERING_ON) ? "BUFFERING ON" : "BUFFERING OFF");
}

U8 *PrintEndStatus(S32 status)
{
	switch(status)
	{
	case END_STATE_EMPTY: 		return (U8*)"EMPTY";
	case END_STATE_0D: 			return (U8*)"0D";
	case END_STATE_0D0A: 		return (U8*)"0D0A";
	case END_STATE_0D0A0D: 		return (U8*)"0D0A0D";
	case END_STATE_0D0A0D0A: 	return (U8*)"0D0A0D0A";
	case END_STATE_30: 			return (U8*)"30";
	default: 					return (U8*)"UNKNOWN";
	}
}

S32	dGetSeqProcID(S32 appCode, U32 uiClientIP)
{
	switch(appCode)
	{
		case SEQ_PROC_A_CALL0:	
			return appCode + ( uiClientIP % gACALLCnt );
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
			return appCode;
		default:				return 0;
    }	
}

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

/*
 * $Log: http_util.c,v $
 * Revision 1.3  2011/09/07 06:30:47  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/04 11:12:12  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.2  2011/08/05 09:04:49  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.7  2011/05/09 15:18:31  jsyoon
 * *** empty log message ***
 *
 * Revision 1.6  2011/01/11 04:09:07  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.5  2009/10/26 08:57:37  pkg
 * A_HTTP LimitLen 처리 추가
 *
 * Revision 1.4  2009/08/18 14:51:57  pkg
 * A_HTTP node full 버그 수정
 *
 * Revision 1.3  2009/07/15 16:12:38  dqms
 * 멀티프로세스 수정
 *
 * Revision 1.2  2009/06/16 15:16:00  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:21  dqms
 * Init TAF_RPPI
 *
 * Revision 1.11  2009/03/02 09:11:12  dark264sh
 * DOWNLOAD VOD : 1x 단말에서 Content-Length, Transfer-Encoding Chunked, Multi Part 없이 Packet-Counter만 있는 경우 처리
 *
 * Revision 1.10  2008/12/18 05:59:16  dark264sh
 * MBOX Streamming 분석 추가
 *
 * Revision 1.9  2008/11/25 12:50:04  dark264sh
 * WIDGET 처리
 *
 * Revision 1.8  2008/09/18 07:40:39  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.7  2008/07/14 07:20:54  dark264sh
 * *** empty log message ***
 *
 * Revision 1.6  2008/06/24 23:40:49  jsyoon
 * *** empty log message ***
 *
 * Revision 1.5  2008/06/22 10:20:42  dark264sh
 * A_FB chunked, multipart, gzip, deflate, min 처리
 *
 * Revision 1.4  2008/06/19 11:48:36  dark264sh
 * 오늘은 서비스 code 변경에 따른 변경
 *
 * Revision 1.3  2008/06/18 12:28:26  dark264sh
 * A_FB 추가
 *
 * Revision 1.2  2008/06/17 12:23:56  dark264sh
 * A_FV, A_EMS 추가
 *
 * Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.7  2007/10/08 04:38:39  dark264sh
 * no message
 *
 * Revision 1.6  2007/09/05 08:45:20  dark264sh
 * *** empty log message ***
 *
 * Revision 1.5  2007/09/05 07:12:26  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2007/09/03 05:29:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2007/08/29 07:41:01  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2007/08/27 13:58:03  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/08/21 12:53:00  dark264sh
 * no message
 *
 * Revision 1.28  2007/05/08 11:37:11  dark264sh
 * 로그 잘못 남긴 부분 수정
 *
 * Revision 1.27  2006/12/06 09:08:24  dark264sh
 * *** empty log message ***
 *
 * Revision 1.26  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.25  2006/11/22 08:08:11  dark264sh
 * *** empty log message ***
 *
 * Revision 1.24  2006/11/16 09:58:49  dark264sh
 * *** empty log message ***
 *
 * Revision 1.23  2006/11/14 03:04:09  dark264sh
 * *** empty log message ***
 *
 * Revision 1.22  2006/11/13 10:39:45  dark264sh
 * *** empty log message ***
 *
 * Revision 1.21  2006/11/10 13:58:22  dark264sh
 * *** empty log message ***
 *
 * Revision 1.20  2006/11/10 12:07:54  dark264sh
 * *** empty log message ***
 *
 * Revision 1.19  2006/11/10 09:32:59  dark264sh
 * *** empty log message ***
 *
 * Revision 1.18  2006/11/08 07:13:41  shlee
 * CONF관련 hasho -> hashg로 변경 및 CONF_CNT 101 CONF_PREA_CNT 811로 변경
 *
 * Revision 1.17  2006/11/06 07:36:54  dark264sh
 * nifo NODE size 4*1024 => 6*1024로 변경하기
 * nifo_tlv_alloc에서 argument로 memset할지 말지 결정하도록 수정
 * nifo_node_free에서 semaphore 삭제
 *
 * Revision 1.16  2006/11/02 07:56:26  dark264sh
 * *** empty log message ***
 *
 * Revision 1.15  2006/11/02 07:19:42  dark264sh
 * REQ 메시지가 두개로 나누어 진 경우 URL 처리가 잘못되는 문제 해결
 *
 * Revision 1.14  2006/10/30 03:12:49  dark264sh
 * ssl browserinfo, model 값 세팅 추가
 *
 * Revision 1.13  2006/10/30 00:48:28  dark264sh
 * *** empty log message ***
 *
 * Revision 1.12  2006/10/27 12:36:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.11  2006/10/27 03:05:12  dark264sh
 * RSTP SSL 처리
 *
 * Revision 1.10  2006/10/25 02:45:53  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2006/10/24 11:08:10  dark264sh
 * *** empty log message ***
 *
 * Revision 1.8  2006/10/19 05:52:44  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2006/10/18 08:53:31  dark264sh
 * nifo debug 코드 추가
 *
 * Revision 1.6  2006/10/12 12:56:00  dark264sh
 * *** empty log message ***
 *
 * Revision 1.5  2006/10/12 12:30:53  dark264sh
 * URL 시작인 http://를 소문자로 바꾸어 주도록 변경
 *
 * Revision 1.4  2006/10/12 07:49:37  dark264sh
 * hash key값 초기화 문제 버그 해결
 *
 * Revision 1.3  2006/10/11 12:29:27  dark264sh
 * URL에서 포트 번호를 빼던 것을 있으면 그냥 두는 것으로 수정
 *
 * Revision 1.2  2006/10/11 11:52:33  dark264sh
 * PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 * Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 * no message
 *
 * Revision 1.72  2006/10/10 07:00:30  dark264sh
 * A_CALL에 전송하는 부분 추가
 * nifo_node_alloc 함수 변경에 따른 변경
 * A_TCP에서 timerN_update의 리턴으로 timerNID 업데이트 하도록 변경
 *
 * Revision 1.71  2006/10/09 13:27:26  dark264sh
 * URL을 http://hostname/path 형태로 만드는 부분 추가 (Port 번호를 뺀 값)
 *
 * Revision 1.70  2006/10/04 01:53:47  dark264sh
 * FailCode, HTTP Status 값 할당 기능 추가
 *
 * Revision 1.69  2006/10/02 00:19:21  dark264sh
 * nifo의 TLV 변경에 따른 변경
 *
 * Revision 1.68  2006/09/29 08:58:32  dark264sh
 * Content-type을 보고 버퍼일 결정 하도록 변경
 *
 * Revision 1.67  2006/09/28 05:05:22  dark264sh
 * content-type으로 buffering 처리 부분 추가 하면서 header에도 동일한 조건이 적용돼서 header를 buffering 하지 않는 문제 처리
 *
 * Revision 1.66  2006/09/27 11:35:40  dark264sh
 * *** empty log message ***
 *
 * Revision 1.65  2006/09/27 08:59:23  dark264sh
 * HOST NAME 찾기
 *
 * Revision 1.64  2006/09/27 07:27:13  dark264sh
 * content-type을 보고 버퍼링 결정을 위한 추가
 *
 * Revision 1.63  2006/09/26 07:47:11  dark264sh
 * HTTP로 전송 하기 위한 Data Cnt, Size 잘못 전달 하는 부분 수정
 *
 * Revision 1.62  2006/09/26 04:39:24  dark264sh
 * HTTP LOG값 잘못 되는 부분 수정
 *
 * Revision 1.61  2006/09/25 08:49:12  dark264sh
 * no message
 *
 * Revision 1.60  2006/09/25 06:28:11  dark264sh
 * http_func.c
 *
 * Revision 1.59  2006/09/25 02:58:47  dark264sh
 * *** empty log message ***
 *
 * Revision 1.58  2006/09/21 09:05:23  dark264sh
 * content-length 받아오는 부분 수정
 *
 * Revision 1.57  2006/09/21 08:39:09  dark264sh
 * print 함수 추가
 *
 * Revision 1.56  2006/09/21 08:37:56  dark264sh
 * print 함수 추가
 *
 * Revision 1.55  2006/09/21 07:16:25  dark264sh
 * no message
 *
 * Revision 1.54  2006/09/21 07:15:40  dark264sh
 * no message
 *
 * Revision 1.53  2006/09/21 06:37:13  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.52  2006/09/21 06:35:32  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.51  2006/09/21 06:32:15  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.50  2006/09/21 06:31:04  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.49  2006/09/21 06:29:44  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.48  2006/09/21 05:38:16  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.47  2006/09/21 05:32:29  dark264sh
 * http body 끝을 판단하는 방법 변경
 *
 * Revision 1.46  2006/09/19 07:20:51  dark264sh
 * no message
 *
 * Revision 1.45  2006/09/18 08:59:11  dark264sh
 * no message
 *
 * Revision 1.44  2006/09/18 08:57:28  dark264sh
 * no message
 *
 * Revision 1.43  2006/09/18 07:20:04  dark264sh
 * no message
 *
 * Revision 1.42  2006/09/18 07:17:52  dark264sh
 * no message
 *
 * Revision 1.41  2006/09/18 06:01:45  dark264sh
 * no message
 *
 * Revision 1.40  2006/09/18 04:49:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.39  2006/09/18 03:15:13  dark264sh
 * no message
 *
 * Revision 1.38  2006/09/15 09:28:52  dark264sh
 * nifo_node_link_nont, nifo_node_link_cont API 변경
 *
 * Revision 1.37  2006/09/14 10:33:31  dark264sh
 * no message
 *
 * Revision 1.36  2006/09/14 08:38:12  dark264sh
 * no message
 *
 * Revision 1.35  2006/09/14 04:35:16  dark264sh
 * no message
 *
 * Revision 1.34  2006/09/14 04:22:52  dark264sh
 * no message
 *
 * Revision 1.33  2006/09/11 02:30:28  dark264sh
 * nifo 변경에 따른 변경
 *
 * Revision 1.32  2006/09/06 11:55:30  dark264sh
 * *** empty log message ***
 *
 * Revision 1.31  2006/09/05 05:37:10  dark264sh
 * *** empty log message ***
 *
 * Revision 1.30  2006/09/05 04:57:19  dark264sh
 * 에러 핸들링, 에러 코드 부분 수정
 *
 * Revision 1.29  2006/09/04 05:31:45  dark264sh
 * 에러값 세팅 부분 수정
 *
 * Revision 1.28  2006/08/29 04:27:57  dark264sh
 * no message
 *
 * Revision 1.27  2006/08/29 01:23:12  dark264sh
 * no message
 *
 * Revision 1.26  2006/08/28 12:20:25  dark264sh
 * no message
 *
 * Revision 1.25  2006/08/28 12:18:45  dark264sh
 * DeleteBuffer 함수 추가
 *
 * Revision 1.24  2006/08/28 12:15:55  dark264sh
 * dProcStatus 함수 변경
 *
 * Revision 1.23  2006/08/28 02:06:13  dark264sh
 * no message
 *
 * Revision 1.22  2006/08/28 01:54:35  dark264sh
 * no message
 *
 * Revision 1.21  2006/08/28 01:38:56  dark264sh
 * no message
 *
 * Revision 1.20  2006/08/28 01:29:34  dark264sh
 * no message
 *
 * Revision 1.19  2006/08/28 01:16:29  dark264sh
 * a_http_api.h => http_api.h 변경
 *
 * Revision 1.18  2006/08/25 07:15:08  dark264sh
 * no message
 *
 * Revision 1.17  2006/08/24 04:08:16  dark264sh
 * HTTP 기본 Flow 구성
 *
 * Revision 1.16  2006/08/01 05:45:55  dark264sh
 * no message
 *
 * Revision 1.15  2006/08/01 05:39:35  dark264sh
 * no message
 *
 * Revision 1.14  2006/08/01 05:37:21  dark264sh
 * no message
 *
 * Revision 1.13  2006/08/01 05:35:04  dark264sh
 * no message
 *
 * Revision 1.12  2006/08/01 05:31:22  dark264sh
 * no message
 *
 * Revision 1.11  2006/08/01 05:27:19  dark264sh
 * no message
 *
 * Revision 1.10  2006/08/01 05:18:48  dark264sh
 * no message
 *
 * Revision 1.9  2006/08/01 05:12:47  dark264sh
 * no message
 *
 * Revision 1.8  2006/08/01 05:06:27  dark264sh
 * no message
 *
 * Revision 1.7  2006/07/31 06:02:40  dark264sh
 * no message
 *
 * Revision 1.6  2006/07/28 12:23:25  dark264sh
 * no message
 *
 * Revision 1.5  2006/07/28 08:28:02  dark264sh
 * no message
 *
 * Revision 1.4  2006/07/28 08:01:11  dark264sh
 * TCP_DATA, TCP_DATAEND의 메시지 처리 pseudo 코드 추가
 *
 * Revision 1.3  2006/07/26 11:21:09  dark264sh
 * A_HTTP
 * TCP_DATA pseudo 코드 추가
 *
 * Revision 1.2  2006/07/26 05:22:43  dark264sh
 * no message
 *
 * Revision 1.1  2006/07/26 05:19:20  dark264sh
 * no message
 *
 */
