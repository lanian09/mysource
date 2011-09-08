/**		@file	o_svcmon_log.c
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_print.c,v 1.4 2011/09/07 07:07:52 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/07 07:07:52 $
 * 		@ref		o_svcmon_init.c o_svcmon_maic.c
 *
 * 		@section	Intro(소개)
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>

// LIB
#include "typedef.h"
#include "hasho.h"
#include "loglib.h"
#include "utillib.h"

// PROJECT
#include "svcmon.h"

// TAM
#include "watch_mon.h"
#include "watch_filter.h"

//#include "o_svcmon_func.h"
#include "o_svcmon_print.h"
#include "o_svcmon_util.h"

void PrintWatchMsg(S32 lvl, S8 *str, S32 dRet, st_WatchMsg *pWatchMsg)
{
	S8				szPCFIP[INET_ADDRSTRLEN];
	S8				szPDSNIP[INET_ADDRSTRLEN];
	S8				szAAAIP[INET_ADDRSTRLEN];
	S8				szHSSIP[INET_ADDRSTRLEN];
	S8				szLNSIP[INET_ADDRSTRLEN];
	S8				szSVCIP[INET_ADDRSTRLEN];

	log_print(lvl, 
		"%s dRet=%d MSGTYPE=%s:%u OFFICE=%s:%u SYSID=%u BSCID=%u BTSID=%u SEC=%u FA=%u SVCL4=%s:%u SVCTYPE=%s:%u "
		"PCFTYPE=%s:%u PCF=%s:%u PDSN=%s:%u AAA=%s:%u HSS=%s:%u LNS=%s:%u SVC=%s:%u RESULT=%u ROAMFLAG=%d",
		str, dRet,
		PrintMSGTYPE(pWatchMsg->usMsgType), pWatchMsg->usMsgType, PrintOFFICE(pWatchMsg->ucOffice), pWatchMsg->ucOffice,
		pWatchMsg->ucSYSID, pWatchMsg->ucBSCID, pWatchMsg->usBTSID, pWatchMsg->ucSec, pWatchMsg->ucFA, 
		PrintSVC(pWatchMsg->usSvcL4Type), pWatchMsg->usSvcL4Type, PrintSVCTYPE(pWatchMsg->ucSvcIdx), pWatchMsg->ucSvcIdx,
		PrintPCFTYPE(pWatchMsg->ucPCFType), pWatchMsg->ucPCFType,
		util_cvtipaddr(szPCFIP, pWatchMsg->uiPCFIP), pWatchMsg->uiPCFIP, util_cvtipaddr(szPDSNIP, pWatchMsg->uiPDSNIP), pWatchMsg->uiPDSNIP, 
		util_cvtipaddr(szAAAIP, pWatchMsg->uiAAAIP), pWatchMsg->uiAAAIP, util_cvtipaddr(szHSSIP, pWatchMsg->uiHSSIP), pWatchMsg->uiHSSIP, 
		util_cvtipaddr(szLNSIP, pWatchMsg->uiLNSIP), pWatchMsg->uiLNSIP, util_cvtipaddr(szSVCIP, pWatchMsg->uiSVCIP), pWatchMsg->uiSVCIP, 
		pWatchMsg->uiResult, pWatchMsg->ucRoamFlag);

}

void PrintMonList(S32 lvl, S32 detail, st_MonList *pMonList, st_MonList_1Min *pMonList1Min)
{
	int		i;
	S8		szTime[BUFSIZ];

	log_print(lvl, "[MonList] ### ### ### ### ###");

	log_print(lvl, "Time=%s:%lu PCFCnt=%u BTSCnt=%u BSCCnt=%u SvcCnt=%u",
			util_printtime(pMonList->lTime, szTime), pMonList->lTime, 
			pMonList->usPCFCnt, pMonList->usBTSCnt, pMonList1Min->usBSCCnt, pMonList->usSvcCnt);

	log_print(lvl, "Time=%s:%lu PDSNListCnt=[%u:%u] AAAListCnt=[%u:%u] HSSListCnt=[%u:%u] LNSListCnt=[%u:%u] FirstListCnt=[%u:%u]", 
			util_printtime(pMonList->stFirstMonList.lTime, szTime), pMonList->stFirstMonList.lTime,
			pMonList->stFirstMonList.usPDSNListCnt, pMonList1Min->stFirstMonList.usPDSNListCnt,
			pMonList->stFirstMonList.usAAAListCnt, pMonList1Min->stFirstMonList.usAAAListCnt,
			pMonList->stFirstMonList.usHSSListCnt, pMonList1Min->stFirstMonList.usHSSListCnt,
			pMonList->stFirstMonList.usLNSListCnt, pMonList1Min->stFirstMonList.usLNSListCnt,
			pMonList->stFirstMonList.usFirstListCnt, pMonList1Min->stFirstMonList.usFirstListCnt);

	/* PDSN */
	log_print(lvl, "[PDSN][5MIN] ### ### ### ### ###");
	for(i = 0; i < pMonList->stFirstMonList.usPDSNListCnt; i++) {
		PrintMonCore(lvl, detail, &pMonList->stFirstMonList.stPDSN[i]);
	}	
	log_print(lvl, "[PDSN][5MIN] *** *** *** *** ***");
	log_print(lvl, "[PDSN][1MIN] ### ### ### ### ###");
	for(i = 0; i < pMonList1Min->stFirstMonList.usPDSNListCnt; i++) {
		PrintMonCore(lvl, detail, &pMonList1Min->stFirstMonList.stPDSN[i]);
	}	
	log_print(lvl, "[PDSN][1MIN] *** *** *** *** ***");

	/* AAA */
	log_print(lvl, "[AAA][5MIN] ### ### ### ### ###");
	for(i = 0; i < pMonList->stFirstMonList.usAAAListCnt; i++) {
		PrintMonCore(lvl, detail, &pMonList->stFirstMonList.stAAA[i]);
	}	
	log_print(lvl, "[AAA][5MIN] *** *** *** *** ***");
	log_print(lvl, "[AAA][1MIN] ### ### ### ### ###");
	for(i = 0; i < pMonList1Min->stFirstMonList.usAAAListCnt; i++) {
		PrintMonCore(lvl, detail, &pMonList1Min->stFirstMonList.stAAA[i]);
	}	
	log_print(lvl, "[AAA][1MIN] *** *** *** *** ***");

	/* HSS */
	log_print(lvl, "[HSS] ### ### ### ### ###");
	for(i = 0; i < pMonList->stFirstMonList.usHSSListCnt; i++)
	{
		PrintMonCore(lvl, detail, &pMonList->stFirstMonList.stHSS[i]);
	}	
	log_print(lvl, "[HSS] *** *** *** *** ***");

	/* LNS */
	log_print(lvl, "[LNS] ### ### ### ### ###");
	for(i = 0; i < pMonList->stFirstMonList.usLNSListCnt; i++)
	{
		PrintMonCore(lvl, detail, &pMonList->stFirstMonList.stLNS[i]);
	}	
	log_print(lvl, "[LNS] *** *** *** *** ***");

	/* FirstMon */
	log_print(lvl, "[FirstMon][5MIN] ### ### ### ### ###");
	for(i = 0; i < pMonList->stFirstMonList.usFirstListCnt; i++) {
		PrintFirstMon(lvl, detail, &pMonList->stFirstMonList.stFirstMon[i]);
	}	
	log_print(lvl, "[FirstMon][5MIN] *** *** *** *** ***");
	log_print(lvl, "[FirstMon][1MIN] ### ### ### ### ###");
	for(i = 0; i < pMonList1Min->stFirstMonList.usFirstListCnt; i++) {
		PrintFirstMon(lvl, detail, &pMonList1Min->stFirstMonList.stFirstMon[i]);
	}	
	log_print(lvl, "[FirstMon][1MIN] *** *** *** *** ***");

	/* SVC */
	log_print(lvl, "[SVC] ### ### ### ### ###");
	for(i = 0; i < pMonList->usSvcCnt; i++)
	{
		PrintMonSvc(lvl, detail, &pMonList->stMonSvc[i]);
	}	
	log_print(lvl, "[SVC] *** *** *** *** ***");

	/* PCF */
	log_print(lvl, "[PCF] ### ### ### ### ###");
	for(i = 0; i < pMonList->usPCFCnt; i++)
	{
		PrintMonCore(lvl, detail, &pMonList->stMonPCF[i]);
	}	
	log_print(lvl, "[PCF] *** *** *** *** ***");

	/* BSC */
	log_print(lvl, "[BSC] ### ### ### ### ###");
	for(i = 0; i < pMonList1Min->usBSCCnt; i++)
	{
		PrintMonBSC(lvl, detail, &pMonList1Min->stMonBSC[i]);
	}	
	log_print(lvl, "[BSC] *** *** *** *** ***");

	/* BTS */
	log_print(lvl, "[BTS] ### ### ### ### ###");
	for(i = 0; i < pMonList->usBTSCnt; i++)
	{
		PrintMonBTS(lvl, detail, &pMonList->stMonBTS[i]);
	}	
	log_print(lvl, "[BTS] *** *** *** *** ***");

	log_print(lvl, "[MonList] *** *** *** *** ***");
}

