/**		@file	o_svcmon_log.c
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_alarm.c,v 1.2 2011/08/31 16:08:07 dhkim Exp $
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
#include "msgdef.h"

// TAM
#include "watch_mon.h"
#include "watch_filter.h"

// OAM
#include "almstat.h"

#include "o_svcmon_alarm.h"
#include "o_svcmon_util.h"
#include "o_svcmon_get.h"
#include "o_svcmon_set.h"
#include "o_svcmon_msgq.h"

extern st_FuncTimeCheckList *pFUNC;


S32 dAlarmMON(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonList *pMonList, st_MonList *pOldMonList)
{
START_FUNC_TIME_CHECK(pFUNC, 10);

	st_FirstMonList		*pFirstMonList = &pMonList->stFirstMonList;

	/* AAA */
	dCheckAAA(pThresHash, &pFirstMonList->stAAA[0], pFirstMonList->usAAAListCnt, pFirstMonList->lTime);

	/* HSS */
	dCheckHSS(pThresHash, &pFirstMonList->stHSS[0], pFirstMonList->usHSSListCnt, pFirstMonList->lTime);

	/* LNS */
	dCheckLNS(pThresHash, &pFirstMonList->stLNS[0], pFirstMonList->usLNSListCnt, pFirstMonList->lTime);

	/* PDSN */
	dCheckPDSN(pThresHash, &pFirstMonList->stPDSN[0], pFirstMonList->usPDSNListCnt, pFirstMonList->lTime);

	/* PCF */
	dCheckPCF(pThresHash, pHash, &pMonList->stMonPCF[0], pMonList->usPCFCnt, &pFirstMonList->stFirstMon[0], pMonList->lTime);

#if 0
	/* BSC */
	dCheckBSC(pThresHash, pHash, &pMonList->stMonBSC[0], pMonList->usBSCCnt, &pFirstMonList->stFirstMon[0], pMonList->lTime);
#endif

	/* BTS */
	dCheckBTS(pThresHash, pHash, &pMonList->stMonBTS[0], pMonList->usBTSCnt, &pFirstMonList->stFirstMon[0], pMonList->lTime);

	/* Service */
	dCheckSVC(pThresHash, pHash, &pMonList->stMonSvc[0], &pOldMonList->stMonSvc[0], pMonList->usSvcCnt, &pFirstMonList->stFirstMon[0], pMonList->lTime);

END_FUNC_TIME_CHECK(pFUNC, 10);
	return 0;
}

S32 dAlarmMON1Min(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonList_1Min *pMonList)
{
START_FUNC_TIME_CHECK(pFUNC, 10);
	st_FirstMonList		*pFirstMonList = &pMonList->stFirstMonList;

	/* AAA */
	dCheckAAA(pThresHash, &pFirstMonList->stAAA[0], pFirstMonList->usAAAListCnt, pFirstMonList->lTime);

	/* PDSN */
	dCheckPDSN(pThresHash, &pFirstMonList->stPDSN[0], pFirstMonList->usPDSNListCnt, pFirstMonList->lTime);

	/* BSC */
	dCheckBSC(pThresHash, pHash, &pMonList->stMonBSC[0], pMonList->usBSCCnt, &pFirstMonList->stFirstMon[0], pMonList->lTime);

END_FUNC_TIME_CHECK(pFUNC, 10);
	return 0;
}

S32 dCheckPDSN(stHASHOINFO *pThresHash, st_MonCore *aPDSN, S32 cnt, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 11);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	st_MonCore		*pPDSN;
	st_MonInfo		*pMonInfo;
	st_MonAlarm		*pMonAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
    {
        pPDSN = &aPDSN[i];
        pMonInfo = &pPDSN->stMonInfo;
        pMonAlarm = &pPDSN->stMonAlarm;

        /* CALL */
        if((pData = getBaseValue(pThresHash, pPDSN->ucOffice, pPDSN->ucSysType, DEF_ALARMTYPE_CALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiCall[0];
            succcnt = pMonInfo->uiCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=CALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(pPDSN->ucOffice), pPDSN->ucOffice, PrintSYSTYPE(pPDSN->ucSysType), pPDSN->ucSysType);
        }

		/* RECALL */ /* INYOUNG */
        if((pData = getBaseValue(pThresHash, pPDSN->ucOffice, pPDSN->ucSysType, DEF_ALARMTYPE_RECALL, 0)) != NULL){
            trialcnt = pMonInfo->uiReCall[0];
            succcnt = pMonInfo->uiReCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
            }
        }
        else{
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=RECALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(pPDSN->ucOffice), pPDSN->ucOffice, PrintSYSTYPE(pPDSN->ucSysType), pPDSN->ucSysType);
        
        }

        /* AAA */
        if((pData = getBaseValue(pThresHash, pPDSN->ucOffice, pPDSN->ucSysType, DEF_ALARMTYPE_AAA, 0)) != NULL) {
            trialcnt = pMonInfo->uiAAA[0];
            succcnt = pMonInfo->uiAAA[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=AAA",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(pPDSN->ucOffice), pPDSN->ucOffice, PrintSYSTYPE(pPDSN->ucSysType), pPDSN->ucSysType);
        }

        /* HSS */
        if((pData = getBaseValue(pThresHash, pPDSN->ucOffice, pPDSN->ucSysType, DEF_ALARMTYPE_HSS, 0)) != NULL) {
            trialcnt = pMonInfo->uiHSS[0];
            succcnt = pMonInfo->uiHSS[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pMonAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=HSS",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(pPDSN->ucOffice), pPDSN->ucOffice, PrintSYSTYPE(pPDSN->ucSysType), pPDSN->ucSysType);
        }

        /* Service */
        for(j = 0; j < CURR_MON_SVC_IDX; j++)
        {
            if(j == SVC_IDX_ROAM) continue;
            alarmtype = DEF_ALARMTYPE_MENU + j;
            if((pData = getBaseValue(pThresHash, pPDSN->ucOffice, pPDSN->ucSysType, alarmtype, 0)) != NULL) {
                trialcnt = pMonInfo->uiService[j][0];
                succcnt = pMonInfo->uiService[j][1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=SVC:%s:%u",
                        __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(pPDSN->ucOffice), 
                        pPDSN->ucOffice, PrintSYSTYPE(pPDSN->ucSysType), pPDSN->ucSysType, PrintALARMTYPE(alarmtype), alarmtype);
            }
        }
    }
END_FUNC_TIME_CHECK(pFUNC, 11);

	return 0;
}

