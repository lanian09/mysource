
/**A.1*  File Inclusion *******************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "watch_mon.h"	/* st_MonTotal */
#include "mmcdef.h"		/* st_TrendInfo */
#include "s_mng_dft.h"

#include "loglib.h"
#include "utillib.h"

extern st_MonTotal      *gMonTotal;
extern st_MonTotal_1Min *gMonTotal1Min;

int dGetPDSNDef(st_MonList *pMonList, int office, int systype, unsigned int ip, int def, unsigned int *pCnt)
{
	int					i;
	st_MonCore			*pPDSN;

	for(i = 0; i < pMonList->stFirstMonList.usPDSNListCnt; i++)
	{
		pPDSN = &pMonList->stFirstMonList.stPDSN[i];

		if((pPDSN->ucOffice == office) && (pPDSN->ucSysType == systype) && (pPDSN->uiIPAddr == ip))
		{
			*pCnt = pPDSN->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetPDSNDef1Min(st_MonList_1Min *pMonList, int office, int systype, unsigned int ip, int def, unsigned int *pCnt)
{
	int					i;
	st_MonCore			*pPDSN;

	for(i = 0; i < pMonList->stFirstMonList.usPDSNListCnt; i++)
	{
		pPDSN = &pMonList->stFirstMonList.stPDSN[i];

		if((pPDSN->ucOffice == office) && (pPDSN->ucSysType == systype) && (pPDSN->uiIPAddr == ip))
		{
			*pCnt = pPDSN->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetAAADef(st_MonList *pMonList, int office, int systype, unsigned int ip, int def, unsigned int *pCnt)
{
	int					i;
	st_MonCore			*pAAA;

	for(i = 0; i < pMonList->stFirstMonList.usAAAListCnt; i++)
	{
		pAAA = &pMonList->stFirstMonList.stAAA[i];

		if((pAAA->ucOffice == office) && (pAAA->ucSysType == systype) && (pAAA->uiIPAddr == ip))
		{
			*pCnt = pAAA->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetAAADef1Min(st_MonList_1Min *pMonList, int office, int systype, unsigned int ip, int def, unsigned int *pCnt)
{
	int					i;
	st_MonCore			*pAAA;

	for(i = 0; i < pMonList->stFirstMonList.usAAAListCnt; i++)
	{
		pAAA = &pMonList->stFirstMonList.stAAA[i];

		if((pAAA->ucOffice == office) && (pAAA->ucSysType == systype) && (pAAA->uiIPAddr == ip))
		{
			*pCnt = pAAA->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetHSSDef(st_MonList *pMonList, int office, int systype, unsigned int ip, int def, unsigned int *pCnt)
{
	int					i;
	st_MonCore			*pHSS;

	for(i = 0; i < pMonList->stFirstMonList.usHSSListCnt; i++)
	{
		pHSS = &pMonList->stFirstMonList.stHSS[i];

		if((pHSS->ucOffice == office) && (pHSS->ucSysType == systype) && (pHSS->uiIPAddr == ip))
		{
			*pCnt = pHSS->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetBSDDef(st_MonList *pMonList, int office, int systype, unsigned int ip, int def, unsigned int *pCnt)
{
	int					i;
	st_MonCore			*pBSD;

	for(i = 0; i < pMonList->stFirstMonList.usLNSListCnt; i++)
	{
		pBSD = &pMonList->stFirstMonList.stLNS[i];

		if((pBSD->ucOffice == office) && (pBSD->ucSysType == systype) && (pBSD->uiIPAddr == ip))
		{
			*pCnt = pBSD->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetPCFDef(st_MonList *pMonList, int office, int systype, unsigned int ip, int def, unsigned int *pCnt)
{
	int					i;
	st_MonCore			*pPCF;

	for(i = 0; i < pMonList->usPCFCnt; i++)
	{
		pPCF = &pMonList->stMonPCF[i];

		if((pPCF->ucOffice == office) && (pPCF->ucSysType == systype) && (pPCF->uiIPAddr == ip))
		{
			*pCnt = pPCF->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetSVCDef(st_MonList *pMonList, int svctype, int l4type, unsigned int ip, int def, unsigned int *pCnt)
{
	int					i;
	st_MonSvc			*pSVC;

	for(i = 0; i < pMonList->usSvcCnt; i++)
	{
		pSVC = &pMonList->stMonSvc[i];

		if((pSVC->ucSvcType == svctype) && (pSVC->SvcL4Type == l4type) && (pSVC->uiIPAddr == ip))
		{
			*pCnt = pSVC->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetBSCDef(st_MonList_1Min *pMonList, int office, int sysid, int bscid, int def, unsigned int *pCnt)
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
			*pCnt = pBSC->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetBTSDef(st_MonList *pMonList, int office, int sysid, int bscid, int btsid, int def, unsigned int *pCnt)
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
			*pCnt = pBTS->stDefect.uiFail[def];
			return 1;
		}
	}

	return -1;
}

int dGetFADef(st_MonList *pMonList, int office, int sysid, int bscid, int btsid, int faid, int def, unsigned int *pCnt)
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
					*pCnt = pFA->stDefectS.usFail[def];
					return 1;

				}
			}
		}
	}

	return -1;
}

int dGetSECDef(st_MonList *pMonList, int office, int sysid, int bscid, int btsid, int faid, int secid, int def, unsigned int *pCnt)
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
							*pCnt = pSec->stDefectS.usFail[def];
							return 1;
						}
					}
				}
			}
		}
	}

	return -1;
}

int dGetDefTrendInfo(int dIndex, st_TrendInfo *pTrend, st_Defect_24Hours_List *pData)
{
	int							i, totalCnt, startIdx, dRet;
	st_Defect_24Hours			*pStat;
	st_MonList					*pMonList;

	unsigned char	szIP[INET_ADDRSTRLEN];

	log_print(LOGN_DEBUG, "IDX=%d CUR=%d USED=%d OFFICE=%u SYSTYPE=%u SUBTYPE=%u L4CODE=%u IP=%s:%u BSCID=%u BTSID=%u FAID=%u SECID=%u DEF=%u",
			dIndex, gMonTotal->dCurIdx, gMonTotal->dUsedCnt, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->ucSubType,
			pTrend->usL4Code, util_cvtipaddr(szIP, pTrend->uiIP), pTrend->uiIP, pTrend->ucBSCID, pTrend->usBTSID,
			pTrend->ucFAID, pTrend->ucSECID, pTrend->ucDefectCode);

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
		pStat = &pData->st_Defect_24Hours[i];
		pMonList = &gMonTotal->stMonList[startIdx];
		pStat->tStart = pMonList->lTime;

		switch(pTrend->ucSysType)
		{
			case SYSTEM_TYPE_SECTOR:
				if((dRet = dGetSECDef(pMonList, pTrend->ucOfficeID, pTrend->ucSYSID, pTrend->ucBSCID, pTrend->usBTSID, pTrend->ucFAID, pTrend->ucSECID, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d SECTOR dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;
			case SYSTEM_TYPE_FA:
				if((dRet = dGetFADef(pMonList, pTrend->ucOfficeID, pTrend->ucSYSID, pTrend->ucBSCID, pTrend->usBTSID, pTrend->ucFAID, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d FA dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;
			case SYSTEM_TYPE_BTS:
				if((dRet = dGetBTSDef(pMonList, pTrend->ucOfficeID, pTrend->ucSYSID, pTrend->ucBSCID, pTrend->usBTSID, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d BTS dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;
			case SYSTEM_TYPE_PCF:
				if((dRet = dGetPCFDef(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d PCF dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;
			case SYSTEM_TYPE_PDSN:
				if((dRet = dGetPDSNDef(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
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
				if((dRet = dGetAAADef(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d AAA dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;
			case SYSTEM_TYPE_HSS:
				if((dRet = dGetHSSDef(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d HSS dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;
			case SYSTEM_TYPE_LNS:
				if((dRet = dGetBSDDef(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d BSD dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;
			case SYSTEM_TYPE_SERVICE:
				if((dRet = dGetSVCDef(pMonList, pTrend->ucSubType, pTrend->usL4Code, pTrend->uiIP, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
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

int dGetDefTrendInfo1Min(int dIndex, st_TrendInfo *pTrend, st_Defect_24Hours_List_1Min *pData)
{
	int							i, totalCnt, startIdx, dRet;
	st_Defect_24Hours			*pStat;
	st_MonList_1Min				*pMonList;

	unsigned char	szIP[INET_ADDRSTRLEN];

	log_print(LOGN_DEBUG, "IDX=%d CUR=%d USED=%d OFFICE=%u SYSTYPE=%u SUBTYPE=%u L4CODE=%u IP=%s:%u BSCID=%u BTSID=%u FAID=%u SECID=%u DEF=%u",
			dIndex, gMonTotal1Min->dCurIdx, gMonTotal1Min->dUsedCnt, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->ucSubType,
			pTrend->usL4Code, util_cvtipaddr(szIP, pTrend->uiIP), pTrend->uiIP, pTrend->ucBSCID, pTrend->usBTSID,
			pTrend->ucFAID, pTrend->ucSECID, pTrend->ucDefectCode);

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
		pStat = &pData->st_Defect_24Hours[i];
		pMonList = &gMonTotal1Min->stMonList1Min[startIdx];
		pStat->tStart = pMonList->lTime;

		switch(pTrend->ucSysType)
		{
			case SYSTEM_TYPE_BSC:
				if((dRet = dGetBSCDef(pMonList, pTrend->ucOfficeID, pTrend->ucSYSID, pTrend->ucBSCID, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d BSC dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
#if 1 /* INYOUNG */
					continue;
#else
					return pData->dCount;
#endif
				}
				break;

			case SYSTEM_TYPE_PDSN:
				if((dRet = dGetPDSNDef1Min(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
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
				if((dRet = dGetAAADef1Min(pMonList, pTrend->ucOfficeID, pTrend->ucSysType, pTrend->uiIP, pTrend->ucDefectCode, &pStat->uDefectInfo)) < 0) {
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