void PrintMonCore(S32 lvl, S32 detail, st_MonCore *pMonCore)
{
	S8		szIP[INET_ADDRSTRLEN];

	log_print(lvl, "Office=%s:%u SysType=%s:%u uiIPAddr=%s:%u", 
		PrintOFFICE(pMonCore->ucOffice), pMonCore->ucOffice, 
		PrintSYSTYPE(pMonCore->ucSysType), pMonCore->ucSysType, util_cvtipaddr(szIP, pMonCore->uiIPAddr), pMonCore->uiIPAddr);		

	if(detail) 
	{
		/* print st_MonAlarm */
		PrintMonAlarm(lvl, &pMonCore->stMonAlarm);

		/* print st_MonInfo */
		PrintMonInfo(lvl, &pMonCore->stMonInfo);

		/* print st_Defect */
		PrintDefect(lvl, &pMonCore->stDefect);
	}
}

void PrintFirstMon(S32 lvl, S32 detail, st_FirstMon *pFirstMon)
{
	log_print(lvl, "Office=%s:%u SysType=%s:%u SubType=%s:%u", 
		PrintOFFICE(pFirstMon->ucOffice), pFirstMon->ucOffice, PrintSYSTYPE(pFirstMon->ucSysType), pFirstMon->ucSysType, 
		PrintSUBTYPE(pFirstMon->ucSysType, pFirstMon->usSubType), pFirstMon->usSubType);

	if(detail)
	{
		/* print st_MonAlarm */
		PrintMonAlarm(lvl, &pFirstMon->stMonAlarm);

		/* print st_MonInfo */
		PrintMonInfo(lvl, &pFirstMon->stMonInfo);

		/* print st_Defect */
		PrintDefect(lvl, &pFirstMon->stDefect);
	}
}

