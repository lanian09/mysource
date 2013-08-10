/**		@file	jnet_util.c
 * 		- JAVA NETWORK CONTENT 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: jnc_util.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:47 $
 * 		@ref		jnet_msgq.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- JAVA NETWORK CONTENT 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "loglib.h"
#include "utillib.h"

#include "common_stg.h"
#include "procid.h"

#include "jnc_func.h"
#include "jnc_msgq.h"
#include "jnc_util.h"


extern struct timeval	stNowTime;
extern int				gACALLCnt;

void MakeJNCHashKey(TCP_INFO *pTCPINFO, JNC_SESS_KEY *pJNCSESSKEY)
{
	pJNCSESSKEY->uiCliIP = pTCPINFO->uiCliIP;
	pJNCSESSKEY->usCliPort = pTCPINFO->usCliPort;
	pJNCSESSKEY->usReserved = 0;
}



S32 dParsingJNCHeader(U32 dLen, U8 *pszBuf, U32 *dOffset, U32 *dRetLen)
{
	U32	i;

	if(pszBuf[0] == '$' && pszBuf[1] =='$')
	{
		return 2; /* HEADER COMPLETE */
	}

	for(i=0; i<dLen; i++) {
		if(pszBuf[i] == '$' && pszBuf[i+1] == '$')
		{
			*dOffset = i;
			*dRetLen = i;
			return 0;
		}
		else if (pszBuf[i] == '&')
		{
			*dOffset = i+1;
			*dRetLen = i;
			return 0;
		}
	}

	return 1; /* HEADER NOT COMPLETE RECV WAIT */
}

S32 dProc_CONNREQ(JNC_SESS_DATA *pNode, U8 *szBuf, U32 dLen)
{
	if(pNode->ucREQParaFlag ==0)
		pNode->ucREQParaFlag = FLD_CPCODE;
	else
	{
		switch (pNode->ucREQParaFlag)
		{
			case FLD_CPCODE:
				strncpy((char*)pNode->szJNCLOG.szCpCode,(char*)szBuf, dLen);
				pNode->szJNCLOG.szCpCode[dLen]=0;
				pNode->ucREQParaFlag= FLD_CONTENT;
				break;
			
			case FLD_CONTENT:
				dLen = (dLen <MAX_MIN_SIZE)?dLen:MAX_CONTENTCODE_SIZE-1;
				strncpy((char*)pNode->szJNCLOG.szContentCode,(char*)szBuf, dLen);
				pNode->szJNCLOG.szContentCode[dLen]=0;
				pNode->ucREQParaFlag= FLD_MIN;
				break;

			case FLD_MIN:
				dLen = (dLen <MAX_MIN_SIZE )? dLen: MAX_MIN_SIZE-1;
				strncpy((char*)pNode->szJNCLOG.szMIN, (char*)szBuf, dLen);
				pNode->szJNCLOG.szMIN[dLen] = 0;
				pNode->ucREQParaFlag = FLD_END;
				break;
		}
	}
	return 1;
}

S32 dProc_RELAY(JNC_SESS_DATA *pNode, U8 *szBuf, U32 dLen)
{
	if (pNode->ucREQParaFlag ==0)
		pNode->ucREQParaFlag=FLD_DATA;
	else
		pNode->ucREQParaFlag= FLD_END;
	return 1;
}

S32 dProc_ACK(JNC_SESS_DATA *pNode, U8 *szBuf, U32 dLen)
{
	if(pNode->ucREQParaFlag==0)
		pNode->ucREQParaFlag=FLD_PHONE;
	else
	{
		switch (pNode->ucREQParaFlag)
		{
			case FLD_PHONE:
				pNode->ucREQParaFlag=FLD_TOTALSIZE;
				dLen = (dLen <MAX_MIN_SIZE )? dLen: MAX_MIN_SIZE-1;
/*GSH
				strncpy(pNode->szJNCLOG.szPhoneNum, szBuf, dLen);
				pNode->szPhoneNum[dLen] = 0;
*/
				strncpy((char*)pNode->szJNCLOG.szMIN, (char*)szBuf, dLen);
				pNode->szJNCLOG.szMIN[dLen] = 0;

				break;

			case FLD_TOTALSIZE:
//				pNode->uiTotalSize = uGetIntNum(szBuf, dLen);
				pNode->ucREQParaFlag = FLD_TIME;
				break;

			case FLD_TIME:
//				pNode->uiTime = uGetIntNum(szBuf, dLen);
				pNode->ucREQParaFlag=FLD_END;
				break;
		}
	}
	return 1;
}