S32 dCheckAAA(stHASHOINFO *pThresHash, st_MonCore *aAAA, S32 cnt, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 12);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	st_MonCore		*pAAA;
	st_MonInfo		*pMonInfo;
	st_MonAlarm		*pMonAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
	{
		pAAA = &aAAA[i];
		pMonInfo = &pAAA->stMonInfo;
		pMonAlarm = &pAAA->stMonAlarm;

		/* AAA */
		if((pData = getBaseValue(pThresHash, pAAA->ucOffice, pAAA->ucSysType, DEF_ALARMTYPE_AAA, 0)) != NULL) {
			trialcnt = pMonInfo->uiAAA[0];
			succcnt = pMonInfo->uiAAA[1];
			if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
				dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
			} 
		}
		else {
			log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=AAA",
					__FILE__, __FUNCTION__, __LINE__,
					PrintOFFICE(pAAA->ucOffice), pAAA->ucOffice, PrintSYSTYPE(pAAA->ucSysType), pAAA->ucSysType);
		}

		if(pAAA->ucSysType == SYSTEM_TYPE_ROAMAAA)
		{
			/* CALL */
			if((pData = getBaseValue(pThresHash, pAAA->ucOffice, pAAA->ucSysType, DEF_ALARMTYPE_CALL, 0)) != NULL) {
				trialcnt = pMonInfo->uiCall[0];
				succcnt = pMonInfo->uiCall[1];
				if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
					dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
				} 
			}
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=CALL",
                        __FILE__, __FUNCTION__, __LINE__,
                        PrintOFFICE(pAAA->ucOffice), pAAA->ucOffice, PrintSYSTYPE(pAAA->ucSysType), pAAA->ucSysType);
           }

			/* ReCall */ /* INYOUNG */
			if((pData = getBaseValue(pThresHash, pAAA->ucOffice, pAAA->ucSysType, DEF_ALARMTYPE_RECALL, 0)) != NULL) {
				trialcnt = pMonInfo->uiReCall[0];
				succcnt = pMonInfo->uiReCall[1];
				if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
					dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
				}
			}
			else {
				log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=RECALL",
						__FILE__, __FUNCTION__, __LINE__,
						PrintOFFICE(pAAA->ucOffice), pAAA->ucOffice, PrintSYSTYPE(pAAA->ucSysType), pAAA->ucSysType);
			}

#if 0
			/* LNS */
			if((pData = getBaseValue(pThresHash, pAAA->ucOffice, pAAA->ucSysType, DEF_ALARMTYPE_LNS, 0)) != NULL) {
				trialcnt = pMonInfo->uiLNS[0];
				succcnt = pMonInfo->uiLNS[1];
				if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
					dSetAlarmFlag(ALARM_TYPE_LNS, 0, alarmvalue, pMonAlarm);
				} 
			}
			else {
				log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=LNS",
						__FILE__, __FUNCTION__, __LINE__,
						PrintOFFICE(pAAA->ucOffice), pAAA->ucOffice, PrintSYSTYPE(pAAA->ucSysType), pAAA->ucSysType);
			}
#endif

			/* Service */
			for(j = 0; j < CURR_MON_SVC_IDX; j++)
			{
				if(j == SVC_IDX_ROAM) continue;
				alarmtype = DEF_ALARMTYPE_MENU + j;
				if((pData = getBaseValue(pThresHash, pAAA->ucOffice, pAAA->ucSysType, alarmtype, 0)) != NULL) {
					trialcnt = pMonInfo->uiService[j][0];
					succcnt = pMonInfo->uiService[j][1];
					if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
						dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
					} 
				}
				else {
					log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=SVC:%s:%u",
							__FILE__, __FUNCTION__, __LINE__, PrintOFFICE(pAAA->ucOffice), 
							pAAA->ucOffice, PrintSYSTYPE(pAAA->ucSysType), pAAA->ucSysType, PrintALARMTYPE(alarmtype), alarmtype);
				}
			}
		}
	}
END_FUNC_TIME_CHECK(pFUNC, 12);

	return 0;
}

S32 dCheckHSS(stHASHOINFO *pThresHash, st_MonCore *aHSS, S32 cnt, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 13);
	int				i, alarmvalue;
	U32				trialcnt, succcnt;
	st_MonCore		*pHSS;
	st_MonInfo		*pMonInfo;
	st_MonAlarm		*pMonAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
	{
		pHSS = &aHSS[i];
		pMonInfo = &pHSS->stMonInfo;
		pMonAlarm = &pHSS->stMonAlarm;

		/* HSS */
		if((pData = getBaseValue(pThresHash, pHSS->ucOffice, pHSS->ucSysType, DEF_ALARMTYPE_HSS, 0)) != NULL) {
			trialcnt = pMonInfo->uiHSS[0];
			succcnt = pMonInfo->uiHSS[1];
			if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
				dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pMonAlarm);
			} 
		}
		else {
			log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=HSS",
					__FILE__, __FUNCTION__, __LINE__,
					PrintOFFICE(pHSS->ucOffice), pHSS->ucOffice, PrintSYSTYPE(pHSS->ucSysType), pHSS->ucSysType);
		}
	}
END_FUNC_TIME_CHECK(pFUNC, 13);

	return 0;
}

