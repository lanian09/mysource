/**		@file	o_svcmon_log.c
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_conf.c,v 1.3 2011/09/07 04:32:00 hhbaek Exp $
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
#include "hasho.h"
#include "utillib.h"

// PROJECT
#include "common_stg.h"
#include "func_time_check.h"
#include "svcmon.h"
#include "path.h"

// TAM
#include "watch_mon.h"
#include "watch_filter.h"

//#include "o_svcmon_func.h"
#include "o_svcmon_print.h"
#include "o_svcmon_util.h"
#include "o_svcmon_get.h"
#include "o_svcmon_set.h"
#include "o_svcmon_conf.h"

extern st_FuncTimeCheckList *pFUNC;
extern S32					dSysNo;


U8			gFA[MAX_MON_FA_CNT] = { 0 };
U8			gSEC[MAX_MON_SEC_CNT] = { 0, 1, 2 };

st_MonList *pGetNextMonList(st_MonTotal *pMON, st_MonList *pBaseMonList)
{
START_FUNC_TIME_CHECK(pFUNC, 30);
	S32				nextidx, nextcnt;
	time_t			curtime, stattime;
	st_MonList		*pMonList;

	nextidx = pMON->dCurIdx + 1;
	nextcnt = pMON->dUsedCnt + 1;
	
	if(nextidx >= TOTAL_MONLIST_CNT) {
		nextidx = 0;
	}
	if(nextcnt > USED_MONLIST_CNT) {
		nextcnt = USED_MONLIST_CNT;
	}
	
	pMON->dCurIdx = nextidx;
	pMON->dUsedCnt = nextcnt;

	pMonList = &pMON->stMonList[pMON->dCurIdx];
	memcpy(pMonList, pBaseMonList, DEF_MONLIST_SIZE);

	curtime = time(NULL);
	stattime = curtime / DEF_MON_PERIOD * DEF_MON_PERIOD;

	pMonList->lTime = stattime;
	pMonList->stFirstMonList.lTime = stattime;

END_FUNC_TIME_CHECK(pFUNC, 30);

	return pMonList;
}

st_MonList_1Min *pGetNextMon1MinList(st_MonTotal_1Min *pMON, st_MonList_1Min *pBaseMonList)
{
START_FUNC_TIME_CHECK(pFUNC, 30);
	S32             nextidx, nextcnt;
	time_t          curtime, stattime;
	st_MonList_1Min *pMonList;

	nextidx = pMON->dCurIdx + 1;
	nextcnt = pMON->dUsedCnt + 1;

	if(nextidx >= TOTAL_MONLIST_1MIN_CNT) {
		nextidx = 0;
	}
	if(nextcnt > USED_MONLIST_1MIN_CNT) {
		nextcnt = USED_MONLIST_1MIN_CNT;
	}

	pMON->dCurIdx = nextidx;
	pMON->dUsedCnt = nextcnt;

	pMonList = &pMON->stMonList1Min[pMON->dCurIdx];
	memcpy(pMonList, pBaseMonList, DEF_MONLIST_1MIN_SIZE);

	curtime = time(NULL);
	stattime = curtime / DEF_MON_PERIOD_1MIN * DEF_MON_PERIOD_1MIN;

	pMonList->lTime = stattime;
	pMonList->stFirstMonList.lTime = stattime;

END_FUNC_TIME_CHECK(pFUNC, 30);

	return pMonList;
}

S32 dMakeBaseMonList(stHASHOINFO *pHash, stHASHOINFO *pNasIPHash, st_WatchFilter *pWatchFilter, st_MonList *pMonList, st_MonList_1Min *pMonList1Min)
{
START_FUNC_TIME_CHECK(pFUNC, 40);
	int					i, j, k, svctype;
	int					inetSvcCnt = 0;
	U32					office, systype, ip;
	U16					*pCnt;
	S8					szIP[INET_ADDRSTRLEN];

	st_MonSvc			*pMonSvc;
	st_WatchService		*pWSVC;

	st_MonCore			*pMonPCF;
	st_WatchPCF			*pWPCF;
    
    //st_WatchPDIF		*pWPDIF;

	st_SubBSC			*pSubBSC;
	st_MonBSC			*pMonBSC;
	st_WatchBSC			*pWBSC;

	st_MonSec			*pMonSec;
	st_MonFA			*pMonFA;
	st_SubBTS			*pSubBTS;
	st_MonBTS			*pMonBTS;
	st_WatchBTS			*pWBTS;

	st_MonCore			*pMonEquip;
	st_FirstMonList		*pFirstMonList;
	st_WatchEquip		*pWEquip;
	st_RoamEquip		*pREquip;
	
	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	stSvcMonHashData;
	st_SvcMonHash_Data	*pData = &stSvcMonHashData;
	stHASHONODE			*pHASHNODE;

	/* reset base st_MonList */
	memset(pMonList, 0x00, sizeof(st_MonList));
	memset(pMonList1Min, 0x00, sizeof(st_MonList_1Min));

	/* reset hash */
	hasho_reset(pHash);

	/* reset hash */
	hasho_reset(pNasIPHash);

	/* SVC */
	log_print(LOGN_CRI, "### WF SVC CNT=%d", pWatchFilter->stWatchServiceList.dCount);
	for(i = 0; i < pWatchFilter->stWatchServiceList.dCount; i++)
	{
		if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
			log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
			break;
		}

		pWSVC = &pWatchFilter->stWatchServiceList.stWatchService[i];

		/* L4_FTP, L4_DNS는 망감시에서 제외 */
		if(pWSVC->dSvcL4Type == L4_FTP || pWSVC->dSvcL4Type == L4_DNS) {
			continue;
		}
	
		if((svctype = getSvcIndex(pWSVC->dSvcL4Type, pWSVC->dSvcL7Type)) < 0) {
			log_print(LOGN_CRI, "F=%s:%s.%d INVALID SVCTYPE=%d SVCL4TYPE=%d SVCL7TYPE=%d", 
					__FILE__, __FUNCTION__, __LINE__, svctype, pWSVC->dSvcL4Type, pWSVC->dSvcL7Type);
			continue;
		}		

		/* setting hash key */
		pKey->ucOffice = 0;
		pKey->ucSysType = SYSTEM_TYPE_SERVICE;
		pKey->usSubType = svctype;
		pKey->uiIP = pWSVC->uiIP;
		pKey->ucSYSID = 0;
		pKey->ucBSCID = 0;
		pKey->usBTSID = 0;

#if 1 /* INYOUNG */
		switch(pWSVC->dSvcL4Type){
			case L4_INET_TCP_USER:
				pKey->uiL4SvcType = L4_INET_TCP;
				break;

			case L4_INET_HTTP_USER:
				pKey->uiL4SvcType = L4_INET_HTTP;
				break;

			default:
				pKey->uiL4SvcType = pWSVC->dSvcL4Type;
				break;
		}
#endif

        pKey->uiReserved = pWSVC->uiReserved;
		pKey->uiArrayType = ARRAY_TYPE_SVC;

		/* setting hash data */
		pData->uiArrayIndex = pMonList->usSvcCnt;
		//pData->uiReserved = 0;
		pData->ui1MinMonFlag = 0;


		/* add hash */
		if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
			pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
			pMonSvc->ucSvcType = pKey->usSubType;
			pMonSvc->SvcL4Type = pKey->uiL4SvcType;
			pMonSvc->uiIPAddr = pKey->uiIP;

#if 1 /* INYOUNG */
			/*
			 * USERGROUP에 대해서 Reserved에 Setting
			 */
			if( (pWSVC->dSvcL4Type == L4_INET_TCP_USER) || (pWSVC->dSvcL4Type == L4_INET_HTTP_USER))
				pMonSvc->ucReserved = 1;
			else
				pMonSvc->ucReserved = 0;
