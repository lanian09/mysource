/**		@file	inet_func.c
 * 		- Internet 사용량을 계산 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: inet_func.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:47 $
 * 		@ref		inet_func.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- Internet 사용량을 계산 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>

// TOP
#include "commdef.h"			/* Capture_Header_Msg */
#include "filter.h"				/* st_Flt_Info */
#include "common_stg.h"
#include "capdef.h"

// LIB headers
#include "loglib.h"
#include "utillib.h"
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */

// .
#include "inet_func.h"

/**
 *	Declare variables
 */
extern st_Flt_Info		*flt_info;

/**
 *	Declare extern func.
 */
extern void invoke_del(void *p);

/**
 *	Implement func.
 */

/** main function.
 *
 *  man Function
 *
 *  @param  argc    :   파라미터 개수
 *  @param  *argv[] :   파라미터
 *
 *  @return         S32
 *  @see            inet_main.c
 *
 *  @exception      Nothing
 *  @note           Nothing
 **/
S32 dProcINETCallStart(stMEMSINFO *pMEMSINFO, stTIMERNINFO *pTIMER, stHASHOINFO *pCALLHASH, stHASHOINFO *pINETHASH, CALL_KEY *pKEY)
{
	INET_CALL_KEY		INETCALLKEY;
	INET_CALL_KEY		*pCALLKEY = &INETCALLKEY;
	INET_CALL_DATA		*pCALLDATA = NULL;
	stHASHONODE			*pHASHNODE;
	U8					szIP[INET_ADDRSTRLEN];
	U32					uiSessStartTime = pKEY->uiReserved;

	log_print(LOGN_DEBUG, "#** START IP=%s TIME=%u", util_cvtipaddr(szIP, pKEY->uiSrcIP), uiSessStartTime);

	pCALLKEY->uiIP = pKEY->uiSrcIP;
	pCALLKEY->uiReserved = 0;

	if((pHASHNODE = hasho_find(pCALLHASH, (U8 *)pCALLKEY)) == NULL)
	{
		pCALLDATA = pCreateCall(pCALLHASH, pCALLKEY, uiSessStartTime);
	}
	else
	{
		pCALLDATA = (INET_CALL_DATA *)nifo_ptr(pCALLHASH, pHASHNODE->offset_Data);
		log_print(LOGN_CRI, "#** START EXIST IP=%s TIME=%u", util_cvtipaddr(szIP, pCALLKEY->uiIP), uiSessStartTime);
		dDelCall(pMEMSINFO, pTIMER, pCALLHASH, pINETHASH, pCALLKEY, pCALLDATA);
		pCALLDATA = pCreateCall(pCALLHASH, pCALLKEY, uiSessStartTime);
	}

	log_print(LOGN_DEBUG, "#** START ADD IP=%s TIME=%u", util_cvtipaddr(szIP, pCALLKEY->uiIP), uiSessStartTime);

	return 0;
}

/** main function.
 *
 *  man Function
 *
 *  @param  argc    :   파라미터 개수
 *  @param  *argv[] :   파라미터
 *
 *  @return         S32
 *  @see            inet_main.c
 *
 *  @exception      Nothing
 *  @note           Nothing
 **/