S32 dCheckLNS(stHASHOINFO *pThresHash, st_MonCore *aLNS, S32 cnt, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 14);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	st_MonCore		*pLNS;
	st_MonInfo		*pMonInfo;
	st_MonAlarm		*pMonAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
    {
        pLNS = &aLNS[i];
        pMonInfo = &pLNS->stMonInfo;
        pMonAlarm = &pLNS->stMonAlarm;

        /* CALL */
        if((pData = getBaseValue(pThresHash, pLNS->ucOffice, pLNS->ucSysType, DEF_ALARMTYPE_CALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiCall[0];
            succcnt = pMonInfo->uiCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=CALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(pLNS->ucOffice), pLNS->ucOffice, PrintSYSTYPE(pLNS->ucSysType), pLNS->ucSysType);
        }

        /* ReCall */ /* INYOUNG */
        if((pData = getBaseValue(pThresHash, pLNS->ucOffice, pLNS->ucSysType, DEF_ALARMTYPE_RECALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiReCall[0];
            succcnt = pMonInfo->uiReCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
            }
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=RECALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(pLNS->ucOffice), pLNS->ucOffice, PrintSYSTYPE(pLNS->ucSysType), pLNS->ucSysType);
        }

        /* AAA */
        if((pData = getBaseValue(pThresHash, pLNS->ucOffice, pLNS->ucSysType, DEF_ALARMTYPE_AAA, 0)) != NULL) {
            trialcnt = pMonInfo->uiAAA[0];
            succcnt = pMonInfo->uiAAA[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=AAA",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(pLNS->ucOffice), pLNS->ucOffice, PrintSYSTYPE(pLNS->ucSysType), pLNS->ucSysType);
        }

#if 0
        /* LNS */
        if((pData = getBaseValue(pThresHash, pLNS->ucOffice, pLNS->ucSysType, DEF_ALARMTYPE_LNS, 0)) != NULL) {
            trialcnt = pMonInfo->uiLNS[0];
            succcnt = pMonInfo->uiLNS[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_LNS, 0, alarmvalue, pMonAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=LNS",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(pLNS->ucOffice), pLNS->ucOffice, PrintSYSTYPE(pLNS->ucSysType), pLNS->ucSysType);
        }
#endif

        /* Service */
        for(j = 0; j < CURR_MON_SVC_IDX; j++)
        {
            if(j == SVC_IDX_ROAM) continue;
            alarmtype = DEF_ALARMTYPE_MENU + j;
            if((pData = getBaseValue(pThresHash, pLNS->ucOffice, pLNS->ucSysType, alarmtype, 0)) != NULL) {
                trialcnt = pMonInfo->uiService[j][0];
                succcnt = pMonInfo->uiService[j][1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=SVC:%s:%u",
                        __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(pLNS->ucOffice), 
                        pLNS->ucOffice, PrintSYSTYPE(pLNS->ucSysType), pLNS->ucSysType, PrintALARMTYPE(alarmtype), alarmtype);
            }
        }
    }
END_FUNC_TIME_CHECK(pFUNC, 14);

	return 0;
}

S32 dCheckPCF(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonCore *aPCF, S32 cnt, st_FirstMon *aFirstMon, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 15);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	S32				office, systype, subtype;
	st_MonCore		*pPCF;
	st_MonInfo		*pMonInfo;
	st_MonAlarm		*pMonAlarm;
	st_FirstMon		*pFirstMon;
	st_MonAlarm		*pFirstAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
    {
        pPCF = &aPCF[i];
        pMonInfo = &pPCF->stMonInfo;
        pMonAlarm = &pPCF->stMonAlarm;

        office = pPCF->ucOffice;
        systype = SYSTEM_TYPE_PCF;
        subtype = 0;

        pFirstMon = getFirstMon(pHash, office, systype, subtype, &aFirstMon[0]);
        pFirstAlarm = &pFirstMon->stMonAlarm;

        /* CALL */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_CALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiCall[0];
            succcnt = pMonInfo->uiCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=CALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }
        
        /* ReCall */ /* INYOUNG */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_RECALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiReCall[0];
            succcnt = pMonInfo->uiReCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pFirstAlarm);
            }
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=RECALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* AAA */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_AAA, 0)) != NULL) {
            trialcnt = pMonInfo->uiAAA[0];
            succcnt = pMonInfo->uiAAA[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=AAA",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* HSS */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_HSS, 0)) != NULL) {
            trialcnt = pMonInfo->uiHSS[0];
            succcnt = pMonInfo->uiHSS[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=HSS",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* Service */
        for(j = 0; j < CURR_MON_SVC_IDX; j++)
        {
            if(j == SVC_IDX_ROAM) continue;
            alarmtype = DEF_ALARMTYPE_MENU + j;
            if((pData = getBaseValue(pThresHash, office, systype, alarmtype, 0)) != NULL) {
                trialcnt = pMonInfo->uiService[j][0];
                succcnt = pMonInfo->uiService[j][1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pFirstAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=SVC:%s:%u",
                        __FILE__, __FUNCTION__, __LINE__,
                        PrintOFFICE(office), office, PrintSYSTYPE(systype), systype, PrintALARMTYPE(alarmtype), alarmtype);
            }
        }
    }
END_FUNC_TIME_CHECK(pFUNC, 15);

	return 0;
}

S32 dCheckBSC(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonBSC *aBSC, S32 cnt, st_FirstMon *aFirstMon, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 16);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	S32				office, systype, subtype;
	st_MonBSC		*pBSC;
	st_MonInfo		*pMonInfo;
	st_MonAlarm		*pMonAlarm;
	st_FirstMon		*pFirstMon;
	st_MonAlarm		*pFirstAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
    {
        pBSC = &aBSC[i];
        pMonInfo = &pBSC->stMonInfo;
        pMonAlarm = &pBSC->stMonAlarm;

        office = (*(st_SubBSC *)&pBSC->uiBSC).ucOffice;
        systype = SYSTEM_TYPE_BSC;
        subtype = 0;

        pFirstMon = getFirstMon(pHash, office, systype, subtype, &aFirstMon[0]);
        pFirstAlarm = &pFirstMon->stMonAlarm;

        /* CALL */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_CALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiCall[0];
            succcnt = pMonInfo->uiCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=CALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* ReCall */ /* INYOUNG */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_RECALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiReCall[0];
            succcnt = pMonInfo->uiReCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pFirstAlarm);
            }
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=RECALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* AAA */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_AAA, 0)) != NULL) {
            trialcnt = pMonInfo->uiAAA[0];
            succcnt = pMonInfo->uiAAA[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=AAA",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* HSS */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_HSS, 0)) != NULL) {
            trialcnt = pMonInfo->uiHSS[0];
            succcnt = pMonInfo->uiHSS[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=HSS",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* Service */
        for(j = 0; j < CURR_MON_SVC_IDX; j++)
        {
            if(j == SVC_IDX_ROAM) continue;
            alarmtype = DEF_ALARMTYPE_MENU + j;
            if((pData = getBaseValue(pThresHash, office, systype, alarmtype, 0)) != NULL) {
                trialcnt = pMonInfo->uiService[j][0];
                succcnt = pMonInfo->uiService[j][1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pFirstAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=SVC:%s:%u",
                        __FILE__, __FUNCTION__, __LINE__,
                        PrintOFFICE(office), office, PrintSYSTYPE(systype), systype, PrintALARMTYPE(alarmtype), alarmtype);
            }
        }
    }
END_FUNC_TIME_CHECK(pFUNC, 16);

	return 0;
}

S32 dCheckBTS(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonBTS *aBTS, S32 cnt, st_FirstMon *aFirstMon, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 17);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	S32				office, systype, subtype;
	st_MonBTS		*pBTS;
	st_MonInfo		*pMonInfo;
	st_MonAlarm		*pMonAlarm;
	st_FirstMon		*pFirstMon, *pFirstMonFA, *pFirstMonSec;
	st_MonAlarm		*pFirstAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
    {
        pBTS = &aBTS[i];
        pMonInfo = &pBTS->stMonInfo;
        pMonAlarm = &pBTS->stMonAlarm;

        office = (*(st_SubBTS *)&pBTS->ullBTS).ucOffice;
        systype = SYSTEM_TYPE_BTS;
        subtype = 0;

        pFirstMon = getFirstMon(pHash, office, systype, subtype, &aFirstMon[0]);
        pFirstAlarm = &pFirstMon->stMonAlarm;

        /*
           systype = SYSTEM_TYPE_FA;
           pFirstMonFA = getFirstMon(pHash, office, systype, subtype, &aFirstMon[0]);
         */
        pFirstMonFA = getFirstMon(pHash, office, SYSTEM_TYPE_FA, subtype, &aFirstMon[0]);

        /*
           systype = SYSTEM_TYPE_SECTOR;
           pFirstMonSec = getFirstMon(pHash, office, systype, subtype, &aFirstMon[0]);
         */
        pFirstMonSec = getFirstMon(pHash, office, SYSTEM_TYPE_SECTOR, subtype, &aFirstMon[0]);
        dCheckFA(pThresHash, &pBTS->stMonFA[0], MAX_MON_FA_CNT, pFirstMonFA, pFirstMonSec, office, stattime);

        /* CALL */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_CALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiCall[0];
            succcnt = pMonInfo->uiCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=CALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }
        
        /* ReCall */ /* INYOUNG */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_RECALL, 0)) != NULL) {
            trialcnt = pMonInfo->uiReCall[0];
            succcnt = pMonInfo->uiReCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pFirstAlarm);
            }
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=RECALL",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* AAA */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_AAA, 0)) != NULL) {
            trialcnt = pMonInfo->uiAAA[0];
            succcnt = pMonInfo->uiAAA[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=AAA",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* HSS */
        if((pData = getBaseValue(pThresHash, office, systype, DEF_ALARMTYPE_HSS, 0)) != NULL) {
            trialcnt = pMonInfo->uiHSS[0];
            succcnt = pMonInfo->uiHSS[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=HSS",
                    __FILE__, __FUNCTION__, __LINE__,
                    PrintOFFICE(office), office, PrintSYSTYPE(systype), systype);
        }

        /* Service */
        for(j = 0; j < CURR_MON_SVC_IDX; j++)
        {
            if(j == SVC_IDX_ROAM) continue;
            alarmtype = DEF_ALARMTYPE_MENU + j;
            if((pData = getBaseValue(pThresHash, office, systype, alarmtype, 0)) != NULL) {
                trialcnt = pMonInfo->uiService[j][0];
                succcnt = pMonInfo->uiService[j][1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pFirstAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=%s:%u ALARMTYPE=SVC:%s:%u",
                        __FILE__, __FUNCTION__, __LINE__,
                        PrintOFFICE(office), office, PrintSYSTYPE(systype), systype, PrintALARMTYPE(alarmtype), alarmtype);
            }
        }
    }
