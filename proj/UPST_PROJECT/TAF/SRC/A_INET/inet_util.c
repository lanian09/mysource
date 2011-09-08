/**		@file	inet_func.c
 * 		- Internet 사용량을 계산 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: inet_util.c,v 1.3 2011/09/07 06:30:47 hhbaek Exp $
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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// DQMS headers
#include "commdef.h"			/* Capture_Header_Msg */
#include "procid.h"
#include "common_stg.h"

// LIB 
#include "loglib.h"
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */

// .
#include "inet_util.h"

/**
 * Declare variables
 */
extern S32			dTCPQID[MAX_MP_NUM];
extern S32			gATCPCnt;
extern S32			dCALLQID[MAX_SMP_NUM];
extern S32			gACALLCnt;

/**
 *	Implement func.
 */
U8 *CVT_TIME(time_t t, U8 *szDest)
{
	struct tm tTime;

	localtime_r(&t, &tTime);

	sprintf(szDest, "%04d%02d%02d%02d%02d%02d",
		tTime.tm_year + 1900, tTime.tm_mon + 1, tTime.tm_mday, tTime.tm_hour, tTime.tm_min, tTime.tm_sec);

	return szDest;
}

void MakeCALLHashKey(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, INET_CALL_KEY *pCALLKEY)
{
	pCALLKEY->uiReserved = 0;
	if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
	{
		pCALLKEY->uiIP = pINFOETH->stIP.dwSrcIP;
	}
	else
	{
		pCALLKEY->uiIP = pINFOETH->stIP.dwDestIP;
	}
}

void MakeINETHashKey(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, INET_KEY *pINETKEY)
{
	pINETKEY->usReserved = 0;
	pINETKEY->uiReserved = 0;
	if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
	{
		pINETKEY->uiCIP = pINFOETH->stIP.dwSrcIP;
		pINETKEY->uiSIP = pINFOETH->stIP.dwDestIP;
		pINETKEY->usSPort = pINFOETH->stUDPTCP.wDestPort;
	}
	else
	{
		pINETKEY->uiCIP = pINFOETH->stIP.dwDestIP;
		pINETKEY->uiSIP = pINFOETH->stIP.dwSrcIP;
		pINETKEY->usSPort = pINFOETH->stUDPTCP.wSrcPort;
	}
}

S32 dGetProcID(U32 uiClientIP, U8 ucProtocol)
{
	S32		idx, qid = 0;
	switch(ucProtocol)
	{
		case DEF_PROTOCOL_TCP:
			return SEQ_PROC_A_ITCP + ( uiClientIP % gATCPCnt );
	}
	return 0;
}

S32 dGetCALLProcID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

/*
 * 	$Log: inet_util.c,v $
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
 * 	Revision 1.4  2011/05/09 09:36:42  dark264sh
 * 	A_INET: A_CALL multi 처리
 * 	
 * 	Revision 1.3  2011/04/16 09:47:31  dark264sh
 * 	A_INET: TCP Protocol인 경우 A_ITCP로 전송
 * 	
 * 	Revision 1.2  2011/04/13 14:15:36  dark264sh
 * 	A_INET: dSendINETLog 처리
 * 	
 * 	Revision 1.1  2011/04/13 13:14:39  dark264sh
 * 	A_INET 추가
 * 	
 */