S32 dProcINETCallStop(stMEMSINFO *pMEMSINFO, stTIMERNINFO *pTIMER, stHASHOINFO *pCALLHASH, stHASHOINFO *pINETHASH, CALL_KEY *pKEY)
{
	INET_CALL_KEY		INETCALLKEY;
	INET_CALL_KEY		*pCALLKEY = &INETCALLKEY;
	INET_CALL_DATA		*pCALLDATA;
	stHASHONODE			*pHASHNODE;
	U8					szIP[INET_ADDRSTRLEN];
	U32					uiLastLogTime = pKEY->uiReserved;

	log_print(LOGN_DEBUG, "**# STOP IP=%s TIME=%u", util_cvtipaddr(szIP, pKEY->uiSrcIP), uiLastLogTime);

	pCALLKEY->uiIP = pKEY->uiSrcIP;
	pCALLKEY->uiReserved = 0;

	if((pHASHNODE = hasho_find(pCALLHASH, (U8 *)pCALLKEY)) == NULL)
	{
		log_print(LOGN_WARN, "**# STOP NOT EXIST IP=%s TIME=%u", util_cvtipaddr(szIP, pCALLKEY->uiIP), uiLastLogTime);
	}
	else
	{
		pCALLDATA = (INET_CALL_DATA *)nifo_ptr(pCALLHASH, pHASHNODE->offset_Data);
		if((uiLastLogTime == 0) || (pCALLDATA->uiSessStartTime < uiLastLogTime))
		{
			log_print(LOGN_DEBUG, "**# STOP DEL IP=%s TIME=%u", util_cvtipaddr(szIP, pCALLKEY->uiIP), uiLastLogTime);
			dDelCall(pMEMSINFO, pTIMER, pCALLHASH, pINETHASH, pCALLKEY, pCALLDATA);
		}
	}

	return 0;
}

/** main function.
 *
 *  man Function
 *
 *  @param  argc    :   파라미터 개수
 *  @param  *argv[] :   파라미터
 *
 *  @return         S32
 *  @see            inet_main.c
 *
 *  @exception      Nothing
 *  @note           Nothing
 **/
