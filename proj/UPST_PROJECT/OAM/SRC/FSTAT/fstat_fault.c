/**
	@file		fstat_fault.h	
	@author		
	@version	
	@date		2011-07-26
	@brief		fstat_fault.c 헤더파일
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <string.h>
#include <time.h>
/* LIB HEADER */
#include "commdef.h"
#include "loglib.h"
#include "nsocklib.h"
/* PRO HEADER */
#include "msgdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "fstat_init.h"

/**
 *	Declare func.
 */
extern void Insert_STAT_FAULTData(st_FAULT *pstData, int dTotCount);
extern TIMERNID timerN_add(stTIMERNINFO *pstTIMERNINFO,void (*invoke_func)(void*),U8 *pArg,U32 uiArgSize,time_t timeout);

/**
	Declare variables
*/
extern short				gdTimeFlag;
extern st_FAULTLIST			*pstSHMFAULTTbl;
extern st_SubSysInfoList	gstSubList;
extern unsigned long long	timerNID;
extern stTIMERNINFO			*pTIMER;

/**
 *	Implements func.
 */
int dCheckFaultList(time_t tLocalTime, st_almsts * stAlmStatus, short usNTAFID)
{
	int			usFaultPos, dTimeStart;
	st_FAULT	*stFAULT;

	if(stAlmStatus->tWhen/300*300 == tLocalTime)
	{
		dTimeStart = gdTimeFlag * MAX_SYS_CNT*2;
		log_print(LOGN_INFO, "UPDATE TIME IS CUR TIME");
	}
	else if(stAlmStatus->tWhen/300*300 == tLocalTime - 300)
		dTimeStart =( gdTimeFlag^ 0x01) * MAX_SYS_CNT*2;
	else
	{
		log_print(LOGN_DEBUG, "UPDATE TIME IS OLD TIME");
		return -1;
	}

	if(stAlmStatus->ucSysType == DEF_NTAM)
	{
		log_print(LOGN_INFO," NTAM FAULT MESSAGE ");

		if(stAlmStatus->ucLocType == LOCTYPE_PROCESS)
		{
			usFaultPos = DEF_NTAM_POS * 2;
			log_print(LOGN_INFO," THIS FAULT TYPE IS SOFTWARE FAULT");
		}
		else
		{
			usFaultPos = DEF_NTAM_POS * 2 +1;
			log_print(LOGN_INFO," THIS FAULT TYPE IS HARDWARE FAULT");
		}
	}
	else
	{
		log_print(LOGN_INFO," NTAF FAULT MESSAGE ");
		if(stAlmStatus->ucLocType == LOCTYPE_PROCESS)
		{
			usFaultPos = usNTAFID * 2;
			log_print(LOGN_INFO," THIS FAULT TYPE IS SOFTWARE FAULT");
		}
		else
		{
			usFaultPos = usNTAFID * 2 +1;
			log_print(LOGN_INFO," THIS FAULT TYPE IS HARDWARE FAULT");
		}
	}

	stFAULT = &(pstSHMFAULTTbl->stFAULT[usFaultPos+dTimeStart]);

	switch(stAlmStatus->ucAlmLevel)
	{
		case CRITICAL :
			stFAULT->uiCRI++;
			break;
		case MAJOR :
			stFAULT->uiMAJ++;
			break;
		case MINOR:
			stFAULT->uiMIN++;
			break;
		case NORMAL:
			stFAULT->uiNORMAL++;
			break;
		case STOP:
			stFAULT->uiSTOP++;
			break;
		default:
			log_print(LOGN_WARN," WRONG ALRM LEVEL");
	}

	log_print(LOGN_INFO,"FAULT SAVE STATTIME[%u] SYSTYPE[%u] SYSID[%d] TYPE[%s] C[%d]MI[%d]MJ[%d]ST[%d]NO[%d]",
		stFAULT->uiStatTime, stFAULT->usSystemType, stFAULT->usSystemID, stFAULT->szType,
		stFAULT->uiCRI, stFAULT->uiMAJ, stFAULT->uiMIN, stFAULT->uiSTOP, stFAULT->uiNORMAL);

	return 0;
}