END_FUNC_TIME_CHECK(pFUNC, 17);

	return 0;
}

S32 dCheckFA(stHASHOINFO *pThresHash, st_MonFA *aFA, S32 cnt, st_FirstMon *pFirstMonFA, st_FirstMon *pFirstMonSec, S32 office, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 18);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	st_MonFA		*pFA;
	st_MonInfoS		*pMonInfoS;
	st_MonAlarm		*pMonAlarm;
	st_MonAlarm		*pFirstAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
    {
        pFA = &aFA[i];
        pMonInfoS = &pFA->stMonInfoS;
        pMonAlarm = &pFA->stMonAlarm;

        pFirstAlarm = &pFirstMonFA->stMonAlarm;

        dCheckSEC(pThresHash, &pFA->stMonSec[0], MAX_MON_SEC_CNT, pFirstMonSec, office, stattime);

        /* CALL */
        if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_FA, DEF_ALARMTYPE_CALL, 0)) != NULL) {
            trialcnt = pMonInfoS->usCall[0];
            succcnt = pMonInfoS->usCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=FA ALARMTYPE=CALL",
                    __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office);
        }
        
        /* ReCall */ /* INYOUNG */
        if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_FA, DEF_ALARMTYPE_RECALL, 0)) != NULL) {
            trialcnt = pMonInfoS->usReCall[0];
            succcnt = pMonInfoS->usReCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pFirstAlarm);
            }
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=FA ALARMTYPE=RECALL",
                    __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office);
        }

        /* AAA */
        if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_FA, DEF_ALARMTYPE_AAA, 0)) != NULL) {
            trialcnt = pMonInfoS->usAAA[0];
            succcnt = pMonInfoS->usAAA[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=FA ALARMTYPE=AAA",
                    __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office);
        }


        /* HSS */
        if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_FA, DEF_ALARMTYPE_HSS, 0)) != NULL) {
            trialcnt = pMonInfoS->usHSS[0];
            succcnt = pMonInfoS->usHSS[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=FA ALARMTYPE=HSS",
                    __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office);
        }


        /* Service */
        for(j = 0; j < CURR_MON_SVC_IDX; j++)
        {
            if(j == SVC_IDX_ROAM) continue;
            alarmtype = DEF_ALARMTYPE_MENU + j;
            if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_FA, alarmtype, 0)) != NULL) {
                trialcnt = pMonInfoS->usService[j][0];
                succcnt = pMonInfoS->usService[j][1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pFirstAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=FA ALARMTYPE=SVC:%s:%u",
                        __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office, PrintALARMTYPE(alarmtype), alarmtype);
            }

        }
    }
