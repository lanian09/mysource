/**		@file	o_svcmon_log.c
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_get.c,v 1.3 2011/09/07 04:32:00 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 04:32:00 $
 * 		@ref		o_svcmon_init.c o_svcmon_maic.c
 *
 * 		@section	Intro(소개)
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"

// PROJECT
#include "common_stg.h"
#include "func_time_check.h"
#include "svcmon.h"

// TAM
#include "watch_mon.h"
#include "watch_filter.h"

#include "o_svcmon_util.h"
#include "o_svcmon_get.h"
#include "o_svcmon_set.h"

extern st_FuncTimeCheckList *pFUNC;

st_FirstMon *getFirstMon(stHASHOINFO *pHash, S32 office, S32 systype, S32 subtype, st_FirstMon *aFirstMon)
{
START_FUNC_TIME_CHECK(pFUNC, 80);
	st_FirstMon		*pFirstMon = NULL;

	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	*pData;
	stHASHONODE			*pHASHNODE;

	/* setting hash key */
	pKey->ucOffice = office;
	pKey->ucSysType = systype;
	pKey->usSubType = subtype;
	pKey->uiIP = 0;
	pKey->ucSYSID = 0;
	pKey->ucBSCID = 0;
	pKey->usBTSID = 0;
	pKey->uiL4SvcType = 0;
	pKey->uiArrayType = ARRAY_TYPE_FIRST;
	pKey->uiReserved = 0;

	if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL) 
	{
		pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);
		pFirstMon = &aFirstMon[pData->uiArrayIndex];
	}
	else
	{
//		log_print(LOGN_CRI, "F=%s:%s.%d hasho_find NULL OFFCIE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u",
		log_print(LOGN_WARN, "F=%s:%s.%d hasho_find NULL OFFCIE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u",
				__FILE__, __FUNCTION__, __LINE__, 
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice,
				PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType);	
	}
END_FUNC_TIME_CHECK(pFUNC, 80);

	return pFirstMon;
}

st_MonCore *getMonCore(stHASHOINFO *pHash, st_WatchMsg *pWatchMsg, S32 systype, st_MonCore *aMonCore, st_MonCore *aMonCore1Min)
{
START_FUNC_TIME_CHECK(pFUNC, 81);
	U32			ip;
	S32			office;
	st_MonCore	*pMonCore = NULL;
	S8			szIP[INET_ADDRSTRLEN];

	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	*pData;
	stHASHONODE			*pHASHNODE;


	switch(systype)
	{
	case SYSTEM_TYPE_PCF:		ip = pWatchMsg->uiPCFIP;	office = pWatchMsg->ucOffice;		break;
	case SYSTEM_TYPE_PDSN:		ip = pWatchMsg->uiPDSNIP;	office = 0;							break;
	case SYSTEM_TYPE_AAA:		ip = pWatchMsg->uiAAAIP;	office = 0;							break;
	case SYSTEM_TYPE_ROAMAAA:	ip = pWatchMsg->uiAAAIP;	office = 0;							break;
	case SYSTEM_TYPE_HSS:		ip = pWatchMsg->uiHSSIP;	office = 0;							break;
	case SYSTEM_TYPE_LNS:		ip = pWatchMsg->uiLNSIP;	office = 0;							break;
	default: 
		log_print(LOGN_CRI, "F=%s:%s.%d UNKNWON SYSTYPE=%u", __FILE__, __FUNCTION__, __LINE__, systype);
END_FUNC_TIME_CHECK(pFUNC, 81);
		return NULL;
	}

	/* setting hash key */
	pKey->ucOffice = office;
	pKey->ucSysType = systype;
	pKey->usSubType = 0;
	pKey->uiIP = ip;
	pKey->ucSYSID = 0;
	pKey->ucBSCID = 0;
	pKey->usBTSID = 0;
	pKey->uiL4SvcType = 0;
	pKey->uiArrayType = ARRAY_TYPE_CORE;
	pKey->uiReserved = 0;

	if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL) 
	{
		pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);

		if( pData->ui1MinMonFlag == 1 && aMonCore1Min != NULL )
			pMonCore = &aMonCore1Min[pData->uiArrayIndex];
		else
			pMonCore = &aMonCore[pData->uiArrayIndex];
	}
	else
	{
//		log_print(LOGN_CRI, "F=%s:%s.%d hasho_find NULL OFFCIE=%s:%u SYSTYPE=%s:%u IP=%s:%u",
		log_print(LOGN_WARN, "F=%s:%s.%d hasho_find NULL OFFCIE=%s:%u SYSTYPE=%s:%u IP=%s:%u",
				__FILE__, __FUNCTION__, __LINE__, 
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice,
				PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP);	
	}
END_FUNC_TIME_CHECK(pFUNC, 81);

	return pMonCore;
}