S32 dProcINETData(stMEMSINFO *pMEMSINFO, stTIMERNINFO *pTIMER, stHASHOINFO *pCALLHASH, stHASHOINFO *pINETHASH, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH)
{
	INET_CALL_KEY		CALLKEY;
	INET_CALL_KEY		*pCALLKEY = &CALLKEY;
	INET_CALL_DATA		*pCALLDATA = NULL;
	INET_KEY			INETKEY;
	INET_KEY			*pINETKEY = &INETKEY;
	INET_KEY			*pSUBDATA;
	INET_DATA			INETDATA;
	INET_DATA			*pINETDATA = NULL;
	LOG_INET			*pLOG = NULL;
	stHASHONODE			*pHASHNODE;
	U8					szSIP[INET_ADDRSTRLEN];
	U8					szCIP[INET_ADDRSTRLEN];
	TIMER_COMMON		TIMERCOMMON;
	U8					*pNODE, *pLOGNODE;
	struct timeval		stNowTime;
	S32					isCreateCall = 0;

	gettimeofday(&stNowTime, NULL);

	MakeCALLHashKey(pCAPHEAD, pINFOETH, pCALLKEY);
	MakeINETHashKey(pCAPHEAD, pINFOETH, pINETKEY);

	log_print(LOGN_DEBUG, "*#* DATA CIP=%s SIP=%s SPORT=%d",
			util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);

	if((pHASHNODE = hasho_find(pCALLHASH, (U8 *)pCALLKEY)) == NULL)
	{
		if((pCALLDATA = pCreateCall(pCALLHASH, pCALLKEY, pCAPHEAD->curtime)) == NULL)
		{
			log_print(LOGN_CRI, "*#* DATA pCreateCall NULL CIP=%s SIP=%s SPORT=%d", 
					util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);
			return -1;	
		}
		log_print(LOGN_DEBUG, "*#* DATA CREATE CALL IP=%s TIME=%u", util_cvtipaddr(szCIP, pCALLKEY->uiIP), pCAPHEAD->curtime);
		isCreateCall = 1;
	}
	else
	{
		pCALLDATA = (INET_CALL_DATA *)nifo_ptr(pCALLHASH, pHASHNODE->offset_Data);
	}

	if((pHASHNODE = hasho_find(pINETHASH, (U8 *)pINETKEY)) == NULL)
	{
		pINETDATA = &INETDATA;
		if((pHASHNODE = hasho_add(pINETHASH, (U8 *)pINETKEY, (U8 *)pINETDATA)) == NULL)
		{
			log_print(LOGN_CRI, "*#* DATA hasho_add NULL CIP=%s SIP=%s SPORT=%d", 
				util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);

			if(isCreateCall) hasho_del(pCALLHASH, (U8 *)pCALLKEY);
			return -2;
		}

		log_print(LOGN_DEBUG, "*#* DATA ADD INET CIP=%s SIP=%s SPORT=%d", 
			util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);

		pINETDATA = (INET_DATA *)nifo_ptr(pINETHASH, pHASHNODE->offset_Data);

		if((pLOGNODE = nifo_node_alloc(pMEMSINFO)) == NULL)
		{
			log_print(LOGN_CRI, "*#* DATA nifo_node_alloc NULL CIP=%s SIP=%s SPORT=%d", 
				util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);

			if(isCreateCall) hasho_del(pCALLHASH, (U8 *)pCALLKEY);
			hasho_del(pINETHASH, (U8 *)pINETKEY);
			return -3;
		}

		if((pLOG = (LOG_INET *)nifo_tlv_alloc(pMEMSINFO, pLOGNODE, LOG_INET_DEF_NUM, LOG_INET_SIZE, DEF_MEMSET_ON)) == NULL)
		{
			log_print(LOGN_CRI, "*#* DATA nifo_tlv_alloc NULL CIP=%s SIP=%s SPORT=%d", 
				util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);

			if(isCreateCall) hasho_del(pCALLHASH, (U8 *)pCALLKEY);
			nifo_node_delete(pMEMSINFO, pLOGNODE);
			hasho_del(pINETHASH, (U8 *)pINETKEY);
			return -4;

		}

		pLOG->uiClientIP = pINETKEY->uiCIP;
		pLOG->uiServerIP = pINETKEY->uiSIP;
		pLOG->usServerPort = pINETKEY->usSPort;
		pLOG->uiFirstPktTime = pCAPHEAD->curtime;
		pLOG->uiFirstPktMTime = pCAPHEAD->ucurtime;
	
		pLOG->uiCallTime = pCAPHEAD->curtime;
		pLOG->uiCallMTime = pCAPHEAD->ucurtime;

		pLOG->uiOpStartTime = stNowTime.tv_sec;
		pLOG->uiOpStartMTime = stNowTime.tv_usec;

		pINETDATA->offset_Log = nifo_offset(pMEMSINFO, pLOG);
		memcpy(&TIMERCOMMON.INETKEY, pINETKEY, INET_KEY_SIZE);
		pINETDATA->timerNID = timerN_add(pTIMER, invoke_del, (U8 *)&TIMERCOMMON, sizeof(TIMER_COMMON), time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_INET_TIMEOUT]);

		if((pNODE = nifo_node_alloc(pMEMSINFO)) == NULL)
		{
			log_print(LOGN_CRI, "*#* DATA nifo_node_alloc NULL CIP=%s SIP=%s SPORT=%d", 
				util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);
				
			if(isCreateCall) hasho_del(pCALLHASH, (U8 *)pCALLKEY);
			nifo_node_delete(pMEMSINFO, pLOGNODE);
			hasho_del(pINETHASH, (U8 *)pINETKEY);
			timerN_del(pTIMER, pINETDATA->timerNID);
			return -5;
		}

		if((pSUBDATA = (INET_KEY *)nifo_tlv_alloc(pMEMSINFO, pNODE, INET_KEY_DEF_NUM, INET_KEY_SIZE, DEF_MEMSET_OFF)) == NULL)
		{
			log_print(LOGN_CRI, "*#* DATA nifo_tlv_alloc NULL CIP=%s SIP=%s SPORT=%d", 
				util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);
				
			nifo_node_delete(pMEMSINFO, pNODE);
			if(isCreateCall) hasho_del(pCALLHASH, (U8 *)pCALLKEY);
			nifo_node_delete(pMEMSINFO, pLOGNODE);
			hasho_del(pINETHASH, (U8 *)pINETKEY);
			timerN_del(pTIMER, pINETDATA->timerNID);
			return -6;
		}

		memcpy(pSUBDATA, pINETKEY, INET_KEY_SIZE);
		if(pCALLDATA->offset_Data == 0)
		{
			pCALLDATA->offset_Data = nifo_offset(pMEMSINFO, pNODE);
		}
		else
		{
			nifo_node_link_nont_prev(pMEMSINFO, nifo_ptr(pMEMSINFO, pCALLDATA->offset_Data), pNODE);
		}	
	}
	else
	{
		pINETDATA = (INET_DATA *)nifo_ptr(pINETHASH, pHASHNODE->offset_Data);
		pINETDATA->timerNID = timerN_update(pTIMER, pINETDATA->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_INET_TIMEOUT]);
		pLOG = (LOG_INET *)nifo_ptr(pMEMSINFO, pINETDATA->offset_Log);
	}

	pLOG->uiLastPktTime = pCAPHEAD->curtime;
	pLOG->uiLastPktMTime = pCAPHEAD->ucurtime;
	pLOG->uiOpEndTime = pCAPHEAD->curtime;
	pLOG->uiOpEndMTime = pCAPHEAD->ucurtime;

	if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
		pLOG->uiUpPacketSize += pINFOETH->stIP.wTotalLength;
		pLOG->uiUpPacketCnt++;
	} else {
		pLOG->uiDnPacketSize += pINFOETH->stIP.wTotalLength;
		pLOG->uiDnPacketCnt++;
	}

	return 0;
}