S32 dProc_DOWNBIL(JNC_SESS_DATA *pNode, U8 *szBuf, U32 dLen)
{
	if(pNode->ucREQParaFlag==0)
		pNode->ucREQParaFlag=FLD_CPCODE;
	else
	{
		switch (pNode->ucREQParaFlag==0)
		{
			case FLD_CPCODE:
				pNode->ucREQParaFlag = FLD_CONTENT;
/*GSH
				if(pNode->szJNCLOG.uiCPCode >0)
					break;
		
				pNode->uiCPCode = uGetIntNum(szBuf, dLen);
				break;
*/
				if(pNode->szJNCLOG.szCpCode !=NULL)
					break;
			strncpy((char*)pNode->szJNCLOG.szCpCode,(char*)szBuf, dLen);
					break;

		case FLD_CONTENT:
				pNode->ucREQParaFlag= FLD_PHONE;

				if(pNode->szJNCLOG.szContentCode[0]!=0)
					break;
				dLen = (dLen<MAX_MIN_SIZE)?dLen:MAX_CONTENTCODE_SIZE-1;
				strncpy((char*)pNode->szJNCLOG.szContentCode, (char*)szBuf, dLen);
				pNode->szJNCLOG.szContentCode[dLen] = 0;
				break;

			case FLD_PHONE:
				pNode->ucREQParaFlag= FLD_USECOUNT;
/*GSH
				if(pNode->szPhoneNum[0]!=0)
					 break;
*/
				if(pNode->szJNCLOG.szMIN[0]!=0)
					break;
				dLen = (dLen<MAX_MIN_SIZE )? dLen:MAX_MIN_SIZE-1;
/*GSH
				strncpy(pNode->szJNCLOG.szPhoneNum, szBuf, dLen);
				pNode->szJNCLOG.szPhoneNum[dLen]= 0;
*/
				
				strncpy((char*)pNode->szJNCLOG.szMIN, (char*)szBuf, dLen);
				pNode->szJNCLOG.szMIN[dLen] = 0;

					break;

			case FLD_USECOUNT:
				pNode->ucREQParaFlag = FLD_USETIME;
//				pNode->uiUseCount = uGetIntNum(szBuf, dLen);
				break;

			case FLD_USETIME:
				pNode->ucREQParaFlag= FLD_END;
//				pNode->uiUseTime = uGetIntNum(szBuf, dLen);
				break;
		}
	}
	return 1;
}
/**
 * STRING을 INT로 바꿔주는 함수
 **/
U32 uGetIntNum( U8* szNum, U32 dLen)
{
	U8	szTemp[11];
	U32	uiIntValue;

	memset(szTemp, 0, 11);
	dLen = (dLen <11)?dLen:10;
	strncpy((char*)szTemp, (char*)szNum, dLen);
	szTemp[dLen] = 0;

	sscanf((char*)szTemp, "%u",&uiIntValue);

	return uiIntValue;
}


/**
* REQUEST 메시지가 시작하는지 여부 확인
**/

S32 dCheckREQStart(U8 *szBuf, U32 dLen)
{
	U32		i = 0;

	if (szBuf[0]<=0x06 && szBuf[0]>0x00)
	{
		for(i=1; i<dLen; i++)
		{
			if(szBuf[i]=='&')
				return szBuf[0];
		
			if(szBuf[i]!=0x0)
				return 0;
		}
		return szBuf[0];
	}
	return 0;
}
/**
 * RESCODE를 받음
 **/


S32 dCheckRESCode(U8 *szBuf,JNC_SESS_DATA *pJNCSESS, U32 dLen)
{
	char szResStr[10];

	if(dLen >=4)
	{
		memcpy(szResStr, szBuf, 4);
		szResStr[4]=0;
		pJNCSESS->szJNCLOG.usResCode = atoi(szResStr);
		return 1;
	}
	else
	{
		pJNCSESS->szJNCLOG.usResCode = 0;
		return 0;
	}
}
/**
 * REQUEST 정보를 PARSING해서 해당 LOG에 저장
 **/