st_MonBTS *getMonBTS(stHASHOINFO *pHash, st_WatchMsg *pWatchMsg, st_MonBTS *aMonBTS)
{
START_FUNC_TIME_CHECK(pFUNC, 82);
	st_MonBTS		*pMonBTS = NULL;

	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	*pData;
	stHASHONODE			*pHASHNODE;

	/* setting hash key */
	pKey->ucOffice = pWatchMsg->ucOffice;
	pKey->ucSysType = SYSTEM_TYPE_BTS;
	pKey->usSubType = 0;
	pKey->uiIP = 0;
	pKey->ucSYSID = pWatchMsg->ucSYSID;
	pKey->ucBSCID = pWatchMsg->ucBSCID;
	pKey->usBTSID = pWatchMsg->usBTSID;
	pKey->uiL4SvcType = 0;
	pKey->uiArrayType = ARRAY_TYPE_BTS;
	pKey->uiReserved = 0;

	if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL) 
	{
		pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);
		pMonBTS = &aMonBTS[pData->uiArrayIndex];
	}
	else
	{
//		log_print(LOGN_CRI, "F=%s:%s.%d hasho_find NULL OFFCIE=%s:%u BSCID=%u BTSID=%u",
		log_print(LOGN_WARN, "F=%s:%s.%d hasho_find NULL OFFCIE=%s:%u BSCID=%u BTSID=%u",
				__FILE__, __FUNCTION__, __LINE__, 
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice, pKey->ucBSCID, pKey->usBTSID);	
	}
END_FUNC_TIME_CHECK(pFUNC, 82);

	return pMonBTS;
}

st_MonBSC *getMonBSC(stHASHOINFO *pHash, st_WatchMsg *pWatchMsg, st_MonBSC *aMonBSC)
{
START_FUNC_TIME_CHECK(pFUNC, 83);
	st_MonBSC		*pMonBSC = NULL;
	
	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	*pData;
	stHASHONODE			*pHASHNODE;

	/* setting hash key */
	pKey->ucOffice = pWatchMsg->ucOffice;
	pKey->ucSysType = SYSTEM_TYPE_BSC;
	pKey->usSubType = 0;
	pKey->uiIP = 0;
	pKey->ucSYSID = pWatchMsg->ucSYSID;
	pKey->ucBSCID = pWatchMsg->ucBSCID;
	pKey->usBTSID = 0;
	pKey->uiL4SvcType = 0;
	pKey->uiArrayType = ARRAY_TYPE_BSC;
	pKey->uiReserved = 0;

	if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL) 
	{
		pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);
		pMonBSC = &aMonBSC[pData->uiArrayIndex];
	}
	else
	{
		log_print(LOGN_WARN, "F=%s:%s.%d hasho_find NULL OFFCIE=%s:%u BSCID=%u",
				__FILE__, __FUNCTION__, __LINE__, 
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice, pKey->ucBSCID);	
	}
END_FUNC_TIME_CHECK(pFUNC, 83);

	return pMonBSC;
}

st_MonSvc *getMonSvc(stHASHOINFO *pHash, st_WatchMsg *pWatchMsg, st_MonSvc *aMonSvc)
{
START_FUNC_TIME_CHECK(pFUNC, 84);
	st_MonSvc		*pMonSvc = NULL;
	S8				szIP[INET_ADDRSTRLEN];

	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	*pData;
	stHASHONODE			*pHASHNODE;

	/* setting hash key */
	pKey->ucOffice = 0;
	pKey->ucSysType = SYSTEM_TYPE_SERVICE;
	pKey->usSubType = pWatchMsg->ucSvcIdx;
	pKey->uiIP = pWatchMsg->uiSVCIP;
	pKey->ucSYSID = 0;
	pKey->ucBSCID = 0;
	pKey->usBTSID = 0;
	pKey->uiL4SvcType = pWatchMsg->usSvcL4Type;
	pKey->uiArrayType = ARRAY_TYPE_SVC;
	pKey->uiReserved = 0;

	if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL) 
	{
		pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);
		pMonSvc = &aMonSvc[pData->uiArrayIndex];
	}
	else
	{
//		log_print(LOGN_CRI, "F=%s:%s.%d hasho_find NULL SUBTYPE=%s:%u IP=%s:%u L4SVC=%s:%d",
		log_print(LOGN_WARN, "F=%s:%s.%d hasho_find NULL SUBTYPE=%s:%u IP=%s:%u L4SVC=%s:%d",
				__FILE__, __FUNCTION__, __LINE__, 
				PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType,
				util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP, PrintSVC(pKey->uiL4SvcType), pKey->uiL4SvcType);	
	}
END_FUNC_TIME_CHECK(pFUNC, 84);

	return pMonSvc;
}

