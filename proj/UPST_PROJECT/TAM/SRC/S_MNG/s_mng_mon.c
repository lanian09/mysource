/**A.1*  File Inclusion *******************************************************/
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//#include <utillib.h>
//#include <tam_mmc_define.h>

#include "s_mng_mon.h"

#include "mmcdef.h"		/* st_TrendInfo */
#include "loglib.h"
#include "utillib.h"

extern st_MonTotal      *gMonTotal;
extern st_MonTotal_1Min *gMonTotal1Min;

void vConvertMonInfo(st_MonInfo *pMonInfo, st_MonInfoS *pMonInfoS)
{
	int		i;

	pMonInfo->uiCall[0] = pMonInfoS->usCall[0];
	pMonInfo->uiCall[1] = pMonInfoS->usCall[1];
	pMonInfo->uiAAA[0] = pMonInfoS->usAAA[0];
	pMonInfo->uiAAA[1] = pMonInfoS->usAAA[1];
	pMonInfo->uiHSS[0] = pMonInfoS->usHSS[0];
	pMonInfo->uiHSS[1] = pMonInfoS->usHSS[1];
	pMonInfo->uiLNS[0] = pMonInfoS->usLNS[0];
	pMonInfo->uiLNS[1] = pMonInfoS->usLNS[1];
	for(i = 0; i < MAX_MON_SVC_IDX; i++)
	{
		pMonInfo->uiService[i][0] = pMonInfoS->usService[i][0];
		pMonInfo->uiService[i][1] = pMonInfoS->usService[i][1];
	}
}