U32 dSetJNCREQInfo(stMEMSINFO *pMEMSINFO,TCP_INFO *pTCPINFO, U8 *pJNCDATA, JNC_SESS_DATA *pJNCSESS)
{
	U32 dDataLen, dStatus, offset, dNextLen, dCurLen, dParaLen; 
	S32 dRet;  
	U8	szParameter[32];

	if((pJNCSESS->usREQBuffLen + pTCPINFO->uiDataSize) >= (MAX_BUFF_LEN)) 
	{
	
		log_print(LOGN_CRI, "REQ INVALID HEADER LEN INFO1 BUFLEN:%u DATALEN:%u",
				pJNCSESS->usREQBuffLen, pTCPINFO->uiDataSize);


		memcpy(&pJNCSESS->szREQBuffer[pJNCSESS->usREQBuffLen], 
				pJNCDATA,(MAX_BUFF_LEN-(pJNCSESS->usREQBuffLen)));


		pJNCSESS->usREQBuffLen = MAX_BUFF_LEN;
		dDataLen = MAX_BUFF_LEN;
	}		
	else {

		memcpy(&pJNCSESS->szREQBuffer[pJNCSESS->usREQBuffLen], 
					pJNCDATA, pTCPINFO->uiDataSize);

		dDataLen = pJNCSESS->usREQBuffLen + pTCPINFO->uiDataSize;
	}

	pJNCSESS->szREQBuffer[dDataLen] = 0x00;

	offset = 0;

	while(1) {
		dStatus = dParsingJNCHeader(dDataLen, pJNCSESS->szREQBuffer+offset,
				&dNextLen, &dCurLen);
		if(dStatus == 2) {
			
			pJNCSESS->szJNCLOG.ucStatus = JNC_STATUS_REQ_END;
			pJNCSESS->ucREQParaFlag = 0;

			dNextLen = 2;
			dDataLen -= dNextLen;

			if (dDataLen > 0)
			{
				log_print(LOGN_INFO, "[CLIENT] ONE REQ ENDED BUT DATA REMAINED LEN[%d]",dDataLen);
				offset+= dNextLen;
				pJNCSESS->usREQBuffLen = dDataLen;

				dRet = dCheckREQStart(pJNCSESS->szREQBuffer+offset,dDataLen);
				if(dRet>0)
				{
					log_print(LOGN_DEBUG,"ONE PACKET INCLUDE DUAL METHOD [0x%0ld][0x%0d]",
						pJNCSESS->szJNCLOG.ucMethod,dRet);
					STG_DiffTIME64(pJNCSESS->szJNCLOG.uiLastPktTime, pJNCSESS->szJNCLOG.uiLastPktMTime, 
							pJNCSESS->szJNCLOG.uiReqStartTime, pJNCSESS->szJNCLOG.uiReqStartMTime, 
							&pJNCSESS->szJNCLOG.llTransGapTime);
			
		            gettimeofday(&stNowTime, NULL);
		            pJNCSESS->szJNCLOG.uiOpEndTime = stNowTime.tv_sec;  
		            pJNCSESS->szJNCLOG.uiOpEndMTime= stNowTime.tv_usec;

					if(dMakeJNCLOGInfo(pMEMSINFO,pJNCSESS)<0)
						log_print(LOGN_CRI,LH"FAIL MAKE JNCLOG [%d]",LT,dRet);

					InitJNCSess(pJNCSESS);

					/* request time 설정 */
					pJNCSESS->szJNCLOG.ucStatus = JNC_STATUS_REQ_DOING;

					pJNCSESS->szJNCLOG.uiReqStartTime = pTCPINFO->uiCapTime;
					pJNCSESS->szJNCLOG.uiReqStartMTime = pTCPINFO->uiCapMTime;

					pJNCSESS->szJNCLOG.uiReqEndTime = pTCPINFO->uiCapTime;
					pJNCSESS->szJNCLOG.uiReqEndMTime = pTCPINFO->uiCapMTime;
					pJNCSESS->szJNCLOG.uiReqAckTime = pTCPINFO->uiAckTime;
					pJNCSESS->szJNCLOG.uiReqAckMTime = pTCPINFO->uiAckMTime;

		            gettimeofday(&stNowTime, NULL);
		            pJNCSESS->szJNCLOG.uiOpStartTime = stNowTime.tv_sec;  
		            pJNCSESS->szJNCLOG.uiOpStartMTime= stNowTime.tv_usec;
	
					/* 새로운 메소드 중심으로 파싱 */
			
					pJNCSESS->szJNCLOG.ucMethod = dRet;
					pJNCSESS->szJNCLOG.usTransID++;

					continue;
				}
			}
			pJNCSESS->usREQBuffLen = 0;
			break;
		}
		else if(dStatus ==1) {
			if(pJNCSESS->szJNCLOG.ucMethod!= CMD_RELAY && pJNCSESS->szJNCLOG.ucMethod!=0)
			{
				/*Header정보 구성이 완료되지 않는 상태에서 Parsing을 아직 할수 없는 경우
				 *REQ Buffer에 임시 저장 
				 */
				memcpy(&pJNCSESS->szREQBuffer[0], &pJNCSESS->szREQBuffer[offset], dDataLen);
				pJNCSESS->usREQBuffLen = dDataLen;
				pJNCSESS->szREQBuffer[pJNCSESS->usREQBuffLen] = 0x00;
			}

			break;
		}
		else {
			dParaLen = (dCurLen<31) ? dCurLen:31;
			strncpy((char*)szParameter, (char*)pJNCSESS->szREQBuffer+offset, dParaLen);
			szParameter[dParaLen] =0;

			if(pJNCSESS->szJNCLOG.ucMethod != CMD_RELAY && pJNCSESS->ucREQParaFlag!= 0)
				log_print(LOGN_INFO, "[CLIENT][PARAMETER]:%s", szParameter);

			switch (pJNCSESS->szJNCLOG.ucMethod)
			{
				case CMD_CONNREQ:
					dProc_CONNREQ(pJNCSESS, pJNCSESS->szREQBuffer+offset, dCurLen);
					break;

				case CMD_RELAY:
					dProc_RELAY(pJNCSESS, pJNCSESS->szREQBuffer+offset, dCurLen);
					break;

				case CMD_ACK:
					dProc_ACK(pJNCSESS, pJNCSESS->szREQBuffer+offset, dCurLen);
					break;

				case CMD_TERMREQ:
					dProc_ACK(pJNCSESS, pJNCSESS->szREQBuffer+offset, dCurLen);
					break;

				case CMD_DOWNBILL:
				case CMD_COUNTBILL:
					dProc_DOWNBIL(pJNCSESS, pJNCSESS->szREQBuffer+offset, dCurLen);
					break;

				default:
					log_print(LOGN_DEBUG, "[CLIENT] THIS METHOD IS UNKNOWN[%ld]",
						pJNCSESS->szJNCLOG.ucMethod);
			}
			offset +=dNextLen;
			dDataLen -= dNextLen;
			pJNCSESS->usREQBuffLen = dDataLen;
		}
	}/*while loof end */	
	if(pJNCSESS->usREQBuffLen >= (MAX_BUFF_LEN)) {
		log_print(LOGN_CRI, LH"REQ INVALID HEADER LEN INFO2 BUFLEN:%u", LT,pJNCSESS->usREQBuffLen);
		pJNCSESS->usREQBuffLen = 0;
	}

	return 0;
}