#endif

			log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

			/* FIRSTMON */
			dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType, pData->ui1MinMonFlag);

			pMonList->usSvcCnt++;

#if 1 /* INYOUNG */
			/*
			 * 인터넷 서비스에 대한 착신 처리
			 */
			switch(pWSVC->dSvcL4Type){

				case L4_INET_TCP:
				case L4_INET_HTTP:
				case L4_INET_TCP_USER:
				case L4_INET_HTTP_USER:

					if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT){
						log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
								__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
						break;
					}

					/* 인터넷 서비스 리스트를 구성할 때, 하나의 IP에 대해서 발신, 착신으로 두 번 구성한다. */
					inetSvcCnt = inetSvcCnt + 2;

					/* Change Key */
					pKey->usSubType = SVC_IDX_RECVCALL;

					if(pWSVC->dSvcL4Type == L4_INET_TCP_USER ||  pWSVC->dSvcL4Type == L4_INET_TCP)
						pKey->uiL4SvcType = L4_INET_TCP_RECV;
					else 
						pKey->uiL4SvcType = L4_INET_HTTP_RECV;

					/* Change Data */
					pData->uiArrayIndex = pMonList->usSvcCnt;

					if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL){
						pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
						pMonSvc->ucSvcType = pKey->usSubType;
						pMonSvc->SvcL4Type = pKey->uiL4SvcType;
						pMonSvc->uiIPAddr = pKey->uiIP;

						if( (pWSVC->dSvcL4Type == L4_INET_TCP_USER) || (pWSVC->dSvcL4Type == L4_INET_HTTP_USER))
							pMonSvc->ucReserved = 1;
						else
							pMonSvc->ucReserved = 0;

						/* FIRSTMON */
						dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE
											, pMonSvc->ucSvcType, pData->ui1MinMonFlag);

						log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
							i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
							PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
							pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
							pData->uiArrayIndex);

						pMonList->usSvcCnt++;

					} else {
						log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
							i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
							PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
							pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
							pData->uiArrayIndex);
					}

					break;

				/* IM 착신 */
				case L4_SIP_MS:
				case L4_SIP_VENDOR:
				case L4_SIP_CSCF:
				case L4_MSRP_MS:
				case L4_MSRP_VENDOR:
				case L4_XCAP:
			
					if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT){
						log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
								__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
						break;
					}

					/* Change Key */
					pKey->usSubType = SVC_IDX_IM_RECV;

					/* Change Data */
					pData->uiArrayIndex = pMonList->usSvcCnt;

					if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL){
						pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
						pMonSvc->ucSvcType = pKey->usSubType;
						pMonSvc->SvcL4Type = pKey->uiL4SvcType;
						pMonSvc->uiIPAddr = pKey->uiIP;

						/* FIRSTMON */
						dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE
											, pMonSvc->ucSvcType, pData->ui1MinMonFlag);

						log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
							i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
							PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
							pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
							pData->uiArrayIndex);

						pMonList->usSvcCnt++;

					} else {
						log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
							i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
							PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
							pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
							pData->uiArrayIndex);
					}

					break;
			}
#endif	/* INYOUNG */

			/* VOD_STREAM 예외 처리 */
			if(pKey->uiL4SvcType == L4_VOD_STREAM) {
				/* ADD L4_RTS_FB */
				if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
					break;
				}
				pKey->uiL4SvcType = L4_RTS_FB;
				pData->uiArrayIndex = pMonList->usSvcCnt;
				if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
					pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
					pMonSvc->ucSvcType = pKey->usSubType;
					pMonSvc->SvcL4Type = pKey->uiL4SvcType;
					pMonSvc->uiIPAddr = pKey->uiIP;

					log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
						pData->uiArrayIndex);

					pMonList->usSvcCnt++;

				} else {
					log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
						pData->uiArrayIndex);
				}

				/* ADD L4_RTS_WB */
				if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
					break;
				}
				pKey->uiL4SvcType = L4_RTS_WB;
				pData->uiArrayIndex = pMonList->usSvcCnt;
				if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
					pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
					pMonSvc->ucSvcType = pKey->usSubType;
					pMonSvc->SvcL4Type = pKey->uiL4SvcType;
					pMonSvc->uiIPAddr = pKey->uiIP;

					log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
						pData->uiArrayIndex);

					pMonList->usSvcCnt++;

				} else {
					log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
						pData->uiArrayIndex);
				}
			}

			/* IM/VT 예외 처리 */
			else if(pKey->uiL4SvcType == L4_SIP_CSCF) {
				/* ADD L4_IM | SVC_IDX_REGI */
				if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
					break;
				}
				pKey->usSubType = SVC_IDX_REGI;
				pData->uiArrayIndex = pMonList->usSvcCnt;
				if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
					pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
					pMonSvc->ucSvcType = pKey->usSubType;
					pMonSvc->SvcL4Type = pKey->uiL4SvcType;
					pMonSvc->uiIPAddr = pKey->uiIP;

					dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType, pData->ui1MinMonFlag);

					log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
						pData->uiArrayIndex);

					pMonList->usSvcCnt++;

				} else {
					log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
						pData->uiArrayIndex);
				}

				/* ADD L4_VT */
				if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
					break;
				}
				pKey->uiL4SvcType = L4_VT;
				pKey->usSubType = SVC_IDX_VT;
				pData->uiArrayIndex = pMonList->usSvcCnt;
				if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
					pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
					pMonSvc->ucSvcType = pKey->usSubType;
					pMonSvc->SvcL4Type = pKey->uiL4SvcType;
					pMonSvc->uiIPAddr = pKey->uiIP;

					dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType, pData->ui1MinMonFlag);

					log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
						pData->uiArrayIndex);

					pMonList->usSvcCnt++;

				} else {
					log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
						pData->uiArrayIndex);
				}

				/* ADD L4_VT | SVC_IDX_REGI */
				if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
					break;
				}
				pKey->usSubType = SVC_IDX_REGI;
				pData->uiArrayIndex = pMonList->usSvcCnt;
				if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
					pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
					pMonSvc->ucSvcType = pKey->usSubType;
					pMonSvc->SvcL4Type = pKey->uiL4SvcType;
					pMonSvc->uiIPAddr = pKey->uiIP;

					dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType, pData->ui1MinMonFlag);

					log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
						pData->uiArrayIndex);

					pMonList->usSvcCnt++;
				} else {
					log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
						pData->uiArrayIndex);
				}

#if 1 /* INYOUNG */
				/* ADD VT 착신 */
				if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
					break;
				}

                pKey->usSubType = SVC_IDX_VT_RECV;
                pData->uiArrayIndex = pMonList->usSvcCnt;
                if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
                    pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
                    pMonSvc->ucSvcType = pKey->usSubType;
                    pMonSvc->SvcL4Type = pKey->uiL4SvcType;
                    pMonSvc->uiIPAddr = pKey->uiIP;
                        
                    dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType, pData->ui1MinMonFlag);
					log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
						pData->uiArrayIndex);

                    pMonList->usSvcCnt++;
                } else {
					log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
						pData->uiArrayIndex);
				}