int dGetPDSN(st_MonList *pMonList, int office, int systype, unsigned int ip, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonCore			*pPDSN;

	for(i = 0; i < pMonList->stFirstMonList.usPDSNListCnt; i++)
	{
		pPDSN = &pMonList->stFirstMonList.stPDSN[i];

		if((pPDSN->ucOffice == office) && (pPDSN->ucSysType == systype) && (pPDSN->uiIPAddr == ip))
		{
			memcpy(pMonInfo, &pPDSN->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetPDSN1Min(st_MonList_1Min *pMonList, int office, int systype, unsigned int ip, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonCore			*pPDSN;

	for(i = 0; i < pMonList->stFirstMonList.usPDSNListCnt; i++)
	{
		pPDSN = &pMonList->stFirstMonList.stPDSN[i];

		if((pPDSN->ucOffice == office) && (pPDSN->ucSysType == systype) && (pPDSN->uiIPAddr == ip))
		{
			memcpy(pMonInfo, &pPDSN->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetAAA(st_MonList *pMonList, int office, int systype, unsigned int ip, st_MonInfo *pMonInfo)
{
	int                 i;
	st_MonCore          *pAAA;

	for(i = 0; i < pMonList->stFirstMonList.usAAAListCnt; i++)
	{
		pAAA = &pMonList->stFirstMonList.stAAA[i];

		if((pAAA->ucOffice == office) && (pAAA->ucSysType == systype) && (pAAA->uiIPAddr == ip))
		{
			memcpy(pMonInfo, &pAAA->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetAAA1Min(st_MonList_1Min *pMonList, int office, int systype, unsigned int ip, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonCore			*pAAA;

	for(i = 0; i < pMonList->stFirstMonList.usAAAListCnt; i++)
	{
		pAAA = &pMonList->stFirstMonList.stAAA[i];

		if((pAAA->ucOffice == office) && (pAAA->ucSysType == systype) && (pAAA->uiIPAddr == ip))
		{
			memcpy(pMonInfo, &pAAA->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}



int dGetHSS(st_MonList *pMonList, int office, int systype, unsigned int ip, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonCore			*pHSS;

	for(i = 0; i < pMonList->stFirstMonList.usHSSListCnt; i++)
	{
		pHSS = &pMonList->stFirstMonList.stHSS[i];

		if((pHSS->ucOffice == office) && (pHSS->ucSysType == systype) && (pHSS->uiIPAddr == ip))
		{
			memcpy(pMonInfo, &pHSS->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetBSD(st_MonList *pMonList, int office, int systype, unsigned int ip, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonCore			*pBSD;

	for(i = 0; i < pMonList->stFirstMonList.usLNSListCnt; i++)
	{
		pBSD = &pMonList->stFirstMonList.stLNS[i];

		if((pBSD->ucOffice == office) && (pBSD->ucSysType == systype) && (pBSD->uiIPAddr == ip))
		{
			memcpy(pMonInfo, &pBSD->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetPCF(st_MonList *pMonList, int office, int systype, unsigned int ip, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonCore			*pPCF;

	for(i = 0; i < pMonList->usPCFCnt; i++)
	{
		pPCF = &pMonList->stMonPCF[i];

		if((pPCF->ucOffice == office) && (pPCF->ucSysType == systype) && (pPCF->uiIPAddr == ip))
		{
			memcpy(pMonInfo, &pPCF->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetSVC(st_MonList *pMonList, int svctype, int l4type, unsigned int ip, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonSvc			*pSVC;

	for(i = 0; i < pMonList->usSvcCnt; i++)
	{
		pSVC = &pMonList->stMonSvc[i];

		if((pSVC->ucSvcType == svctype) && (pSVC->SvcL4Type == l4type) && (pSVC->uiIPAddr == ip))
		{
			memcpy(pMonInfo, &pSVC->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetBSC(st_MonList_1Min *pMonList, int office, int sysid, int bscid, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonBSC			*pBSC;
	st_SubBSC			*pSubBSC;

	for(i = 0; i < pMonList->usBSCCnt; i++)
	{
		pBSC = &pMonList->stMonBSC[i];
		pSubBSC = (st_SubBSC *)&pBSC->uiBSC;

		if((pSubBSC->ucOffice == office) && (pSubBSC->ucBSCID == bscid) && (pSubBSC->ucSYSID == sysid))
		{
			memcpy(pMonInfo, &pBSC->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetBTS(st_MonList *pMonList, int office, int sysid, int bscid, int btsid, st_MonInfo *pMonInfo)
{
	int					i;
	st_MonBTS			*pBTS;
	st_SubBTS			*pSubBTS;

	for(i = 0; i < pMonList->usBTSCnt; i++)
	{
		pBTS = &pMonList->stMonBTS[i];
		pSubBTS = (st_SubBTS *)&pBTS->ullBTS;

		if((pSubBTS->ucOffice == office) && (pSubBTS->ucBSCID == bscid) && (pSubBTS->usBTSID == btsid) && (pSubBTS->ucSYSID == sysid))
		{
			memcpy(pMonInfo, &pBTS->stMonInfo, sizeof(st_MonInfo));
			return 1;
		}
	}

	return -1;
}

int dGetFA(st_MonList *pMonList, int office, int sysid, int bscid, int btsid, int faid, st_MonInfo *pMonInfo)
{
	int					i, j;
	st_MonBTS			*pBTS;
	st_SubBTS			*pSubBTS;
	st_MonFA			*pFA;

	for(i = 0; i < pMonList->usBTSCnt; i++)
	{
		pBTS = &pMonList->stMonBTS[i];
		pSubBTS = (st_SubBTS *)&pBTS->ullBTS;

		if((pSubBTS->ucOffice == office) && (pSubBTS->ucBSCID == bscid) && (pSubBTS->usBTSID == btsid) && (pSubBTS->ucSYSID == sysid))
		{
			for(j = 0; j < MAX_MON_FA_CNT; j++)
			{
				pFA = &pBTS->stMonFA[j];
				if(pFA->ucFA == faid)
				{
					vConvertMonInfo(pMonInfo, &pFA->stMonInfoS);
					return 1;

				}
			}
		}
	}

	return -1;
}

int dGetSEC(st_MonList *pMonList, int office, int sysid, int bscid, int btsid, int faid, int secid, st_MonInfo *pMonInfo)
{
	int					i, j, k;
	st_MonBTS			*pBTS;
	st_SubBTS			*pSubBTS;
	st_MonFA			*pFA;
	st_MonSec			*pSec;

	for(i = 0; i < pMonList->usBTSCnt; i++)
	{
		pBTS = &pMonList->stMonBTS[i];
		pSubBTS = (st_SubBTS *)&pBTS->ullBTS;

		if((pSubBTS->ucOffice == office) && (pSubBTS->ucBSCID == bscid) && (pSubBTS->usBTSID == btsid) && (pSubBTS->ucSYSID == sysid))
		{
			for(j = 0; j < MAX_MON_FA_CNT; j++)
			{
				pFA = &pBTS->stMonFA[j];
				if(pFA->ucFA == faid)
				{
					for(k = 0; k < MAX_MON_SEC_CNT; k++)
					{
						pSec = &pFA->stMonSec[k];
						if(pSec->ucSec == secid) {
							vConvertMonInfo(pMonInfo, &pSec->stMonInfoS);
							return 1;
						}
					}
				}
			}
		}
	}

	return -1;
}

int dGetTrendInfo(int dIndex, st_TrendInfo *pTrend, st_PtStatus_24Hours_List *pData)
{
	int							i, totalCnt, startIdx, dRet;
	st_PtStatus_24Hours			*pStat;
	st_MonList					*pMonList;

	unsigned char	szIP[INET_ADDRSTRLEN];

	log_print(LOGN_DEBUG, "IDX=%d CUR=%d USED=%d OFFICE=%u SYSTYPE=%u SUBTYPE=%u L4CODE=%u IP=%s:%u BSCID=%u BTSID=%u FAID=%u SECID=%u",
			dIndex, gMonTotal->dCurIdx, gMonTotal->dUsedCnt, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->ucSubType,
			pTrend->usL4Code, util_cvtipaddr(szIP, pTrend->uiIP), pTrend->uiIP, pTrend->ucBSCID, pTrend->usBTSID,
			pTrend->ucFAID, pTrend->ucSECID);

	pData->dCount = 0;
	pData->cFlag = 0;

	totalCnt = gMonTotal->dUsedCnt - dIndex * FIVE_MIN_PER_HOUR_COUNT;
	if(totalCnt <= 0) {
#if 1 /* INYOUNG */
		return -1;
#else
		return 0;
#endif
	}
	startIdx = gMonTotal->dCurIdx - dIndex * FIVE_MIN_PER_HOUR_COUNT - 1;
	if(startIdx < 0) {
		startIdx = TOTAL_MONLIST_CNT + startIdx;
	}

	for(i = 0; i < FIVE_MIN_PER_HOUR_COUNT; i++)
	{
		pStat = &pData->stPtStatus_24Hours[i];
		pMonList = &gMonTotal->stMonList[startIdx];
		pStat->tStartPt = pMonList->lTime;

		switch(pTrend->ucSysType)
		{
		case SYSTEM_TYPE_SECTOR:
			if((dRet = dGetSEC(pMonList, pTrend->ucOfficeID, pTrend->ucSYSID, pTrend->ucBSCID, pTrend->usBTSID, pTrend->ucFAID, pTrend->ucSECID, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d SECTOR dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		case SYSTEM_TYPE_FA:
			if((dRet = dGetFA(pMonList, pTrend->ucOfficeID, pTrend->ucSYSID, pTrend->ucBSCID, pTrend->usBTSID, pTrend->ucFAID, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d FA dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		case SYSTEM_TYPE_BTS:
			if((dRet = dGetBTS(pMonList, pTrend->ucOfficeID, pTrend->ucSYSID, pTrend->ucBSCID, pTrend->usBTSID, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d BTS dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		case SYSTEM_TYPE_PCF:
			if((dRet = dGetPCF(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d PCF dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		case SYSTEM_TYPE_PDSN:
			if((dRet = dGetPDSN(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d PDSN dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		case SYSTEM_TYPE_AAA:
		case SYSTEM_TYPE_ROAMAAA:
			if((dRet = dGetAAA(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d AAA dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		case SYSTEM_TYPE_HSS:
			if((dRet = dGetHSS(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d HSS dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		case SYSTEM_TYPE_LNS:
			if((dRet = dGetBSD(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d BSD dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		case SYSTEM_TYPE_SERVICE:
			if((dRet = dGetSVC(pMonList, pTrend->ucSubType, pTrend->usL4Code, pTrend->uiIP, &pStat->stMonInfo)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d SVC dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
				continue;
#else
				return pData->dCount;
#endif
			}
			break;
		default:
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN SYSTYPE=%u", __FILE__, __FUNCTION__, __LINE__, pTrend->ucSysType);
#if 1 /* INYOUNG */
			return -2;
#else
			return 0;
#endif
		}

		pData->dCount++;

		startIdx--;
		if(startIdx < 0) {
			startIdx = TOTAL_MONLIST_CNT - 1;
		}

		totalCnt--;
		if(totalCnt <= 0) {
			return pData->dCount;
		}

	}

	return pData->dCount;
}

int dGetTrendInfo1Min(int dIndex, st_TrendInfo *pTrend, st_PtStatus_24Hours_List_1Min *pData)
{
	int							i, totalCnt, startIdx, dRet;
	st_PtStatus_24Hours			*pStat;
	st_MonList_1Min				*pMonList;

	unsigned char	szIP[INET_ADDRSTRLEN];

	log_print(LOGN_DEBUG, "IDX=%d CUR=%d USED=%d OFFICE=%u SYSTYPE=%u SUBTYPE=%u L4CODE=%u IP=%s:%u BSCID=%u BTSID=%u FAID=%u SECID=%u",
			dIndex, gMonTotal1Min->dCurIdx, gMonTotal1Min->dUsedCnt, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->ucSubType,
			pTrend->usL4Code, util_cvtipaddr(szIP, pTrend->uiIP), pTrend->uiIP, pTrend->ucBSCID, pTrend->usBTSID,
			pTrend->ucFAID, pTrend->ucSECID);

	pData->dCount = 0;
	pData->cFlag = 0;

	totalCnt = gMonTotal1Min->dUsedCnt - dIndex * ONE_MIN_PER_HOUR_COUNT;
	if(totalCnt <= 0) {
#if 1 /* INYOUNG */
		return -1;
#else
		return 0;
#endif
	}
	startIdx = gMonTotal1Min->dCurIdx - dIndex * ONE_MIN_PER_HOUR_COUNT - 1;
	if(startIdx < 0) {
		startIdx = TOTAL_MONLIST_1MIN_CNT + startIdx;
	}

	for(i = 0; i < ONE_MIN_PER_HOUR_COUNT; i++)
	{
		pStat = &pData->stPtStatus_24Hours[i];
		pMonList = &gMonTotal1Min->stMonList1Min[startIdx];
		pStat->tStartPt = pMonList->lTime;

		switch(pTrend->ucSysType)
		{
			case SYSTEM_TYPE_BSC:
				if((dRet = dGetBSC(pMonList, pTrend->ucOfficeID, pTrend->ucSYSID, pTrend->ucBSCID, &pStat->stMonInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d BSC dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;

			case SYSTEM_TYPE_PDSN:
				if((dRet = dGetPDSN1Min(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, &pStat->stMonInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d PDSN dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;

			case SYSTEM_TYPE_AAA:
			case SYSTEM_TYPE_ROAMAAA:
				if((dRet = dGetAAA1Min(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, &pStat->stMonInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d AAA dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;

			default:
				log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN SYSTYPE=%u", __FILE__, __FUNCTION__, __LINE__, pTrend->ucSysType);
#if 1 /* INYOUNG */
				return -2;
#else
				return 0;
#endif
		}

		pData->dCount++;

		startIdx--;
		if(startIdx < 0) {
			startIdx = TOTAL_MONLIST_1MIN_CNT - 1;
		}

		totalCnt--;
		if(totalCnt <= 0) {
			return pData->dCount;
		}

	}

	return pData->dCount;
}