/**
 * JNCMessage 가 들어오면 처리하는 함수 파싱, 로그 전달등 
 **/



U32 dJNCMessage(stMEMSINFO *pMEMSINFO, stHASHOINFO *pJNCHASH, JNC_SESS_KEY *pJNCSESSKEY,JNC_SESS_DATA *pJNCSESS, TCP_INFO *pTCPINFO, U8 *pNode, U8 *pDATA)
{
	U8	*pJNCDATA;
	S32	dRet;
	U32	dRtxFlag; 

	dRtxFlag = pTCPINFO->ucRtx;

	pJNCDATA = &pDATA[pTCPINFO->uiSOffset];

	pJNCSESS->szJNCLOG.uiLastPktTime = pTCPINFO->uiCapTime;
	pJNCSESS->szJNCLOG.uiLastPktMTime = pTCPINFO->uiCapMTime;

	if(dRtxFlag == DEF_FROM_SERVER) {

		pJNCSESS->szJNCLOG.ucStatus = JNC_STATUS_RES_DOING;
			
		UpdateJNCSess(pTCPINFO, pJNCSESS);
		pJNCSESS->szJNCLOG.uiMNAckTime = pTCPINFO->uiAckTime;
		pJNCSESS->szJNCLOG.uiMNAckMTime = pTCPINFO->uiAckMTime;

		if(pJNCSESS->ucResStartFlag == 1) /* 1, start 0, ing*/
		{

			if(pJNCSESS->szJNCLOG.ucMethod == CMD_CONNREQ || 
				pJNCSESS->szJNCLOG.ucMethod == CMD_COUNTBILL)
			{
				dRet = dCheckRESCode(pJNCDATA, pJNCSESS,pTCPINFO->uiDataSize);
				if(dRet>0)
				{
					pJNCSESS->szJNCLOG.ucStatus =JNC_STATUS_RES_END;
				}
			
			}

			pJNCSESS->szJNCLOG.uiResStartTime = pTCPINFO->uiCapTime;
			pJNCSESS->szJNCLOG.uiResStartMTime = pTCPINFO->uiCapMTime;
			pJNCSESS->szJNCLOG.uiResEndTime = pTCPINFO->uiCapTime;
			pJNCSESS->szJNCLOG.uiResEndMTime = pTCPINFO->uiCapMTime;

			pJNCSESS->ucResStartFlag = 0;
		}
	}
	else if(dRtxFlag ==DEF_FROM_CLIENT) {
		
		dRet = dCheckREQStart(pJNCDATA, pTCPINFO->uiDataSize);
		if(dRet == 0) {
			log_print(LOGN_INFO, "[CLIENT]THIS MESSAGE IS NOT STARTED WITH METHOD");
			if(pJNCSESS->ucREQParaFlag==FLD_END || pJNCSESS->szJNCLOG.ucMethod == 0)
			{
				pJNCSESS->usREQBuffLen = 0;
//				log_print (LOGN_INFO, "[CLIENT] [REQ IP SZ]:[%d]", pJNCSESS->uiIPREQByte);
				return 0;
			}
			else {

				pJNCSESS->szJNCLOG.uiReqEndTime = pTCPINFO->uiCapTime;
				pJNCSESS->szJNCLOG.uiReqEndMTime = pTCPINFO->uiCapMTime;
				pJNCSESS->szJNCLOG.uiReqAckTime = pTCPINFO->uiAckTime;
				pJNCSESS->szJNCLOG.uiReqAckMTime = pTCPINFO->uiAckMTime;

				dSetJNCREQInfo(pMEMSINFO,pTCPINFO, pJNCDATA, pJNCSESS);
				UpdateJNCSess(pTCPINFO, pJNCSESS);
			}
		}
		else {
			/* 세션 생성후 첫 메소드를 제외한 나머지는 다음 메소드가 전달될경우 LOG를 보낸다.*/
			if(pJNCSESS->szJNCLOG.usTransID !=0)
			{
				if(pJNCSESS->ucResStartFlag == 0)
				{
					pJNCSESS->szJNCLOG.uiResEndTime = pTCPINFO->uiCapTime;
					pJNCSESS->szJNCLOG.uiResEndMTime = pTCPINFO->uiCapMTime;
					pJNCSESS->ucResStartFlag = 1;
				}
				STG_DiffTIME64(pJNCSESS->szJNCLOG.uiLastPktTime, pJNCSESS->szJNCLOG.uiLastPktMTime, 
							pJNCSESS->szJNCLOG.uiReqStartTime, pJNCSESS->szJNCLOG.uiReqStartMTime, 
							&pJNCSESS->szJNCLOG.llTransGapTime);
			
			    gettimeofday(&stNowTime, NULL);
			    pJNCSESS->szJNCLOG.uiOpEndTime = stNowTime.tv_sec;  
			    pJNCSESS->szJNCLOG.uiOpEndMTime= stNowTime.tv_usec;

				if(dMakeJNCLOGInfo(pMEMSINFO,pJNCSESS)<0)
					log_print(LOGN_CRI,LH"FAIL MAKE JNCLOG [%d]",LT, dRet);
				InitJNCSess(pJNCSESS);
	            gettimeofday(&stNowTime, NULL);
	            pJNCSESS->szJNCLOG.uiOpStartTime = stNowTime.tv_sec;  
	            pJNCSESS->szJNCLOG.uiOpStartMTime= stNowTime.tv_usec;


			}
			/* request time 설정 */
			pJNCSESS->szJNCLOG.ucStatus = JNC_STATUS_REQ_DOING;

			pJNCSESS->szJNCLOG.uiReqStartTime = pTCPINFO->uiCapTime;
			pJNCSESS->szJNCLOG.uiReqStartMTime = pTCPINFO->uiCapMTime;

			pJNCSESS->szJNCLOG.uiReqEndTime = pTCPINFO->uiCapTime;
			pJNCSESS->szJNCLOG.uiReqEndMTime = pTCPINFO->uiCapMTime;
			pJNCSESS->szJNCLOG.uiReqAckTime = pTCPINFO->uiAckTime;
			pJNCSESS->szJNCLOG.uiReqAckMTime = pTCPINFO->uiAckMTime;


			/* 새로운 메소드 중심으로 파싱 */
			pJNCSESS->szJNCLOG.ucMethod = dRet;
			pJNCSESS->szJNCLOG.usTransID++;

			dSetJNCREQInfo(pMEMSINFO,pTCPINFO, pJNCDATA, pJNCSESS);
			UpdateJNCSess(pTCPINFO, pJNCSESS);
		}
	}
	else {
		log_print(LOGN_CRI, "[%s][%s,%d] INVALID RTX FLAG[%d]", 
						__FILE__, __FUNCTION__,__LINE__,dRtxFlag);
		return -1;
	}
	return 0;
}