END_FUNC_TIME_CHECK(pFUNC, 18);

	return 0;
}

S32 dCheckSEC(stHASHOINFO *pThresHash, st_MonSec *aSEC, S32 cnt, st_FirstMon *pFirstMon, S32 office, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 19);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	st_MonSec		*pSEC;
	st_MonInfoS		*pMonInfoS;
	st_MonAlarm		*pMonAlarm;
	st_MonAlarm		*pFirstAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
    {
        pSEC = &aSEC[i];
        pMonInfoS = &pSEC->stMonInfoS;
        pMonAlarm = &pSEC->stMonAlarm;

        pFirstAlarm = &pFirstMon->stMonAlarm;

        /* CALL */
        if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_SECTOR, DEF_ALARMTYPE_CALL, 0)) != NULL) {
            trialcnt = pMonInfoS->usCall[0];
            succcnt = pMonInfoS->usCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SECTOR ALARMTYPE=CALL",
                    __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office);
        }

        /* ReCall */ /* INYOUNG */
        if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_SECTOR, DEF_ALARMTYPE_RECVCALL, 0)) != NULL) {
            trialcnt = pMonInfoS->usReCall[0];
            succcnt = pMonInfoS->usReCall[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pFirstAlarm);
            }
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SECTOR ALARMTYPE=RECALL",
                    __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office);
        }


        /* AAA */
        if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_SECTOR, DEF_ALARMTYPE_AAA, 0)) != NULL) {
            trialcnt = pMonInfoS->usAAA[0];
            succcnt = pMonInfoS->usAAA[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SECTOR ALARMTYPE=AAA",
                    __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office);
        }


        /* HSS */
        if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_SECTOR, DEF_ALARMTYPE_HSS, 0)) != NULL) {
            trialcnt = pMonInfoS->usHSS[0];
            succcnt = pMonInfoS->usHSS[1];
            if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pMonAlarm);
                dSetAlarmFlag(ALARM_TYPE_HSS, 0, alarmvalue, pFirstAlarm);
            } 
        }
        else {
            log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SECTOR ALARMTYPE=HSS",
                    __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office);
        }


        /* Service */
        for(j = 0; j < CURR_MON_SVC_IDX; j++)
        {
            if(j == SVC_IDX_ROAM) continue;
            alarmtype = DEF_ALARMTYPE_MENU + j;
            if((pData = getBaseValue(pThresHash, office, SYSTEM_TYPE_SECTOR, alarmtype, 0)) != NULL) {
                trialcnt = pMonInfoS->usService[j][0];
                succcnt = pMonInfoS->usService[j][1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
                    dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pFirstAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SECTOR ALARMTYPE=SVC:%s:%u",
                        __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office, PrintALARMTYPE(alarmtype), alarmtype);
            }
        }
    }
END_FUNC_TIME_CHECK(pFUNC, 19);

	return 0;
}

