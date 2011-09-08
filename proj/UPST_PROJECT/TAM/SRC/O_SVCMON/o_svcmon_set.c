/**		@file	o_svcmon_log.c
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_set.c,v 1.2 2011/08/31 16:08:08 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/08/31 16:08:08 $
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

#include "o_svcmon_util.h"
#include "o_svcmon_get.h"
#include "o_svcmon_set.h"

extern st_FuncTimeCheckList *pFUNC;

S32 dSetFirstMon(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_FirstMon *pFirstMon)
{
START_FUNC_TIME_CHECK(pFUNC, 90);
	int				type = pWatchMsg->usMsgType;
	int				index = pWatchMsg->ucSvcIdx;
	int				defectidx = -1;
	st_MonInfo		*pMonInfo = &pFirstMon->stMonInfo;
	st_Defect		*pDefect = &pFirstMon->stDefect;

	if(pWatchMsg->uiResult > 0) {
		if((defectidx = getDefectIndex(pDefHash, pWatchMsg->uiResult)) >= 0) {
			pDefect->uiFail[defectidx]++;	
		} else {
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN DEFECT=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->uiResult);
END_FUNC_TIME_CHECK(pFUNC, 90);
			return -1;
		}
	}

	switch(type)
    {
        case WATCH_TYPE_A11:
            pMonInfo->uiCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiCall[1]++;	
			break;
		case WATCH_TYPE_RECALL: /* INYOUNG */
            pMonInfo->uiReCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiReCall[1]++;
            break;
        case WATCH_TYPE_AAA:
            pMonInfo->uiAAA[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiAAA[1]++;	
            break;
        case WATCH_TYPE_HSS:
            pMonInfo->uiHSS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiHSS[1]++;	
            break;
        case WATCH_TYPE_LNS:
            pMonInfo->uiLNS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiLNS[1]++;	
            break;
        case WATCH_TYPE_SVC:
            pMonInfo->uiService[index][0]++;	
            if(pWatchMsg->uiResult == 0) pMonInfo->uiService[index][1]++;	
            break;
        case WATCH_TYPE_A11AAA:
            pMonInfo->uiCall[0]++;
            pMonInfo->uiAAA[0]++;
            if(pWatchMsg->uiResult == 0) {
                pMonInfo->uiCall[1]++;	
                pMonInfo->uiAAA[1]++;	
            }
            break;
    }

END_FUNC_TIME_CHECK(pFUNC, 90);
	return 0;
}

S32 dSetCore(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonCore *pMonCore)
{
START_FUNC_TIME_CHECK(pFUNC, 91);
	int				type = pWatchMsg->usMsgType;
	int				index = pWatchMsg->ucSvcIdx;
	int				defectidx = -1;
	st_MonInfo		*pMonInfo = &pMonCore->stMonInfo;
	st_Defect		*pDefect = &pMonCore->stDefect;

	if(pWatchMsg->uiResult > 0) {
		if((defectidx = getDefectIndex(pDefHash, pWatchMsg->uiResult)) >= 0) {
			pDefect->uiFail[defectidx]++;	
		} else {
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN DEFECT=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->uiResult);
END_FUNC_TIME_CHECK(pFUNC, 91);
			return -1;
		}
	}

	switch(type)
    {
        case WATCH_TYPE_A11:
            pMonInfo->uiCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiCall[1]++;	
			break;
		case WATCH_TYPE_RECALL:			/* INYOUNG */
            pMonInfo->uiReCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiReCall[1]++;
            break;
        case WATCH_TYPE_AAA:
            pMonInfo->uiAAA[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiAAA[1]++;	
            break;
        case WATCH_TYPE_HSS:
            pMonInfo->uiHSS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiHSS[1]++;	
            break;
        case WATCH_TYPE_LNS:
            pMonInfo->uiLNS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiLNS[1]++;	
            break;
        case WATCH_TYPE_SVC:
            pMonInfo->uiService[index][0]++;	
            if(pWatchMsg->uiResult == 0) pMonInfo->uiService[index][1]++;	
            break;
        case WATCH_TYPE_A11AAA:
            pMonInfo->uiCall[0]++;
            pMonInfo->uiAAA[0]++;
            if(pWatchMsg->uiResult == 0) {
                pMonInfo->uiCall[1]++;	
                pMonInfo->uiAAA[1]++;	
            }
            break;
    }

END_FUNC_TIME_CHECK(pFUNC, 91);
	return 0;
}