/**
 * 패킷의 방향성을 결정하는 함수
 */

char *PrintRtx(U8 ucRtxType)
{
	return ((ucRtxType ==DEF_FROM_SERVER)? "FORM_SERVER" :"FROM_CLIENT");
}

/**
 * 로그 생성및 초기화 하는 함수
 **/
U32 dMakeJNCLOGInfo(stMEMSINFO *pMEMSINFO,JNC_SESS_DATA *pJNCSESS)
{
	
	U8 	*pLOGNODE;
	LOG_JNC_TRANS	*pLOGJNC;
	S32		dSeqProcID;

	if((pLOGNODE = nifo_node_alloc(pMEMSINFO)) ==NULL) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
		return -1;
	}
	if((pLOGJNC = (LOG_JNC_TRANS *)nifo_tlv_alloc(pMEMSINFO, pLOGNODE, LOG_JNC_TRANS_DEF_NUM, LOG_JNC_TRANS_SIZE, DEF_MEMSET_ON))== NULL) {
		log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc NULL", __FUNCTION__, __LINE__);
	  //nifo_node_delete(pMEMSINFO, pLOGNODE);
		return -2;
	}
	
	memcpy(pLOGJNC, &pJNCSESS->szJNCLOG, LOG_JNC_TRANS_SIZE);
	pLOGJNC->usUserErrorCode =  dCheckUserErrorCode(pLOGJNC);
	pLOGJNC->usL7FailCode = pLOGJNC->usUserErrorCode;

	PrintLOGINFO_IMP(pLOGJNC);
	
	PrintLOGINFO(pLOGJNC);

	dSeqProcID = SEQ_PROC_A_CALL + ( pLOGJNC->uiClientIP % gACALLCnt );
	if(dSend_JNC_Data(pMEMSINFO, dSeqProcID , pLOGNODE)<0)
	{
		log_print(LOGN_CRI, "[%s,%d] Send Fail!!",__FUNCTION__,__LINE__);
		return -3;
	}	