st_MonFA *getMonFA(st_WatchMsg *pWatchMsg, st_MonBTS *pMonBTS)
{
START_FUNC_TIME_CHECK(pFUNC, 85);
	int				i;
	st_MonFA		*pMonFA;

	for(i = 0; i < MAX_MON_FA_CNT; i++)
	{
		pMonFA = &pMonBTS->stMonFA[i];
		if(pMonFA->ucFA == pWatchMsg->ucFA) {
END_FUNC_TIME_CHECK(pFUNC, 85);
			return pMonFA;
		}
	}

//	log_print(LOGN_CRI, "F=%s:%s.%d MonFA NULL FA=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->ucFA);
	log_print(LOGN_WARN, "F=%s:%s.%d MonFA NULL FA=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->ucFA);

END_FUNC_TIME_CHECK(pFUNC, 85);
	return NULL;
}

st_MonSec *getMonSec(st_WatchMsg *pWatchMsg, st_MonFA *pMonFA)
{
START_FUNC_TIME_CHECK(pFUNC, 86);
	int				i;
	st_MonSec		*pMonSec;

	for(i = 0; i < MAX_MON_SEC_CNT; i++)
	{
		pMonSec = &pMonFA->stMonSec[i];
		if(pMonSec->ucSec == pWatchMsg->ucSec) {
END_FUNC_TIME_CHECK(pFUNC, 86);
			return pMonSec;
		}
	}

//	log_print(LOGN_CRI, "F=%s:%s.%d MonSec NULL SEC=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->ucSec);
	log_print(LOGN_WARN, "F=%s:%s.%d MonSec NULL SEC=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->ucSec);

END_FUNC_TIME_CHECK(pFUNC, 86);
	return NULL;
}

st_MonSvc *getMonRoam(stHASHOINFO *pHash, stHASHOINFO *pNasIPHash, st_WatchMsg *pWatchMsg, st_MonSvc *aMonSvc)
{
START_FUNC_TIME_CHECK(pFUNC, 87);
	st_MonSvc		*pMonSvc = NULL;
	S8				szIP[INET_ADDRSTRLEN];

	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	*pData;
	stHASHONODE			*pHASHNODE;

	/* setting hash key */
	pKey->ucOffice = 0;
	pKey->ucSysType = SYSTEM_TYPE_SERVICE;
	pKey->usSubType = SVC_IDX_ROAM;
//	pKey->uiIP = pWatchMsg->uiPDSNIP;
	pKey->uiIP = getNasIP(pNasIPHash, pWatchMsg->uiPDSNIP);
	pKey->ucSYSID = 0;
	pKey->ucBSCID = 0;
	pKey->usBTSID = 0;
	pKey->uiL4SvcType = 0;
	pKey->uiArrayType = ARRAY_TYPE_SVC;
	pKey->uiReserved = 0;

	if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL) 
	{
		pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);
		pMonSvc = &aMonSvc[pData->uiArrayIndex];
	}
	else
	{
//		log_print(LOGN_CRI, "F=%s:%s.%d hasho_find NULL SUBTYPE=%s:%u IP=%s:%u L4SVC=%s:%d",
		log_print(LOGN_WARN, "F=%s:%s.%d hasho_find NULL SUBTYPE=%s:%u IP=%s:%u L4SVC=%s:%d",
				__FILE__, __FUNCTION__, __LINE__, 
				PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType,
				util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP, PrintSVC(pKey->uiL4SvcType), pKey->uiL4SvcType);	
	}
END_FUNC_TIME_CHECK(pFUNC, 87);

	return pMonSvc;
}

/*
 * $Log: o_svcmon_get.c,v $
 * Revision 1.3  2011/09/07 04:32:00  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/31 16:08:07  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 08:58:48  dcham
 * *** empty log message ***
 *
 * Revision 1.18  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.2  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.17  2010/03/29 12:23:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.16  2010/03/04 06:00:53  dark264sh
 * ROAM NASIP NetMask 처리
 *
 * Revision 1.15  2010/02/25 11:13:45  dark264sh
 * O_SVCMON ROAM 처리
 *
 * Revision 1.14  2010/02/25 07:33:07  dark264sh
 * BSD => LNS로 변경
 *
 * Revision 1.13  2010/02/24 12:19:46  dark264sh
 * 망감시 헤더 변경에 따른 변경
 *
 * Revision 1.12  2009/08/24 00:14:20  pkg
 * *** empty log message ***
 *
 * Revision 1.11  2009/08/22 13:43:55  pkg
 * *** empty log message ***
 *
 * Revision 1.10  2009/08/22 12:04:02  pkg
 * O_SVCMON BTSID=0인 경우 BSC, BTS 중복되는 버그 수정
 *
 * Revision 1.9  2009/07/20 02:44:20  dark264sh
 * O_SVCMON SYSID 추가
 *
 * Revision 1.8  2009/07/10 12:35:44  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2009/07/10 06:32:23  dark264sh
 * 망감시 관련 서비스 구분 변경
 *
 * Revision 1.6  2009/07/07 15:44:56  dark264sh
 * O_SVCMON log_print 변경
 *
 * Revision 1.5  2009/07/01 17:29:54  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/21 13:34:33  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/20 10:51:39  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/18 17:01:48  dark264sh
 * O_SVCMON Filter 처리
 *
 * Revision 1.1  2009/06/15 08:06:04  dark264sh
 * O_SVCMON 기본 동작 처리
 *
 */