/** main function.
 *
 *  man Function
 *
 *  @param  argc    :   파라미터 개수
 *  @param  *argv[] :   파라미터
 *
 *  @return         S32
 *  @see            inet_main.c
 *
 *  @exception      Nothing
 *  @note           Nothing
 **/
INET_CALL_DATA *pCreateCall(stHASHOINFO *pCALLHASH, INET_CALL_KEY *pCALLKEY, U32 uiSessStartTime)
{
	INET_CALL_DATA			INETCALLDATA;
	INET_CALL_DATA			*pCALLDATA = &INETCALLDATA;
	stHASHONODE				*pHASHNODE;
	U8						szIP[INET_ADDRSTRLEN];

	pCALLDATA->uiSessStartTime = uiSessStartTime;
	pCALLDATA->offset_Data = 0;

	if((pHASHNODE = hasho_add(pCALLHASH, (U8 *)pCALLKEY, (U8 *)pCALLDATA)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d hasho_add NULL IP=%s", 
				__FILE__, __FUNCTION__, __LINE__, util_cvtipaddr(szIP, pCALLKEY->uiIP));
		pCALLDATA = NULL;
	}
	else
	{
		pCALLDATA = (INET_CALL_DATA *)nifo_ptr(pCALLHASH, pHASHNODE->offset_Data);
	}

	return pCALLDATA;
}

/** main function.
 *
 *  man Function
 *
 *  @param  argc    :   파라미터 개수
 *  @param  *argv[] :   파라미터
 *
 *  @return         S32
 *  @see            inet_main.c
 *
 *  @exception      Nothing
 *  @note           Nothing
 **/