void PrintMonSvc(S32 lvl, S32 detail, st_MonSvc *pMonSvc)
{
	S8		szIP[INET_ADDRSTRLEN];

	log_print(lvl, "SvcType=%s:%u SvcL4Type=%s:%u uiIPAddr=%s:%u", 
		PrintSUBTYPE(SYSTEM_TYPE_SERVICE, pMonSvc->ucSvcType), pMonSvc->ucSvcType, 
		PrintSVC(pMonSvc->SvcL4Type), pMonSvc->SvcL4Type, util_cvtipaddr(szIP, pMonSvc->uiIPAddr), pMonSvc->uiIPAddr);

	if(detail)
	{
		/* print st_MonAlarm */
		PrintMonAlarm(lvl, &pMonSvc->stMonAlarm);

		/* print st_MonInfo */
		PrintMonInfo(lvl, &pMonSvc->stMonInfo);

		/* print st_Defect */
		PrintDefect(lvl, &pMonSvc->stDefect);
	}
}

void PrintMonBSC(S32 lvl, S32 detail, st_MonBSC *pMonBSC)
{
	st_SubBSC	*pSubBSC = (st_SubBSC *)&pMonBSC->uiBSC;

	log_print(lvl, "Office=%s:%u SYSID=%u BSCID=%u", 
		PrintOFFICE(pSubBSC->ucOffice), pSubBSC->ucOffice, pSubBSC->ucSYSID, pSubBSC->ucBSCID);		

	if(detail) 
	{
		/* print st_MonAlarm */
		PrintMonAlarm(lvl, &pMonBSC->stMonAlarm);

		/* print st_MonInfo */
		PrintMonInfo(lvl, &pMonBSC->stMonInfo);

		/* print st_Defect */
		PrintDefect(lvl, &pMonBSC->stDefect);
	}
}

