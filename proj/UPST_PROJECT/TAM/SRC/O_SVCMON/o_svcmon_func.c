/**		@file	o_svcmon_log.c
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_func.c,v 1.2 2011/08/31 16:08:07 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/08/31 16:08:07 $
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

// PROJECT
#include "common_stg.h"
#include "func_time_check.h"
#include "svcmon.h"

// TAM
#include "watch_mon.h"
#include "watch_filter.h"

#include "o_svcmon_func.h"
#include "o_svcmon_print.h"
#include "o_svcmon_util.h"
#include "o_svcmon_get.h"
#include "o_svcmon_set.h"
#include "o_svcmon_conf.h"

extern st_FuncTimeCheckList *pFUNC;

S32 dProcMON(stHASHOINFO *pDefHash, stHASHOINFO *pMonHash, stHASHOINFO *pNasIPHash, st_WatchMsg *pWatchMsg, st_MonList *pMonList, st_MonList *pBaseList, st_MonList_1Min *pMonList1Min, st_MonList_1Min *pBaseList1Min)
{
START_FUNC_TIME_CHECK(pFUNC, 70);
	S32				dRet, pcftype;

	st_MonCore		*pMonPCF;
	st_MonBTS		*pMonBTS;
	st_MonBSC		*pMonBSC;
	st_MonSvc		*pMonSvc;

	PrintWatchMsg(LOGN_INFO, "dProcMON RCV WATCH", 0, pWatchMsg);

	switch(pWatchMsg->usSvcL4Type)
	{
	case L4_DN_2G_NODN:
	case L4_DN_VOD_NODN:
	case L4_MMS_UP_NODN:
	case L4_MMS_DN_NODN:
	case L4_WIDGET_NODN:
	case L4_EMS_NO:
		log_print(LOGN_INFO, "SKIP SVC MSG SVCL4CODE=%u", pWatchMsg->usSvcL4Type);
		return 0;
	}

	if(pWatchMsg->ucRoamFlag) {
		dProcROAM(pDefHash, pMonHash, pNasIPHash, pWatchMsg, pMonList, pMonList1Min);
		return 0;
	}

	/* FA 예외 처리 | 0값이 사용되는데, 세팅이 잘못된 경우 */
	if(pWatchMsg->ucFA == 4) pWatchMsg->ucFA = 0;

	if((pcftype = getPCFType(pWatchMsg->ucPCFType)) < 0) {
		if(pWatchMsg->usMsgType == WATCH_TYPE_HSS && pWatchMsg->uiPCFIP == 0) {
			/* HSS 메시지의 경우 Call Mapping 안되는 경우도 전송 되므로 예외 처리 함 */
			/* uiPCFIP == 0 인 경우 Call Mapping에 실패한 메시지 */
			log_print(LOGN_INFO, "SKIP HSS MSG MSGTYPE=%u PCFIP=%u", pWatchMsg->usMsgType, pWatchMsg->uiPCFIP);
			dProcFirstList(pDefHash, pMonHash, pWatchMsg, &pMonList->stFirstMonList, &pMonList1Min->stFirstMonList, pcftype);
			return 0;
		} 
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcMON getPCFTYPE UNKNOWN PCFTYPE", pcftype, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcMON getPCFTYPE UNKNOWN PCFTYPE", pcftype, pWatchMsg);
		}
	}
	
	/* PCF */
	if((pMonPCF = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_PCF, &pMonList->stMonPCF[0], NULL)) != NULL) {
		if((dRet = dSetCore(pDefHash, pWatchMsg, pMonPCF)) < 0) {
//			PrintWatchMsg(LOGN_CRI, "dProcMON PCF dSetCore", dRet, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcMON PCF dSetCore", dRet, pWatchMsg);
		}
		else {
			if(pcftype < 0) pcftype = PCF_TYPE_EvDO;
		}
	}
	else {
//		PrintWatchMsg(LOGN_CRI, "dProcMON PCF getMonCore NULL", 0, pWatchMsg);
		PrintWatchMsg(LOGN_WARN, "dProcMON PCF getMonCore NULL", 0, pWatchMsg);
		if((pMonPCF = pMakePCF(pMonHash, pBaseList, pMonList, pWatchMsg)) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonPCF)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcMON PCF dSetCore", dRet, pWatchMsg);
			}
			pcftype = PCF_TYPE_EvDO;
			pWatchMsg->ucUnknownType += UNKNOWN_TYPE_PCF;
		}
		else {
			PrintWatchMsg(LOGN_WARN, "dProcMON PCF pMakePCF NULL", 0, pWatchMsg);
		}
	}

	if(pcftype == PCF_TYPE_EvDO) {
		/* BTS */
		if((pMonBTS = getMonBTS(pMonHash, pWatchMsg, &pMonList->stMonBTS[0])) != NULL) {
			if((dRet = dSetBTS(pDefHash, pWatchMsg, pMonBTS)) < 0) {
//				PrintWatchMsg(LOGN_CRI, "dProcMON BTS dSetBTS", dRet, pWatchMsg);
				PrintWatchMsg(LOGN_WARN, "dProcMON BTS dSetBTS", dRet, pWatchMsg);
			}
		}
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcMON BTS getMonBTS NULL", 0, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcMON BTS getMonBTS NULL", 0, pWatchMsg);
			if((pMonBTS = pMakeBTS(pMonHash, pBaseList, pMonList, pWatchMsg)) != NULL) {
				if((dRet = dSetBTS(pDefHash, pWatchMsg, pMonBTS)) < 0) {
					PrintWatchMsg(LOGN_WARN, "dProcMON BTS dSetBTS", dRet, pWatchMsg);
				}
				pWatchMsg->ucUnknownType += UNKNOWN_TYPE_BTS;	
			}
			else {
				PrintWatchMsg(LOGN_WARN, "dProcMON BTS pMakeBTS NULL", 0, pWatchMsg);
			}
		}

		/* BSC */
		if((pMonBSC = getMonBSC(pMonHash, pWatchMsg, &pMonList1Min->stMonBSC[0])) != NULL) {
			if((dRet = dSetBSC(pDefHash, pWatchMsg, pMonBSC)) < 0) {
//				PrintWatchMsg(LOGN_CRI, "dProcMON BSC dSetBSC", dRet, pWatchMsg);
				PrintWatchMsg(LOGN_WARN, "dProcMON BSC dSetBSC", dRet, pWatchMsg);
			}
		}
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcMON BSC getMonBSC NULL", 0, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcMON BSC getMonBSC NULL", 0, pWatchMsg);
			if((pMonBSC = pMakeBSC(pMonHash, pBaseList1Min, pMonList1Min, pWatchMsg)) != NULL) {
				if((dRet = dSetBSC(pDefHash, pWatchMsg, pMonBSC)) < 0) {
					PrintWatchMsg(LOGN_WARN, "dProcMON BSC dSetBSC", dRet, pWatchMsg);
				}
				pWatchMsg->ucUnknownType += UNKNOWN_TYPE_BSC;
            }
            else {
                PrintWatchMsg(LOGN_WARN, "dProcMON BSC pMakeBSC NULL", 0, pWatchMsg);
            }
	
		}
	}

	/* SVC */	
	if(pWatchMsg->usMsgType == WATCH_TYPE_SVC) {
		/* PHONE 서비스 예외 처리 */
		if(pWatchMsg->ucSvcIdx == SVC_IDX_PHONE) {
			pWatchMsg->uiSVCIP = 0;
		}

		if((pMonSvc = getMonSvc(pMonHash, pWatchMsg, &pMonList->stMonSvc[0])) != NULL) {
			if((dRet = dSetSvc(pDefHash, pWatchMsg, pMonSvc)) < 0) {
//				PrintWatchMsg(LOGN_CRI, "dProcMON SVC dSetSvc", dRet, pWatchMsg);
				PrintWatchMsg(LOGN_WARN, "dProcMON SVC dSetSvc", dRet, pWatchMsg);
			}
			
//			log_print(LOG_DEBUG, "**ZMOT** FIND!");

		}
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcMON SVC getMonSvc NULL", 0, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcMON SVC getMonSvc NULL", 0, pWatchMsg);