S32 dCheckSVC(stHASHOINFO *pThresHash, stHASHOINFO *pHash, st_MonSvc *aSVC, st_MonSvc *oSVC, S32 cnt, st_FirstMon *aFirstMon, time_t stattime)
{
START_FUNC_TIME_CHECK(pFUNC, 20);
	int				i, j, alarmtype, alarmvalue;
	U32				trialcnt, succcnt;
	S32				office, systype, subtype;
	st_MonSvc		*pSVC, *pOldSVC;
	st_MonInfo		*pMonInfo;
	st_MonAlarm		*pMonAlarm, *pOldMonAlarm;
	st_FirstMon		*pFirstMon;
	st_MonAlarm		*pFirstAlarm;
	st_ThresHash_Data	*pData;

	for(i = 0; i < cnt; i++)
	{
		pSVC = &aSVC[i];
		pOldSVC = &oSVC[i];
		pMonInfo = &pSVC->stMonInfo;
		pMonAlarm = &pSVC->stMonAlarm;
		pOldMonAlarm = &pOldSVC->stMonAlarm;

		office = 0;
		systype = SYSTEM_TYPE_SERVICE;
		subtype = pSVC->ucSvcType;

		pFirstMon = getFirstMon(pHash, office, systype, subtype, &aFirstMon[0]);
		pFirstAlarm = &pFirstMon->stMonAlarm;

		if(pSVC->ucSvcType != SVC_IDX_ROAM)
		{
			/* Service */
			alarmtype = DEF_ALARMTYPE_MENU + subtype;

			switch(pSVC->ucSvcType) {
				case SVC_IDX_CORP:			/* 법인 */
				case SVC_IDX_INET:			/* 인터넷 발신 */
				case SVC_IDX_RECVCALL:		/* 인터넷 착신 */
					if((pData = getBaseValue(pThresHash, 0, SYSTEM_TYPE_SERVICE, alarmtype, pSVC->uiIPAddr)) != NULL) {
						trialcnt = pMonInfo->uiService[subtype][0];
						succcnt = pMonInfo->uiService[subtype][1];
						if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
							dSetAlarmFlag(ALARM_TYPE_SVC, subtype, alarmvalue, pMonAlarm);
							dSetAlarmFlag(ALARM_TYPE_SVC, subtype, alarmvalue, pFirstAlarm);
							log_print(LOGN_DEBUG, "[%s.%d] [ALARM] SVC=%s ALM=%s IP=%u", __FUNCTION__, __LINE__
										, PrintSVCTYPE(pSVC->ucSvcType), PrintALMVALUE(alarmvalue), pSVC->uiIPAddr);
						}

						alarmvalue = isAlarmAll(pData, stattime, trialcnt, succcnt);
						dSetSvcConsole(subtype, alarmvalue, pSVC, pMonAlarm, pOldMonAlarm, pSVC->uiIPAddr
									, trialcnt, succcnt);
					} else {
						log_print(LOGN_DEBUG, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SVC ALARMTYPE=SVC:%s:%u IP=%u"
							, __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office, PrintALARMTYPE(alarmtype)
							, alarmtype, pSVC->uiIPAddr);

						/* default IP = 0 */
						if((pData = getBaseValue(pThresHash, 0, SYSTEM_TYPE_SERVICE, alarmtype, 0)) != NULL) {
							trialcnt = pMonInfo->uiService[subtype][0];
							succcnt = pMonInfo->uiService[subtype][1];
							if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
								dSetAlarmFlag(ALARM_TYPE_SVC, subtype, alarmvalue, pMonAlarm);
								dSetAlarmFlag(ALARM_TYPE_SVC, subtype, alarmvalue, pFirstAlarm);
								log_print(LOGN_DEBUG, "[%s.%d] [ALARM] SVC=%s ALM=%s IP=%u", __FUNCTION__, __LINE__
										, PrintSVCTYPE(pSVC->ucSvcType), PrintALMVALUE(alarmvalue), pSVC->uiIPAddr);
							}

							alarmvalue = isAlarmAll(pData, stattime, trialcnt, succcnt);
							dSetSvcConsole(subtype, alarmvalue, pSVC, pMonAlarm, pOldMonAlarm, pSVC->uiIPAddr
										, trialcnt, succcnt);
						} else {
							log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SVC ALARMTYPE=SVC:%s:%u"
									, __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office, PrintALARMTYPE(alarmtype)
									, alarmtype);
						}
					}
					break;

				default:
					if((pData = getBaseValue(pThresHash, 0, SYSTEM_TYPE_SERVICE, alarmtype, 0)) != NULL) {
						trialcnt = pMonInfo->uiService[subtype][0];
						succcnt = pMonInfo->uiService[subtype][1];
						if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
							dSetAlarmFlag(ALARM_TYPE_SVC, subtype, alarmvalue, pMonAlarm);
							dSetAlarmFlag(ALARM_TYPE_SVC, subtype, alarmvalue, pFirstAlarm);
							log_print(LOGN_DEBUG, "[%s.%d] [ALARM] SVC=%s ALM=%s IP=%u", __FUNCTION__, __LINE__
									, PrintSVCTYPE(pSVC->ucSvcType), PrintALMVALUE(alarmvalue), pSVC->uiIPAddr);
						}

						alarmvalue = isAlarmAll(pData, stattime, trialcnt, succcnt);
						dSetSvcConsole(subtype, alarmvalue, pSVC, pMonAlarm, pOldMonAlarm, pSVC->uiIPAddr
									, trialcnt, succcnt);
					} else {
						log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SVC ALARMTYPE=SVC:%s:%u",
							__FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office, PrintALARMTYPE(alarmtype), alarmtype);
					}

					break;
			} // switch
		}
		else
        {
            /* CALL */
            if((pData = getBaseValue(pThresHash, 0, ROAM_ALARM_SYSTYPE, DEF_ALARMTYPE_CALL, 0)) != NULL) {
                trialcnt = pMonInfo->uiCall[0];
                succcnt = pMonInfo->uiCall[1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pMonAlarm);
                    dSetAlarmFlag(ALARM_TYPE_CALL, 0, alarmvalue, pFirstAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL SYSTYPE=ROAM ALARMTYPE=CALL",
                        __FILE__, __FUNCTION__, __LINE__);
            }

            /* ReCall */ /* INYOUNG */
            if((pData = getBaseValue(pThresHash, 0, ROAM_ALARM_SYSTYPE, DEF_ALARMTYPE_RECALL, 0)) != NULL) {
                trialcnt = pMonInfo->uiReCall[0];
                succcnt = pMonInfo->uiReCall[1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pMonAlarm);
                    dSetAlarmFlag(ALARM_TYPE_RECALL, 0, alarmvalue, pFirstAlarm);
                } 
            }
                else {
                    log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL SYSTYPE=ROAM ALARMTYPE=RECALL",
                            __FILE__, __FUNCTION__, __LINE__);
                }
            
            /* AAA */
            if((pData = getBaseValue(pThresHash, 0, ROAM_ALARM_SYSTYPE, DEF_ALARMTYPE_AAA, 0)) != NULL) {
                trialcnt = pMonInfo->uiAAA[0];
                succcnt = pMonInfo->uiAAA[1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pMonAlarm);
                    dSetAlarmFlag(ALARM_TYPE_AAA, 0, alarmvalue, pFirstAlarm);
                } 
            }
            else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL SYSTYPE=ROAM ALARMTYPE=AAA",
                        __FILE__, __FUNCTION__, __LINE__);
            }

#if 0
            /* LNS */
            if((pData = getBaseValue(pThresHash, 0, ROAM_ALARM_SYSTYPE, DEF_ALARMTYPE_LNS, 0)) != NULL) {
                trialcnt = pMonInfo->uiLNS[0];
                succcnt = pMonInfo->uiLNS[1];
                if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                    dSetAlarmFlag(ALARM_TYPE_LNS, 0, alarmvalue, pMonAlarm);
                } 
            } else {
                log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL SYSTYPE=ROAM ALARMTYPE=LNS",
                        __FILE__, __FUNCTION__, __LINE__);
            }
#endif

            /* Service */
            for(j = 0; j < CURR_MON_SVC_IDX; j++)
            {
                if(j == SVC_IDX_ROAM) continue;
                alarmtype = DEF_ALARMTYPE_MENU + j;
                if((pData = getBaseValue(pThresHash, 0, ROAM_ALARM_SYSTYPE, alarmtype, 0)) != NULL) {
                    trialcnt = pMonInfo->uiService[j][0];
                    succcnt = pMonInfo->uiService[j][1];
                    if((alarmvalue = isAlarm(pData, stattime, trialcnt, succcnt))) {
                        dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pMonAlarm);
                        dSetAlarmFlag(ALARM_TYPE_SVC, j, alarmvalue, pFirstAlarm);
                    }
                } else {
                    log_print(LOGN_CRI, "F=%s:%s.%d getBaseValue NULL OFFICE=%s:%u SYSTYPE=SVC ALARMTYPE=SVC:%s:%u",
                            __FILE__, __FUNCTION__, __LINE__, PrintOFFICE(office), office, PrintALARMTYPE(alarmtype), alarmtype);
                }
            }
        }
	}
END_FUNC_TIME_CHECK(pFUNC, 20);

	return 0;
}