#endif

			}
			/* WIPI 예외 처리 */
			else if(pKey->uiL4SvcType == L4_WIPI) {
				/* ADD SVC_IDX_DN */
				if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
					break;
				}
				pKey->usSubType = SVC_IDX_DN;
				pData->uiArrayIndex = pMonList->usSvcCnt;
				if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
					pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
					pMonSvc->ucSvcType = pKey->usSubType;
					pMonSvc->SvcL4Type = pKey->uiL4SvcType;
					pMonSvc->uiIPAddr = pKey->uiIP;

					dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType, pData->ui1MinMonFlag);
					log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
						pData->uiArrayIndex);

					pMonList->usSvcCnt++;

				} else {
					log_print(LOGN_CRI, "HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
						i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
						PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), 
						pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, 
						pData->uiArrayIndex);
				}
			}
		}
		else {
			log_print(LOGN_CRI, 
				"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}	

	/* PHONE 서비스 예외 처리 */
	if(pMonList->usSvcCnt < MAX_MON_SVC_CNT-1) {

		/* setting hash key */
		pKey->ucOffice = 0;
		pKey->ucSysType = SYSTEM_TYPE_SERVICE;
		pKey->usSubType = SVC_IDX_PHONE;
		pKey->uiIP = 0;
		pKey->ucSYSID = 0;
		pKey->ucBSCID = 0;
		pKey->usBTSID = 0;
		pKey->uiL4SvcType = L4_PHONE;
		pKey->uiArrayType = ARRAY_TYPE_SVC;
		pKey->uiReserved = 0;

		/* setting hash data */
		pData->uiArrayIndex = pMonList->usSvcCnt;
		//pData->uiReserved = 0;
		pData->ui1MinMonFlag = 0;

		/* add hash */
		if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
			pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
			pMonSvc->ucSvcType = pKey->usSubType;
			pMonSvc->SvcL4Type = pKey->uiL4SvcType;
			pMonSvc->uiIPAddr = pKey->uiIP;

			/* FIRSTMON */
			dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType, pData->ui1MinMonFlag);

			log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
				pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
				pData->uiArrayIndex);

			pMonList->usSvcCnt++;
		}
		else {
			log_print(LOGN_CRI, 
				"HASH NULL OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}

		pKey->uiL4SvcType = L4_PHONE_ETC;
		pData->uiArrayIndex = pMonList->usSvcCnt;

		if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
			pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
			pMonSvc->ucSvcType = pKey->usSubType;
			pMonSvc->SvcL4Type = pKey->uiL4SvcType;
			pMonSvc->uiIPAddr = pKey->uiIP;

			log_print(LOGN_CRI,"I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP),
				pKey->uiIP, pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType,
				pData->uiArrayIndex);

			pMonList->usSvcCnt++;

		} else {
			log_print(LOGN_CRI, 
				"HASH NULL OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}
	else {
		log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW SVC MAX=%d FILTER=%d", 
				__FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT, pWatchFilter->stWatchServiceList.dCount);
	}


	/* PCF */
	log_print(LOGN_CRI, "### WF PCF CNT=%d", pWatchFilter->stWatchPCFList.dCount);
	for(i = 0; i < pWatchFilter->stWatchPCFList.dCount; i++)
	{
		if(pMonList->usPCFCnt >= MAX_MON_PCF_CNT) {
			log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW PCF MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_PCF_CNT, pWatchFilter->stWatchPCFList.dCount);
			break;
		}

		pWPCF = &pWatchFilter->stWatchPCFList.stWatchPCF[i];

		/* setting hash key */
		pKey->ucOffice = pWPCF->ucOffice;
		pKey->ucSysType = SYSTEM_TYPE_PCF;
		pKey->usSubType = 0;
		pKey->uiIP = pWPCF->uiIP;
		pKey->ucSYSID = 0;
		pKey->ucBSCID = 0;
		pKey->usBTSID = 0;
		pKey->uiL4SvcType = 0;
		pKey->uiArrayType = ARRAY_TYPE_CORE;
		pKey->uiReserved = 0;

		/* setting hash data */
		pData->uiArrayIndex = pMonList->usPCFCnt;
		//pData->uiReserved = 0;
		pData->ui1MinMonFlag = 0;

		/* add hash */
		if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
			pMonPCF = &pMonList->stMonPCF[pMonList->usPCFCnt];
			pMonPCF->ucOffice = pKey->ucOffice;
			pMonPCF->ucSysType = pKey->ucSysType;
			pMonPCF->uiIPAddr = pKey->uiIP;

			/* FIRSTMON */
			dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pMonPCF->ucOffice, SYSTEM_TYPE_PCF, 0, pData->ui1MinMonFlag);

			log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

			pMonList->usPCFCnt++;
		}
		else {
			log_print(LOGN_CRI, 
				"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}	
    
	/* BSC */
	log_print(LOGN_CRI, "### WF BSC CNT=%d", pWatchFilter->stWatchBSCList.dCount);
	for(i = 0; i < pWatchFilter->stWatchBSCList.dCount; i++)
	{
#if 0
		if(pMonList->usBSCCnt >= MAX_MON_BSC_CNT) {
			log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW BSC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_BSC_CNT, pWatchFilter->stWatchBSCList.dCount);
			break;
		}
#endif
		if(pMonList1Min->usBSCCnt >= MAX_MON_BSC_CNT) {
			log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW BSC MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_BSC_CNT, pWatchFilter->stWatchBSCList.dCount);
			break;
		}

		pWBSC = &pWatchFilter->stWatchBSCList.stWatchBSC[i];

		/* setting hash key */
		pKey->ucOffice = pWBSC->ucOffice;
		pKey->ucSysType = SYSTEM_TYPE_BSC;
		pKey->usSubType = 0;
		pKey->uiIP = 0;
		pKey->ucSYSID = pWBSC->ucSYSID;
		pKey->ucBSCID = pWBSC->ucBSCID;
		pKey->usBTSID = 0;
		pKey->uiL4SvcType = 0;
		pKey->uiArrayType = ARRAY_TYPE_BSC;
		pKey->uiReserved = 0;

		/* setting hash data */
		//pData->uiArrayIndex = pMonList->usBSCCnt;
		pData->uiArrayIndex = pMonList1Min->usBSCCnt;
		//pData->uiReserved = 0;
		pData->ui1MinMonFlag = 1;

		/* add hash */
		if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
			//pMonBSC = &pMonList->stMonBSC[pMonList->usBSCCnt];
			pMonBSC = &pMonList1Min->stMonBSC[pMonList1Min->usBSCCnt];
			pSubBSC = (st_SubBSC *)&pMonBSC->uiBSC;

			pSubBSC->ucOffice = pKey->ucOffice;
			pSubBSC->ucSYSID = pKey->ucSYSID;
			pSubBSC->ucBSCID = pKey->ucBSCID;

			/* FIRSTMON */
			dMakeBaseFirstMon(pHash, &pMonList1Min->stFirstMonList, pSubBSC->ucOffice, SYSTEM_TYPE_BSC, 0, pData->ui1MinMonFlag);

			log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

			pMonList1Min->usBSCCnt++;
		}
		else {
			log_print(LOGN_CRI, 
				"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}	

	/* BTS */
	log_print(LOGN_CRI, "### WF BTS CNT=%d", pWatchFilter->stWatchBTSList.dCount);
	for(i = 0; i < pWatchFilter->stWatchBTSList.dCount; i++)
	{
		if(pMonList->usBTSCnt >= MAX_MON_BTS_CNT) {
			log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW BTS MAX=%d FILTER=%d", 
					__FILE__, __FUNCTION__, __LINE__, MAX_MON_BTS_CNT, pWatchFilter->stWatchBTSList.dCount);
			break;
		}

		pWBTS = &pWatchFilter->stWatchBTSList.stWatchBTS[i];

		/* setting hash key */
		pKey->ucOffice = pWBTS->ucOffice;
		pKey->ucSysType = SYSTEM_TYPE_BTS;
		pKey->usSubType = 0;
		pKey->uiIP = 0;
		pKey->ucSYSID = pWBTS->ucSYSID;
		pKey->ucBSCID = pWBTS->ucBSCID;
		pKey->usBTSID = pWBTS->usBTSID;
		pKey->uiL4SvcType = 0;
		pKey->uiArrayType = ARRAY_TYPE_BTS;
		pKey->uiReserved = 0;

		/* setting hash data */
		pData->uiArrayIndex = pMonList->usBTSCnt;
		//pData->uiReserved = 0;
		pData->ui1MinMonFlag = 0;

		/* add hash */
		if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
			pMonBTS = &pMonList->stMonBTS[pMonList->usBTSCnt];
			pSubBTS = (st_SubBTS *)&pMonBTS->ullBTS;

			pSubBTS->ucOffice = pKey->ucOffice;
			pSubBTS->ucSYSID = pKey->ucSYSID;
			pSubBTS->ucBSCID = pKey->ucBSCID;
			pSubBTS->usBTSID = pKey->usBTSID;

			/* FIRSTMON */
			dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_BTS, 0, pData->ui1MinMonFlag);
			dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_FA, 0, pData->ui1MinMonFlag);
			dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_SECTOR, 0, pData->ui1MinMonFlag);

			/* FA */
			for(j = 0; j < MAX_MON_FA_CNT; j++)
			{
				pMonFA = &pMonBTS->stMonFA[j];
				pMonFA->ucFA = gFA[j];

				/* SECTOR */
				for(k = 0; k < MAX_MON_SEC_CNT; k++)
				{
					pMonSec = &pMonFA->stMonSec[k];
					pMonSec->ucSec = gSEC[k];
				}
			}

			log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

			pMonList->usBTSCnt++;
		}	
		else {
			log_print(LOGN_CRI, 
				"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}	

	/* EQUIP */
	pFirstMonList = &pMonList->stFirstMonList;
	log_print(LOGN_CRI, "### WF EQUIP CNT=%d", pWatchFilter->stWatchEquipList.dCount);
	for(i = 0; i < pWatchFilter->stWatchEquipList.dCount; i++)
	{
		pWEquip = &pWatchFilter->stWatchEquipList.stWatchEquip[i];

		switch(pWEquip->dType)
		{
			case SYSTEM_TYPE_PDSN:
				if( pWEquip->dMon1MinFlag == 1 )
					pFirstMonList = &pMonList1Min->stFirstMonList;
				else
					pFirstMonList = &pMonList->stFirstMonList;

				if(pFirstMonList->usPDSNListCnt >= MAX_MON_PDSN_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW PDSN MAX=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MON_PDSN_CNT);
					continue;
				}
				pMonEquip = &pFirstMonList->stPDSN[pFirstMonList->usPDSNListCnt];

				office = 0;
				systype = SYSTEM_TYPE_PDSN;
				ip = pWEquip->uiIP;

				pCnt = &pFirstMonList->usPDSNListCnt;
				break;

			case SYSTEM_TYPE_AAA:
				if( pWEquip->dMon1MinFlag == 1 )
					pFirstMonList = &pMonList1Min->stFirstMonList;
				else
					pFirstMonList = &pMonList->stFirstMonList;

				if(pFirstMonList->usAAAListCnt >= MAX_MON_AAA_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW AAA MAX=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MON_AAA_CNT);
					continue;
				}
				pMonEquip = &pFirstMonList->stAAA[pFirstMonList->usAAAListCnt];

				office = 0;
				systype = SYSTEM_TYPE_AAA;
				ip = pWEquip->uiIP;

				pCnt = &pFirstMonList->usAAAListCnt;
				break;

			case SYSTEM_TYPE_HSS:
				if( pWEquip->dMon1MinFlag == 1 )
					pFirstMonList = &pMonList1Min->stFirstMonList;
				else
					pFirstMonList = &pMonList->stFirstMonList;

				if(pFirstMonList->usHSSListCnt >= MAX_MON_HSS_CNT) {
					log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW HSS MAX=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MON_HSS_CNT);
					continue;
				}
				pMonEquip = &pFirstMonList->stHSS[pFirstMonList->usHSSListCnt];

				office = 0;
				systype = SYSTEM_TYPE_HSS;
				ip = pWEquip->uiIP;

				pCnt = &pFirstMonList->usHSSListCnt;
				break;

			case SYSTEM_TYPE_LNS:
				continue;

			default:
				log_print(LOGN_CRI, "F=%s:%s.%d INVALID EQUIP=%d", __FILE__, __FUNCTION__, __LINE__, pWEquip->dType);
				continue;
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

		/* setting hash data */
		pData->uiArrayIndex = *pCnt;
		//pData->uiReserved = 0;
		pData->ui1MinMonFlag = pWEquip->dMon1MinFlag;

		/* add hash */
		if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
			pMonEquip->ucOffice = pKey->ucOffice;
			pMonEquip->ucSysType = pKey->ucSysType;
			pMonEquip->uiIPAddr = pKey->uiIP;

			log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

			(*pCnt)++;
		}
		else {
			log_print(LOGN_CRI, 
				"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}	

	/* LOAM EQUIP */
	log_print(LOGN_CRI, "### WF LOAM EQUIP CNT=%d", pWatchFilter->stLoamEquipList.dCount);
	for(i = 0; i < pWatchFilter->stLoamEquipList.dCount; i++)
	{
		pREquip = &pWatchFilter->stLoamEquipList.stRoamEquip[i];

		switch(pREquip->dType)
		{
#if 0
		case ROAM_JAPAN_PDSN:
		case ROAM_CHINA_LAC:
			if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
				log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW ROAM SVC MAX=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT);
				continue;
			}

			/* setting hash key */
			pKey->ucOffice = 0;
			pKey->ucSysType = SYSTEM_TYPE_SERVICE;
			pKey->usSubType = SVC_IDX_ROAM;
			pKey->uiIP = pWEquip->uiIP;
			pKey->ucSYSID = 0;
			pKey->ucBSCID = 0;
			pKey->usBTSID = 0;
			pKey->uiL4SvcType = 0;
			pKey->uiArrayType = ARRAY_TYPE_SVC;
			pKey->uiReserved = 0;
    
			/* setting hash data */
			pData->uiArrayIndex = pMonList->usSvcCnt;
			pData->uiReserved = 0;

			/* add hash */
			if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
				pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
				pMonSvc->ucSvcType = pKey->usSubType;
				pMonSvc->SvcL4Type = pKey->uiL4SvcType;
				pMonSvc->uiIPAddr = pKey->uiIP;

				/* FIRSTMON */
				dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType);

				log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
					i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
					pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

				pMonList->usSvcCnt++;

			}
			else {
				log_print(LOGN_CRI, 
					"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
					i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
					pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
			}

			break;
#endif
		case SYSTEM_TYPE_LNS:
			if( pREquip->dMon1MinFlag == 1 )
				pFirstMonList = &pMonList1Min->stFirstMonList;
			else
				pFirstMonList = &pMonList->stFirstMonList;

			if(pFirstMonList->usLNSListCnt >= MAX_MON_LNS_CNT) {
				log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW LNS MAX=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MON_LNS_CNT);
				continue;
			}
			pMonEquip = &pFirstMonList->stLNS[pFirstMonList->usLNSListCnt];

			/* setting hash key */
			pKey->ucOffice = 0;
			pKey->ucSysType = SYSTEM_TYPE_LNS;
			pKey->usSubType = 0;
			pKey->uiIP = pREquip->uiIP;
			pKey->ucSYSID = 0;
			pKey->ucBSCID = 0;
			pKey->usBTSID = 0;
			pKey->uiL4SvcType = 0;
			pKey->uiArrayType = ARRAY_TYPE_CORE;
			pKey->uiReserved = 0;

			/* setting hash data */
			pData->uiArrayIndex = pFirstMonList->usLNSListCnt;
			//pData->uiReserved = 0;
			pData->ui1MinMonFlag = pREquip->dMon1MinFlag;

			/* add hash */
			if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
				pMonEquip->ucOffice = pKey->ucOffice;
				pMonEquip->ucSysType = pKey->ucSysType;
				pMonEquip->uiIPAddr = pKey->uiIP;

				log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
					i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
					pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

				pFirstMonList->usLNSListCnt++;
			}
			else {
				log_print(LOGN_CRI, 
					"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
					i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
					pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
			}
			break;
		case SYSTEM_TYPE_ROAMAAA:
			if( pREquip->dMon1MinFlag == 1 )
				pFirstMonList = &pMonList1Min->stFirstMonList;
			else
				pFirstMonList = &pMonList->stFirstMonList;

			if(pFirstMonList->usAAAListCnt >= MAX_MON_AAA_CNT) {
				log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW AAA MAX=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MON_AAA_CNT);
				continue;
			}
			pMonEquip = &pFirstMonList->stAAA[pFirstMonList->usAAAListCnt];

			/* setting hash key */
			pKey->ucOffice = 0;
			pKey->ucSysType = SYSTEM_TYPE_ROAMAAA;
			pKey->usSubType = 0;
			pKey->uiIP = pREquip->uiIP;
			pKey->ucSYSID = 0;
			pKey->ucBSCID = 0;
			pKey->usBTSID = 0;
			pKey->uiL4SvcType = 0;
			pKey->uiArrayType = ARRAY_TYPE_CORE;
			pKey->uiReserved = 0;

			/* setting hash data */
			pData->uiArrayIndex = pFirstMonList->usAAAListCnt;
			//pData->uiReserved = 0;
			pData->ui1MinMonFlag = pREquip->dMon1MinFlag;

			/* add hash */
			if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
				pMonEquip->ucOffice = pKey->ucOffice;
				pMonEquip->ucSysType = pKey->ucSysType;
				pMonEquip->uiIPAddr = pKey->uiIP;

				log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
					i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
					pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

				pFirstMonList->usAAAListCnt++;
			}
			else {
				log_print(LOGN_CRI, 
					"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
					i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
					pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
			}

			break;
		default:
			if(pMonList->usSvcCnt >= MAX_MON_SVC_CNT) {
				log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW ROAM SVC MAX=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MON_SVC_CNT);
				continue;
			}

			/* setting hash key */
			pKey->ucOffice = 0;
			pKey->ucSysType = SYSTEM_TYPE_SERVICE;
			pKey->usSubType = SVC_IDX_ROAM;
			pKey->uiIP = pREquip->uiIP;
			pKey->ucSYSID = 0;
			pKey->ucBSCID = 0;
			pKey->usBTSID = 0;
			pKey->uiL4SvcType = 0;
			pKey->uiArrayType = ARRAY_TYPE_SVC;
			pKey->uiReserved = 0;
    
			/* setting hash data */
			pData->uiArrayIndex = pMonList->usSvcCnt;
			//pData->uiReserved = 0;
			pData->ui1MinMonFlag = 0;

			/* add hash */
			if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
				pMonSvc = &pMonList->stMonSvc[pMonList->usSvcCnt];
				pMonSvc->ucSvcType = pKey->usSubType;
				pMonSvc->SvcL4Type = pKey->uiL4SvcType;
				pMonSvc->uiIPAddr = pKey->uiIP;

				/* FIRSTMON */
				dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, 0, SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType, pData->ui1MinMonFlag);

				log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
					i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
					pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);


				pMonList->usSvcCnt++;

				dMakeNasIPHash(pNasIPHash, pREquip->uiIP, pREquip->uiNetMask);

			}
			else {
				log_print(LOGN_CRI, 
					"HASH NULL I=%d OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
					i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
					pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
			}

			break;
		}
	}

	log_print(LOGN_CRI, "TOTAL INTERNET SERVICE COUNT=%d", inetSvcCnt);
	log_print(LOGN_CRI, "TOTAL SERVICE HASH COUNT=%hu", pMonList->usSvcCnt);