void PrintMonBTS(S32 lvl, S32 detail, st_MonBTS *pMonBTS)
{
	int			i;
	st_SubBTS	*pSubBTS = (st_SubBTS *)&pMonBTS->ullBTS;

	log_print(lvl, "Office=%s:%u SYSID=%u BSCID=%u BTSID=%u", 
		PrintOFFICE(pSubBTS->ucOffice), pSubBTS->ucOffice, pSubBTS->ucSYSID, pSubBTS->ucBSCID, pSubBTS->usBTSID);		

	if(detail) 
	{
		/* print st_MonAlarm */
		PrintMonAlarm(lvl, &pMonBTS->stMonAlarm);

		/* print st_MonInfo */
		PrintMonInfo(lvl, &pMonBTS->stMonInfo);

		/* print st_Defect */
		PrintDefect(lvl, &pMonBTS->stDefect);
	}

	for(i = 0; i < MAX_MON_FA_CNT; i++)
	{
		PrintMonFA(lvl, detail, &pMonBTS->stMonFA[i]);
	}
}

void PrintMonFA(S32 lvl, S32 detail, st_MonFA *pMonFA)
{
	int		i;
	log_print(lvl, "FA=%u", pMonFA->ucFA);		

	if(detail) 
	{
		/* print st_MonAlarm */
		PrintMonAlarm(lvl, &pMonFA->stMonAlarm);

		/* print st_MonInfoS */
		PrintMonInfoS(lvl, &pMonFA->stMonInfoS);

		/* print st_DefectS */
		PrintDefectS(lvl, &pMonFA->stDefectS);
	}

	for(i = 0; i < MAX_MON_SEC_CNT; i++)
	{
		PrintMonSec(lvl, detail, &pMonFA->stMonSec[i]);
	}
}

void PrintMonSec(S32 lvl, S32 detail, st_MonSec *pMonSec)
{
	log_print(lvl, "Sector=%u", pMonSec->ucSec);		

	if(detail) 
	{
		/* print st_MonAlarm */
		PrintMonAlarm(lvl, &pMonSec->stMonAlarm);

		/* print st_MonInfoS */
		PrintMonInfoS(lvl, &pMonSec->stMonInfoS);

		/* print st_DefectS */
		PrintDefectS(lvl, &pMonSec->stDefectS);
	}
}

void PrintMonAlarm(S32 lvl, st_MonAlarm *pMonAlarm)
{
	st_SubAlarmSysStatus	*pSYS = (st_SubAlarmSysStatus *)&pMonAlarm->ucAlarmSysStatus;
	st_SubAlarmSvcStatus	*pSVC = (st_SubAlarmSvcStatus *)&pMonAlarm->szAlarmSvcStatus[0];
	
	if(pMonAlarm->ucAlarm > 0) {
		log_print(lvl, "Alarm=%u Call=%u AAA=%u HSS=%u LNS=%u MENU=%u DN=%u STREAM=%u MMS=%u "
				"WIDGET=%u PHONE=%u EMS=%u BANK=%u FV=%u IM=%u VT=%u ETC=%u CORP=%u REGI=%u",
				pMonAlarm->ucAlarm, pSYS->ucCall, pSYS->ucAAA, pSYS->ucHSS, pSYS->ucLNS, 
				pSVC->ucMENU, pSVC->ucDN, pSVC->ucSTREAM, pSVC->ucMMS, pSVC->ucWIDGET, pSVC->ucPHONE, 
				pSVC->ucEMS, pSVC->ucBANK, pSVC->ucFV, pSVC->ucIM, pSVC->ucVT, pSVC->ucETC, 
				pSVC->ucCORP, pSVC->ucREGI);
	}
}

void PrintMonInfo(S32 lvl, st_MonInfo *pMonInfo)
{
	int		i;

	if(pMonInfo->uiCall[0] > 0) {
		log_print(lvl, "CALL Trial=%u Succ=%u", pMonInfo->uiCall[0], pMonInfo->uiCall[1]);
	}

	if(pMonInfo->uiAAA[0] > 0) {
		log_print(lvl, "AAA Trial=%u Succ=%u", pMonInfo->uiAAA[0], pMonInfo->uiAAA[1]);
	}

	if(pMonInfo->uiHSS[0] > 0) {
		log_print(lvl, "HSS Trial=%u Succ=%u", pMonInfo->uiHSS[0], pMonInfo->uiHSS[1]);
	}

	if(pMonInfo->uiLNS[0] > 0) {
		log_print(lvl, "LNS Trial=%u Succ=%u", pMonInfo->uiLNS[0], pMonInfo->uiLNS[1]);
	}

	for(i = 0; i < CURR_MON_SVC_IDX; i++)
	{
		if(pMonInfo->uiService[i][0] > 0) {
			log_print(lvl, "SVC=%s:%d Trial=%u Succ=%u", PrintSUBTYPE(SYSTEM_TYPE_SERVICE, i), i, pMonInfo->uiService[i][0], pMonInfo->uiService[i][1]);
		}
	}
}