S32 dSetBTS(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonBTS *pMonBTS)
{
START_FUNC_TIME_CHECK(pFUNC, 92);
	int				dRet;
	int				type = pWatchMsg->usMsgType;
	int				index = pWatchMsg->ucSvcIdx;
	int				defectidx = -1;
	st_MonInfo		*pMonInfo = &pMonBTS->stMonInfo;
	st_Defect		*pDefect = &pMonBTS->stDefect;
	st_MonFA		*pMonFA;

	if(pWatchMsg->uiResult > 0) {
		if((defectidx = getDefectIndex(pDefHash, pWatchMsg->uiResult)) >= 0) {
			pDefect->uiFail[defectidx]++;	
		} else {
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN DEFECT=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->uiResult);
END_FUNC_TIME_CHECK(pFUNC, 92);
			return -1;
		}
	}

	switch(type)
    {
        case WATCH_TYPE_A11:
            pMonInfo->uiCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiCall[1]++;	
			break;
        case WATCH_TYPE_RECALL: /* INYOUNG */
            pMonInfo->uiReCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiReCall[1]++;
            break;
        case WATCH_TYPE_AAA:
            pMonInfo->uiAAA[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiAAA[1]++;	
            break;
        case WATCH_TYPE_HSS:
            pMonInfo->uiHSS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiHSS[1]++;	
            break;
        case WATCH_TYPE_LNS:
            pMonInfo->uiLNS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiLNS[1]++;	
            break;
        case WATCH_TYPE_SVC:
            pMonInfo->uiService[index][0]++;	
            if(pWatchMsg->uiResult == 0) pMonInfo->uiService[index][1]++;	
            break;
    }

	if((pMonFA = getMonFA(pWatchMsg, pMonBTS)) != NULL) {
		if((dRet = dSetFA(pDefHash, pWatchMsg, pMonFA)) < 0) {
			log_print(LOGN_CRI, "F=%s:%s.%d dSetFA dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
END_FUNC_TIME_CHECK(pFUNC, 92);
			return -2;
		}
	} else {
		log_print(LOGN_CRI, "F=%s:%s.%d getMonFA NULL", __FILE__, __FUNCTION__, __LINE__);
END_FUNC_TIME_CHECK(pFUNC, 92);
		return -3;
	}

END_FUNC_TIME_CHECK(pFUNC, 92);
	return 0;
}

S32 dSetBSC(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonBSC *pMonBSC)
{
START_FUNC_TIME_CHECK(pFUNC, 93);
	int				type = pWatchMsg->usMsgType;
	int				index = pWatchMsg->ucSvcIdx;
	int				defectidx = -1;
	st_MonInfo		*pMonInfo = &pMonBSC->stMonInfo;
	st_Defect		*pDefect = &pMonBSC->stDefect;

	if(pWatchMsg->uiResult > 0) {
		if((defectidx = getDefectIndex(pDefHash, pWatchMsg->uiResult)) >= 0) {
			pDefect->uiFail[defectidx]++;	
		} else {
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN DEFECT=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->uiResult);
END_FUNC_TIME_CHECK(pFUNC, 93);
			return -1;
		}
	}

	switch(type)
    {
        case WATCH_TYPE_A11:
            pMonInfo->uiCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiCall[1]++;	
			break;
		case WATCH_TYPE_RECALL: /* INYOUNG */
            pMonInfo->uiReCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiReCall[1]++;
            break;
        case WATCH_TYPE_AAA:
            pMonInfo->uiAAA[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiAAA[1]++;	
            break;
        case WATCH_TYPE_HSS:
            pMonInfo->uiHSS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiHSS[1]++;	
            break;
        case WATCH_TYPE_LNS:
            pMonInfo->uiLNS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiLNS[1]++;	
            break;
        case WATCH_TYPE_SVC:
            pMonInfo->uiService[index][0]++;	
            if(pWatchMsg->uiResult == 0) pMonInfo->uiService[index][1]++;	
            break;
    }

END_FUNC_TIME_CHECK(pFUNC, 93);
	return 0;
}

S32 dSetSvc(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonSvc *pMonSvc)
{
START_FUNC_TIME_CHECK(pFUNC, 94);
	int				index = pMonSvc->ucSvcType;
	int				defectidx = -1;
	st_MonInfo		*pMonInfo = &pMonSvc->stMonInfo;
	st_Defect		*pDefect = &pMonSvc->stDefect;

	if(pWatchMsg->uiResult > 0) {
		if((defectidx = getDefectIndex(pDefHash, pWatchMsg->uiResult)) >= 0) {
			pDefect->uiFail[defectidx]++;	
		} else {
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN DEFECT=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->uiResult);
END_FUNC_TIME_CHECK(pFUNC, 94);
			return -1;
		}
	}
	
	pMonInfo->uiService[index][0]++;	
	if(pWatchMsg->uiResult == 0) pMonInfo->uiService[index][1]++;	

END_FUNC_TIME_CHECK(pFUNC, 94);
	return 0;
}

