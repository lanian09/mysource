/**		@file	m_svcmon_log.c
 * 		- M_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: m_svcmon_func.c,v 1.5 2011/09/07 07:07:51 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.5 $
 * 		@date		$Date: 2011/09/07 07:07:51 $
 * 		@ref		m_svcmon_init.c m_svcmon_maic.c
 *
 * 		@section	Intro(소개)
 * 		- M_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		@section	Requirement
 *
 **/
//System Header
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
//Common Header
#include "msgdef.h"
#include "procid.h"
//LIB Header
#include "typedef.h"
#include "path.h"
#include "loglib.h"
#include "utillib.h"
//Local Header
#include "m_svcmon_msgq.h"
#include "m_svcmon_util.h"
#include "m_svcmon_func.h"

extern S32			dSISVCMON;

S32 dProcSvcMonMsg2(st_MonTotal *pMON, st_SvcMonMsg *pSvcMonMsg)
{
	int			dRet;
	S8			szTime[BUFSIZ];
	st_SI_DB	*pSIDB;
	st_MonList	*pMonList = NULL;
	FILE		*fp;

	st_MsgQ		stMsgQ;
	st_MsgQSub	*pMsgQSub = (st_MsgQSub *)&stMsgQ.llMType;

	log_print(LOGN_DEBUG, "RCV TIME=%s IDX=%u", util_printtime(pSvcMonMsg->lTime, szTime), pSvcMonMsg->uiIdx);

	pMonList = &pMON->stMonList[pSvcMonMsg->uiIdx];

	pMsgQSub->usType = DEF_SYS;
	pMsgQSub->usSvcID = SID_STATUS;
	pMsgQSub->usMsgID = MID_SVC_MONITOR;
	pSIDB = (st_SI_DB *)stMsgQ.szBody;
	stMsgQ.usBodyLen = sizeof(st_SI_DB);

	memcpy(pSIDB->date, szTime, SIDB_DATE_LEN);		
	pSIDB->date[SIDB_DATE_LEN] = 0x00;
	sprintf(pSIDB->filename, "DQMS01_FQMD_ID%04d_T%s.DAT", pMON->dID, szTime);
	if(++pMON->dID > 9999) pMON->dID = 0;
	sprintf(pSIDB->name, "%s/%s", SVCMON_DATA_PATH, pSIDB->filename);

	/* save file */
	if((fp = fopen(pSIDB->name, "wb")) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d fopen=%d:%s FILE=%s", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno), pSIDB->name);
		return -1;
	}

	if((dRet = fwrite(pMonList, DEF_MONLIST_SIZE, 1, fp)) == 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d fwrite=%d:%s FILE=%s", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno), pSIDB->name);
		return -2;
	}	

	log_print(LOGN_DEBUG, "SAVE NAME=%s FILENAME=%s DATE=%s", pSIDB->name, pSIDB->filename, pSIDB->date);

	/* send signal to si_svcmon */
	if((dRet = dSendMsg(SEQ_PROC_SI_SVCMON, &stMsgQ)) < 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d dSendMsg dRet=%d FILE=%s", __FILE__, __FUNCTION__, __LINE__, dRet, pSIDB->name);
		return -3;
	}

	log_print(LOGN_DEBUG, "SEND NAME=%s FILENAME=%s DATE=%s", pSIDB->name, pSIDB->filename, pSIDB->date);

	return 0;
}