S32 dDelCall(stMEMSINFO *pMEMSINFO, stTIMERNINFO *pTIMER, stHASHOINFO *pCALLHASH, stHASHOINFO *pINETHASH, INET_CALL_KEY *pCALLKEY, INET_CALL_DATA *pCALLDATA)
{
	OFFSET				offset;
	INET_KEY			*pINETKEY;
	INET_DATA			*pINETDATA;
	LOG_INET			*pLOG;
	stHASHONODE			*pHASHNODE;
	U8					*pNODE, *pNEXT;
	U8					szCIP[INET_ADDRSTRLEN];
	U8					szSIP[INET_ADDRSTRLEN];
	struct timeval		stTime;

	gettimeofday(&stTime, NULL);

	log_print(LOGN_DEBUG, "--- DEL CALL IP=%s", util_cvtipaddr(szCIP, pCALLKEY->uiIP));

	pNODE = nifo_ptr(pMEMSINFO, pCALLDATA->offset_Data);
	while(pNODE != NULL)
	{
		pNEXT = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNODE)->nont.offset_next), NIFO, nont);

		offset = nifo_offset(pMEMSINFO, pNODE);
		pINETKEY = (INET_KEY *)nifo_get_value(pMEMSINFO, INET_KEY_DEF_NUM, offset);

		if((pHASHNODE = hasho_find(pINETHASH, (U8 *)pINETKEY)) == NULL)
		{
			log_print(LOGN_CRI, "--- DEL CALL NOT EXIST SESS CIP=%s SIP=%s SPORT=%d",
					util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);
		}
		else
		{
			pINETDATA = (INET_DATA *)nifo_ptr(pINETHASH, pHASHNODE->offset_Data);
			pLOG = (LOG_INET *)nifo_ptr(pMEMSINFO, pINETDATA->offset_Log);

			log_print(LOGN_DEBUG, "--- DEL CALL EXIST SESS CIP=%s SIP=%s SPORT=%d",
					util_cvtipaddr(szCIP, pINETKEY->uiCIP), util_cvtipaddr(szSIP, pINETKEY->uiSIP), pINETKEY->usSPort);

			pLOG->uiOpEndTime = stTime.tv_sec;
			pLOG->uiOpEndMTime = stTime.tv_usec;

			dSendINETLog(pMEMSINFO, pLOG, dGetCALLProcID(pLOG->uiClientIP));

			hasho_del(pINETHASH, (U8 *)pINETKEY);
			timerN_del(pTIMER, pINETDATA->timerNID);
		}

		nifo_node_unlink_nont(pMEMSINFO, pNODE);
		nifo_node_delete(pMEMSINFO, pNODE);

		if(pNODE == pNEXT) pNEXT = NULL;
		pNODE = pNEXT;
		
	}

	hasho_del(pCALLHASH, (U8 *)pCALLKEY);

	return 0;
}

/*
 * 	$Log: inet_func.c,v $
 * 	Revision 1.3  2011/09/07 06:30:47  hhbaek
 * 	*** empty log message ***
 * 	
 * 	Revision 1.2  2011/09/04 12:16:51  hhbaek
 * 	*** empty log message ***
 * 	
 * 	Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * 	NEW OAM SYSTEM
 * 	
 * 	Revision 1.3  2011/08/17 13:00:01  hhbaek
 * 	A_INET
 * 	
 * 	Revision 1.2  2011/08/10 09:57:43  uamyd
 * 	modified and block added
 * 	
 * 	Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * 	init DQMS2
 * 	
 * 	Revision 1.11  2011/05/10 18:14:40  dark264sh
 * 	A_INET: log_print Level 변경
 * 	
 * 	Revision 1.10  2011/05/09 09:34:20  dark264sh
 * 	A_INET: A_CALL multi 처리
 * 	
 * 	Revision 1.9  2011/04/20 10:03:16  dark264sh
 * 	A_INET: Call Stop 전송시 SessStartTime 비교 추가
 * 	
 * 	Revision 1.8  2011/04/19 05:32:05  dark264sh
 * 	A_INET: if 변경
 * 	
 * 	Revision 1.7  2011/04/17 10:52:04  dark264sh
 * 	*** empty log message ***
 * 	
 * 	Revision 1.6  2011/04/17 10:42:54  dark264sh
 * 	*** empty log message ***
 * 	
 * 	Revision 1.5  2011/04/17 09:15:37  dark264sh
 * 	A_INET: timer 등록 시점 변경
 * 	
 * 	Revision 1.4  2011/04/16 12:09:12  dark264sh
 * 	A_INET: CallTime 세팅
 * 	
 * 	Revision 1.3  2011/04/13 15:37:55  dark264sh
 * 	A_INET: dProcINETData 처리
 * 	
 * 	Revision 1.2  2011/04/13 14:15:36  dark264sh
 * 	A_INET: dSendINETLog 처리
 * 	
 * 	Revision 1.1  2011/04/13 13:14:38  dark264sh
 * 	A_INET 추가
 * 	
 */