S32 dSetFA(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonFA *pMonFA)
{
START_FUNC_TIME_CHECK(pFUNC, 95);
	int				dRet;
	int				type = pWatchMsg->usMsgType;
	int				index = pWatchMsg->ucSvcIdx;
	int				defectidx = -1;
	st_MonInfoS		*pMonInfoS = &pMonFA->stMonInfoS;
	st_DefectS		*pDefectS = &pMonFA->stDefectS;
	st_MonSec		*pMonSec;

	if(pWatchMsg->uiResult > 0) {
		if((defectidx = getDefectIndex(pDefHash, pWatchMsg->uiResult)) >= 0) {
			pDefectS->usFail[defectidx]++;	
		} else {
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN DEFECT=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->uiResult);
END_FUNC_TIME_CHECK(pFUNC, 95);
			return -1;
		}
	}

	switch(type)
    {
        case WATCH_TYPE_A11:
            pMonInfoS->usCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usCall[1]++;	
			break;
		case WATCH_TYPE_RECALL: /* INYOUNG */
            pMonInfoS->usReCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usReCall[1]++;
            break;
        case WATCH_TYPE_AAA:
            pMonInfoS->usAAA[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usAAA[1]++;	
            break;
        case WATCH_TYPE_HSS:
            pMonInfoS->usHSS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usHSS[1]++;	
            break;
        case WATCH_TYPE_LNS:
            pMonInfoS->usLNS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usLNS[1]++;	
            break;
        case WATCH_TYPE_SVC:
            pMonInfoS->usService[index][0]++;	
            if(pWatchMsg->uiResult == 0) pMonInfoS->usService[index][1]++;	
            break;
    }

	if((pMonSec = getMonSec(pWatchMsg, pMonFA)) != NULL) {
		if((dRet = dSetSec(pDefHash, pWatchMsg, pMonSec)) < 0) {
			log_print(LOGN_CRI, "F=%s:%s.%d dSetSec dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
END_FUNC_TIME_CHECK(pFUNC, 95);
			return -2;
		}
	} else {
		log_print(LOGN_CRI, "F=%s:%s.%d getMonSec NULL", __FILE__, __FUNCTION__, __LINE__);
END_FUNC_TIME_CHECK(pFUNC, 95);
		return -3;
	}

END_FUNC_TIME_CHECK(pFUNC, 95);
	return 0;
}

S32 dSetSec(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonSec *pMonSec)
{
START_FUNC_TIME_CHECK(pFUNC, 96);
	int				type = pWatchMsg->usMsgType;
	int				index = pWatchMsg->ucSvcIdx;
	int				defectidx = -1;
	st_MonInfoS		*pMonInfoS = &pMonSec->stMonInfoS;
	st_DefectS		*pDefectS = &pMonSec->stDefectS;

	if(pWatchMsg->uiResult > 0) {
		if((defectidx = getDefectIndex(pDefHash, pWatchMsg->uiResult)) >= 0) {
			pDefectS->usFail[defectidx]++;	
		} else {
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN DEFECT=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->uiResult);
END_FUNC_TIME_CHECK(pFUNC, 96);
			return -1;
		}
	}

	switch(type)
    {
        case WATCH_TYPE_A11:
            pMonInfoS->usCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usCall[1]++;	
        case WATCH_TYPE_RECALL: /* INYOUNG */
            pMonInfoS->usReCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usReCall[1]++;
            break;
        case WATCH_TYPE_AAA:
            pMonInfoS->usAAA[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usAAA[1]++;	
            break;
        case WATCH_TYPE_HSS:
            pMonInfoS->usHSS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usHSS[1]++;	
            break;
        case WATCH_TYPE_LNS:
            pMonInfoS->usLNS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfoS->usLNS[1]++;	
            break;
        case WATCH_TYPE_SVC:
            pMonInfoS->usService[index][0]++;	
            if(pWatchMsg->uiResult == 0) pMonInfoS->usService[index][1]++;	
            break;
    }

END_FUNC_TIME_CHECK(pFUNC, 96);
	return 0;
}

S32 dSetRoam(stHASHOINFO *pDefHash, st_WatchMsg *pWatchMsg, st_MonSvc *pMonSvc)
{
START_FUNC_TIME_CHECK(pFUNC, 97);
	int				type = pWatchMsg->usMsgType;
//	int				index = pMonSvc->ucSvcType;
	int				index = pWatchMsg->ucSvcIdx;
	int				defectidx = -1;
	st_MonInfo		*pMonInfo = &pMonSvc->stMonInfo;
	st_Defect		*pDefect = &pMonSvc->stDefect;

	if(pWatchMsg->uiResult > 0) {
		if((defectidx = getDefectIndex(pDefHash, pWatchMsg->uiResult)) >= 0) {
			pDefect->uiFail[defectidx]++;	
		} else {
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN DEFECT=%u", __FILE__, __FUNCTION__, __LINE__, pWatchMsg->uiResult);
END_FUNC_TIME_CHECK(pFUNC, 94);
			return -1;
		}
	}
	
	switch(type)
    {
        case WATCH_TYPE_A11:
            pMonInfo->uiCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiCall[1]++;	
			break;
		case WATCH_TYPE_RECALL: /* INYOUNG */
            pMonInfo->uiReCall[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiReCall[1]++;
            break;
        case WATCH_TYPE_AAA:
            pMonInfo->uiAAA[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiAAA[1]++;	
            break;
        case WATCH_TYPE_LNS:
            pMonInfo->uiLNS[0]++;
            if(pWatchMsg->uiResult == 0) pMonInfo->uiLNS[1]++;	
            break;
        case WATCH_TYPE_SVC:
            pMonInfo->uiService[index][0]++;	
            if(pWatchMsg->uiResult == 0) pMonInfo->uiService[index][1]++;	
            break;
        case WATCH_TYPE_A11AAA:
            pMonInfo->uiCall[0]++;
            pMonInfo->uiAAA[0]++;
            if(pWatchMsg->uiResult == 0) {
                pMonInfo->uiCall[1]++;	
                pMonInfo->uiAAA[1]++;	
            }
            break;
    }

END_FUNC_TIME_CHECK(pFUNC, 97);
	return 0;
}

/*
 * $Log: o_svcmon_set.c,v $
 * Revision 1.2  2011/08/31 16:08:08  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 08:58:49  dcham
 * *** empty log message ***
 *
 * Revision 1.13  2011/04/24 19:24:28  innaei
 * *** empty log message ***
 *
 * Revision 1.12  2011/04/13 08:50:18  jhbaek
 * *** empty log message ***
 *
 * Revision 1.11  2011/01/11 04:09:18  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:10  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.10  2010/03/06 13:07:28  dark264sh
 * O_SVCMON SVCMON에 Roaming 관련 시도수/성공수 세팅 버그 수정
 *
 * Revision 1.9  2010/02/25 11:13:45  dark264sh
 * O_SVCMON ROAM 처리
 *
 * Revision 1.8  2010/02/24 12:19:46  dark264sh
 * 망감시 헤더 변경에 따른 변경
 *
 * Revision 1.7  2009/08/22 20:05:11  pkg
 * O_SVCMON DEFECT를 먼저 처리하도록 변경
 *
 * Revision 1.6  2009/07/10 06:33:15  dark264sh
 * 망감시 관련 서비스 구분 변경
 *
 * Revision 1.5  2009/07/01 16:32:00  dark264sh
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