return 0;
}
/**
 * 패킷 데이터가 들어왔을때 기본 정보를 업데이트 하는 함수
 **/

void UpdateJNCSess(TCP_INFO *pTCPINFO, JNC_SESS_DATA *pJNCSESS)
{

	pJNCSESS->szJNCLOG.usL4FailCode = pTCPINFO->usL4FailCode;
    pJNCSESS->szJNCLOG.ucTcpClientStatus = pTCPINFO->ucTcpClientStatus;
    pJNCSESS->szJNCLOG.ucTcpServerStatus = pTCPINFO->ucTcpServerStatus;

    pJNCSESS->szJNCLOG.uiIPDataUpPktCnt += pTCPINFO->uiIPDataUpPktCnt;
    pJNCSESS->szJNCLOG.uiIPDataDnPktCnt += pTCPINFO->uiIPDataDnPktCnt;
    pJNCSESS->szJNCLOG.uiIPTotUpPktCnt += pTCPINFO->uiIPTotUpPktCnt;
    pJNCSESS->szJNCLOG.uiIPTotDnPktCnt += pTCPINFO->uiIPTotDnPktCnt;
    pJNCSESS->szJNCLOG.uiIPDataUpRetransCnt += pTCPINFO->uiIPDataUpRetransCnt;
    pJNCSESS->szJNCLOG.uiIPDataDnRetransCnt += pTCPINFO->uiIPDataDnRetransCnt;
    pJNCSESS->szJNCLOG.uiIPTotUpRetransCnt += pTCPINFO->uiIPTotUpRetransCnt;
    pJNCSESS->szJNCLOG.uiIPTotDnRetransCnt += pTCPINFO->uiIPTotDnRetransCnt;
    pJNCSESS->szJNCLOG.uiIPDataUpPktSize += pTCPINFO->uiIPDataUpPktSize;
    pJNCSESS->szJNCLOG.uiIPDataDnPktSize += pTCPINFO->uiIPDataDnPktSize;
    pJNCSESS->szJNCLOG.uiIPTotUpPktSize += pTCPINFO->uiIPTotUpPktSize;
	pJNCSESS->szJNCLOG.uiIPTotDnPktSize += pTCPINFO->uiIPTotDnPktSize;
	pJNCSESS->szJNCLOG.uiIPDataUpRetransSize += pTCPINFO->uiIPDataUpRetransSize;
	pJNCSESS->szJNCLOG.uiIPDataDnRetransSize += pTCPINFO->uiIPDataDnRetransSize;
	pJNCSESS->szJNCLOG.uiIPTotUpRetransSize += pTCPINFO->uiIPTotUpRetransSize;
	pJNCSESS->szJNCLOG.uiIPTotDnRetransSize += pTCPINFO->uiIPTotDnRetransSize;

	if(pTCPINFO->ucRtx == DEF_FROM_SERVER)
		pJNCSESS->szJNCLOG.uiTcpDnBodySize += pTCPINFO->uiDataSize;
	else
		pJNCSESS->szJNCLOG.uiTcpUpBodySize += pTCPINFO->uiDataSize;

}

void InitJNCSess(JNC_SESS_DATA *pJNCSESS)
{
	pJNCSESS->szJNCLOG.usL4FailCode = 0;
	pJNCSESS->szJNCLOG.usL7FailCode = 0;
	pJNCSESS->szJNCLOG.usUserErrorCode = 0;
	pJNCSESS->szJNCLOG.ucStatus = 0;
    pJNCSESS->szJNCLOG.ucTcpClientStatus = 0;
    pJNCSESS->szJNCLOG.ucTcpServerStatus = 0;
	pJNCSESS->szJNCLOG.usResCode = 0;

    pJNCSESS->szJNCLOG.uiIPDataUpPktCnt = 0;
    pJNCSESS->szJNCLOG.uiIPDataDnPktCnt = 0;
    pJNCSESS->szJNCLOG.uiIPTotUpPktCnt = 0;
    pJNCSESS->szJNCLOG.uiIPTotDnPktCnt = 0;
    pJNCSESS->szJNCLOG.uiIPDataUpRetransCnt = 0;
    pJNCSESS->szJNCLOG.uiIPDataDnRetransCnt = 0;
    pJNCSESS->szJNCLOG.uiIPTotUpRetransCnt = 0;
    pJNCSESS->szJNCLOG.uiIPTotDnRetransCnt = 0;
    pJNCSESS->szJNCLOG.uiIPDataUpPktSize = 0;
    pJNCSESS->szJNCLOG.uiIPDataDnPktSize = 0;
    pJNCSESS->szJNCLOG.uiIPTotUpPktSize = 0;
	pJNCSESS->szJNCLOG.uiIPTotDnPktSize = 0;
	pJNCSESS->szJNCLOG.uiIPDataUpRetransSize = 0;
	pJNCSESS->szJNCLOG.uiIPDataDnRetransSize = 0;
	pJNCSESS->szJNCLOG.uiIPTotUpRetransSize = 0;
	pJNCSESS->szJNCLOG.uiIPTotDnRetransSize = 0;

	pJNCSESS->szJNCLOG.uiTcpUpBodySize = 0;
	pJNCSESS->szJNCLOG.uiTcpDnBodySize = 0;

	pJNCSESS->szJNCLOG.uiOpStartTime = 0;
	pJNCSESS->szJNCLOG.uiOpStartMTime = 0;
	pJNCSESS->szJNCLOG.uiOpEndTime = 0;
	pJNCSESS->szJNCLOG.uiOpEndMTime = 0;

	pJNCSESS->szJNCLOG.ucMethod = 0;
}