END_FUNC_TIME_CHECK(pFUNC, 40);

	return 0;
}

S32 dMakeBaseFirstMon(stHASHOINFO *pHash, st_FirstMonList *pFirstMonList, S32 office, S32 systype, S32 subtype, UINT ui1MinMonFlag)
{
START_FUNC_TIME_CHECK(pFUNC, 50);
	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	stSvcMonHashData;
	st_SvcMonHash_Data	*pData = &stSvcMonHashData;
	st_FirstMon			*pFirstMon;
	stHASHONODE			*pHASHNODE;
	S8					szIP[INET_ADDRSTRLEN];

	if(pFirstMonList->usFirstListCnt >= MAX_MON_FIRST_CNT) {
		log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW FIRSTMON OFFICE=%d SYSTYPE=%d SUBTYPE=%d MAX=%d", 
				__FILE__, __FUNCTION__, __LINE__, office, systype, subtype, MAX_MON_FIRST_CNT);
END_FUNC_TIME_CHECK(pFUNC, 50);
		return -1;
	}	

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

	pData->uiArrayIndex = pFirstMonList->usFirstListCnt;
	//pData->uiReserved = 0;
	pData->ui1MinMonFlag = ui1MinMonFlag;

	/* add hash */
	if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) {
		pFirstMon = &pFirstMonList->stFirstMon[pFirstMonList->usFirstListCnt];

		pFirstMon->ucOffice = office;
		pFirstMon->ucSysType = systype;
		pFirstMon->usSubType = subtype;

		log_print(LOGN_INFO, "FIRSTMON OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
			PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
			PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
			pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

		pFirstMonList->usFirstListCnt++;
	} else {

		log_print(LOGN_INFO, "NULL FIRSTMON OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
			PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
			PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
			pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
	}



END_FUNC_TIME_CHECK(pFUNC, 50);

	return 0;
}

S32 dMakeThresHash(stHASHOINFO *pThresHash, st_WatchFilter *pWatchFilter)
{
START_FUNC_TIME_CHECK(pFUNC, 60);
	int					i, j, index;

	st_ThresHash_Key	stThresHashKey;
	st_ThresHash_Key	*pKey = &stThresHashKey;
	st_ThresHash_Data	stThresHashData;
	st_ThresHash_Data	*pData = &stThresHashData;
	stHASHONODE			*pHASHNODE;

	st_WatchThresholdList	*pThresList = &pWatchFilter->stWatchThresholdList;
	st_WatchThreshold		*pThres;

	/* reset hash */
	hasho_reset(pThresHash);

	log_print(LOGN_CRI, "### WF THRES CNT=%d", pThresList->dCount);
	for(i = 0; i < pThresList->dCount; i++)
	{
		pThres = &pThresList->stWatchThreshold[i];

		pKey->ucOffice = pThres->ucOffice;
		pKey->ucSysType = pThres->ucSysType;
		pKey->ucAlarmType = pThres->ucAlarmType;
		pKey->ucReserved = 0;
		pKey->uiIP = pThres->uiIP;

		for(j = 0; j < 24; j++)
		{
			index = pThres->ucStartTime + j;
			if(index >= 24) index = index - 24;
			if(j < pThres->ucRange) {
				pData->ucDayFlag[index] = DEF_THRESFLAG_DAY; 
			} else {
				pData->ucDayFlag[index] = DEF_THRESFLAG_NIGHT; 
			}
			log_print(LOGN_CRI, "DAY j=%d index=%d value=%d", j, index, pData->ucDayFlag[index]);
		}

		pData->ucDayTimeRate = pThres->ucDayTimeRate;
		pData->ucNightTimeRate = pThres->ucNightTimeRate;
		pData->uiDayTimeMinTrial = pThres->uiDayTimeMinTrial;
		pData->uiNightTimeMinTrial = pThres->uiNightTimeMinTrial;
		pData->uiPeakTrial = pThres->uiPeakTrial;

		log_print(LOGN_CRI, "I=%d OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=%s:%u IP=%u DR=%u NR=%u DM=%u NM=%u P=%u S=%u R=%u",
				i, PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintALARMTYPE(pKey->ucAlarmType), pKey->ucAlarmType, pKey->uiIP, pData->ucDayTimeRate, pData->ucNightTimeRate,
				pData->uiDayTimeMinTrial, pData->uiNightTimeMinTrial, pData->uiPeakTrial, pThres->ucStartTime, pThres->ucRange);

		if((pHASHNODE = hasho_add(pThresHash, (U8 *)pKey, (U8 *)pData)) == NULL) {
			log_print(LOGN_CRI, "dMakeThresHash hasho_add NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=%s:%u IP=%u",
					PrintOFFICE(pKey->ucOffice), pKey->ucOffice,
					PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
					PrintALARMTYPE(pKey->ucAlarmType), pKey->ucAlarmType, pKey->uiIP);
		}
	}
END_FUNC_TIME_CHECK(pFUNC, 60);

	return 0;
}

S32 dMakeNasIPHash(stHASHOINFO *pNasIPHash, U32 uiNasIP, U32 uiNetMask)
{
START_FUNC_TIME_CHECK(pFUNC, 61);
	U32					uiBaseIP;
	S32					dLoop, i;

	st_NasIPHash_Key	stNasIPHashKey;
	st_NasIPHash_Key	*pKey = &stNasIPHashKey;
	st_NasIPHash_Data	stNasIPHash_Data;
	st_NasIPHash_Data	*pData = &stNasIPHash_Data;
	stHASHONODE			*pHASHNODE;

	S8					szPOOLIP[INET_ADDRSTRLEN];
	S8					szNASIP[INET_ADDRSTRLEN];
	S8					szBASEIP[INET_ADDRSTRLEN];

	uiBaseIP = GetSubNet(uiNasIP, uiNetMask);
	dLoop = GetSubNetLoopCnt(uiNetMask);

	for(i = 0; i < dLoop; i++)
	{
		pKey->uiNasIP = uiBaseIP + i;
		pData->uiNasIPPool = uiNasIP;
	
		log_print(LOGN_INFO, "NASIP IPPOOL=%s:%u NETMASK=%u BASEIP=%s:%u NASIP=%s:%u LOOP=%d i=%d",
					util_cvtipaddr(szPOOLIP, uiNasIP), uiNasIP, uiNetMask, util_cvtipaddr(szBASEIP, uiBaseIP), uiBaseIP,
					util_cvtipaddr(szNASIP, pKey->uiNasIP), pKey->uiNasIP, dLoop, i);	

		if((pHASHNODE = hasho_add(pNasIPHash, (U8 *)pKey, (U8 *)pData)) == NULL) {
			log_print(LOGN_CRI, "dMakeNasIPHash hasho_add NULL NASIPPOOL=%s:%u NETMASK=%u BASEIP=%s:%u NASIP=%s:%u LOOP=%d i=%d",
					util_cvtipaddr(szPOOLIP, uiNasIP), uiNasIP, uiNetMask, util_cvtipaddr(szBASEIP, uiBaseIP), uiBaseIP,
					util_cvtipaddr(szNASIP, pKey->uiNasIP), pKey->uiNasIP, dLoop, i);	
		}
	}

END_FUNC_TIME_CHECK(pFUNC, 61);

	return 0;
}

st_MonCore *pMakePCF(stHASHOINFO *pHash, st_MonList *pBaseList, st_MonList *pMonList, st_WatchMsg *pWatchMsg)
{
	S8					szIP[INET_ADDRSTRLEN];
	st_MonCore			*pMonPCF = NULL;

	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	stSvcMonHashData;
	st_SvcMonHash_Data	*pData = &stSvcMonHashData;
	stHASHONODE			*pHASHNODE;

	if(pBaseList->usPCFCnt != pMonList->usPCFCnt)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d DIFF PCF BASE=%d MON=%d",
			__FILE__, __FUNCTION__, __LINE__, pBaseList->usPCFCnt, pMonList->usPCFCnt);
		return NULL;
	}

	if(pMonList->usPCFCnt >= MAX_MON_PCF_CNT)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW PCF MAX=%d CNT=%d",
			__FILE__, __FUNCTION__, __LINE__, MAX_MON_PCF_CNT, pMonList->usPCFCnt);
		return NULL;
	}

	/* setting hash key */
	pKey->ucOffice = OFFICE_UNKNOWN;
	pKey->ucSysType = SYSTEM_TYPE_PCF;
	pKey->usSubType = 0;
	pKey->uiIP = pWatchMsg->uiPCFIP;
	pKey->ucSYSID = 0;
	pKey->ucBSCID = 0;
	pKey->usBTSID = 0;
	pKey->uiL4SvcType = 0;
	pKey->uiArrayType = ARRAY_TYPE_CORE;
	pKey->uiReserved = 0;

	/* setting hash data */
	pData->uiArrayIndex = pMonList->usPCFCnt;
	//pData->uiReserved = 0;
	pData->ui1MinMonFlag = 0;

	/* add hash */
	if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) 
	{
		log_print(LOGN_CRI, "dMakePCF OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
			PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
			PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
			pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

		pMonPCF = &pBaseList->stMonPCF[pBaseList->usPCFCnt];
		pMonPCF->ucOffice = pKey->ucOffice;
		pMonPCF->ucSysType = pKey->ucSysType;
		pMonPCF->uiIPAddr = pKey->uiIP;

		/* FIRSTMON */
		dMakeBaseFirstMon(pHash, &pBaseList->stFirstMonList, pMonPCF->ucOffice, SYSTEM_TYPE_PCF, 0, pData->ui1MinMonFlag);
		pBaseList->usPCFCnt++;

		pMonPCF = &pMonList->stMonPCF[pMonList->usPCFCnt];
		pMonPCF->ucOffice = pKey->ucOffice;
		pMonPCF->ucSysType = pKey->ucSysType;
		pMonPCF->uiIPAddr = pKey->uiIP;

		/* FIRSTMON */
		dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pMonPCF->ucOffice, SYSTEM_TYPE_PCF, 0, pData->ui1MinMonFlag);
		pMonList->usPCFCnt++;
	}
	else 
	{
		if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL)
		{
			pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);
			pMonPCF = &pMonList->stMonPCF[pData->uiArrayIndex];
		}
		else
		{
			log_print(LOGN_CRI,
				"HASH NULL dMakePCF OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}

	return pMonPCF;
}

st_MonBSC *pMakeBSC(stHASHOINFO *pHash, st_MonList_1Min *pBaseList, st_MonList_1Min *pMonList, st_WatchMsg *pWatchMsg)
{
	S8					szIP[INET_ADDRSTRLEN];
	st_SubBSC			*pSubBSC;
	st_MonBSC			*pMonBSC = NULL;

	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	stSvcMonHashData;
	st_SvcMonHash_Data	*pData = &stSvcMonHashData;
	stHASHONODE			*pHASHNODE;

	if(pBaseList->usBSCCnt != pMonList->usBSCCnt)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d DIFF BSC BASE=%d MON=%d",
			__FILE__, __FUNCTION__, __LINE__, pBaseList->usBSCCnt, pMonList->usBSCCnt);
		return NULL;
	}

	if(pMonList->usBSCCnt >= MAX_MON_BSC_CNT)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW BSC MAX=%d CNT=%d",
			__FILE__, __FUNCTION__, __LINE__, MAX_MON_BSC_CNT, pMonList->usBSCCnt);
		return NULL;
	}

	/* setting hash key */
	pKey->ucOffice = OFFICE_UNKNOWN;
	pKey->ucSysType = SYSTEM_TYPE_BSC;
	pKey->usSubType = 0;
	pKey->uiIP = 0;
	pKey->ucSYSID = pWatchMsg->ucSYSID;
	pKey->ucBSCID = pWatchMsg->ucBSCID;
	pKey->usBTSID = 0;
	pKey->uiL4SvcType = 0;
	pKey->uiArrayType = ARRAY_TYPE_BSC;
	pKey->uiReserved = 0;

	/* setting hash data */
	pData->uiArrayIndex = pMonList->usBSCCnt;
	//pData->uiReserved = 0;
	pData->ui1MinMonFlag = 1;

	/* add hash */
	if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) 
	{
		log_print(LOGN_CRI, "dMakeBSC OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
			PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
			PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
			pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

		pMonBSC = &pBaseList->stMonBSC[pBaseList->usBSCCnt];
		pSubBSC = (st_SubBSC *)&pMonBSC->uiBSC;

		pSubBSC->ucOffice = pKey->ucOffice;
		pSubBSC->ucSYSID = pKey->ucSYSID;
		pSubBSC->ucBSCID = pKey->ucBSCID;

		/* FIRSTMON */
		dMakeBaseFirstMon(pHash, &pBaseList->stFirstMonList, pSubBSC->ucOffice, SYSTEM_TYPE_BSC, 0, pData->ui1MinMonFlag);
		pBaseList->usBSCCnt++;


		pMonBSC = &pMonList->stMonBSC[pMonList->usBSCCnt];
		pSubBSC = (st_SubBSC *)&pMonBSC->uiBSC;

		pSubBSC->ucOffice = pKey->ucOffice;
		pSubBSC->ucSYSID = pKey->ucSYSID;
		pSubBSC->ucBSCID = pKey->ucBSCID;

		/* FIRSTMON */
		dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pSubBSC->ucOffice, SYSTEM_TYPE_BSC, 0, pData->ui1MinMonFlag);
		pMonList->usBSCCnt++;
	}
	else 
	{
		if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL)
		{
			pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);
			pMonBSC = &pMonList->stMonBSC[pData->uiArrayIndex];
		}
		else
		{
			log_print(LOGN_CRI,
				"HASH NULL dMakeBSC OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}

	return pMonBSC;
}

st_MonBTS *pMakeBTS(stHASHOINFO *pHash, st_MonList *pBaseList, st_MonList *pMonList, st_WatchMsg *pWatchMsg)
{
	int					j, k;
	S8					szIP[INET_ADDRSTRLEN];
	st_SubBTS			*pSubBTS;
	st_MonBTS			*pMonBTS = NULL;
	st_MonFA			*pMonFA;
	st_MonSec			*pMonSec;

	st_SvcMonHash_Key	stSvcMonHashKey;
	st_SvcMonHash_Key	*pKey = &stSvcMonHashKey;
	st_SvcMonHash_Data	stSvcMonHashData;
	st_SvcMonHash_Data	*pData = &stSvcMonHashData;
	stHASHONODE			*pHASHNODE;

	if(pBaseList->usBTSCnt != pMonList->usBTSCnt)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d DIFF BTS BASE=%d MON=%d",
			__FILE__, __FUNCTION__, __LINE__, pBaseList->usBTSCnt, pMonList->usBTSCnt);
		return NULL;
	}

	if(pMonList->usBTSCnt >= MAX_MON_BTS_CNT)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d OVERFLOW BTS MAX=%d CNT=%d",
			__FILE__, __FUNCTION__, __LINE__, MAX_MON_BTS_CNT, pMonList->usBTSCnt);
		return NULL;
	}

	/* setting hash key */
	pKey->ucOffice = OFFICE_UNKNOWN;
	pKey->ucSysType = SYSTEM_TYPE_BTS;
	pKey->usSubType = 0;
	pKey->uiIP = 0;
	pKey->ucSYSID = pWatchMsg->ucSYSID;
	pKey->ucBSCID = pWatchMsg->ucBSCID;
	pKey->usBTSID = pWatchMsg->usBTSID;
	pKey->uiL4SvcType = 0;
	pKey->uiArrayType = ARRAY_TYPE_BTS;
	pKey->uiReserved = 0;

	/* setting hash data */
	pData->uiArrayIndex = pMonList->usBTSCnt;
	//pData->uiReserved = 0;
	pData->ui1MinMonFlag = 0;

	/* add hash */
	if((pHASHNODE = hasho_add(pHash, (U8 *)pKey, (U8 *)pData)) != NULL) 
	{
		log_print(LOGN_CRI, "dMakeBTS OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
			PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
			PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
			pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);

		pMonBTS = &pBaseList->stMonBTS[pBaseList->usBTSCnt];
		pSubBTS = (st_SubBTS *)&pMonBTS->ullBTS;

		pSubBTS->ucOffice = pKey->ucOffice;
		pSubBTS->ucSYSID = pKey->ucSYSID;
		pSubBTS->ucBSCID = pKey->ucBSCID;
		pSubBTS->usBTSID = pKey->usBTSID;

		/* FIRSTMON */
		dMakeBaseFirstMon(pHash, &pBaseList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_BTS, 0, pData->ui1MinMonFlag);
		dMakeBaseFirstMon(pHash, &pBaseList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_FA, 0, pData->ui1MinMonFlag);
		dMakeBaseFirstMon(pHash, &pBaseList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_SECTOR, 0, pData->ui1MinMonFlag);

		/* FA */
		for(j = 0; j < MAX_MON_FA_CNT; j++)
		{
			pMonFA = &pMonBTS->stMonFA[j];
			pMonFA->ucFA = gFA[j];

			/* SECTOR */
			for(k = 0; k < MAX_MON_SEC_CNT; k++)
			{
				pMonSec = &pMonFA->stMonSec[k];
				pMonSec->ucSec = gSEC[k];
			}
		}

		pBaseList->usBTSCnt++;


		pMonBTS = &pMonList->stMonBTS[pMonList->usBTSCnt];
		pSubBTS = (st_SubBTS *)&pMonBTS->ullBTS;

		pSubBTS->ucOffice = pKey->ucOffice;
		pSubBTS->ucSYSID = pKey->ucSYSID;
		pSubBTS->ucBSCID = pKey->ucBSCID;
		pSubBTS->usBTSID = pKey->usBTSID;

		/* FIRSTMON */
		dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_BTS, 0, pData->ui1MinMonFlag);
		dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_FA, 0, pData->ui1MinMonFlag);
		dMakeBaseFirstMon(pHash, &pMonList->stFirstMonList, pSubBTS->ucOffice, SYSTEM_TYPE_SECTOR, 0, pData->ui1MinMonFlag);

		/* FA */
		for(j = 0; j < MAX_MON_FA_CNT; j++)
		{
			pMonFA = &pMonBTS->stMonFA[j];
			pMonFA->ucFA = gFA[j];

			/* SECTOR */
			for(k = 0; k < MAX_MON_SEC_CNT; k++)
			{
				pMonSec = &pMonFA->stMonSec[k];
				pMonSec->ucSec = gSEC[k];
			}
		}

		pMonList->usBTSCnt++;
	}
	else 
	{
		if((pHASHNODE = hasho_find(pHash, (U8 *)pKey)) != NULL)
		{
			pData = (st_SvcMonHash_Data *)nifo_ptr(pHash, pHASHNODE->offset_Data);
			pMonBTS = &pMonList->stMonBTS[pData->uiArrayIndex];
		}
		else
		{
			log_print(LOGN_CRI,
				"HASH NULL dMakeBTS OFFICE=%s:%u SYSTYPE=%s:%u SUBTYPE=%s:%u IP=%s:%u SYSID=%u BSCID=%u BTSID=%u L4=%u TYPE=%u IDX=%u",
				PrintOFFICE(pKey->ucOffice), pKey->ucOffice, PrintSYSTYPE(pKey->ucSysType), pKey->ucSysType,
				PrintSUBTYPE(pKey->ucSysType, pKey->usSubType), pKey->usSubType, util_cvtipaddr(szIP, pKey->uiIP), pKey->uiIP,
				pKey->ucSYSID, pKey->ucBSCID, pKey->usBTSID, pKey->uiL4SvcType, pKey->uiArrayType, pData->uiArrayIndex);
		}
	}

	return pMonBTS;
}