S32 dSetSvcConsole(S32 subtype, S32 alarmvalue, st_MonSvc *pSVC, st_MonAlarm *pMonAlarm, st_MonAlarm *pOldMonAlarm, UINT ip, S32 trialcnt, S32 succcnt)
{
	U8			ucNewMax, ucNewRate, ucNewMin;
	U8			ucOldMax, ucOldRate, ucOldMin;
	U8			tmpl4type = 0;
	long long	rateloadval = 0;

	ucOldMax = ((pOldMonAlarm->usAlarmSvcStatus[subtype] & DEF_ALARMTYPE_MAX) > 0) ? 1 : 0;
	ucOldRate = ((pOldMonAlarm->usAlarmSvcStatus[subtype] & DEF_ALARMTYPE_RATE) > 0) ? 1 : 0;
	ucOldMin = ((pOldMonAlarm->usAlarmSvcStatus[subtype] & DEF_ALARMTYPE_MIN) > 0) ? 1 : 0;

	ucNewMax = ((alarmvalue & DEF_ALARMTYPE_MAX) > 0) ? 1 : 0;
	ucNewRate = ((alarmvalue & DEF_ALARMTYPE_RATE) > 0) ? 1 : 0;
	ucNewMin = ((alarmvalue & DEF_ALARMTYPE_MIN) > 0) ? 1 : 0;

	log_print(LOGN_DEBUG, "[%s.%d] [ALARM] OLD[%d]=0x%02x NEW=0x%02x SVCTYPE=%s IP=%u TRIAL=%d SUCC=%d"
			, __FUNCTION__, __LINE__
			, subtype, pOldMonAlarm->usAlarmSvcStatus[subtype], alarmvalue, PrintSVCTYPE(subtype), ip, trialcnt, succcnt);

	switch(pSVC->SvcL4Type) {			/* 인터넷서비스 마스크 처리시에 변동이 발생한 부분에 대해서 OMP에서 체크해 주기 위한 부분 */
		case L4_INET_TCP:
			if(pSVC->ucReserved == 1)
				tmpl4type = 1;
			else
				tmpl4type = 2;
			break;
		case L4_INET_TCP_RECV:
			if(pSVC->ucReserved == 1)
				tmpl4type = 3;
			else
				tmpl4type = 4;
			break;
		case L4_INET_HTTP:
			if(pSVC->ucReserved == 1)
				tmpl4type = 5;
			else 
				tmpl4type = 6;
			break;
		case L4_INET_HTTP_RECV:
			if(pSVC->ucReserved == 1)
				tmpl4type = 7;
			else
				tmpl4type = 8;
			break;
	}


	/* 최대시도수 */
	if(ucNewMax) {
		if(ucOldMax == 0) {
			Send_CondMess(SYSTYPE_TAM, LOCTYPE_SVC, subtype, MON_ALARMTYPE_MAX, CRITICAL, NORMAL, tmpl4type, ip, trialcnt);
		}
	} else {
		if(ucOldMax) {
			Send_CondMess(SYSTYPE_TAM, LOCTYPE_SVC, subtype, MON_ALARMTYPE_MAX, NORMAL, CRITICAL, tmpl4type, ip, trialcnt);
		}
	}

	/* 성공률 */
	if(ucNewRate) {
		if(ucOldRate == 0) {
			if(trialcnt > 0)
				rateloadval = (float)succcnt / (float)trialcnt * 100.0;

			Send_CondMess(SYSTYPE_TAM, LOCTYPE_SVC, subtype, MON_ALARMTYPE_RATE, CRITICAL, NORMAL, tmpl4type, ip, rateloadval);
		}
	} else {
		if(ucOldRate) {
			if(trialcnt > 0)
				rateloadval = (float)succcnt / (float)trialcnt * 100.0;

			Send_CondMess(SYSTYPE_TAM, LOCTYPE_SVC, subtype, MON_ALARMTYPE_RATE, NORMAL, CRITICAL, tmpl4type, ip, rateloadval);
		}
	}

	/* 최소시도수 */
	if(ucNewMin) {
		if(ucOldMin == 0) {
			Send_CondMess(SYSTYPE_TAM, LOCTYPE_SVC, subtype, MON_ALARMTYPE_MIN, CRITICAL, NORMAL, tmpl4type, ip, trialcnt);
		}
	} else {
		if(ucOldMin) {
			Send_CondMess(SYSTYPE_TAM, LOCTYPE_SVC, subtype, MON_ALARMTYPE_MIN, NORMAL, CRITICAL, tmpl4type, ip, trialcnt);
		}
	}

	pMonAlarm->usAlarmSvcStatus[subtype] = alarmvalue;
	log_print(LOGN_DEBUG, "[%s.%d] [ALARM] CURRENT SET. SUBTYPE=%d:%s ALM=0x%02x IP=%u"
			, __FUNCTION__, __LINE__
			, subtype, PrintSVCTYPE(subtype), pMonAlarm->usAlarmSvcStatus[subtype], ip);

	return 0;
}

