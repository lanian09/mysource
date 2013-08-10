/**
	@file		fstat_load.c
	@author		
	@version	
	@date		2011-07-26
	@brief		fstat_load.c
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <string.h>
#include <time.h>
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
#include "nsocklib.h"
/* PRO HEADER */
#include "msgdef.h"

/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "fstat_init.h"
#include "fstat_load.h"

/**
	Declare variables
*/
extern short				gdTimeFlag;
extern unsigned long long	timerNID;
extern st_LOADLIST			*pstSHMLOADTbl;
extern st_SubSysInfoList	gstSubList;
extern stTIMERNINFO			*pTIMER;

/**
 *	Declare func.
 */
extern void Insert_STAT_LOADData(st_LOAD *pstData, int dTotCount);


/**
 *	Implements func.
 */
int dCheckNTAMLoadList(time_t tLocalTime, st_NTAM * stNTAM)
{
	int dTimeStart;
	st_LOAD * stNTAMLOAD;
	int dAvgValue;

/*
	log_print(LOGN_INFO,"NTAMLOAD RECV CPU[%lld]:[%lld], MEM[%lld]:[%lld], QUE[%lld]:[%lld] NIFO[%lld]:[%lld]",
		stNTAM->cpusts.llCur, stNTAM->cpusts.lMax, stNTAM->memsts.llCur, stNTAM->memsts.lMax,
		stNTAM->queuests.llCur, stNTAM->queuests.lMax, stNTAM->nifosts[0].llCur, stNTAM->nifosts[0].lMax);

	log_print(LOGN_INFO,"NTAMLOAD RECV	disk1[%lld]:[%lld],disk2[%lld]:[%lld],disk3[%lld]:[%lld],disk4[%lld]:[%lld]",
		stNTAM->disksts[0].llCur, stNTAM->disksts[0].lMax, stNTAM->disksts[1].llCur, stNTAM->disksts[1].lMax,
		stNTAM->disksts[2].llCur, stNTAM->disksts[2].lMax, stNTAM->disksts[3].llCur, stNTAM->disksts[3].lMax);
*/

	if(stNTAM->tUpTime/300*300 == tLocalTime)
	{
		dTimeStart = gdTimeFlag * MAX_SYS_CNT;
		log_print(LOGN_INFO, "UPDATE TIME IS CUR TIME");

	}
	else if(stNTAM->tUpTime/300*300 == tLocalTime - 300)
	{
		dTimeStart =( gdTimeFlag^ 0x01) * MAX_SYS_CNT;
	}
	else
	{
		log_print(LOGN_DEBUG, "UPDATE TIME IS OLD TIME");
		return -1;
	}

	stNTAMLOAD = &(pstSHMLOADTbl->stLOAD[DEF_NTAM_POS+dTimeStart]);

	/* cpu*/
	if(stNTAM->cpusts.lMax > 0){
		dAvgValue = stNTAM->cpusts.llCur * 10000/stNTAM->cpusts.lMax;
	}else
		dAvgValue = 0;

	stNTAMLOAD->usCPUAVG =
	((stNTAMLOAD->usCPUAVG * stNTAMLOAD->usCount) + dAvgValue)/(stNTAMLOAD->usCount+1);
	if(	stNTAMLOAD->usCPUMAX < dAvgValue)
	{
		stNTAMLOAD->usCPUMAX = dAvgValue;
	}
	if(	stNTAMLOAD->usCPUMIN > dAvgValue)
	{
		stNTAMLOAD->usCPUMIN = dAvgValue;
	}

	/* memory*/
	if(stNTAM->memsts.lMax/100 > 0){
		dAvgValue = stNTAM->memsts.llCur * 100/(stNTAM->memsts.lMax/100);
	}else
		dAvgValue = 0;

	stNTAMLOAD->usMEMAVG =
	(stNTAMLOAD->usMEMAVG * stNTAMLOAD->usCount + dAvgValue)/(stNTAMLOAD->usCount+1);
	if(	stNTAMLOAD->usMEMMAX < dAvgValue)
	{
		stNTAMLOAD->usMEMMAX = dAvgValue;
	}
	if(	stNTAMLOAD->usMEMMIN > dAvgValue)
	{
		stNTAMLOAD->usMEMMIN = dAvgValue;
	}

	/*queue*/
	if(stNTAM->queuests.lMax/100 > 0){
		dAvgValue = stNTAM->queuests.llCur * 100/(stNTAM->queuests.lMax/100);
	}else
		dAvgValue = 0;

	stNTAMLOAD->usQUEAVG =
	(stNTAMLOAD->usQUEAVG * stNTAMLOAD->usCount + dAvgValue)/(stNTAMLOAD->usCount+1);
	if(	stNTAMLOAD->usQUEMAX < dAvgValue)
	{
		stNTAMLOAD->usQUEMAX = dAvgValue;
	}
	if(	stNTAMLOAD->usQUEMIN > dAvgValue)
	{
		stNTAMLOAD->usQUEMIN = dAvgValue;
	}

	// TODO nifostst[]
	/*nifo*/
	if(stNTAM->nifosts.lMax/100 > 0){
		dAvgValue = stNTAM->nifosts.llCur * 100/(stNTAM->nifosts.lMax/100);
	}else
		dAvgValue = 0;

	stNTAMLOAD->usNIFOAVG =
	(stNTAMLOAD->usNIFOAVG * stNTAMLOAD->usCount + dAvgValue)/(stNTAMLOAD->usCount+1);
	if(	stNTAMLOAD->usNIFOMAX < dAvgValue)
	{
		stNTAMLOAD->usNIFOMAX = dAvgValue;
	}
	if(	stNTAMLOAD->usNIFOMIN > dAvgValue)
	{
		stNTAMLOAD->usNIFOMIN = dAvgValue;
	}

	/*disk1*/
	if(stNTAM->disksts[0].lMax/100 > 0) {
		dAvgValue = stNTAM->disksts[0].llCur * 100/(stNTAM->disksts[0].lMax/100);
/*	}else
		dAvgValue = 0;
*/
		stNTAMLOAD->usDISK1AVG =
		(stNTAMLOAD->usDISK1AVG * stNTAMLOAD->usCount + dAvgValue)/(stNTAMLOAD->usCount+1);
		if(	stNTAMLOAD->usDISK1MAX < dAvgValue)
		{
			stNTAMLOAD->usDISK1MAX = dAvgValue;
		}
		if(	stNTAMLOAD->usDISK1MIN > dAvgValue)
		{
			stNTAMLOAD->usDISK1MIN = dAvgValue;
		}
	}

	/*disk2*/
	if(stNTAM->disksts[1].lMax/100 > 0) {
		dAvgValue = stNTAM->disksts[1].llCur * 100/(stNTAM->disksts[1].lMax/100);
/*	}else
		dAvgValue = 0;
*/

		stNTAMLOAD->usDISK2AVG =
		(stNTAMLOAD->usDISK2AVG * stNTAMLOAD->usCount + dAvgValue)/(stNTAMLOAD->usCount+1);
		if(	stNTAMLOAD->usDISK2MAX < dAvgValue)
		{
			stNTAMLOAD->usDISK2MAX =  dAvgValue;
		}
		if(	stNTAMLOAD->usDISK2MIN >  dAvgValue)
		{
			stNTAMLOAD->usDISK2MIN =  dAvgValue;
		}

	}

	/*disk3*/
	if(stNTAM->disksts[2].lMax/100 > 0) {
		dAvgValue = stNTAM->disksts[2].llCur*100/(stNTAM->disksts[2].lMax/100);
/*	}else
		dAvgValue = 0;
*/

		stNTAMLOAD->usDISK3AVG =
		(stNTAMLOAD->usDISK3AVG * stNTAMLOAD->usCount + dAvgValue)/(stNTAMLOAD->usCount+1);
		if(	stNTAMLOAD->usDISK3MAX <  dAvgValue)
		{
			stNTAMLOAD->usDISK3MAX =  dAvgValue;
		}
		if(	stNTAMLOAD->usDISK3MIN >  dAvgValue)
		{
			stNTAMLOAD->usDISK3MIN =  dAvgValue;
		}
	}
	/*disk4*/
	if(stNTAM->disksts[3].lMax/100 > 0) {
		dAvgValue = stNTAM->disksts[3].llCur*100/(stNTAM->disksts[3].lMax/100);
/*	}else
		dAvgValue = 0;
*/
		stNTAMLOAD->usDISK4AVG =
		(stNTAMLOAD->usDISK4AVG * stNTAMLOAD->usCount + dAvgValue)/(stNTAMLOAD->usCount+1);
		if(	stNTAMLOAD->usDISK4MAX <  dAvgValue)
		{
			stNTAMLOAD->usDISK4MAX =  dAvgValue;
		}
		if(	stNTAMLOAD->usDISK4MIN >  dAvgValue)
		{
			stNTAMLOAD->usDISK4MIN =  dAvgValue;
		}
	}

	stNTAMLOAD->usCount++;
	log_print(LOGN_INFO,"NTAMLOAD SAVE CPU[%u][%u][%u] MEM[%u][%u][%u] QUE[%u][%u][%u] NIFO[%u][%u][%u] COUNT[%u]"
		,stNTAMLOAD->usCPUAVG,stNTAMLOAD->usCPUMAX, stNTAMLOAD->usCPUMIN
		,stNTAMLOAD->usMEMAVG,stNTAMLOAD->usMEMMAX, stNTAMLOAD->usMEMMIN
		,stNTAMLOAD->usQUEAVG,stNTAMLOAD->usQUEMAX, stNTAMLOAD->usQUEMIN
		,stNTAMLOAD->usNIFOAVG,stNTAMLOAD->usNIFOMAX, stNTAMLOAD->usNIFOMIN
		,stNTAMLOAD->usCount);
	log_print(LOGN_INFO,"NTAMLOAD SAVE DISK1[%u][%u][%u] DISK2[%u][%u][%u] DISK3[%u][%u][%u] DISK4[%u][%u][%u]"
		,stNTAMLOAD->usDISK1AVG,stNTAMLOAD->usDISK1MAX, stNTAMLOAD->usDISK1MIN
		,stNTAMLOAD->usDISK2AVG,stNTAMLOAD->usDISK1MAX, stNTAMLOAD->usDISK2MIN
		,stNTAMLOAD->usDISK3AVG,stNTAMLOAD->usDISK2MAX, stNTAMLOAD->usDISK3MIN
		,stNTAMLOAD->usDISK4AVG,stNTAMLOAD->usDISK3MAX, stNTAMLOAD->usDISK4MIN);

return 0;

}