void Stat_FAULTFunc(void* p)
{
	BLOCK_KEY	BLOCKKEY;
	int			i, dTotCount, dIndex, dTimeFlag;
	time_t		tStatTime;
	st_FAULT	*stFAULT, FAULTData[MAX_SYS_CNT*2];

	dTotCount	= 0;
	dTimeFlag	= gdTimeFlag^0x01;

	memset(&FAULTData[0], 0x00, sizeof(st_FAULT)*MAX_SYS_CNT*2);

    for(i = 0; i < MAX_SUBSYS_NUM; i++)
    {
		if(i == 0)
			dIndex = DEF_NTAM_POS;
		else
		{
			dIndex = gstSubList.stInfo[i-1].dNo;

			if( (gstSubList.stInfo[i-1].dType > DEF_SGTAF_TYPE) || (gstSubList.stInfo[i-1].dType == 0))
				continue;
		}

        memcpy(&FAULTData[dTotCount], &(pstSHMFAULTTbl->stFAULT[dTimeFlag*MAX_SYS_CNT*2+dIndex*2]), sizeof(st_FAULT));
		dTotCount++;

		stFAULT = &(pstSHMFAULTTbl->stFAULT[dTimeFlag*MAX_SYS_CNT*2+dIndex*2]);
		log_print(LOGN_INFO,"FAULT CONV STATTIME[%u] SYSTYPE[%u] SYSID[%d] TYPE[%s] C[%d]MI[%d]MJ[%d]ST[%d]NO[%d]i[%d]count[%d]",
			stFAULT->uiStatTime, stFAULT->usSystemType, stFAULT->usSystemID, stFAULT->szType,
			stFAULT->uiCRI, stFAULT->uiMAJ, stFAULT->uiMIN, stFAULT->uiSTOP, stFAULT->uiNORMAL, i, dTotCount);

        memcpy(&FAULTData[dTotCount], &(pstSHMFAULTTbl->stFAULT[dTimeFlag*MAX_SYS_CNT*2+dIndex*2+1]), sizeof(st_FAULT));
        dTotCount++;

		stFAULT = &(pstSHMFAULTTbl->stFAULT[dTimeFlag*MAX_SYS_CNT*2+dIndex*2+1]);
		log_print(LOGN_INFO,"FAULT CONV STATTIME[%u] SYSTYPE[%u] SYSID[%d] TYPE[%s] C[%d]MI[%d]MJ[%d]ST[%d]NO[%d]i[%d]count[%d]"
			, stFAULT->uiStatTime, stFAULT->usSystemType, stFAULT->usSystemID, stFAULT->szType
			, stFAULT->uiCRI, stFAULT->uiMAJ, stFAULT->uiMIN, stFAULT->uiSTOP, stFAULT->uiNORMAL, i, dTotCount);
    }

	for(i = 0; i < dTotCount;i++)
	{
		log_print(LOGN_INFO,"FAULT INS STATTIME[%u] SYSTYPE[%u] SYSID[%d] TYPE[%s] C[%d]MI[%d]MJ[%d]ST[%d]NO[%d]",
			FAULTData[i].uiStatTime, FAULTData[i].usSystemType, FAULTData[i].usSystemID, FAULTData[i].szType,
			FAULTData[i].uiCRI, FAULTData[i].uiMAJ, FAULTData[i].uiMIN, FAULTData[i].uiSTOP, FAULTData[i].uiNORMAL);
	}
	Insert_STAT_FAULTData(FAULTData, dTotCount);

	tStatTime = time(NULL)/300*300+300;

	InitSHMFAULT(dTimeFlag, (unsigned int)tStatTime);

	BLOCKKEY.usLoadFaultFlag = MID_STAT_FAULT;
	BLOCKKEY.usgdTimeFlag = gdTimeFlag;

	timerNID = timerN_add(pTIMER, Stat_FAULTFunc, (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), time(NULL)/300*300+360);
}

void InitSHMFAULT(short TimeFlag, unsigned int uiStatTime)
{
	int		i, dIndex;

    for(i = 0; i< MAX_SUBSYS_NUM; i++)
    {
		if(i == 0)
		{
			dIndex = DEF_NTAM_POS;
			pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 + dIndex*2].usSystemType	= DEF_NTAM_POS;
			pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 + dIndex*2].usSystemID		= DEF_NTAM_POS;
			pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 + dIndex*2+1].usSystemType	= DEF_NTAM_POS;
			pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 + dIndex*2+1].usSystemID	= DEF_NTAM_POS;
		}
		else
		{
			if( (gstSubList.stInfo[i-1].dType > DEF_SGTAF_TYPE) || (gstSubList.stInfo[i-1].dType == 0))
				continue;

			dIndex = gstSubList.stInfo[i-1].dNo;
			pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].usSystemType	= 1;
			pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].usSystemID		= gstSubList.stInfo[i-1].dNo;
			pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].usSystemType	= 1;
			pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].usSystemID	= gstSubList.stInfo[i-1].dNo;
		}

		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].uiStatTime	= uiStatTime;
		memcpy(pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].szType, "SOFTWARE", 9);
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].uiCRI	= 0;
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].uiMAJ	= 0;
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].uiMIN	= 0;
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].uiSTOP	= 0;
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2].uiNORMAL	= 0;


		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].uiStatTime	= uiStatTime;
		memcpy(pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].szType, "HARDWARE", 9);
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].uiCRI		= 0;
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].uiMAJ		= 0;
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].uiMIN		= 0;
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].uiSTOP		= 0;
		pstSHMFAULTTbl->stFAULT[TimeFlag*MAX_SYS_CNT*2 +dIndex*2+1].uiNORMAL	= 0;
	}
	log_print(LOGN_INFO, "INIT FAULTTABLE STATTIME[%u], TIMEFLAG[%d]", uiStatTime, TimeFlag);
}