S32 dProcSvcMonMsg(st_MonTotal *pMON, st_SvcMonMsg *pSvcMonMsg)
{
	int				dRet, i, j, k;
	S8				szTime[BUFSIZ];
	st_SFILE_List	stSFILEList;
	st_SFILE		*pSFILE, *pData, *pDef;
	st_MonList		*pMonList = NULL;
	st_FirstMonList	*pFirstList = NULL;
	st_MonCore		*pCore = NULL;
	st_MonSvc		*pSvc = NULL;
	st_MonBTS		*pBTS = NULL;
	st_MonFA		*pFA = NULL;
	st_MonSec		*pSec = NULL;
	U8				szFile[MAX_SVCMONFILE_CNT][MAX_SVCMONFNAME_SIZE] = {
						"CORMON", "CORDFT", "BSCMON", "BSCDFT", "BTSMON", "BTSDFT", "FAMON", 
						"FADFT", "SECMON", "SECDFT", "SVCMON", "SVCDFT" };

	st_MsgQ		stMsgQ;
	st_MsgQSub	*pMsgQSub = (st_MsgQSub *)&stMsgQ.llMType;

	log_print(LOGN_DEBUG, "RCV TIME=%s IDX=%u", "ffe", 2);//util_printtime(pSvcMonMsg->lTime, szTime), pSvcMonMsg->uiIdx);

	pMonList = &pMON->stMonList[pSvcMonMsg->uiIdx];
	pFirstList = &pMonList->stFirstMonList;

	/* open file */
	for(i = 0; i < MAX_SVCMONFILE_CNT; i++)
	{
		pSFILE = &stSFILEList.stSFILE[i];

		memcpy(pSFILE->stSIDB.date, szTime, SIDB_DATE_LEN);		
		pSFILE->stSIDB.date[SIDB_DATE_LEN] = 0x00;
		sprintf(pSFILE->stSIDB.filename, "DQM01_%s_M_%.*s.DAT", szFile[i], 12, szTime);
		sprintf(pSFILE->stSIDB.name, "%s/%s", SVCMON_DATA_PATH, pSFILE->stSIDB.filename);

		umask(000);
		if((pSFILE->fp = fopen(pSFILE->stSIDB.name, "ab")) == NULL) {
			log_print(LOGN_CRI, "F=%s:%s.%d fopen=%d:%s FILE=%s", 
				__FILE__, __FUNCTION__, __LINE__, errno, strerror(errno), pSFILE->stSIDB.name);
			if( pSFILE->fp != NULL ) /* added by uamyd 20100903 : if file exist, close */
				vCloseFile(i+1, &stSFILEList);
#if _DEBUG_
			else log_print(LOGN_INFO, "pSFILE->fp is NULL ...");
#endif
			return -1;
		}
		log_print(LOGN_DEBUG, "OPEN NAME=%s FILENAME=%s DATE=%s", pSFILE->stSIDB.name, pSFILE->stSIDB.filename, pSFILE->stSIDB.date);
	}

	/* save file */
	/* CORE : PCF */
	for(i = 0; i < pMonList->usPCFCnt; i++)
	{
		pCore = &pMonList->stMonPCF[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* CORE : PDSN */
	for(i = 0; i < pFirstList->usPDSNListCnt; i++)
	{
		pCore = &pFirstList->stPDSN[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* CORE : AAA */
	for(i = 0; i < pFirstList->usAAAListCnt; i++)
	{
		pCore = &pFirstList->stAAA[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* CORE : HSS */
	for(i = 0; i < pFirstList->usHSSListCnt; i++)
	{
		pCore = &pFirstList->stHSS[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* CORE : LNS */
	for(i = 0; i < pFirstList->usLNSListCnt; i++)
	{
		pCore = &pFirstList->stLNS[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* SERVICE */
	for(i = 0; i < pMonList->usSvcCnt; i++)
	{
		pSvc = &pMonList->stMonSvc[i];
		pData = &stSFILEList.stSFILE[MONTYPE_SVC_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_SVC_DEFECT];

		vSaveSVCData(pData->fp, pSvc->ucSvcType, pSvc->uiIPAddr, pSvc->SvcL4Type, &pSvc->stMonAlarm, &pSvc->stMonInfo);
		vSaveSVCDefect(pDef->fp, pSvc->ucSvcType, pSvc->uiIPAddr, pSvc->SvcL4Type, &pSvc->stDefect);
	}

	/* BTS */
	for(i = 0; i < pMonList->usBTSCnt; i++)
	{
		pBTS = &pMonList->stMonBTS[i];
		pData = &stSFILEList.stSFILE[MONTYPE_BTS_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_BTS_DEFECT];

		vSaveBTSData(pData->fp, pBTS->ullBTS, &pBTS->stMonAlarm, &pBTS->stMonInfo);
		vSaveBTSDefect(pDef->fp, pBTS->ullBTS, &pBTS->stDefect);

		/* FA */
		for(j = 0; j < MAX_MON_FA_CNT; j++)
		{
			pFA = &pBTS->stMonFA[j];
			pData = &stSFILEList.stSFILE[MONTYPE_FA_DATA];
			pDef = &stSFILEList.stSFILE[MONTYPE_FA_DEFECT];

			vSaveFAData(pData->fp, pBTS->ullBTS, pFA->ucFA, &pFA->stMonAlarm, &pFA->stMonInfoS);
			vSaveFADefect(pDef->fp, pBTS->ullBTS, pFA->ucFA, &pFA->stDefectS);

			/* SEC */
			for(k = 0; k < MAX_MON_SEC_CNT; k++)
			{
				pSec = &pFA->stMonSec[k];
				pData = &stSFILEList.stSFILE[MONTYPE_SEC_DATA];
				pDef = &stSFILEList.stSFILE[MONTYPE_SEC_DEFECT];

				vSaveSECData(pData->fp, pBTS->ullBTS, pFA->ucFA, pSec->ucSec, &pSec->stMonAlarm, &pSec->stMonInfoS);
				vSaveSECDefect(pDef->fp, pBTS->ullBTS, pFA->ucFA, pSec->ucSec, &pSec->stDefectS);
			}
		}
	}

	/* close file */
	vCloseFile(MAX_SVCMONFILE_CNT, &stSFILEList);

	/* send file */
	pMsgQSub->usType = DEF_SYS;
	pMsgQSub->usSvcID = SID_STATUS;
	pMsgQSub->usMsgID = MID_SVC_MONITOR;
	stMsgQ.usBodyLen = sizeof(st_SI_DB);

	for(i = 0; i < MAX_SVCMONFILE_CNT; i++)
	{
		pSFILE = &stSFILEList.stSFILE[i];

		memcpy(stMsgQ.szBody, &pSFILE->stSIDB, sizeof(st_SI_DB));

		/* send signal to si_svcmon */
		if((dRet = dSendMsg(dSISVCMON, &stMsgQ)) < 0) {
			log_print(LOGN_CRI, "F=%s:%s.%d dSendMsg dRet=%d FILE=%s", __FILE__, __FUNCTION__, __LINE__, dRet, pSFILE->stSIDB.name);
			return -3;
		}

		log_print(LOGN_DEBUG, "SEND NAME=%s FILENAME=%s DATE=%s", pSFILE->stSIDB.name, pSFILE->stSIDB.filename, pSFILE->stSIDB.date);
	}
	if(++pMON->dID > 9999) pMON->dID = 0;

	return 0;
}

S32 dProcSvcMon1MinMsg(st_MonTotal_1Min *pMON, st_SvcMonMsg *pSvcMonMsg)
{
	int				dRet, i;
	S8				szTime[BUFSIZ];
	st_SFILE_List	stSFILEList;
	st_SFILE		*pSFILE, *pData, *pDef;
	st_MonList_1Min	*pMonList = NULL;
	st_FirstMonList	*pFirstList = NULL;
	st_MonCore		*pCore = NULL;
	st_MonBSC		*pBSC = NULL;
	U8				szFile[MAX_SVCMONFILE_CNT][MAX_SVCMONFNAME_SIZE] = {
						"CORMON", "CORDFT", "BSCMON", "BSCDFT", "BTSMON", "BTSDFT", "FAMON", 
						"FADFT", "SECMON", "SECDFT", "SVCMON", "SVCDFT" };

	st_MsgQ		stMsgQ;
	st_MsgQSub	*pMsgQSub = (st_MsgQSub *)&stMsgQ.llMType;

	log_print(LOGN_DEBUG, "RCV TIME=%s IDX=%u", util_printtime(pSvcMonMsg->lTime, szTime), pSvcMonMsg->uiIdx);
	if( pSvcMonMsg->uiIdx > TOTAL_MONLIST_1MIN_CNT ) {
		log_print(LOGN_CRI, "IDX TOO HIGH. pSvcMonMsg->uiIdx[%u]", pSvcMonMsg->uiIdx);
		return -1;
	}

	pMonList = &pMON->stMonList1Min[pSvcMonMsg->uiIdx];
	pFirstList = &pMonList->stFirstMonList;

	/* open file */
	for(i = 0; i < MAX_SVCMONFILE_CNT; i++)
	{
		pSFILE = &stSFILEList.stSFILE[i];

		memcpy(pSFILE->stSIDB.date, szTime, SIDB_DATE_LEN);		
		pSFILE->stSIDB.date[SIDB_DATE_LEN] = 0x00;
		sprintf(pSFILE->stSIDB.filename, "DQM01_%s_M_%.*s.DAT", szFile[i], 12, szTime);
		sprintf(pSFILE->stSIDB.name, "%s/%s", SVCMON_DATA_PATH, pSFILE->stSIDB.filename);

		umask(000);
		if((pSFILE->fp = fopen(pSFILE->stSIDB.name, "ab")) == NULL) {
			log_print(LOGN_CRI, "F=%s:%s.%d fopen=%d:%s FILE=%s", 
				__FILE__, __FUNCTION__, __LINE__, errno, strerror(errno), pSFILE->stSIDB.name);
			if( pSFILE->fp != NULL ) /* added by uamyd 20100903 : if file exist, close */
				vCloseFile(i+1, &stSFILEList);
#if _DEBUG_
			else log_print(LOGN_INFO, "pSFILE->fp is NULL ...");
#endif
			return -1;
		}
		log_print(LOGN_DEBUG, "OPEN NAME=%s FILENAME=%s DATE=%s", pSFILE->stSIDB.name, pSFILE->stSIDB.filename, pSFILE->stSIDB.date);
	}

	/* save file */
	/* CORE : PDSN */
	for(i = 0; i < pFirstList->usPDSNListCnt; i++)
	{
		pCore = &pFirstList->stPDSN[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* CORE : AAA */
	for(i = 0; i < pFirstList->usAAAListCnt; i++)
	{
		pCore = &pFirstList->stAAA[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* CORE : HSS */
	for(i = 0; i < pFirstList->usHSSListCnt; i++)
	{
		pCore = &pFirstList->stHSS[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* CORE : LNS */
	for(i = 0; i < pFirstList->usLNSListCnt; i++)
	{
		pCore = &pFirstList->stLNS[i];
		pData = &stSFILEList.stSFILE[MONTYPE_CORE_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_CORE_DEFECT];

		vSaveCOREData(pData->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stMonAlarm, &pCore->stMonInfo);
		vSaveCOREDefect(pDef->fp, pCore->ucOffice, pCore->ucSysType, pCore->uiIPAddr, &pCore->stDefect);
	}

	/* BSC */
	for(i = 0; i < pMonList->usBSCCnt; i++)
	{
		pBSC = &pMonList->stMonBSC[i];
		pData = &stSFILEList.stSFILE[MONTYPE_BSC_DATA];
		pDef = &stSFILEList.stSFILE[MONTYPE_BSC_DEFECT];

		vSaveBSCData(pData->fp, pBSC->uiBSC, &pBSC->stMonAlarm, &pBSC->stMonInfo);
		vSaveBSCDefect(pDef->fp, pBSC->uiBSC, &pBSC->stDefect);
	}

	/* close file */
	vCloseFile(MAX_SVCMONFILE_CNT, &stSFILEList);

	/* send file */
	pMsgQSub->usType = DEF_SYS;
	pMsgQSub->usSvcID = SID_STATUS;
	pMsgQSub->usMsgID = MID_SVC_MONITOR;
	stMsgQ.usBodyLen = sizeof(st_SI_DB);

	for(i = 0; i < MAX_SVCMONFILE_CNT; i++)
	{
		pSFILE = &stSFILEList.stSFILE[i];

		memcpy(stMsgQ.szBody, &pSFILE->stSIDB, sizeof(st_SI_DB));

		/* send signal to si_svcmon */
		if((dRet = dSendMsg(dSISVCMON, &stMsgQ)) < 0) {
			log_print(LOGN_CRI, "F=%s:%s.%d dSendMsg dRet=%d FILE=%s", __FILE__, __FUNCTION__, __LINE__, dRet, pSFILE->stSIDB.name);
			return -3;
		}

		log_print(LOGN_DEBUG, "SEND NAME=%s FILENAME=%s DATE=%s", pSFILE->stSIDB.name, pSFILE->stSIDB.filename, pSFILE->stSIDB.date);
	}
	if(++pMON->dID > 9999) pMON->dID = 0;

	return 0;
}

void vCloseFile(int cnt, st_SFILE_List *pList)
{
	int		i;

	for(i = 0; i < cnt; i++) {
		if( pList->stSFILE[i].fp != NULL )	/* added by uamyd, 20100913 */
			fclose(pList->stSFILE[i].fp);
	}
}

void vSaveCOREData(FILE *fp, U32 office, U32 systype, U32 ip, st_MonAlarm *pAlarm, st_MonInfo *pInfo)
{
	S8						szIP[INET_ADDRSTRLEN];

	if(office != OFFICE_UNKNOWN) 
	{
		/* Office */
		fprintf(fp, "%u\t", office);
		/* SysType */
		fprintf(fp, "%u\t", systype);
		/* IP */
		fprintf(fp, "%s\t", util_cvtipaddr(szIP, ip));
	
		vSaveData(fp, pAlarm, pInfo);

		fprintf(fp, "\n");
	}
}

void vSaveCOREDefect(FILE *fp, U32 office, U32 systype, U32 ip, st_Defect *pDefect)
{
	int			i;
	S8			szIP[INET_ADDRSTRLEN];

	if(office != OFFICE_UNKNOWN) 
	{
		for(i = 0; i < MAX_MON_DEFECT_IDX; i++)
		{
			if(pDefect->uiFail[i] > 0) 
			{
				/* Office */
				fprintf(fp, "%u\t", office);
				/* SysType */
				fprintf(fp, "%u\t", systype);
				/* IP */
				fprintf(fp, "%s\t", util_cvtipaddr(szIP, ip));

				/* Defect Index */
				fprintf(fp, "%u\t", i);
				/* Error Count */
				fprintf(fp, "%u\t", pDefect->uiFail[i]);
		
				fprintf(fp, "\n");
			}
		}
	}
}

void vSaveBSCData(FILE *fp, U32 bsc, st_MonAlarm *pAlarm, st_MonInfo *pInfo)
{
	st_SubBSC				*pSubBSC = (st_SubBSC *)&bsc;

	if(pSubBSC->ucOffice != OFFICE_UNKNOWN) 
	{
		/* Office */
		fprintf(fp, "%u\t", pSubBSC->ucOffice);
		/* SYSID */
		fprintf(fp, "%u\t", pSubBSC->ucSYSID);
		/* BSCID */
		fprintf(fp, "%u\t", pSubBSC->ucBSCID);
	
		vSaveData(fp, pAlarm, pInfo);

		fprintf(fp, "\n");
	}
}

void vSaveBSCDefect(FILE *fp, U32 bsc, st_Defect *pDefect)
{
	int			i;
	st_SubBSC	*pSubBSC = (st_SubBSC *)&bsc;

	if(pSubBSC->ucOffice != OFFICE_UNKNOWN) 
	{
		for(i = 0; i < MAX_MON_DEFECT_IDX; i++)
		{
			if(pDefect->uiFail[i] > 0) 
			{
				/* Office */
				fprintf(fp, "%u\t", pSubBSC->ucOffice);
				/* SYSID */
				fprintf(fp, "%u\t", pSubBSC->ucSYSID);
				/* BSCID */
				fprintf(fp, "%u\t", pSubBSC->ucBSCID);

				/* Defect Index */
				fprintf(fp, "%u\t", i);
				/* Error Count */
				fprintf(fp, "%u\t", pDefect->uiFail[i]);
		
				fprintf(fp, "\n");
			}
		}
	}
}

void vSaveBTSData(FILE *fp, U64 bts, st_MonAlarm *pAlarm, st_MonInfo *pInfo)
{
	st_SubBTS				*pSubBTS = (st_SubBTS *)&bts;

	if(pSubBTS->ucOffice != OFFICE_UNKNOWN) 
	{
		/* Office */
		fprintf(fp, "%u\t", pSubBTS->ucOffice);
		/* SYSID */
		fprintf(fp, "%u\t", pSubBTS->ucSYSID);
		/* BSCID */
		fprintf(fp, "%u\t", pSubBTS->ucBSCID);
		/* BTSID */
		fprintf(fp, "%u\t", pSubBTS->usBTSID);
	
		vSaveData(fp, pAlarm, pInfo);

		fprintf(fp, "\n");
	}
}

void vSaveBTSDefect(FILE *fp, U64 bts, st_Defect *pDefect)
{
	int			i;
	st_SubBTS	*pSubBTS = (st_SubBTS *)&bts;

	if(pSubBTS->ucOffice != OFFICE_UNKNOWN) 
	{
		for(i = 0; i < MAX_MON_DEFECT_IDX; i++)
		{
			if(pDefect->uiFail[i] > 0) 
			{
				/* Office */
				fprintf(fp, "%u\t", pSubBTS->ucOffice);
				/* SYSID */
				fprintf(fp, "%u\t", pSubBTS->ucSYSID);
				/* BSCID */
				fprintf(fp, "%u\t", pSubBTS->ucBSCID);
				/* BTSID */
				fprintf(fp, "%u\t", pSubBTS->usBTSID);

				/* Defect Index */
				fprintf(fp, "%u\t", i);
				/* Error Count */
				fprintf(fp, "%u\t", pDefect->uiFail[i]);
		
				fprintf(fp, "\n");
			}
		}
	}
}

void vSaveFAData(FILE *fp, U64 bts, U32 fa, st_MonAlarm *pAlarm, st_MonInfoS *pInfoS)
{
	st_SubBTS				*pSubBTS = (st_SubBTS *)&bts;

	if(pSubBTS->ucOffice != OFFICE_UNKNOWN) 
	{
		/* Office */
		fprintf(fp, "%u\t", pSubBTS->ucOffice);
		/* SYSID */
		fprintf(fp, "%u\t", pSubBTS->ucSYSID);
		/* BSCID */
		fprintf(fp, "%u\t", pSubBTS->ucBSCID);
		/* BTSID */
		fprintf(fp, "%u\t", pSubBTS->usBTSID);
		/* FAID */
		fprintf(fp, "%u\t", fa);
	
		vSaveDataS(fp, pAlarm, pInfoS);

		fprintf(fp, "\n");
	}
}

void vSaveFADefect(FILE *fp, U64 bts, U32 fa, st_DefectS *pDefectS)
{
	int			i;
	st_SubBTS	*pSubBTS = (st_SubBTS *)&bts;

	if(pSubBTS->ucOffice != OFFICE_UNKNOWN) 
	{
		for(i = 0; i < MAX_MON_DEFECT_IDX; i++)
		{
			if(pDefectS->usFail[i] > 0) 
			{
				/* Office */
				fprintf(fp, "%u\t", pSubBTS->ucOffice);
				/* SYSID */
				fprintf(fp, "%u\t", pSubBTS->ucSYSID);
				/* BSCID */
				fprintf(fp, "%u\t", pSubBTS->ucBSCID);
				/* BTSID */
				fprintf(fp, "%u\t", pSubBTS->usBTSID);
				/* FAID */
				fprintf(fp, "%u\t", fa);

				/* Defect Index */
				fprintf(fp, "%u\t", i);
				/* Error Count */
				fprintf(fp, "%u\t", pDefectS->usFail[i]);
		
				fprintf(fp, "\n");
			}
		}
	}
}

void vSaveSECData(FILE *fp, U64 bts, U32 fa, U32 sec, st_MonAlarm *pAlarm, st_MonInfoS *pInfoS)
{
	st_SubBTS				*pSubBTS = (st_SubBTS *)&bts;

	if(pSubBTS->ucOffice != OFFICE_UNKNOWN) 
	{
		/* Office */
		fprintf(fp, "%u\t", pSubBTS->ucOffice);
		/* SYSID */
		fprintf(fp, "%u\t", pSubBTS->ucSYSID);
		/* BSCID */
		fprintf(fp, "%u\t", pSubBTS->ucBSCID);
		/* BTSID */
		fprintf(fp, "%u\t", pSubBTS->usBTSID);
		/* FAID */
		fprintf(fp, "%u\t", fa);
		/* SECID */
		fprintf(fp, "%u\t", sec);
	
		vSaveDataS(fp, pAlarm, pInfoS);

		fprintf(fp, "\n");
	}
}

void vSaveSECDefect(FILE *fp, U64 bts, U32 fa, U32 sec, st_DefectS *pDefectS)
{
	int			i;
	st_SubBTS	*pSubBTS = (st_SubBTS *)&bts;

	if(pSubBTS->ucOffice != OFFICE_UNKNOWN) 
	{
		for(i = 0; i < MAX_MON_DEFECT_IDX; i++)
		{
			if(pDefectS->usFail[i] > 0) 
			{
				/* Office */
				fprintf(fp, "%u\t", pSubBTS->ucOffice);
				/* SYSID */
				fprintf(fp, "%u\t", pSubBTS->ucSYSID);
				/* BSCID */
				fprintf(fp, "%u\t", pSubBTS->ucBSCID);
				/* BTSID */
				fprintf(fp, "%u\t", pSubBTS->usBTSID);
				/* FAID */
				fprintf(fp, "%u\t", fa);
				/* SECID */
				fprintf(fp, "%u\t", sec);

				/* Defect Index */
				fprintf(fp, "%u\t", i);
				/* Error Count */
				fprintf(fp, "%u\t", pDefectS->usFail[i]);
		
				fprintf(fp, "\n");
			}
		}
	}
}

void vSaveSVCData(FILE *fp, U32 svcidx, U32 ip, U32 l4svctype, st_MonAlarm *pAlarm, st_MonInfo *pInfo)
{
	S8						szIP[INET_ADDRSTRLEN];

	/* SvcIdx */
	fprintf(fp, "%u\t", svcidx);
	/* IP */
	fprintf(fp, "%s\t", util_cvtipaddr(szIP, ip));
	/* L4 SVC Type */
	fprintf(fp, "%u\t", l4svctype);
	
	vSaveData(fp, pAlarm, pInfo);

	fprintf(fp, "\n");
}

void vSaveSVCDefect(FILE *fp, U32 svcidx, U32 ip, U32 l4svctype, st_Defect *pDefect)
{
	int			i;
	S8			szIP[INET_ADDRSTRLEN];

	for(i = 0; i < MAX_MON_DEFECT_IDX; i++)
	{
		if(pDefect->uiFail[i] > 0) 
		{
			/* SvcIdx */
			fprintf(fp, "%u\t", svcidx);
			/* IP */
			fprintf(fp, "%s\t", util_cvtipaddr(szIP, ip));
			/* L4 SVC Type */
			fprintf(fp, "%u\t", l4svctype);

			/* Defect Index */
			fprintf(fp, "%u\t", i);
			/* Error Count */
			fprintf(fp, "%u\t", pDefect->uiFail[i]);
		
			fprintf(fp, "\n");
		}
	}
}

void vSaveData(FILE *fp, st_MonAlarm *pAlarm, st_MonInfo *pInfo)
{
	st_SubAlarmSysStatus	*pSYS = (st_SubAlarmSysStatus *)&pAlarm->ucAlarmSysStatus;
	st_SubAlarmSvcStatus	*pSVC = (st_SubAlarmSvcStatus *)&pAlarm->szAlarmSvcStatus[0];

	/* Total Alarm */
	fprintf(fp, "%u\t", pAlarm->ucAlarm);
	/* Call Alarm */
	fprintf(fp, "%u\t", pSYS->ucCall);
	/* ReCall Alarm */
	//fprintf(fp, "%u\t", pSYS->ucReCall);
	/* AAA Alarm */
	fprintf(fp, "%u\t", pSYS->ucAAA);
	/* HSS Alarm */
	fprintf(fp, "%u\t", pSYS->ucHSS);
	/* LNS Alarm */
	fprintf(fp, "%u\t", pSYS->ucLNS);
	/* SVC01:MENU Alarm */
	fprintf(fp, "%u\t", pSVC->ucMENU);
	/* SVC02:DN Alarm */
	fprintf(fp, "%u\t", pSVC->ucDN);
	/* SVC03:STREAM Alarm */
	fprintf(fp, "%u\t", pSVC->ucSTREAM);
	/* SVC04:MMS Alarm */
	fprintf(fp, "%u\t", pSVC->ucMMS);
	/* SVC05:WIDGET Alarm */
	fprintf(fp, "%u\t", pSVC->ucWIDGET);
	/* SVC06:PHONE Alarm */
	fprintf(fp, "%u\t", pSVC->ucPHONE);
	/* SVC07:EMS Alarm */
	fprintf(fp, "%u\t", pSVC->ucEMS);
	/* SVC08:BANK Alarm */
	fprintf(fp, "%u\t", pSVC->ucBANK);
	/* SVC09:FV Alarm */
	fprintf(fp, "%u\t", pSVC->ucFV);
	/* SVC10:IM Alarm */
	fprintf(fp, "%u\t", pSVC->ucIM);
	/* SVC11:VT Alarm */
	fprintf(fp, "%u\t", pSVC->ucVT);
	/* SVC12:ETC Alarm */
	fprintf(fp, "%u\t", pSVC->ucETC);
	/* SVC13:CORP Alarm */
	fprintf(fp, "%u\t", pSVC->ucCORP);
	/* SVC14:REGI Alarm */
	fprintf(fp, "%u\t", pSVC->ucREGI);
	/* SVC15:ROAM Alarm */
	//fprintf(fp, "%u\t", pSVC->ucROAM);
	fprintf(fp, "%u\t", 0);
	/* Call Trial */
	fprintf(fp, "%u\t", pInfo->uiCall[0]);
	/* Call Success */
	fprintf(fp, "%u\t", pInfo->uiCall[1]);
	/* ReCall Trial */
	//fprintf(fp, "%u\t", pInfo->uiReCall[0]);
	/* ReCall Success */
	//fprintf(fp, "%u\t", pInfo->uiReCall[1]);
	/* AAA Trial */
	fprintf(fp, "%u\t", pInfo->uiAAA[0]);
	/* AAA Success */
	fprintf(fp, "%u\t", pInfo->uiAAA[1]);
	/* HSS Trial */
	fprintf(fp, "%u\t", pInfo->uiHSS[0]);
	/* HSS Success */
	fprintf(fp, "%u\t", pInfo->uiHSS[1]);
	/* LNS Trial */
	fprintf(fp, "%u\t", pInfo->uiLNS[0]);
	/* LNS Success */
	fprintf(fp, "%u\t", pInfo->uiLNS[1]);
	/* SVC01:MENU Trial */
	fprintf(fp, "%u\t", pInfo->uiService[0][0]);
	/* SVC01:MENU Success */
	fprintf(fp, "%u\t", pInfo->uiService[0][1]);
	/* SVC02:DN Trial */
	fprintf(fp, "%u\t", pInfo->uiService[1][0]);
	/* SVC02:DN Success */
	fprintf(fp, "%u\t", pInfo->uiService[1][1]);
	/* SVC03:STREAM Trial */
	fprintf(fp, "%u\t", pInfo->uiService[2][0]);
	/* SVC03:STREAM Success */
	fprintf(fp, "%u\t", pInfo->uiService[2][1]);
	/* SVC04:MMS Trial */
	fprintf(fp, "%u\t", pInfo->uiService[3][0]);
	/* SVC04:MMS Success */
	fprintf(fp, "%u\t", pInfo->uiService[3][1]);
	/* SVC05:WIDGET Trial */
	fprintf(fp, "%u\t", pInfo->uiService[4][0]);
	/* SVC05:WIDGET Success */
	fprintf(fp, "%u\t", pInfo->uiService[4][1]);
	/* SVC06:PHONE Trial */
	fprintf(fp, "%u\t", pInfo->uiService[5][0]);
	/* SVC06:PHONE Success */
	fprintf(fp, "%u\t", pInfo->uiService[5][1]);
	/* SVC07:EMS Trial */
	fprintf(fp, "%u\t", pInfo->uiService[6][0]);
	/* SVC07:EMS Success */
	fprintf(fp, "%u\t", pInfo->uiService[6][1]);
	/* SVC08:BANK Trial */
	fprintf(fp, "%u\t", pInfo->uiService[7][0]);
	/* SVC08:BANK Success */
	fprintf(fp, "%u\t", pInfo->uiService[7][1]);
	/* SVC09:FV Trial */
	fprintf(fp, "%u\t", pInfo->uiService[8][0]);
	/* SVC09:FV Success */
	fprintf(fp, "%u\t", pInfo->uiService[8][1]);
	/* SVC10:IM Trial */
	fprintf(fp, "%u\t", pInfo->uiService[9][0]);
	/* SVC10:IM Success */
	fprintf(fp, "%u\t", pInfo->uiService[9][1]);
	/* SVC11:VT Trial */
	fprintf(fp, "%u\t", pInfo->uiService[10][0]);
	/* SVC11:VT Success */
	fprintf(fp, "%u\t", pInfo->uiService[10][1]);
	/* SVC12:ETC Trial */
	fprintf(fp, "%u\t", pInfo->uiService[11][0]);
	/* SVC12:ETC Success */
	fprintf(fp, "%u\t", pInfo->uiService[11][1]);
	/* SVC13:CORP Trial */
	fprintf(fp, "%u\t", pInfo->uiService[12][0]);
	/* SVC13:CORP Success */
	fprintf(fp, "%u\t", pInfo->uiService[12][1]);
	/* SVC14:REGI Trial */
	fprintf(fp, "%u\t", pInfo->uiService[13][0]);
	/* SVC14:REGI Success */
	fprintf(fp, "%u\t", pInfo->uiService[13][1]);
	/* SVC19:ROAM Trial */
	fprintf(fp, "%u\t", pInfo->uiService[18][0]);
	/* SVC19:ROAM Success */
	fprintf(fp, "%u\t", pInfo->uiService[18][1]);
	/* SVC15:INET Trial */
	//fprintf(fp, "%u\t", pInfo->uiService[14][0]);
	/* SVC15:INET Success */
	//fprintf(fp, "%u\t", pInfo->uiService[14][1]);
	/* SVC16:INET_RECV Trial */
	//fprintf(fp, "%u\t", pInfo->uiService[15][0]);
	/* SVC16:INET_RECV Success */
	//fprintf(fp, "%u\t", pInfo->uiService[15][1]);
	/* SVC17:IM_RECV Trial */
	//fprintf(fp, "%u\t", pInfo->uiService[16][0]);
	/* SVC17:IM_RECV Success */
	//fprintf(fp, "%u\t", pInfo->uiService[16][1]);
	/* SVC18:VT_RECV Trial */
	//fprintf(fp, "%u\t", pInfo->uiService[17][0]);
	/* SVC18:VT_RECV Success */
	//fprintf(fp, "%u\t", pInfo->uiService[17][1]);
	/* SVC19:ROAM Trial */
	//fprintf(fp, "%u\t", pInfo->uiService[18][0]);
	/* SVC19:ROAM Success */
	//fprintf(fp, "%u\t", pInfo->uiService[18][1]);
}

void vSaveDataS(FILE *fp, st_MonAlarm *pAlarm, st_MonInfoS *pInfoS)
{
	st_SubAlarmSysStatus	*pSYS = (st_SubAlarmSysStatus *)&pAlarm->ucAlarmSysStatus;
	st_SubAlarmSvcStatus	*pSVC = (st_SubAlarmSvcStatus *)&pAlarm->szAlarmSvcStatus[0];

	/* Total Alarm */
	fprintf(fp, "%u\t", pAlarm->ucAlarm);
	/* Call Alarm */
	fprintf(fp, "%u\t", pSYS->ucCall);
	/* ReCall Alarm */
	//fprintf(fp, "%u\t", pSYS->ucReCall);
	/* AAA Alarm */
	fprintf(fp, "%u\t", pSYS->ucAAA);
	/* HSS Alarm */
	fprintf(fp, "%u\t", pSYS->ucHSS);
	/* LNS Alarm */
	fprintf(fp, "%u\t", pSYS->ucLNS);
	/* SVC01:MENU Alarm */
	fprintf(fp, "%u\t", pSVC->ucMENU);
	/* SVC02:DN Alarm */
	fprintf(fp, "%u\t", pSVC->ucDN);
	/* SVC03:STREAM Alarm */
	fprintf(fp, "%u\t", pSVC->ucSTREAM);
	/* SVC04:MMS Alarm */
	fprintf(fp, "%u\t", pSVC->ucMMS);
	/* SVC05:WIDGET Alarm */
	fprintf(fp, "%u\t", pSVC->ucWIDGET);
	/* SVC06:PHONE Alarm */
	fprintf(fp, "%u\t", pSVC->ucPHONE);
	/* SVC07:EMS Alarm */
	fprintf(fp, "%u\t", pSVC->ucEMS);
	/* SVC08:BANK Alarm */
	fprintf(fp, "%u\t", pSVC->ucBANK);
	/* SVC09:FV Alarm */
	fprintf(fp, "%u\t", pSVC->ucFV);
	/* SVC10:IM Alarm */
	fprintf(fp, "%u\t", pSVC->ucIM);
	/* SVC11:VT Alarm */
	fprintf(fp, "%u\t", pSVC->ucVT);
	/* SVC12:ETC Alarm */
	fprintf(fp, "%u\t", pSVC->ucETC);
	/* SVC13:CORP Alarm */
	fprintf(fp, "%u\t", pSVC->ucCORP);
	/* SVC14:REGI Alarm */
	fprintf(fp, "%u\t", pSVC->ucREGI);
	/* SVC15:ROAM Alarm */
	//fprintf(fp, "%u\t", pSVC->ucROAM);
	fprintf(fp, "%u\t", 0);
	/* Call Trial */
	fprintf(fp, "%u\t", pInfoS->usCall[0]);
	/* Call Success */
	fprintf(fp, "%u\t", pInfoS->usCall[1]);
	/* ReCall Trial */
	//fprintf(fp, "%u\t", pInfoS->usReCall[0]);
	/* ReCall Success */
	//fprintf(fp, "%u\t", pInfoS->usReCall[1]);
	/* AAA Trial */
	fprintf(fp, "%u\t", pInfoS->usAAA[0]);
	/* AAA Success */
	fprintf(fp, "%u\t", pInfoS->usAAA[1]);
	/* HSS Trial */
	fprintf(fp, "%u\t", pInfoS->usHSS[0]);
	/* HSS Success */
	fprintf(fp, "%u\t", pInfoS->usHSS[1]);
	/* LNS Trial */
	fprintf(fp, "%u\t", pInfoS->usLNS[0]);
	/* LNS Success */
	fprintf(fp, "%u\t", pInfoS->usLNS[1]);
	/* SVC01:MENU Trial */
	fprintf(fp, "%u\t", pInfoS->usService[0][0]);
	/* SVC01:MENU Success */
	fprintf(fp, "%u\t", pInfoS->usService[0][1]);
	/* SVC02:DN Trial */
	fprintf(fp, "%u\t", pInfoS->usService[1][0]);
	/* SVC02:DN Success */
	fprintf(fp, "%u\t", pInfoS->usService[1][1]);
	/* SVC03:STREAM Trial */
	fprintf(fp, "%u\t", pInfoS->usService[2][0]);
	/* SVC03:STREAM Success */
	fprintf(fp, "%u\t", pInfoS->usService[2][1]);
	/* SVC04:MMS Trial */
	fprintf(fp, "%u\t", pInfoS->usService[3][0]);
	/* SVC04:MMS Success */
	fprintf(fp, "%u\t", pInfoS->usService[3][1]);
	/* SVC05:WIDGET Trial */
	fprintf(fp, "%u\t", pInfoS->usService[4][0]);
	/* SVC05:WIDGET Success */
	fprintf(fp, "%u\t", pInfoS->usService[4][1]);
	/* SVC06:PHONE Trial */
	fprintf(fp, "%u\t", pInfoS->usService[5][0]);
	/* SVC06:PHONE Success */
	fprintf(fp, "%u\t", pInfoS->usService[5][1]);
	/* SVC07:EMS Trial */
	fprintf(fp, "%u\t", pInfoS->usService[6][0]);
	/* SVC07:EMS Success */
	fprintf(fp, "%u\t", pInfoS->usService[6][1]);
	/* SVC08:BANK Trial */
	fprintf(fp, "%u\t", pInfoS->usService[7][0]);
	/* SVC08:BANK Success */
	fprintf(fp, "%u\t", pInfoS->usService[7][1]);
	/* SVC09:FV Trial */
	fprintf(fp, "%u\t", pInfoS->usService[8][0]);
	/* SVC09:FV Success */
	fprintf(fp, "%u\t", pInfoS->usService[8][1]);
	/* SVC10:IM Trial */
	fprintf(fp, "%u\t", pInfoS->usService[9][0]);
	/* SVC10:IM Success */
	fprintf(fp, "%u\t", pInfoS->usService[9][1]);
	/* SVC11:VT Trial */
	fprintf(fp, "%u\t", pInfoS->usService[10][0]);
	/* SVC11:VT Success */
	fprintf(fp, "%u\t", pInfoS->usService[10][1]);
	/* SVC12:ETC Trial */
	fprintf(fp, "%u\t", pInfoS->usService[11][0]);
	/* SVC12:ETC Success */
	fprintf(fp, "%u\t", pInfoS->usService[11][1]);
	/* SVC13:CORP Trial */
	fprintf(fp, "%u\t", pInfoS->usService[12][0]);
	/* SVC13:CORP Success */
	fprintf(fp, "%u\t", pInfoS->usService[12][1]);
	/* SVC14:REGI Trial */
	fprintf(fp, "%u\t", pInfoS->usService[13][0]);
	/* SVC14:REGI Success */
	fprintf(fp, "%u\t", pInfoS->usService[13][1]);
	/* SVC19:ROAM Trial */
	fprintf(fp, "%u\t", pInfoS->usService[18][0]);
	/* SVC19:ROAM Success */
	fprintf(fp, "%u\t", pInfoS->usService[18][1]);

	/* SVC15:INET Trial */
	//fprintf(fp, "%u\t", pInfoS->usService[14][0]);
	/* SVC15:INET Success */
	//fprintf(fp, "%u\t", pInfoS->usService[14][1]);
	/* SVC16:INET_RECV Trial */
	//fprintf(fp, "%u\t", pInfoS->usService[15][0]);
	/* SVC16:INET Success */
	//fprintf(fp, "%u\t", pInfoS->usService[15][1]);
	/* SVC17:IM_RECV Trial */
	//fprintf(fp, "%u\t", pInfoS->usService[16][0]);
	/* SVC17:IM_RECV Success */
	//fprintf(fp, "%u\t", pInfoS->usService[16][1]);
	/* SVC18:VT_RECV Trial */
	//fprintf(fp, "%u\t", pInfoS->usService[17][0]);
	/* SVC18:VT_RECVSuccess */
	//fprintf(fp, "%u\t", pInfoS->usService[17][1]);
	/* SVC19:ROAM Trial */
	//fprintf(fp, "%u\t", pInfoS->usService[18][0]);
	/* SVC19:ROAM Success */
	//fprintf(fp, "%u\t", pInfoS->usService[18][1]);
}

/*
 * $Log: m_svcmon_func.c,v $
 * Revision 1.5  2011/09/07 07:07:51  hhbaek
 * *** empty log message ***
 *
 * Revision 1.4  2011/09/07 04:31:59  hhbaek
 * *** empty log message ***
 *
 * Revision 1.3  2011/09/05 05:33:00  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/01 07:49:50  dcham
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 10:59:21  dcham
 * *** empty log message ***
 *
 * Revision 1.15  2011/04/24 21:10:14  innaei
 * *** empty log message ***
 *
 * Revision 1.14  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.4  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.3  2010/10/04 07:32:44  uamyd
 * added module what is displayed PCF/BSC/BTS Equip Info
 *
 * Revision 1.2  2010/09/03 05:34:33  dqms
 * if file is not exist, don't run closing
 *
 * Revision 1.1.1.1  2010/08/23 01:13:10  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.13  2010/03/29 12:22:41  dark264sh
 * *** empty log message ***
 *
 * Revision 1.12  2010/02/25 11:29:45  dark264sh
 * M_SVCMON ROAM 처리
 *
 * Revision 1.11  2010/02/24 12:02:30  dark264sh
 * 망감시 헤더 변경에 따른 변경
 *
 * Revision 1.10  2009/10/21 11:43:53  pkg
 * 망감시 REGI 서비스 추가 및 define값 변경
 *
 * Revision 1.9  2009/10/12 08:46:21  pkg
 * M_SVCMON 법인 서비스 적용
 *
 * Revision 1.8  2009/09/09 12:20:46  pkg
 * M_SVCMON 법인서비스 관련 예외 처리
 *
 * Revision 1.7  2009/08/25 12:41:20  pkg
 * M_SVCMON HSS BTS 버그 수정
 *
 * Revision 1.6  2009/07/26 07:36:50  dark264sh
 * M_SVCMON L4SvcType 추가
 *
 * Revision 1.5  2009/07/20 04:27:15  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2009/07/19 13:38:00  dark264sh
 * M_SVCMON 저장 파일명 변경
 *
 * Revision 1.3  2009/07/14 06:42:38  dark264sh
 * M_SVCMON 파일 저장 ASCII로 변경
 *
 * Revision 1.2  2009/06/20 13:37:27  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2009/06/16 08:05:51  dark264sh
 * M_SVCMON 기본 동작 처리
 *
 */