S32 dSetAlarmFlag(S32 systype, S32 subtype, S32 alarmvalue, st_MonAlarm *pMonAlarm)
{
	st_SubAlarmSysStatus	*pSYS = (st_SubAlarmSysStatus *)&pMonAlarm->ucAlarmSysStatus;
	st_SubAlarmSvcStatus	*pSVC = (st_SubAlarmSvcStatus *)&pMonAlarm->szAlarmSvcStatus[0];

	switch(systype)
	{
		case ALARM_TYPE_CALL:	pSYS->ucCall = getAlarm(pSYS->ucCall, alarmvalue);		break;
		case ALARM_TYPE_RECALL:	pSYS->ucReCall = getAlarm(pSYS->ucReCall, alarmvalue);	break; /* INYOUNG */
		case ALARM_TYPE_AAA:	pSYS->ucAAA = getAlarm(pSYS->ucAAA, alarmvalue);		break;
		case ALARM_TYPE_HSS:	pSYS->ucHSS = getAlarm(pSYS->ucHSS, alarmvalue);		break;
		case ALARM_TYPE_LNS:	pSYS->ucLNS = getAlarm(pSYS->ucLNS, alarmvalue);		break;
		case ALARM_TYPE_SVC:
			switch(subtype) {
				case SVC_IDX_MENU:		pSVC->ucMENU = getAlarm(pSVC->ucMENU, alarmvalue);				break;
				case SVC_IDX_DN:		pSVC->ucDN = getAlarm(pSVC->ucDN, alarmvalue);					break;
				case SVC_IDX_STREAM:	pSVC->ucSTREAM = getAlarm(pSVC->ucSTREAM, alarmvalue);			break;
				case SVC_IDX_MMS:		pSVC->ucMMS = getAlarm(pSVC->ucMMS, alarmvalue);				break;
				case SVC_IDX_WIDGET:	pSVC->ucWIDGET = getAlarm(pSVC->ucWIDGET, alarmvalue);			break;
				case SVC_IDX_PHONE:		pSVC->ucPHONE = getAlarm(pSVC->ucPHONE, alarmvalue);			break;
				case SVC_IDX_EMS:		pSVC->ucEMS = getAlarm(pSVC->ucEMS, alarmvalue);				break;
				case SVC_IDX_BANK:		pSVC->ucBANK = getAlarm(pSVC->ucBANK, alarmvalue);				break;
				case SVC_IDX_FV:		pSVC->ucFV = getAlarm(pSVC->ucFV, alarmvalue);					break;
				case SVC_IDX_IM:		pSVC->ucIM = getAlarm(pSVC->ucIM, alarmvalue);					break;
				case SVC_IDX_VT:		pSVC->ucVT = getAlarm(pSVC->ucVT, alarmvalue);					break;
				case SVC_IDX_ETC:		pSVC->ucETC = getAlarm(pSVC->ucETC, alarmvalue);				break;
				case SVC_IDX_CORP:		pSVC->ucCORP = getAlarm(pSVC->ucCORP, alarmvalue);				break;
				case SVC_IDX_REGI:		pSVC->ucREGI = getAlarm(pSVC->ucREGI, alarmvalue);				break;
				/* INYOUNG */
				//case SVC_IDX_ROAM:	pSVC->ucROAM = getAlarm(pSVC->ucROAM, alarmvalue);				break;
				case SVC_IDX_INET:		pSVC->ucINET = getAlarm(pSVC->ucINET, alarmvalue);				break;
				case SVC_IDX_RECVCALL:	pSVC->ucINET_RECV = getAlarm(pSVC->ucINET_RECV, alarmvalue);	break;
				case SVC_IDX_IM_RECV:	pSVC->ucIM_RECV = getAlarm(pSVC->ucIM_RECV, alarmvalue);		break;
				case SVC_IDX_VT_RECV:	pSVC->ucVT_RECV = getAlarm(pSVC->ucVT_RECV, alarmvalue);		break;
				default:		
					log_print(LOGN_CRI, "F=%s:%s.%d INVALID SVCTYPE=%d", __FILE__, __FUNCTION__, __LINE__, subtype);
					return -1;
			}

		break;

	default:
		log_print(LOGN_CRI, "F=%s:%s.%d INVALID SYSTYPE=%d", __FILE__, __FUNCTION__, __LINE__, systype);
		return -2;
	}

	pMonAlarm->ucAlarm = getAlarm(pMonAlarm->ucAlarm, alarmvalue);
	return 0;
}

/*
 * $Log: o_svcmon_alarm.c,v $
 * Revision 1.2  2011/08/31 16:08:07  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 08:58:48  dcham
 * *** empty log message ***
 *
 * Revision 1.30  2011/06/29 09:11:11  innaei
 * *** empty log message ***
 *
 * Revision 1.29  2011/06/28 18:51:17  innaei
 * *** empty log message ***
 *
 * Revision 1.28  2011/06/23 15:35:34  innaei
 * *** empty log message ***
 *
 * Revision 1.27  2011/06/23 14:47:57  innaei
 * *** empty log message ***
 *
 * Revision 1.26  2011/06/23 02:42:26  innaei
 * *** empty log message ***
 *
 * Revision 1.25  2011/06/23 00:58:18  innaei
 * *** empty log message ***
 *
 * Revision 1.24  2011/04/24 19:24:28  innaei
 * *** empty log message ***
 *
 * Revision 1.23  2011/04/15 09:12:13  jhbaek
 * *** empty log message ***
 *
 * Revision 1.22  2011/04/13 08:50:18  jhbaek
 * *** empty log message ***
 *
 * Revision 1.21  2011/04/12 07:42:45  jsyoon
 * *** empty log message ***
 *
 * Revision 1.20  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.2  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.19  2010/03/17 04:46:03  dqms
 * *** empty log message ***
 *
 * Revision 1.18  2010/03/05 13:45:22  dark264sh
 * *** empty log message ***
 *
 * Revision 1.17  2010/03/05 11:19:47  dark264sh
 * O_SVCMON ROAM Alarm 세팅 버그 수정
 *
 * Revision 1.16  2010/03/04 13:20:26  dark264sh
 * *** empty log message ***
 *
 * Revision 1.15  2010/02/25 11:18:47  dark264sh
 * O_SVCMON ROAM 처리
 *
 * Revision 1.14  2010/02/25 11:13:45  dark264sh
 * O_SVCMON ROAM 처리
 *
 * Revision 1.13  2010/02/25 07:33:07  dark264sh
 * BSD => LNS로 변경
 *
 * Revision 1.12  2010/02/24 12:19:46  dark264sh
 * 망감시 헤더 변경에 따른 변경
 *
 * Revision 1.11  2009/10/21 11:45:08  pkg
 * 망감시 REGI 서비스 추가 및 define값 변경
 *
 * Revision 1.10  2009/08/23 18:58:14  pkg
 * O_SVCMON BTS, FA, SEC 알람 설정 버그 수정
 *
 * Revision 1.9  2009/07/20 02:44:20  dark264sh
 * O_SVCMON SYSID 추가
 *
 * Revision 1.8  2009/07/19 20:12:38  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2009/07/10 06:30:13  dark264sh
 * 망감시 관련 서비스 구분 변경
 *
 * Revision 1.6  2009/06/22 05:23:11  dark264sh
 * O_SVCMON MonSvc Alarm Check시 해당 서비스만 체크 하도록 변경
 *
 * Revision 1.5  2009/06/21 13:34:33  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/20 15:51:23  dark264sh
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