//			log_print(LOG_DEBUG, "**ZMOT** DON'T FIND!");
		}
	}

	/* FirstMon */
	dProcFirstList(pDefHash, pMonHash, pWatchMsg, &pMonList->stFirstMonList, &pMonList1Min->stFirstMonList, pcftype);

END_FUNC_TIME_CHECK(pFUNC, 70);

	return 0;
}

S32 dProcFirstList(stHASHOINFO *pDefHash, stHASHOINFO *pMonHash, st_WatchMsg *pWatchMsg, st_FirstMonList *pFirstMonList, st_FirstMonList *pFirstMonList1Min, S32 pcftype)
{
START_FUNC_TIME_CHECK(pFUNC, 71);
	int				dRet, type = pWatchMsg->usMsgType;

	st_MonCore		*pMonPDSN;
	st_MonCore		*pMonAAA;
	st_MonCore		*pMonHSS;
	st_MonCore		*pMonLNS;

	switch(type)
	{
	case WATCH_TYPE_RECALL:
	case WATCH_TYPE_A11:
		/* PDSN */
		if((pMonPDSN = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_PDSN, &pFirstMonList->stPDSN[0], &pFirstMonList1Min->stPDSN[0])) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonPDSN)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcFirstList A11:PDSN dSetCore", dRet, pWatchMsg);
			}
		}
		else {
			PrintWatchMsg(LOGN_WARN, "dProcFirstList A11:PDSN getMonCore NULL", 0, pWatchMsg);
		}
		/* First Mon */
		dProcFirst(pDefHash, pMonHash, pWatchMsg, &pFirstMonList->stFirstMon[0], pFirstMonList->usFirstListCnt, &pFirstMonList1Min->stFirstMon[0], pFirstMonList1Min->usFirstListCnt, pcftype);
		break;
	case WATCH_TYPE_AAA:
		/* PDSN */
		if((pMonPDSN = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_PDSN, &pFirstMonList->stPDSN[0], &pFirstMonList1Min->stPDSN[0])) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonPDSN)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcFirstList AAA:PDSN dSetCore", dRet, pWatchMsg);
			}
		}
		else {
			PrintWatchMsg(LOGN_WARN, "dProcFirstList AAA:PDSN getMonCore NULL", 0, pWatchMsg);
		}
		/* AAA */
		if((pMonAAA = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_AAA, &pFirstMonList->stAAA[0], &pFirstMonList1Min->stAAA[0])) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonAAA)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcFirstList AAA:AAA dSetCore", dRet, pWatchMsg);
			}
		}
		else {
			PrintWatchMsg(LOGN_WARN, "dProcFirstList AAA:AAA getMonCore NULL", 0, pWatchMsg);
		}
		/* First Mon */
		dProcFirst(pDefHash, pMonHash, pWatchMsg, &pFirstMonList->stFirstMon[0], pFirstMonList->usFirstListCnt, &pFirstMonList1Min->stFirstMon[0], pFirstMonList1Min->usFirstListCnt, pcftype);
		break;
	case WATCH_TYPE_HSS:
		/* HSS 메시지의 경우 Call Mapping 안되는 경우도 전송 되므로 예외 처리 함 */
		/* uiPDSNIP == 0 인 경우 Call Mapping에 실패한 메시지 */
		if(pWatchMsg->uiPDSNIP != 0) {
			/* PDSN */
			if((pMonPDSN = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_PDSN, &pFirstMonList->stPDSN[0], &pFirstMonList1Min->stPDSN[0])) != NULL) {
				if((dRet = dSetCore(pDefHash, pWatchMsg, pMonPDSN)) < 0) {
					PrintWatchMsg(LOGN_WARN, "dProcFirstList HSS:PDSN dSetCore", dRet, pWatchMsg);
				}
			}
			else {
				PrintWatchMsg(LOGN_WARN, "dProcFirstList HSS:PDSN getMonCore NULL", 0, pWatchMsg);
			}
			/* First Mon */
			dProcFirst(pDefHash, pMonHash, pWatchMsg, &pFirstMonList->stFirstMon[0], pFirstMonList->usFirstListCnt, &pFirstMonList1Min->stFirstMon[0], pFirstMonList1Min->usFirstListCnt, pcftype);
		}
		/* HSS */
		if((pMonHSS = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_HSS, &pFirstMonList->stHSS[0], NULL)) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonHSS)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcFirstList HSS:HSS dSetCore", dRet, pWatchMsg);
			}
		}
		else {
			PrintWatchMsg(LOGN_WARN, "dProcFirstList HSS:HSS getMonCore NULL", 0, pWatchMsg);
		}
		break;
	case WATCH_TYPE_LNS:
		/* PDSN */
		if((pMonPDSN = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_PDSN, &pFirstMonList->stPDSN[0], &pFirstMonList1Min->stPDSN[0])) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonPDSN)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcFirstList LNS:PDSN dSetCore", dRet, pWatchMsg);
			}
		}
		else {
			PrintWatchMsg(LOGN_CRI, "dProcFirstList LNS:PDSN getMonCore NULL", 0, pWatchMsg);
		}
		/* LNS */
		if((pMonLNS = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_LNS, &pFirstMonList->stLNS[0], NULL)) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonLNS)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcFirstList LNS:LNS dSetCore", dRet, pWatchMsg);
			}
		}
		else {
			PrintWatchMsg(LOGN_WARN, "dProcFirstList LNS:LNS getMonCore NULL", 0, pWatchMsg);
		}
		/* First Mon */
		dProcFirst(pDefHash, pMonHash, pWatchMsg, &pFirstMonList->stFirstMon[0], pFirstMonList->usFirstListCnt, &pFirstMonList->stFirstMon[0], pFirstMonList->usFirstListCnt, pcftype);
		break;
	case WATCH_TYPE_SVC:
		/* PDSN */
		if((pMonPDSN = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_PDSN, &pFirstMonList->stPDSN[0], &pFirstMonList1Min->stPDSN[0])) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonPDSN)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcFirstList SVC:PDSN dSetCore", dRet, pWatchMsg);
			}
		}
		else {
			PrintWatchMsg(LOGN_WARN, "dProcFirstList SVC:PDSN getMonCore NULL", 0, pWatchMsg);
		}
		/* First Mon */
		dProcFirst(pDefHash, pMonHash, pWatchMsg, &pFirstMonList->stFirstMon[0], pFirstMonList->usFirstListCnt, &pFirstMonList->stFirstMon[0], pFirstMonList->usFirstListCnt, pcftype);
		break;
	default:
		PrintWatchMsg(LOGN_WARN, "dProcFirstList UNKNOWN MSGTYPE", type, pWatchMsg);
		break;
	}