int dCheckNTAFLoadList(time_t tLocalTime, st_NTAF *stNTAF, short usNTAFID)
{
	int			i, dAvgValue, dTimeStart;
	st_LOAD		*stNTAFLOAD;

	log_print(LOGN_INFO, "NTAFLOAD RECV NTAFID[%d] CPU[%lld]:[%lld], MEM[%lld]:[%lld], QUE[%lld]:[%lld], NIFO[%lld][%lld], TRAFFIC[%lld][%lld], DISK[%lld:%lld][%lld:%lld][%lld:%lld][%lld:%lld]",
		usNTAFID,
		stNTAF->cpusts.llCur,stNTAF->cpusts.lMax,
		stNTAF->memsts.llCur, stNTAF->memsts.lMax,
		stNTAF->quests.llCur, stNTAF->quests.lMax,
		stNTAF->nifosts.llCur, stNTAF->nifosts.lMax,
		stNTAF->bytests.llCur, stNTAF->bytests.lMax,
		stNTAF->disksts[0].llCur, stNTAF->disksts[0].lMax,
		stNTAF->disksts[1].llCur, stNTAF->disksts[1].lMax,
		stNTAF->disksts[2].llCur, stNTAF->disksts[2].lMax,
		stNTAF->disksts[3].llCur, stNTAF->disksts[3].lMax);

	if(stNTAF->tUpTime/300*300 == tLocalTime)
	{
		dTimeStart = gdTimeFlag * MAX_SYS_CNT;
		log_print(LOGN_INFO, "UPDATE TIME IS CUR TIME");
	}
	else if(stNTAF->tUpTime/300*300 == tLocalTime - 300)
		dTimeStart =( gdTimeFlag^ 0x01) * MAX_SYS_CNT;
	else
	{
		log_print(LOGN_DEBUG, "UPDATE TIME IS OLD TIME LOC:[%ld]UP:[%lld]",tLocalTime,stNTAF->tUpTime);
		return -1;
	}

	stNTAFLOAD = &(pstSHMLOADTbl->stLOAD[usNTAFID+dTimeStart]);
	log_print(LOGN_INFO,"NTAFLOAD RECV NTAFID[%d][%d]", stNTAFLOAD->usSystemType, stNTAFLOAD->usSystemID);

	/*	CPU	*/
	if(stNTAF->cpusts.lMax > 0)
		dAvgValue = stNTAF->cpusts.llCur * 10000/stNTAF->cpusts.lMax;
	else
		dAvgValue = 0;

	stNTAFLOAD->usCPUAVG = (stNTAFLOAD->usCPUAVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
	if(	stNTAFLOAD->usCPUMAX < dAvgValue)
		stNTAFLOAD->usCPUMAX = dAvgValue;

	if(	stNTAFLOAD->usCPUMIN > dAvgValue)
		stNTAFLOAD->usCPUMIN = dAvgValue;

	/*	MEMORY	*/
	if(stNTAF->memsts.lMax/100 > 0)
		dAvgValue = stNTAF->memsts.llCur * 100/(stNTAF->memsts.lMax/100);
	else
		dAvgValue = 0;

	stNTAFLOAD->usMEMAVG = (stNTAFLOAD->usMEMAVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
	if(	stNTAFLOAD->usMEMMAX < dAvgValue)
		stNTAFLOAD->usMEMMAX = dAvgValue;

	if(	stNTAFLOAD->usMEMMIN > dAvgValue)
		stNTAFLOAD->usMEMMIN = dAvgValue;

	/*	QUEUE	*/
	if(stNTAF->quests.lMax/100 > 0)
		dAvgValue = stNTAF->quests.llCur * 100/(stNTAF->quests.lMax/100);
	else
		dAvgValue = 0;

	stNTAFLOAD->usQUEAVG = (stNTAFLOAD->usQUEAVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
	if(	stNTAFLOAD->usQUEMAX < dAvgValue)
		stNTAFLOAD->usQUEMAX = dAvgValue;

	if(	stNTAFLOAD->usQUEMIN > dAvgValue)
		stNTAFLOAD->usQUEMIN = dAvgValue;

	/*	NIFO	*/
	if(stNTAF->nifosts.lMax/100 > 0)
		dAvgValue = stNTAF->nifosts.llCur * 100/(stNTAF->nifosts.lMax/100);
	else
		dAvgValue = 0;

	stNTAFLOAD->usNIFOAVG = (stNTAFLOAD->usNIFOAVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
	if(	stNTAFLOAD->usNIFOMAX < dAvgValue)
		stNTAFLOAD->usNIFOMAX = dAvgValue;

	if(	stNTAFLOAD->usNIFOMIN > dAvgValue)
		stNTAFLOAD->usNIFOMIN = dAvgValue;

	/*	TRAFFIC	*/
	if(stNTAF->bytests.lMax/100 > 0)
		dAvgValue = stNTAF->bytests.llCur * 100/(stNTAF->bytests.lMax/100);
	else
		dAvgValue = 0;

	stNTAFLOAD->uTrafficAVG = (stNTAFLOAD->uTrafficAVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
	if(	stNTAFLOAD->uTrafficMAX < dAvgValue)
		stNTAFLOAD->uTrafficMAX = dAvgValue;

	if(	stNTAFLOAD->uTrafficMIN > dAvgValue)
		stNTAFLOAD->uTrafficMIN = dAvgValue;

	for(i = 0; i < MAX_NTAF_DISK_COUNT; i++)
	{
		if(stNTAF->disksts[i].lMax/100 > 0)
			dAvgValue = stNTAF->disksts[i].llCur * 100/(stNTAF->disksts[i].lMax/100);
		else
			dAvgValue = 0;

		switch(i)
		{
			case 0:
				stNTAFLOAD->usDISK1AVG = (stNTAFLOAD->usDISK1AVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
				if(	stNTAFLOAD->usDISK1MAX < dAvgValue)
					stNTAFLOAD->usDISK1MAX = dAvgValue;

				if(	dAvgValue && (stNTAFLOAD->usDISK1MIN>dAvgValue))
					stNTAFLOAD->usDISK1MIN = dAvgValue;
				break;
			case 1:
				stNTAFLOAD->usDISK2AVG = (stNTAFLOAD->usDISK2AVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
				if(	stNTAFLOAD->usDISK2MAX < dAvgValue)
					stNTAFLOAD->usDISK2MAX = dAvgValue;

				if(	dAvgValue && (stNTAFLOAD->usDISK2MIN>dAvgValue))
					stNTAFLOAD->usDISK2MIN = dAvgValue;
				break;
			case 2:
				stNTAFLOAD->usDISK3AVG = (stNTAFLOAD->usDISK3AVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
				if(	stNTAFLOAD->usDISK3MAX < dAvgValue)
					stNTAFLOAD->usDISK3MAX = dAvgValue;

				if(	dAvgValue && (stNTAFLOAD->usDISK3MIN>dAvgValue))
					stNTAFLOAD->usDISK3MIN = dAvgValue;
				break;
			case 3:
				stNTAFLOAD->usDISK4AVG = (stNTAFLOAD->usDISK4AVG * stNTAFLOAD->usCount + dAvgValue)/(stNTAFLOAD->usCount+1);
				if(	stNTAFLOAD->usDISK4MAX < dAvgValue)
					stNTAFLOAD->usDISK4MAX = dAvgValue;

				if(	dAvgValue && (stNTAFLOAD->usDISK4MIN>dAvgValue))
					stNTAFLOAD->usDISK4MIN = dAvgValue;
				break;
		}
	}
	stNTAFLOAD->usCount++;

	log_print(LOGN_INFO,"NTAFLOAD SAVE NTAFID[%d][%u][%u] CPU[%u][%u][%u] MEM[%u][%u][%u] QUE[%u][%u][%u] NIFO[%u][%u][%u] TRAFFIC[%u][%u][%u] DISK1[%u][%u][%u] DISK2[%u][%u][%u] DISK3[%u][%u][%u] DISK4[%u][%u][%u] COUNT[%u]" ,
		usNTAFID, stNTAFLOAD->usSystemID, usNTAFID+dTimeStart,
		stNTAFLOAD->usCPUAVG, stNTAFLOAD->usCPUMAX, stNTAFLOAD->usCPUMIN,
		stNTAFLOAD->usMEMAVG, stNTAFLOAD->usMEMMAX, stNTAFLOAD->usMEMMIN,
		stNTAFLOAD->usQUEAVG, stNTAFLOAD->usQUEMAX, stNTAFLOAD->usQUEMIN,
		stNTAFLOAD->usNIFOAVG, stNTAFLOAD->usNIFOMAX, stNTAFLOAD->usNIFOMIN,
		stNTAFLOAD->uTrafficAVG, stNTAFLOAD->uTrafficMAX, stNTAFLOAD->uTrafficMIN,
		stNTAFLOAD->usDISK1AVG, stNTAFLOAD->usDISK1MAX, stNTAFLOAD->usDISK1MIN,
		stNTAFLOAD->usDISK2AVG, stNTAFLOAD->usDISK2MAX, stNTAFLOAD->usDISK2MIN,
		stNTAFLOAD->usDISK3AVG, stNTAFLOAD->usDISK3MAX, stNTAFLOAD->usDISK3MIN,
		stNTAFLOAD->usDISK4AVG, stNTAFLOAD->usDISK4MAX, stNTAFLOAD->usDISK4MIN,
		stNTAFLOAD->usCount);

	return 0;
}

void InitSHMLOAD(short TimeFlag, unsigned int uiStatTime)
{
	int		i, dIndex;

	for(i = 0; i < MAX_SYS_CNT; i++)
	{
		if(i == 0)
		{
			dIndex = DEF_NTAM_POS;
			pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usSystemType		= DEF_NTAM_POS;
			pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usSystemID		= DEF_NTAM_POS;
		}
		else
		{
			if( (gstSubList.stInfo[i-1].dType>DEF_SGTAF_TYPE) || (gstSubList.stInfo[i-1].dType==0))
				continue;

			dIndex	= gstSubList.stInfo[i-1].dNo;
			pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usSystemType	= 1;		/*	NTAF SYSTEMTYPE		*/
			pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usSystemID	= gstSubList.stInfo[i-1].dNo;
		}

		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].uiStatTime	= uiStatTime;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usCPUAVG		= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usCPUMAX		= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usCPUMIN		= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usMEMAVG		= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usMEMMAX		= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usMEMMIN		= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usQUEAVG		= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usQUEMAX		= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usQUEMIN		= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usNIFOAVG	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usNIFOMAX	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usNIFOMIN	= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].uTrafficAVG	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].uTrafficMAX	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].uTrafficMIN	= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK1AVG	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK1MAX	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK1MIN	= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK2AVG	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK2MAX	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK2MIN	= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK3AVG	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK3MAX	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK3MIN	= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK4AVG	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK4MAX	= 0;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK4MIN	= 10000;
		pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usCount		= 0;

		log_print(LOGN_INFO, "######### INIT LOADSHM [%d]########", TimeFlag*MAX_SYS_CNT+dIndex);
		log_print(LOGN_INFO, "usSystemType[%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usSystemType);
		log_print(LOGN_INFO, "usSystemID  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usSystemID);
		log_print(LOGN_INFO, "uiStatTime  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].uiStatTime);
		log_print(LOGN_INFO, "usCPUAVG    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usCPUAVG);
		log_print(LOGN_INFO, "usCPUMAX    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usCPUMAX);
		log_print(LOGN_INFO, "usCPUMIN    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usCPUMIN);
		log_print(LOGN_INFO, "usMEMAVG    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usMEMAVG);
		log_print(LOGN_INFO, "usMEMMAX    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usMEMMAX);
		log_print(LOGN_INFO, "usMEMMIN    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usMEMMIN);
		log_print(LOGN_INFO, "usQUEAVG    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usQUEAVG);
		log_print(LOGN_INFO, "usQUEMAX    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usQUEMAX);
		log_print(LOGN_INFO, "usQUEMIN    [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usQUEMIN);
		log_print(LOGN_INFO, "usNIFOAVG   [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usNIFOAVG);
		log_print(LOGN_INFO, "usNIFOMAX   [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usNIFOMAX);
		log_print(LOGN_INFO, "usNIFOMIN   [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usNIFOMIN);
		log_print(LOGN_INFO, "uTrafficAVG [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].uTrafficAVG);
		log_print(LOGN_INFO, "uTrafficMAX [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].uTrafficMAX);
		log_print(LOGN_INFO, "uTrafficMIN [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].uTrafficMIN);
		log_print(LOGN_INFO, "usDISK1AVG  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK1AVG);
		log_print(LOGN_INFO, "usDISK1MAX  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK1MAX);
		log_print(LOGN_INFO, "usDISK1MIN  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK1MIN);
		log_print(LOGN_INFO, "usDISK2AVG  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK2AVG);
		log_print(LOGN_INFO, "usDISK2MAX  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK2MAX);
		log_print(LOGN_INFO, "usDISK2MIN  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK2MIN);
		log_print(LOGN_INFO, "usDISK3AVG  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK3AVG);
		log_print(LOGN_INFO, "usDISK3MAX  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK3MAX);
		log_print(LOGN_INFO, "usDISK3MIN  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK3MIN);
		log_print(LOGN_INFO, "usDISK4AVG  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK4AVG);
		log_print(LOGN_INFO, "usDISK4MAX  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK4MAX);
		log_print(LOGN_INFO, "usDISK4MIN  [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usDISK4MIN);
		log_print(LOGN_INFO, "usCount 	   [%d]", pstSHMLOADTbl->stLOAD[TimeFlag*MAX_SYS_CNT+dIndex].usCount);
	}

	log_print(LOGN_INFO, "INIT LOADTABLE STATTIME[%u], TIMEFLAG[%d]", uiStatTime, TimeFlag);
}

void Stat_LOADFunc(void *p)
{
	BLOCK_KEY	BLOCKKEY;
	int			i, dTotCount,dIndex, dTimeFlag;
	time_t		tStatTime;
	st_LOAD		*stLOAD, LOADData[MAX_SYS_CNT];

	dTotCount	= 0;
	dTimeFlag	= gdTimeFlag^0x01;

	for(i = 0; i < MAX_SUBSYS_NUM; i++)
	{
		if(i==0)
			dIndex = DEF_NTAM_POS;
		else
		{
			if( (gstSubList.stInfo[i-1].dType > DEF_SGTAF_TYPE) || (gstSubList.stInfo[i-1].dType == 0))
				continue;

			dIndex = gstSubList.stInfo[i-1].dNo;
		}

		memcpy(&LOADData[i], &(pstSHMLOADTbl->stLOAD[dTimeFlag*MAX_SYS_CNT+dIndex]), sizeof(st_LOAD));
		stLOAD = &(pstSHMLOADTbl->stLOAD[dTimeFlag*MAX_SYS_CNT+dIndex]);

		log_print(LOGN_INFO,"LOAD CONV STATTIME[%u] SYSTYPE[%u] SYSID[%u] CPU[%u][%u][%u] MEM[%u][%u][%u] QUE[%u][%u][%u] NIFO[%u][%u][%u] TRAFFIC[%u][%u][%u] COUNT[%u]",
			stLOAD->uiStatTime, stLOAD->usSystemType, stLOAD->usSystemID,
			stLOAD->usCPUAVG, stLOAD->usCPUMAX, stLOAD->usCPUMIN,
			stLOAD->usMEMAVG, stLOAD->usMEMMAX, stLOAD->usMEMMIN,
			stLOAD->usQUEAVG, stLOAD->usQUEMAX, stLOAD->usQUEMIN,
			stLOAD->usNIFOAVG, stLOAD->usNIFOMAX, stLOAD->usNIFOMIN,
			stLOAD->uTrafficAVG, stLOAD->uTrafficMAX, stLOAD->uTrafficMIN,
			stLOAD->usCount);
		log_print(LOGN_INFO,"LOAD CONV DISK1[%u][%u][%u] DISK2[%u][%u][%u] DISK3[%u][%u][%u] DISK4[%u][%u][%u]",
			stLOAD->usDISK1AVG, stLOAD->usDISK1MAX, stLOAD->usDISK1MIN,
			stLOAD->usDISK2AVG, stLOAD->usDISK2MAX, stLOAD->usDISK2MIN,
			stLOAD->usDISK3AVG, stLOAD->usDISK3MAX, stLOAD->usDISK3MIN,
			stLOAD->usDISK4AVG, stLOAD->usDISK4MAX, stLOAD->usDISK4MIN);

		dTotCount ++;
	}

	for(i = 0; i < dTotCount; i++)
	{
		log_print(LOGN_INFO,"LOAD INS STATTIME[%u] SYSTYPE[%u] SYSID[%u] CPU[%u][%u][%u] MEM[%u][%u][%u] QUE[%u][%u][%u] NIFO[%u][%u][%u] TRAFFIC[%u][%u][%u] COUNT[%u]",
			LOADData[i].uiStatTime, LOADData[i].usSystemType, LOADData[i].usSystemID,
			LOADData[i].usCPUAVG, LOADData[i].usCPUMAX, LOADData[i].usCPUMIN,
			LOADData[i].usMEMAVG, LOADData[i].usMEMMAX, LOADData[i].usMEMMIN,
			LOADData[i].usQUEAVG, LOADData[i].usQUEMAX, LOADData[i].usQUEMIN,
			LOADData[i].usNIFOAVG, LOADData[i].usNIFOMAX, LOADData[i].usNIFOMIN,
			LOADData[i].uTrafficAVG, LOADData[i].uTrafficMAX, LOADData[i].uTrafficMIN,
			LOADData[i].usCount);
		log_print(LOGN_INFO,"LOAD INS DISK1[%u][%u][%u] DISK2[%u][%u][%u] DISK3[%u][%u][%u] DISK4[%u][%u][%u]",
			LOADData[i].usDISK1AVG, LOADData[i].usDISK1MAX, LOADData[i].usDISK1MIN,
			LOADData[i].usDISK2AVG, LOADData[i].usDISK2MAX, LOADData[i].usDISK2MIN,
			LOADData[i].usDISK3AVG, LOADData[i].usDISK3MAX, LOADData[i].usDISK3MIN,
			LOADData[i].usDISK4AVG, LOADData[i].usDISK4MAX, LOADData[i].usDISK4MIN);
	}

	Insert_STAT_LOADData(LOADData, dTotCount);

	tStatTime = time(NULL)/300*300+300;
	InitSHMLOAD(dTimeFlag, (unsigned int)tStatTime);

	BLOCKKEY.usLoadFaultFlag	= MID_STAT_LOAD;
	BLOCKKEY.usgdTimeFlag		= dTimeFlag;

	timerNID = timerN_add(pTIMER, Stat_LOADFunc, (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), time(NULL)/300*300+360);
}