#if 0
S32 dGetPlatformTypeOld(S32 l4Code)
{
	S32     platformType = 0;
    switch(l4Code)
    {
	    case L4_WAP20:
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
																																		    default:																														        log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN L4CODE[%d]", __FILE__, __FUNCTION__, __LINE__, l4Code);
			platformType = 0;
	        break;
																																	    }
	return platformType;
}
#endif


void PrintLOGINFO_IMP(LOG_JNC_TRANS *pJNCLOG)
{
	U8              szSIP[INET_ADDRSTRLEN];
	U8              szDIP[INET_ADDRSTRLEN];
    log_print(LOGN_DEBUG, "MAKE LOGINFO SIP[%s] SPORT[%u] DIP[%s] DPORT[%u] MTD[0x%0ld],TID[%u]",
	            util_cvtipaddr(szSIP, pJNCLOG->uiClientIP), pJNCLOG->usClientPort,
	            util_cvtipaddr(szDIP, pJNCLOG->uiServerIP), pJNCLOG->usServerPort,
				pJNCLOG->ucMethod,pJNCLOG->usTransID);
}


void PrintLOGINFO(LOG_JNC_TRANS *pJNCLOG)
{
	int dLog = LOGN_INFO;
	U8              szSIP[INET_ADDRSTRLEN];
	U8              szDIP[INET_ADDRSTRLEN];
	log_print(dLog,"****************** JNC LOGINFO ******************");
	log_print(dLog," ");
    log_print(dLog, "SIP[%s] SPORT[%u] DIP[%s] DPORT[%u]",
			util_cvtipaddr(szSIP, pJNCLOG->uiClientIP), pJNCLOG->usClientPort, 
			util_cvtipaddr(szDIP, pJNCLOG->uiServerIP), pJNCLOG->usServerPort);

	log_print(dLog,"CallTime---------[%u][%u]",pJNCLOG->uiCallTime,pJNCLOG->uiCallMTime);
	log_print(dLog,"TcpSynTime-------[%u][%u]",pJNCLOG->uiTcpSynTime,pJNCLOG->uiTcpSynMTime);
	log_print(dLog,"NASNAME----------[%u]",pJNCLOG->uiNASName);
	log_print(dLog,"BrowserInfo------[%s]",pJNCLOG->szBrowserInfo);
	log_print(dLog,"Model------------[%s]",pJNCLOG->szModel);
	log_print(dLog,"NetOption--------[%s]",pJNCLOG->szNetOption);
	log_print(dLog,"TransID----------[%u]",pJNCLOG->usTransID);
	log_print(dLog,"PlatFormType-----[%ld]",pJNCLOG->usPlatformType);
	log_print(dLog,"SvcL4Type--------[%ld]",pJNCLOG->usSvcL4Type);
	log_print(dLog,"SvcL7Type--------[%ld]",pJNCLOG->usSvcL7Type);
	log_print(dLog,"SubSysNo---------[%d]",pJNCLOG->ucSubSysNo);
	log_print(dLog,"Method-----------[0x%0ld]",pJNCLOG->ucMethod);
	log_print(dLog,"ResCode----------[%d]",pJNCLOG->usResCode);
	log_print(dLog,"CONTENTCODE------[%s]",pJNCLOG->szContentCode);
	log_print(dLog,"CPCODE-----------[%s]",pJNCLOG->szCpCode);
	log_print(dLog,"MIN--------------[%s]",pJNCLOG->szMIN);
	log_print(dLog," ");

	log_print(dLog,"*************** JNC LOGINFO[TIME] ***************");
	log_print(dLog,"ReqStartTime-----[%u][%u]",pJNCLOG->uiReqStartTime,pJNCLOG->uiReqStartMTime);
	log_print(dLog,"ReqEndTime-------[%u][%u]",pJNCLOG->uiReqEndTime,pJNCLOG->uiReqEndMTime);
	log_print(dLog,"ReqAckTime-------[%u][%u]",pJNCLOG->uiReqAckTime,pJNCLOG->uiReqAckMTime);
	log_print(dLog,"ResStartTime-----[%u][%u]",pJNCLOG->uiResStartTime,pJNCLOG->uiResStartMTime);
	log_print(dLog,"ResEndTime-------[%u][%u]",pJNCLOG->uiResEndTime,pJNCLOG->uiResEndMTime);
	log_print(dLog,"MNAckTime--------[%u][%u]",pJNCLOG->uiMNAckTime,pJNCLOG->uiMNAckMTime);
	log_print(dLog,"LastPktTime------[%u][%u]",pJNCLOG->uiLastPktTime,pJNCLOG->uiLastPktMTime);
	log_print(dLog,"OpStartTime------[%u][%u]",pJNCLOG->uiOpStartTime,pJNCLOG->uiOpStartMTime);
	log_print(dLog,"OpEndTime--------[%u][%u]",pJNCLOG->uiOpEndTime,pJNCLOG->uiOpEndMTime);
	log_print(dLog,"TransGapTime-----[%lld]",pJNCLOG->llTransGapTime);
	log_print(dLog," ");

	log_print(dLog,"************** JNC LOGINFO[STATUS] **************");
	log_print(dLog,"STATUS--------SRV[%ld] CLI[%ld]",pJNCLOG->ucTcpServerStatus,pJNCLOG->ucTcpClientStatus);
	log_print(dLog,"FAILCODE------ L4[%ld] L7[%d]",pJNCLOG->usL4FailCode,pJNCLOG->usL7FailCode);
	log_print(dLog," ");

	log_print(dLog,"*********** JNC LOGINFO[COUNT & SIZE] ***********");
	log_print(dLog,"IPDataPktCnt-------UP[%u] DOWN[%u]",pJNCLOG->uiIPDataUpPktCnt,pJNCLOG->uiIPDataDnPktCnt);
	log_print(dLog,"IPTotPktCnt--------UP[%u] DOWN[%u]",pJNCLOG->uiIPTotUpPktCnt,pJNCLOG->uiIPTotDnPktCnt);
	log_print(dLog,"IPDataRestransCnt--UP[%u] DOWN[%u]",pJNCLOG->uiIPDataUpRetransCnt, pJNCLOG->uiIPDataDnRetransCnt);
	log_print(dLog,"IPTotRetransCnt----UP[%u] DOWN[%u]",pJNCLOG->uiIPTotUpRetransCnt, pJNCLOG->uiIPTotDnRetransCnt);
	log_print(dLog,"IPDataPktSize------UP[%u] DOWN[%u]",pJNCLOG->uiIPDataUpPktSize,pJNCLOG->uiIPDataDnPktSize);
	log_print(dLog,"IPTotPktSize-------UP[%u] DOWN[%u]",pJNCLOG->uiIPTotUpPktSize,pJNCLOG->uiIPTotDnPktSize);
	log_print(dLog,"IPDataRetransSize--UP[%u] DOWN[%u]",pJNCLOG->uiIPDataUpRetransSize,pJNCLOG->uiIPDataDnRetransSize);
	log_print(dLog,"IPTotRetransSize---UP[%u] DOWN[%u]",pJNCLOG->uiIPTotUpRetransSize,pJNCLOG->uiIPTotDnRetransSize);
	log_print(dLog,"TcpBodySize--------UP[%u] DOWN[%u]",pJNCLOG->uiTcpUpBodySize,pJNCLOG->uiTcpDnBodySize);
	log_print(dLog," ");
	log_print(dLog,"*************************************************");
	log_print(dLog," ");
}

S32 dCheckUserErrorCode(LOG_JNC_TRANS *pJNCLOG )
{
	if(pJNCLOG->ucStatus == JNC_STATUS_REQ_DOING){
		return JNC_ERR_NOREQEND;
	}
	if(pJNCLOG->ucMethod == CMD_CONNREQ || pJNCLOG->ucMethod == CMD_DOWNBILL) 
	{
		if(pJNCLOG->ucStatus == JNC_STATUS_REQ_END){
			return JNC_ERR_NORESSTART;
		}
		else if(pJNCLOG->ucStatus == JNC_STATUS_RES_DOING){
			return JNC_ERR_NORESEND;
		}
	}
	if(pJNCLOG->ucMethod == CMD_RELAY && pJNCLOG->ucStatus ==JNC_STATUS_REQ_END)
			return JNC_ERR_NORESSTART;
	if(pJNCLOG->ucMethod == CMD_CONNREQ ||pJNCLOG->ucMethod == CMD_RELAY ||pJNCLOG->ucMethod == CMD_DOWNBILL){
		if(pJNCLOG->uiMNAckTime == 0 && pJNCLOG->uiMNAckMTime ==0)
		{
				return JNC_ERR_NOMNACK;
		}
	}
	return 0;
}