END_FUNC_TIME_CHECK(pFUNC, 71);

	return 0;
}

S32 dProcFirst(stHASHOINFO *pDefHash, stHASHOINFO *pMonHash, st_WatchMsg *pWatchMsg, st_FirstMon *aFirstMon, S32 cnt, st_FirstMon *aFirstMon1Min, S32 cnt1Min, S32 pcftype)
{
START_FUNC_TIME_CHECK(pFUNC, 72);
	int					dRet;
	U8					office;
	st_FirstMon			*pFirstSec;
	st_FirstMon			*pFirstFA;
	st_FirstMon			*pFirstBTS;
	st_FirstMon			*pFirstBSC;
	st_FirstMon			*pFirstPCF;
	st_FirstMon			*pFirstSVC;

	if(pcftype == PCF_TYPE_EvDO) 
	{
		office = ((pWatchMsg->ucUnknownType & UNKNOWN_TYPE_BTS) == UNKNOWN_TYPE_BTS) ? OFFICE_UNKNOWN : pWatchMsg->ucOffice;
		/* SECTOR */
		if((pFirstSec = getFirstMon(pMonHash, office, SYSTEM_TYPE_SECTOR, 0, &aFirstMon[0])) != NULL) {
			if((dRet = dSetFirstMon(pDefHash, pWatchMsg, pFirstSec)) < 0) {
//				PrintWatchMsg(LOGN_CRI, "dProcFirst SECTOR dSetFirstMon", dRet, pWatchMsg);
				PrintWatchMsg(LOGN_WARN, "dProcFirst SECTOR dSetFirstMon", dRet, pWatchMsg);
			}
		}
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcFirst SECTOR getFirstMon NULL", 0, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcFirst SECTOR getFirstMon NULL", 0, pWatchMsg);
		}

		/* FA */
		if((pFirstFA = getFirstMon(pMonHash, office, SYSTEM_TYPE_FA, 0, &aFirstMon[0])) != NULL) {
			if((dRet = dSetFirstMon(pDefHash, pWatchMsg, pFirstFA)) < 0) {
//				PrintWatchMsg(LOGN_CRI, "dProcFirst FA dSetFirstMon", dRet, pWatchMsg);
				PrintWatchMsg(LOGN_WARN, "dProcFirst FA dSetFirstMon", dRet, pWatchMsg);
			}
		}
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcFirst FA getFirstMon NULL", 0, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcFirst FA getFirstMon NULL", 0, pWatchMsg);
		}

		/* BTS */
		if((pFirstBTS = getFirstMon(pMonHash, office, SYSTEM_TYPE_BTS, 0, &aFirstMon[0])) != NULL) {
			if((dRet = dSetFirstMon(pDefHash, pWatchMsg, pFirstBTS)) < 0) {
//				PrintWatchMsg(LOGN_CRI, "dProcFirst BTS dSetFirstMon", dRet, pWatchMsg);
				PrintWatchMsg(LOGN_WARN, "dProcFirst BTS dSetFirstMon", dRet, pWatchMsg);
			}
		}
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcFirst BTS getFirstMon NULL", 0, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcFirst BTS getFirstMon NULL", 0, pWatchMsg);
		}

		office = ((pWatchMsg->ucUnknownType & UNKNOWN_TYPE_BSC) == UNKNOWN_TYPE_BSC) ? OFFICE_UNKNOWN : pWatchMsg->ucOffice;
		/* BSC */
		if((pFirstBSC = getFirstMon(pMonHash, office, SYSTEM_TYPE_BSC, 0, &aFirstMon[0])) != NULL) {
			if((dRet = dSetFirstMon(pDefHash, pWatchMsg, pFirstBSC)) < 0) {
//				PrintWatchMsg(LOGN_CRI, "dProcFirst BSC dSetFirstMon", dRet, pWatchMsg);
				PrintWatchMsg(LOGN_WARN, "dProcFirst BSC dSetFirstMon", dRet, pWatchMsg);
			}
		}
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcFirst BSC getFirstMon NULL", 0, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcFirst BSC getFirstMon NULL", 0, pWatchMsg);
		}
	}

	office = ((pWatchMsg->ucUnknownType & UNKNOWN_TYPE_PCF) == UNKNOWN_TYPE_PCF) ? OFFICE_UNKNOWN : pWatchMsg->ucOffice;
	/* PCF */
	if((pFirstPCF = getFirstMon(pMonHash, office, SYSTEM_TYPE_PCF, 0, &aFirstMon[0])) != NULL) {
		if((dRet = dSetFirstMon(pDefHash, pWatchMsg, pFirstPCF)) < 0) {
//			PrintWatchMsg(LOGN_CRI, "dProcFirst PCF dSetFirstMon", dRet, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcFirst PCF dSetFirstMon", dRet, pWatchMsg);
		}
	}
	else {
//		PrintWatchMsg(LOGN_CRI, "dProcFirst PCF getFirstMon NULL", 0, pWatchMsg);
		PrintWatchMsg(LOGN_WARN, "dProcFirst PCF getFirstMon NULL", 0, pWatchMsg);
	}

	/* Service */
	if(pWatchMsg->usMsgType == WATCH_TYPE_SVC) {
		if((pFirstSVC = getFirstMon(pMonHash, 0, SYSTEM_TYPE_SERVICE, pWatchMsg->ucSvcIdx, &aFirstMon[0])) != NULL) {
			if((dRet = dSetFirstMon(pDefHash, pWatchMsg, pFirstSVC)) < 0) {
//				PrintWatchMsg(LOGN_CRI, "dProcFirst SVC dSetFirstMon", dRet, pWatchMsg);
				PrintWatchMsg(LOGN_WARN, "dProcFirst SVC dSetFirstMon", dRet, pWatchMsg);
			}
		}
		else {
//			PrintWatchMsg(LOGN_CRI, "dProcFirst SVC getFirstMon NULL", 0, pWatchMsg);
			PrintWatchMsg(LOGN_WARN, "dProcFirst SVC getFirstMon NULL", 0, pWatchMsg);
		}
	}