int dGetSYSCFG(void)
{ 
	FILE	*fa; 
	char	szBuf[1024], szType[64], szTmp[64], szInfo[64];
	int		i = 0;

	if( (fa = fopen(FILE_SYS_CONFIG, "r")) == NULL) {
		log_print(LOGN_CRI,"LOAD SYSTEM CONFIG : %s FILE OPEN FAIL (%s)", FILE_SYS_CONFIG, strerror(errno));
		return -1;
	}

	while(fgets(szBuf, 1024, fa) != NULL)
	{
		if(szBuf[0] != '#') {
			log_print(LOGN_WARN,"FAILED IN dGetSYSCFG() : %s File [%d] row format error", FILE_SYS_CONFIG, i);
			continue;
		}

		i++;
		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@') {
			if(sscanf(&szBuf[2], "%s %s %s", szType, szTmp, szInfo) == 3) {
				if(strcmp(szType, "SYS") == 0) {
					if(strcmp(szTmp, "NO") == 0) {
						dSysNo = atoi(szInfo);
						return 1;
					}
				}
			}
		}
	}
	fclose(fa);
	
	return -1;
}


/*
 * $Log: o_svcmon_conf.c,v $
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
 * Revision 1.60  2011/06/27 08:21:42  innaei
 * *** empty log message ***
 *
 * Revision 1.59  2011/06/27 08:19:08  innaei
 * *** empty log message ***
 *
 * Revision 1.58  2011/06/27 08:17:26  innaei
 * *** empty log message ***
 *
 * Revision 1.57  2011/06/23 00:58:18  innaei
 * *** empty log message ***
 *
 * Revision 1.56  2011/04/26 21:02:41  innaei
 * *** empty log message ***
 *
 * Revision 1.55  2011/04/25 17:19:29  innaei
 * *** empty log message ***
 *
 * Revision 1.54  2011/04/25 04:52:05  innaei
 * *** empty log message ***
 *
 * Revision 1.53  2011/04/24 19:24:28  innaei
 * *** empty log message ***
 *
 * Revision 1.52  2011/04/24 12:44:55  innaei
 * *** empty log message ***
 *
 * Revision 1.51  2011/04/22 19:17:23  innaei
 * *** empty log message ***
 *
 * Revision 1.50  2011/04/20 14:00:45  innaei
 * *** empty log message ***
 *
 * Revision 1.49  2011/04/20 06:20:41  innaei
 * *** empty log message ***
 *
 * Revision 1.48  2011/04/17 12:36:52  innaei
 * *** empty log message ***
 *
 * Revision 1.47  2011/04/15 09:12:13  jhbaek
 * *** empty log message ***
 *
 * Revision 1.46  2011/04/14 11:46:04  jhbaek
 * *** empty log message ***
 *
 * Revision 1.45  2011/04/13 08:50:18  jhbaek
 * *** empty log message ***
 *
 * Revision 1.44  2011/04/12 07:42:45  jsyoon
 * *** empty log message ***
 *
 * Revision 1.43  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.2  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.42  2010/03/31 07:22:00  dark264sh
 * *** empty log message ***
 *
 * Revision 1.41  2010/03/31 07:15:49  dark264sh
 * *** empty log message ***
 *
 * Revision 1.40  2010/03/29 12:23:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.39  2010/03/05 13:52:01  dark264sh
 * *** empty log message ***
 *
 * Revision 1.38  2010/03/05 11:19:47  dark264sh
 * O_SVCMON ROAM Alarm 세팅 버그 수정
 *
 * Revision 1.37  2010/03/04 12:04:21  dark264sh
 * *** empty log message ***
 *
 * Revision 1.36  2010/03/04 11:52:24  dark264sh
 * NASIP log_print 수정
 *
 * Revision 1.35  2010/03/04 06:00:53  dark264sh
 * ROAM NASIP NetMask 처리
 *
 * Revision 1.34  2010/03/03 08:27:26  dark264sh
 * ROAM 관련 구조 변경에 따른 수정
 *
 * Revision 1.33  2010/03/02 06:27:00  dark264sh
 * ROAM 관련 코드가 변경
 *
 * Revision 1.32  2010/02/26 10:24:20  dark264sh
 * O_SVCMON ROAM AAA 처리
 *
 * Revision 1.31  2010/02/25 11:13:45  dark264sh
 * O_SVCMON ROAM 처리
 *
 * Revision 1.30  2010/02/25 07:33:07  dark264sh
 * BSD => LNS로 변경
 *
 * Revision 1.29  2010/02/24 12:19:46  dark264sh
 * 망감시 헤더 변경에 따른 변경
 *
 * Revision 1.28  2009/10/21 11:45:08  pkg
 * 망감시 REGI 서비스 추가 및 define값 변경
 *
 * Revision 1.27  2009/08/26 07:58:06  pkg
 * O_SVCMON WIPI DOWNLOAD 예외 처리 추가
 *
 * Revision 1.26  2009/08/24 00:14:20  pkg
 * *** empty log message ***
 *
 * Revision 1.25  2009/08/22 19:30:30  pkg
 * *** empty log message ***
 *
 * Revision 1.24  2009/08/22 19:24:38  pkg
 * *** empty log message ***
 *
 * Revision 1.23  2009/08/22 19:05:20  pkg
 * *** empty log message ***
 *
 * Revision 1.22  2009/08/22 18:58:48  pkg
 * O_SVCMON VT/IM 예외 처리
 *
 * Revision 1.21  2009/08/22 13:43:55  pkg
 * *** empty log message ***
 *
 * Revision 1.20  2009/08/22 12:04:02  pkg
 * O_SVCMON BTSID=0인 경우 BSC, BTS 중복되는 버그 수정
 *
 * Revision 1.19  2009/08/22 11:30:57  pkg
 * O_SVCMON BTS 사이즈 변경
 *
 * Revision 1.18  2009/07/27 06:56:09  dark264sh
 * *** empty log message ***
 *
 * Revision 1.17  2009/07/27 05:34:37  dark264sh
 * O_SVCMON L4_DNS 예외 처리
 *
 * Revision 1.16  2009/07/20 02:44:20  dark264sh
 * O_SVCMON SYSID 추가
 *
 * Revision 1.15  2009/07/13 05:57:48  dark264sh
 * L4_FTP는 망감시에서 제외
 *
 * Revision 1.14  2009/07/12 12:04:01  dark264sh
 * O_SVCMON DEF_PLATFORM_PHONE 관련 예외 처리 추가
 *
 * Revision 1.13  2009/07/12 10:39:17  dark264sh
 * O_SVCMON L4_RTS_FB, L4_RTS_WB 관련 예외 처리 추가
 *
 * Revision 1.12  2009/07/10 06:31:28  dark264sh
 * 망감시 관련 서비스 구분 변경
 *
 * Revision 1.11  2009/07/07 15:43:48  dark264sh
 * O_SVCMON log_print 변경
 *
 * Revision 1.10  2009/07/02 14:44:36  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2009/07/02 07:55:00  dark264sh
 * *** empty log message ***
 *
 * Revision 1.8  2009/07/01 17:29:33  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2009/07/01 11:58:41  dark264sh
 * O_SVCMON FA, SEC 값 변경
 *
 * Revision 1.6  2009/06/29 15:46:37  dark264sh
 * *** empty log message ***
 *
 * Revision 1.5  2009/06/29 15:36:41  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/27 11:34:31  dark264sh
 * O_SVCMON lTime 세팅 추가
 *
 * Revision 1.3  2009/06/21 13:34:33  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/20 15:51:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2009/06/18 17:01:48  dark264sh
 * O_SVCMON Filter 처리
 *
 */