void PrintMonInfoS(S32 lvl, st_MonInfoS *pMonInfoS)
{
	int		i;

	if(pMonInfoS->usCall[0] > 0) {
		log_print(lvl, "CALL Trial=%u Succ=%u", pMonInfoS->usCall[0], pMonInfoS->usCall[1]);
	}

	if(pMonInfoS->usAAA[0] > 0) {
		log_print(lvl, "AAA Trial=%u Succ=%u", pMonInfoS->usAAA[0], pMonInfoS->usAAA[1]);
	}

	if(pMonInfoS->usHSS[0] > 0) {
		log_print(lvl, "HSS Trial=%u Succ=%u", pMonInfoS->usHSS[0], pMonInfoS->usHSS[1]);
	}

	if(pMonInfoS->usLNS[0] > 0) {
		log_print(lvl, "LNS Trial=%u Succ=%u", pMonInfoS->usLNS[0], pMonInfoS->usLNS[1]);
	}

	for(i = 0; i < CURR_MON_SVC_IDX; i++)
	{
		if(pMonInfoS->usService[i][0] > 0) {
			log_print(lvl, "SVC=%s:%d Trial=%u Succ=%u", PrintSUBTYPE(SYSTEM_TYPE_SERVICE, i), i, pMonInfoS->usService[i][0], pMonInfoS->usService[i][1]);
		}
	}
}

void PrintDefect(S32 lvl, st_Defect *pMonDefect)
{
	int		i;
	for(i = 0; i < MAX_MON_DEFECT_IDX; i++)
	{
		if(pMonDefect->uiFail[i] > 0) {
			log_print(lvl, "DEFECT=%d CNT=%u", i, pMonDefect->uiFail[i]);
		}
	}
}

void PrintDefectS(S32 lvl, st_DefectS *pMonDefectS)
{
	int		i;
	for(i = 0; i < MAX_MON_DEFECT_IDX; i++)
	{
		if(pMonDefectS->usFail[i] > 0) {
			log_print(lvl, "DEFECT=%d CNT=%u", i, pMonDefectS->usFail[i]);
		}
	}
}

/*
 * $Log: o_svcmon_print.c,v $
 * Revision 1.4  2011/09/07 07:07:52  hhbaek
 * *** empty log message ***
 *
 * Revision 1.3  2011/09/07 04:32:00  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/31 16:08:08  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 08:58:49  dcham
 * *** empty log message ***
 *
 * Revision 1.14  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.2  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.13  2010/03/29 12:23:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.12  2010/02/25 11:13:45  dark264sh
 * O_SVCMON ROAM 처리
 *
 * Revision 1.11  2010/02/25 07:33:07  dark264sh
 * BSD => LNS로 변경
 *
 * Revision 1.10  2010/02/24 12:19:46  dark264sh
 * 망감시 헤더 변경에 따른 변경
 *
 * Revision 1.9  2009/10/21 11:45:08  pkg
 * 망감시 REGI 서비스 추가 및 define값 변경
 *
 * Revision 1.8  2009/09/07 11:14:43  pkg
 * 법인 서비스 추가
 *
 * Revision 1.7  2009/07/20 02:44:20  dark264sh
 * O_SVCMON SYSID 추가
 *
 * Revision 1.6  2009/07/10 12:16:22  dark264sh
 * *** empty log message ***
 *
 * Revision 1.5  2009/07/10 06:32:50  dark264sh
 * 망감시 관련 서비스 구분 변경
 *
 * Revision 1.4  2009/07/01 17:30:11  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2009/07/01 17:10:17  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/07/01 16:51:36  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2009/06/21 13:34:33  dark264sh
 * *** empty log message ***
 *
 */