END_FUNC_TIME_CHECK(pFUNC, 72);
	
	return 0;
}

S32 dProcROAM(stHASHOINFO *pDefHash, stHASHOINFO *pMonHash, stHASHOINFO *pNasIPHash, st_WatchMsg *pWatchMsg, st_MonList *pMonList, st_MonList_1Min *pMonList1Min)
{
START_FUNC_TIME_CHECK(pFUNC, 73);
	S32				dRet;
	st_FirstMonList	*pFirstMonList = &pMonList->stFirstMonList;
	st_FirstMonList	*pFirstMonList1Min = &pMonList1Min->stFirstMonList;
	
	st_MonCore		*pMonAAA;
	st_MonCore		*pMonLNS;
	st_FirstMon		*pFirstSVC;
	st_MonSvc		*pMonSvc;

	/* AAA */
	if((pMonAAA = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_ROAMAAA, &pFirstMonList->stAAA[0], &pFirstMonList1Min->stAAA[0])) != NULL) {
		if((dRet = dSetCore(pDefHash, pWatchMsg, pMonAAA)) < 0) {
			PrintWatchMsg(LOGN_WARN, "dProcROAM AAA dSetCore", dRet, pWatchMsg);
		}   
	}
	else {
		PrintWatchMsg(LOGN_WARN, "dProcROAM AAA getMonCore NULL", 0, pWatchMsg);
	} 
	if(pWatchMsg->uiLNSIP > 0)
	{
		/* LNS */
		if((pMonLNS = getMonCore(pMonHash, pWatchMsg, SYSTEM_TYPE_LNS, &pFirstMonList->stLNS[0], NULL)) != NULL) {
			if((dRet = dSetCore(pDefHash, pWatchMsg, pMonLNS)) < 0) {
				PrintWatchMsg(LOGN_WARN, "dProcROAM LNS dSetCore", dRet, pWatchMsg);
			}   
		}
		else {
			PrintWatchMsg(LOGN_WARN, "dProcROAM LNS getMonCore NULL", 0, pWatchMsg);
		} 
	}
	/* ROAM FirstMon */
	if((pFirstSVC = getFirstMon(pMonHash, 0, SYSTEM_TYPE_SERVICE, SVC_IDX_ROAM, &pFirstMonList->stFirstMon[0])) != NULL) {
		if((dRet = dSetFirstMon(pDefHash, pWatchMsg, pFirstSVC)) < 0) {
			PrintWatchMsg(LOGN_WARN, "dProcROAM ROAM dSetFirstMon", dRet, pWatchMsg);
		}
	}
	else {                                                                                                    
		PrintWatchMsg(LOGN_WARN, "dProcROAM ROAM getFirstMon NULL", 0, pWatchMsg);
	}
	/* ROAM */
	if((pMonSvc = getMonRoam(pMonHash, pNasIPHash, pWatchMsg, &pMonList->stMonSvc[0])) != NULL) {
		if((dRet = dSetRoam(pDefHash, pWatchMsg, pMonSvc)) < 0) {
			PrintWatchMsg(LOGN_WARN, "dProcROAM ROAM dSetRoam", dRet, pWatchMsg);
		}
	}
	else {
		PrintWatchMsg(LOGN_WARN, "dProcROAM A11AAA:ROAM getMonRoam NULL", 0, pWatchMsg);
	}

END_FUNC_TIME_CHECK(pFUNC, 73);

	return 0;
}

/*
 * $Log: o_svcmon_func.c,v $
 * Revision 1.2  2011/08/31 16:08:07  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 08:58:48  dcham
 * *** empty log message ***
 *
 * Revision 1.27  2011/05/13 14:06:48  jsyoon
 * *** empty log message ***
 *
 * Revision 1.26  2011/04/26 21:02:41  innaei
 * *** empty log message ***
 *
 * Revision 1.25  2011/04/25 17:38:41  jsyoon
 * *** empty log message ***
 *
 * Revision 1.24  2011/04/20 06:20:41  innaei
 * *** empty log message ***
 *
 * Revision 1.23  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.2  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.1.1.1  2010/08/23 01:13:10  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.22  2010/03/31 07:31:35  dark264sh
 * *** empty log message ***
 *
 * Revision 1.21  2010/03/31 07:29:17  dark264sh
 * *** empty log message ***
 *
 * Revision 1.20  2010/03/29 12:23:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.19  2010/03/04 13:02:36  dark264sh
 * O_SVCMON LNS 예외 처리
 *
 * Revision 1.18  2010/03/04 06:00:53  dark264sh
 * ROAM NASIP NetMask 처리
 *
 * Revision 1.17  2010/02/25 11:13:45  dark264sh
 * O_SVCMON ROAM 처리
 *
 * Revision 1.16  2010/02/25 07:33:07  dark264sh
 * BSD => LNS로 변경
 *
 * Revision 1.15  2010/02/24 12:19:46  dark264sh
 * 망감시 헤더 변경에 따른 변경
 *
 * Revision 1.14  2009/09/10 18:13:54  pkg
 * O_SVCMON FA=4인 경우 FA=0으로 변경 | 예외처리
 *
 * Revision 1.13  2009/08/24 00:14:20  pkg
 * *** empty log message ***
 *
 * Revision 1.12  2009/08/23 23:54:30  pkg
 * *** empty log message ***
 *
 * Revision 1.11  2009/08/23 23:47:43  pkg
 * *** empty log message ***
 *
 * Revision 1.10  2009/08/23 23:37:17  pkg
 * O_SVCMON HSS 예외 처리
 *
 * Revision 1.9  2009/08/22 17:54:31  pkg
 * O_SVCMON L4_EMS_NO 예외 처리
 *
 * Revision 1.8  2009/07/23 13:38:32  dark264sh
 * O_SVCMON NODN 메시지 망감시에서 제외하는 처리
 *
 * Revision 1.7  2009/07/12 12:04:01  dark264sh
 * O_SVCMON DEF_PLATFORM_PHONE 관련 예외 처리 추가
 *
 * Revision 1.6  2009/07/10 06:31:55  dark264sh
 * 망감시 관련 서비스 구분 변경
 *
 * Revision 1.5  2009/07/01 17:10:17  dark264sh
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
