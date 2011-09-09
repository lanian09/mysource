/**A.1*  FILE INCLUSION *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <mysql/mysql.h>
#include <linux/limits.h>
#include <ctype.h>

//#include "lgt_nms.h"		/* SIDB_DATE_SIZE */
#include "rppi_def.h"		/* RPPISESS_KEY_SIZE */

#include "s_mng_init.h"
#include "s_mng_func.h"
#include "s_mng_flt.h"		/* st_Sub_GI_NTAF_List */
#include "s_mng_mmc.h"

#include "db_api.h"

#include "path.h"
#include "commdef.h"
#include "sshmid.h"
#include "mmcdef.h"			/* MAX_TMR_REC */
#include "sockio.h"			/* st_subsys_mng */
#include "msgdef.h"

#include "hasho.h"

#include "common_stg.h"

#include "loglib.h"
#include "dblib.h"
#include "ipclib.h"
#include "filelib.h"
#include "nsocklib.h"		/* SIDB_DATE_SIZE */
#include "tools.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/


/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
st_Sub_GI_NTAF_List		stGINTAF_SubFidb;
st_subsys_mng			*pstSubSys;
int    					gdCNTOnOff;
int						FinishFlag;

stHASHOINFO  			*pMODELINFO1;
stHASHOINFO  			*pMODELINFO2;
stHASHOINFO				*pIRMINFO;

/** D.2* DECLARATION OF EXTERN VARIABLES ******************/
extern MYSQL			stMySQL;
extern st_Taf_Svcinfo   stTafSvclist;
extern st_WatchFilter	*gWatchFilter;
extern st_Flt_Info		*flt_info;
extern st_TraceList		*trace_tbl;
extern int				JiSTOPFlag;

/** E.1* DEFINITION OF FUNCTIONS **************************/
extern int dReadTimerFile(TIMER_INFO *pstData);
extern int dMakeIRMHash(void);

/** E.2* DEFINITION OF FUNCTIONS **************************/

int dInit_IPC(void)
{
	int		dRet;

	/*
		trace_tbl의 shared memory가 새로 만들어질 경우, 이전 trace_tbl 내용을 파일로부터 읽어온다.
		 - Writer: Han-jin Park
		 - DAte: 2008.09.19
	*/
	if( (dRet = shm_init(S_SSHM_TRACE_INFO, sizeof(st_TraceList), (void**)&trace_tbl)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN Init_shm(S_SSHM_TRACE_INFO) dRet[%d]", LT, dRet);
		return -1;
	}
	else if(dRet == SHM_CREATE)
	{
		memset(trace_tbl, 0x00, sizeof(st_TraceList));
		if( (dRet = dGetTraceTblList(FILE_TRACE_TBL, trace_tbl)) < 0)
		{
			if(dRet == -1)
				log_print(LOGN_CRI, LH"FILE(%s) IS NOT FOUND", LT, FILE_TRACE_TBL);
			else
			{
				log_print(LOGN_CRI, LH"ERROR IN dGetTraceTblList(%s) dRet[%d]", LT, FILE_TRACE_TBL, dRet);
				return -10;
			}
		}
		else
			log_print(LOGN_DEBUG,LH"SUCCEED a dGetTraceTblList(%s)", LT, FILE_TRACE_TBL);
	}

	/* Model 관리 Hash1 Table */
    if( (pMODELINFO1 = hasho_init(S_SSHM_MODELHASH1, RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE, HDATA_MODEL_SIZE, HASH_MODELINFO_CNT, 0)) == NULL)
	{
        log_print(LOGN_CRI, LH"ERROR IN hasho_init(pMODELINFO1[%p])", LT, pMODELINFO1);
        return -11;
    }

	if( (pMODELINFO2 = hasho_init(S_SSHM_MODELHASH2, RPPISESS_KEY_SIZE, RPPISESS_KEY_SIZE, HDATA_MODEL_SIZE, HASH_MODELINFO_CNT, 0)) == NULL)
	{
        log_print(LOGN_CRI, LH"ERROR IN hasho_init(pMODELINFO2[%p])", LT, pMODELINFO2);
        return -12;
    }

	/* IRM 관리 Hash Table */
	if( (pIRMINFO = hasho_init(0, DEF_IMSIHASH_KEY_SIZE, DEF_IMSIHASH_KEY_SIZE, DEF_IMSIHASH_DATA_SIZE, DEF_IRMHASH_CNT, 0)) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN hasho_init(pIRMINFO[%p])", LT, pIRMINFO);
		return -13;
	}

	return 0;
} /* end of dInit_IPC */

void SetUpSignal(void)
{
	JiSTOPFlag = 1;

    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT, UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP, IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);

    signal(SIGCLD, SIG_IGN);
	log_print(LOGN_CRI, LH"SIGNAL HANDLER WAS INSTALLED[%d]", LT, JiSTOPFlag);
} /* end of SetUpSignal */

void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    FinishFlag = sign;

	log_print(LOGN_CRI, LH"signal number[%d]", LT, sign);
}

void FinishProgram(MYSQL *pstMySQL)
{
	int		dRet;
	char	FilePath[PATH_MAX];

    log_print(LOGN_CRI, LH"PROGRAM IS NORMALLY TERMINATED, Cause = signal number(%d)", LT, FinishFlag);
	db_disconn(&stMySQL);

	sprintf(FilePath, "%s", FILE_WATCHFILTER);

	if( (dRet = dWriteWatchFilter(FilePath)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dWriteWatchFilter() dRet[%d]", LT, dRet);
		if( (dRet = remove(FilePath)) == -1)
		{
			log_print(LOGN_CRI, LH"FAILED IN remove(%s) errno[%d-%s]", LT, FilePath, errno, strerror(errno));
			exit(-1);
		}
		exit(-2);
	}

	exit(FinishFlag);
} /* end of FinishProgram */

void IgnoreSignal(int sign)
{
	if(sign != SIGALRM)
		log_print(LOGN_CRI, LH"Diswanted signal(%d) is received.", LT, sign);

	signal(sign, IgnoreSignal);
} /* end of IgnoreSignal */

int dInit_Info(void)
{
	int		dRet;

	if( (dRet = dInitLogLvl()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInitLogLvl() dRet[%d]", LT, dRet);

	if( (dRet = dReadTimerFile(&flt_info->stTimerInfo)) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dReadTimerFile() dRet[%d]", LT, dRet);

	if( (dRet = dInit_SubSystem_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_SubSystem_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_GINTAF_SubSystem_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_GINTAF_SubSystem_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_Ntaf_Alarm()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_Ntaf_Alarm() dRet[%d]", LT, dRet);

	if( (dRet = dInit_FltSvc_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_FltSvc_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_FltSCTP_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_FltSCTP_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_FltIPPool_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_FltIPPool_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_FltThres_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_FltThres_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_User_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_User_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_Equip_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_Equip_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_WatchFltSVC_Info()) < 0)
		log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltSVC_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_WatchInfoAccess()) < 0)
		log_print(LOGN_CRI,LH"ERROR IN dInit_WatchInfoAccess() dRet[%d]", LT, dRet);

	if( (dRet = dInit_CTNInfo()) < 0)
		log_print(LOGN_CRI,LH"ERROR IN dInit_CTNInfo() dRet[%d]", LT, dRet);

	if( (dRet = dInit_WatchFltEquip_Info()) < 0)
		log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltEquip_Info() dRet[%d]", LT, dRet);

	if( (dRet = dInit_WatchInfoMonThreshold()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_WatchInfoMonThreshold() dRet[%d]", LT, dRet);

	if( (dRet = dInit_WatchInfoDefectThreshold()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_WatchInfoDefectThreshold() dRet[%d]", LT, dRet);

	/* INYOUNG */
	InitTamSvclist();

	return 0;
}

int dInit_SubSystem_Info(void)
{
    st_NtafIP_db    stNtafIP;
    int             i, dRet, dVal;

    memset(&stNtafIP, 0x00, sizeof(st_NtafIP_db));
    if( (dRet =  dRead_FLT_Tmf(&stNtafIP)) < 0)
	{
        log_print(LOGN_CRI, LH"ERROR IN dRead_FLT_Tmf() dRet[%d]", LT, dRet);
		return -1;
	}

	for(i = 0; i < dRet; i++)
	{
		dVal = stNtafIP.sNo[i];
		if(dVal > 0)
		{
			pstSubSys->sys[dVal-1].usSysNo  = dVal;
			pstSubSys->sys[dVal-1].uiIP	  = stNtafIP.uiIP[i];
			log_print(LOGN_CRI, "i=%d IP=%u SYSNO=%hu]", i, pstSubSys->sys[dVal-1].uiIP, pstSubSys->sys[dVal-1].usSysNo);
		}
	}

	return 0;
}

/*	START: GI NTAF에게만 SCTP filter 정보를 보내주기 위하여	*/
int dInit_GINTAF_SubSystem_Info(void)
{
	st_Sub_GI_NTAF_List		stGINtafIP;
	int						i, dRet;

	memset(&stGINtafIP, 0x00, sizeof(st_NtafIP_db));
	if( (dRet =  dRead_FLT_GINTAF_Tmf(&stGINtafIP)) < 0)
	{
		log_print(LOGN_CRI,LH"FAILED IN dRead_GINTAF_FLT_Tmf() dRet[%d]", LT, dRet);
		return -1;
	}

	for(i = 0; i < dRet; i++)
	{
		if( (stGINtafIP.usSysNo[i] > 0) && (stGINtafIP.usSysType[i] == 1))
		{
			stGINTAF_SubFidb.usSysType[stGINtafIP.usSysNo[i]-1]	= stGINtafIP.usSysType[i];
			stGINTAF_SubFidb.usSysNo[stGINtafIP.usSysNo[i]-1]	= stGINtafIP.usSysNo[i];
			stGINTAF_SubFidb.uiIP[stGINtafIP.usSysNo[i]-1]		= stGINtafIP.uiIP[i];
			log_print(LOGN_CRI, LH"i=%d IP=%u SYSNO=%hu SYSTYPE=%hu", LT,
				i, stGINTAF_SubFidb.uiIP[stGINtafIP.usSysNo[i]-1], stGINTAF_SubFidb.usSysNo[stGINtafIP.usSysNo[i]-1], stGINTAF_SubFidb.usSysType[stGINtafIP.usSysNo[i]-1]);
		}
	}

	return 0;
}
/*	END: Writer: HAN-JIN PARK, Date: 2009.05.11	*/

int dInit_FltSvc_Info(void)
{
	int				i, dCount, dRet;
	st_SvcMmc		stSvcMmc[MAX_SVR_CNT];
	pst_SvcInfo		pstSvcInfo;

	memset(&stSvcMmc, 0x00, sizeof(st_SvcMmc) * MAX_SVR_CNT);
	if( (dRet = dGetSvcInfo(&stMySQL, stSvcMmc, &dCount, NULL)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetSvcInfo() dRet[%d]", LT, dRet);
		return -1;
	}

	flt_info->stSvcInfoShm.dCount = dCount;
	log_print(LOGN_CRI, LH"FLT SVC SELECT COUNT=%d", LT, dCount);

	for(i = 0; i < dCount; i++)
	{
		pstSvcInfo					= &flt_info->stSvcInfoShm.stSvcInfo[i];
		pstSvcInfo->dIdx			= i+1;
		pstSvcInfo->uSvcIP			= stSvcMmc[i].uSvcIP;
		pstSvcInfo->huPort			= stSvcMmc[i].huPort;
		pstSvcInfo->cFlag			= stSvcMmc[i].cFlag;
		pstSvcInfo->cSysType		= stSvcMmc[i].cSysType;
		pstSvcInfo->huL4Code		= stSvcMmc[i].huL4Code;
		pstSvcInfo->huL7Code		= stSvcMmc[i].huL7Code;
		pstSvcInfo->huAppCode		= stSvcMmc[i].huAppCode;
		strcpy(pstSvcInfo->szDesc, stSvcMmc[i].szDesc);

		log_print(LOGN_CRI, "I[%d] uSvcIP[%u] huPort[%hu] cFlag[%hu] cSysType[%hu] huL4Code[%hu] huL7Code[%hu] huAppCode[%hu] szDesc[%s]",
			pstSvcInfo->dIdx, pstSvcInfo->uSvcIP, pstSvcInfo->huPort, pstSvcInfo->cFlag, pstSvcInfo->cSysType,
			pstSvcInfo->huL4Code, pstSvcInfo->huL7Code, pstSvcInfo->huAppCode, pstSvcInfo->szDesc);
	}

	return 0;
}

int dInit_WatchFltSVC_Info(void)
{
	int						dRet;
	st_WatchServiceList		stFLTService;

	memset(&stFLTService, 0x00, sizeof(st_WatchServiceList));
	if( (dRet = dGetWatchServiceList(&stMySQL, &stFLTService)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetWatchServiceList() dRet[%d]", LT, dRet);
		return -1;
	}
	log_print(LOGN_CRI, LH"stFLTService.dCount[%d]", LT, stFLTService.dCount);

	memcpy(&(gWatchFilter->stWatchServiceList), &stFLTService, sizeof(st_WatchServiceList));

	if( (dRet = dSendMsg_O_SVCMON(MID_FLT_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg_O_SVCMON() dRet[%d]", LT, dRet);
		return -2;
	}

	return 0;
}

int dInit_WatchInfoMonThreshold(void)
{
	int						dRet;
	st_WatchThresholdList	stWatchThreshold;

	memset(&stWatchThreshold, 0x00, sizeof(st_WatchThresholdList));
	if( (dRet = dGetWatchInfoMonThreshold(&stMySQL, &stWatchThreshold)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetWatchServiceList() dRet[%d]", LT, dRet);
		return -3;
	}
	memcpy(&(gWatchFilter->stWatchThresholdList), &stWatchThreshold, sizeof(st_WatchThresholdList));

	if( (dRet = dSendMsg_O_SVCMON(MID_FLT_MON_THRES)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg_O_SVCMON() dRet[%d]", LT, dRet);
		return -4;
	}

	return 0;
}

int dInit_WatchInfoDefectThreshold(void)
{
	int						dRet;
	st_DefectThresholdList	stWatchDefectThreshold;

	memset(&stWatchDefectThreshold, 0x00, sizeof(st_DefectThresholdList));
	if( (dRet = dGetWatchInfoDefectThreshold(&stMySQL, &stWatchDefectThreshold)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetWatchInfoDefectThreshold() dRet[%d]", LT, dRet);
		return -1;
	}
	memcpy(&(gWatchFilter->stDefectThresholdList), &stWatchDefectThreshold, sizeof(stWatchDefectThreshold));

	if( (dRet = dSendRPPI(MID_FLT_DEFECT_THRES)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendRPPI() dRet[%d]", LT, dRet);
		return -2;
	}

	if( (dRet = dSendROAM(MID_FLT_DEFECT_THRES)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendROAM() dRet[%d]", LT, dRet);
		return -3;
	}

	return 0;
}

int dInit_WatchFltEquip_Info(void)
{
	int					dCount, dRet;
	char				sFilePath[PATH_MAX], sFileName[PATH_MAX], sDate[SIDB_DATE_SIZE];
	time_t				tCurTime;
	struct tm			stTmCur;
	st_WatchEquipList	stWatchEquipList;
	st_LoamEquipList	stLoamEquipList;

	memset(&stWatchEquipList, 0x00, sizeof(st_WatchEquipList));
	if( (dRet = dGetFltEquipCount(&stMySQL, &dCount)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dGetFltEquipCount() dRet[%d]", LT, dRet);
		return -1;
	}

	if(dCount > (MAX_MON_PDSN_CNT+MAX_MON_AAA_CNT+MAX_MON_HSS_CNT+MAX_MON_LNS_CNT)){
		log_print(LOGN_CRI, LH"Equip COUNT[%d] is over MAX_EQUIP_CNT[%d]", LT,
			dCount, (MAX_MON_PDSN_CNT+MAX_MON_AAA_CNT+MAX_MON_HSS_CNT+MAX_MON_LNS_CNT));
	}

	if( (dRet = dGetFltEquipList(&stMySQL, &stWatchEquipList)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dGetFltEquipList() dRet[%d]", LT, dRet);
		return -2;
	}
	memcpy(&(gWatchFilter->stWatchEquipList), &stWatchEquipList, sizeof(st_WatchEquipList));

	memset(&stLoamEquipList, 0x00, sizeof(st_LoamEquipList));
	if( (dRet = dGetFltRoamEquipCount(&stMySQL, &dCount)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dGetFltRoamEquipCount() dRet[%d]", LT, dRet);
		return -3;
	}

	if(dCount > MAX_ROAM_EQUIP_CNT){
		log_print(LOGN_CRI, LH"Roam Equip COUNT[%d] is over MAX_ROAM_EQUIP_CNT[%d]", LT,
			dCount, MAX_ROAM_EQUIP_CNT);
	}

	if( (dRet = dGetFltRoamEquipList(&stMySQL, &stLoamEquipList)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dGetFltRoamEquipList() dRet[%d]", LT, dRet);
		return -4;
	}
	memcpy(&(gWatchFilter->stLoamEquipList), &stLoamEquipList, sizeof(st_LoamEquipList));

	if( (dRet = dSendMsg_O_SVCMON(MID_FLT_EQUIP)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg_O_SVCMON() dRet[%d]", LT, dRet);
		return -5;
	}

	if( (dRet = dSendROAM(MID_FLT_EQUIP)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dSendROAM() dRet[%d]", LT, dRet);
		return -6;
	}

	memset(sFilePath, 0x00, PATH_MAX);
	sprintf(sFilePath, "%s/TAMDB_LOG", START_PATH);

	time(&tCurTime);
	localtime_r((const time_t *)&tCurTime, &stTmCur);

	memset(sDate, 0x00, SIDB_DATE_SIZE);
	sprintf(sDate, "%04d%02d%02d", stTmCur.tm_year+1900, stTmCur.tm_mon+1, stTmCur.tm_mday);
	sprintf(sFileName, "DQMS_FEQP_ID0001_T%s%02d%02d%02d.DAT",
		sDate, stTmCur.tm_hour, stTmCur.tm_min, stTmCur.tm_sec);

	if( (dRet = dSyncInfoEquipTAMDB(&stMySQL, sFilePath, sFileName)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dSyncInfoEquipTAMDB(sFilePath[%s], sFileName[%s]) dRet[%d]", LT,
			sFilePath, sFileName, dRet);
		return -7;
	}

	if( (dRet = dSendMsg_SIDB(sDate, sFilePath, sFileName)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg_SIDB() dRet[%d]", LT, dRet);
		return -8;
	}

	return 0;
}

int dMake_DefectInfoFile(void)
{
	int			dRet;
	FILE		*fPipe;
	char		sFullPath[PATH_MAX], sFilePath[PATH_MAX], sFileName[PATH_MAX], sDate[SIDB_DATE_SIZE], sCommand[BUFSIZ];
	time_t		tCurTime;
	struct tm	stTmCur;

	memset(sFilePath, 0x00, PATH_MAX);
	sprintf(sFilePath, "%s/TAMDB_LOG", START_PATH);

	time(&tCurTime);
	localtime_r((const time_t *)&tCurTime, &stTmCur);

	memset(sDate, 0x00, SIDB_DATE_SIZE);
	sprintf(sDate, "%04d%02d%02d", stTmCur.tm_year+1900, stTmCur.tm_mon+1, stTmCur.tm_mday);
	sprintf(sFileName, "DQMS_FDEF_ID0001_T%s%02d%02d%02d.DAT",
		sDate, stTmCur.tm_hour, stTmCur.tm_min, stTmCur.tm_sec);

	sprintf(sFullPath, "%s/%s", sFilePath, sFileName);

	sprintf(sCommand, "perl %s/SCRIPT/MakeDefect.pl %s", START_PATH, sFullPath);
	if( (fPipe = popen(sCommand, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN popen(sCommand[%s])", LT, sCommand);
		return -1;
	}

	if( (dRet = pclose(fPipe)) == -1)
	{
		log_print(LOGN_CRI, LH"ERROR IN pclose(sCommand[%s]) errno[%d:%s]", LT,
			sCommand, errno, strerror(errno));
	}

	if( (dRet = dSendMsg_SIDB(sDate, sFilePath, sFileName)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg_SIDB() dRet[%d]", LT, dRet);
		return -2;
	}

	return 0;
}


int dInit_FltSCTP_Info(void)
{
	int				i, dCount, dRet;
	st_SCTP_DB		stSCTPList[MAX_SCTP_COUNT];
	st_SCTP			*pstSCTP;

	memset(&stSCTPList, 0x00, sizeof(st_SCTP_DB)*MAX_SCTP_COUNT);
	if( (dRet = dGetSCTPInfo(&stMySQL, stSCTPList, &dCount, NULL)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetSCTPInfo() dRet[%d]", LT, dRet);
		return -1;
	}

	flt_info->stSCTPShm.dCount = dCount;
	log_print(LOGN_CRI, LH"FLT_SCTP SELECT COUNT=%d", LT, dCount);

	for(i = 0; i < dCount; i++)
	{
		pstSCTP					= &flt_info->stSCTPShm.stSCTP[i];
		pstSCTP->dIdx			= i+1;
		pstSCTP->uSCTPIP		= stSCTPList[i].uSCTPIP;
		pstSCTP->cSysType		= stSCTPList[i].cSysType;
		pstSCTP->cDirection		= stSCTPList[i].cDirection;
		pstSCTP->huGroupID		= stSCTPList[i].huGroupID;
		strcpy(pstSCTP->szDesc, stSCTPList[i].szDesc);

		log_print(LOGN_CRI, "dIdx[%d] uSCTPIP[%u] cSysType[%hu] cDirection[%hu] huGroupID[%hu] szDesc[%s]",
			pstSCTP->dIdx, pstSCTP->uSCTPIP, pstSCTP->cSysType, pstSCTP->cDirection, pstSCTP->huGroupID, pstSCTP->szDesc);
	}

	return 0;
}

int dInit_CTNInfo(void)
{
	FILE			*fp;
	int				i, j, k, dLine=0, dRet, dOldStatus;
	char			sFilePath[PATH_MAX], sYYYYMMDD[9], sBuf[BUFSIZ];
	size_t			szStrLen;
	time_t			tCurrent;
	struct tm		stCurrent;

	stHASHOINFO		*pMODELINFO;
	stHASHONODE		*pHASHONODE;
	st_CallKey		stRPPIKey;
	HData_Model		stModelHash;
	st_Model_Stat	stModelStat, *pstModelStat;	/* added by uamyd 20100930 for Model-IMSI count*/

	dOldStatus = gWatchFilter->stModelInfoList.dActiveStatus;
	switch(dOldStatus)
	{
		case 1:
			pMODELINFO	= pMODELINFO2;
			gWatchFilter->stModelInfoList.dActiveStatus	= 2;
			break;
		case 2:
			pMODELINFO	= pMODELINFO1;
			gWatchFilter->stModelInfoList.dActiveStatus	= 1;
			break;
		default:
			log_print(LOGN_CRI, LH"FAILED IN dInit_CTNInfo(gWatchFilter->stModelInfoList.dActiveStatus[%d])", LT,
				gWatchFilter->stModelInfoList.dActiveStatus);
			return -1;
	}

	if( (tCurrent = time(NULL)) == -1)
	{
		log_print(LOGN_CRI, LH"FAILED IN time(NULL) errno[%d-%s]", LT, errno, strerror(errno));
		gWatchFilter->stModelInfoList.dActiveStatus = dOldStatus;
		return -2;
	}

	/*	Filename is made in one day ago.	*/
	tCurrent -= SEC_OF_DAY;

	if(localtime_r( (const time_t*)&tCurrent, &stCurrent) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN localtime_t(tCurrent[%lu]) errno[%d-%s]", LT,
				tCurrent, errno, strerror(errno));
		gWatchFilter->stModelInfoList.dActiveStatus = dOldStatus;
		return -3;
	}

	if( (dRet = strftime(sYYYYMMDD, 9, "%Y%m%d", &stCurrent)) != 8)
	{
		log_print(LOGN_CRI, LH"FAILED IN strftime(sYYYYMMDD[%s]) errno[%d-%s]", LT,
				sYYYYMMDD, errno, strerror(errno));
		gWatchFilter->stModelInfoList.dActiveStatus = dOldStatus;
		return -4;
	}

	sprintf(sFilePath, "%sDQMS1_FUSR_ID0000_T%s000000.DAT", "/home/ftp_dqms/", sYYYYMMDD);

	if( (fp = fopen(sFilePath, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(sFilePath[%s]) errno[%d-%s]", LT,
				sFilePath, errno, strerror(errno));
		gWatchFilter->stModelInfoList.dActiveStatus = dOldStatus;
		return -5;
	}

	/* 2009.09.20 */
	hasho_reset(pMODELINFO);
	memset(&stRPPIKey, 0x00, MAX_MIN_SIZE);

	/* added by uamyd 20100930 for Model-IMSI count */
	memset(&stModelStat, 0x00, sizeof(st_Model_Stat));
	pstModelStat = &stModelStat;

	log_print(LOGN_CRI, "START MODEL_INFO LOADING!! -> dActiveStatus[%d]", gWatchFilter->stModelInfoList.dActiveStatus);

	while(fgets(sBuf, BUFSIZ, fp) != NULL)
	{
		if(dLine >= MAX_MODEL_INFO)
		{
			log_print(LOGN_CRI, LH"ModelInfoList count[%d] is over MAX_MODEL_INFO[%d]", LT,
					dLine, MAX_MODEL_INFO);
			break;
		}

		szStrLen = strlen(sBuf);
		for(i = 0, j = -1, k = 0; i < szStrLen; i++)
		{
			if( (sBuf[i] == ' ') || (sBuf[i] == '\t') || (sBuf[i] == '\r') || (sBuf[i] == '\n'))
			{
				if(k)
				{
					switch(j)
					{
						case 0:
							stModelHash.szMIN[k] = 0x00;
							if(strlen((char*)stModelHash.szMIN) >= MAX_MIN_SIZE)
							{
								log_print(LOGN_CRI, LH"stModelHash.szMIN[%s] >= MAX_MIN_SIZE[%d]", LT,
									stModelHash.szMIN, MAX_MIN_SIZE);
							}
							break;
						case 1:
							stRPPIKey.szIMSI[k] = 0x00;
							if(strlen((char*)stRPPIKey.szIMSI) >= MAX_MIN_SIZE)
							{
								log_print(LOGN_CRI, LH"stRPPIKey.szIMSI[%s] >= MAX_MIN_SIZE[%d]", LT,
									stRPPIKey.szIMSI, MAX_MIN_SIZE);
							}
							break;
						case 2:
							stModelHash.szModel[k] = 0x00;
							if(strlen((char*)stModelHash.szModel) >= MAX_MODEL_SIZE)
							{
								log_print(LOGN_CRI, LH"stModelHash.szModel[%s] >= MAX_MODEL_SIZE[%d]", LT,
									stModelHash.szModel, MAX_MODEL_SIZE);
							}
							break;
					}
				}
				k = 0;
			}
			else
			{
				if(!k)
					j++;

				switch(j)
				{
					case 0:
						stModelHash.szMIN[k++] = sBuf[i];
						break;
					case 1:
						stRPPIKey.szIMSI[k++] = sBuf[i];
						break;
					case 2:
						stModelHash.szModel[k++] = sBuf[i];
						break;
					default:
						log_print(LOGN_CRI, LH"j[%d] is between 0 and 2.", LT, j);
						break;
				}
			}
		}

		if(j < 3)
		{
			if ( (pHASHONODE = hasho_add(pMODELINFO, (U8*)(&stRPPIKey), (U8*)(&stModelHash))) == NULL)
			{
				log_print(LOGN_CRI, LH"ERROR IN hasho_add(szIMSI[%s] szModel[%s] szMIN[%s])", LT,
								  stRPPIKey.szIMSI, stModelHash.szModel, stModelHash.szMIN );
				continue;
			}

			/* added uamyd 20100930 for Model-IMSI count */
			if( gdCNTOnOff == _MSCTN_COUNT_ON )
				dRecordModelCustCnt(pstModelStat, (char*)stModelHash.szModel, strlen((char*)stModelHash.szModel));

			dLine++;
		}
		else
		{
			log_print(LOGN_CRI, LH"j[%d] stModelHash.szMIN[%s] stRPPIKey.szIMSI[%s] stModelHash.szModel[%s]", LT,
					j, stModelHash.szMIN, stRPPIKey.szIMSI, stModelHash.szModel);
			continue;
		}
	}


	if( (dRet = fclose(fp)) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN fclose(%s) errno[%d-%s]", LT,
				sFilePath, errno, strerror(errno));
		gWatchFilter->stModelInfoList.dActiveStatus = dOldStatus;
		return -6;
	}

	/* added uamyd 20100930 for Model-IMSI count */
	if( gdCNTOnOff == _MSCTN_COUNT_ON )
		dCreateTBSql_ModelCustCnt(pstModelStat);

	log_print( LOGN_CRI, "END MODEL_INFO LOADING!!");

	if( (dRet = dSendRPPI(MID_FLT_MODEL)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendRPPI() dRet[%d]", LT, dRet);
		gWatchFilter->stModelInfoList.dActiveStatus = dOldStatus;
		return -7;
	}
	else
		log_print(LOGN_DEBUG, LH"SUCCESS IN dSendRPPI(MID_FLT_MODEL[%d])", LT, MID_FLT_MODEL);

	if( (dRet = dSendROAM(MID_FLT_MODEL)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendROAM() dRet[%d]", LT, dRet);
		gWatchFilter->stModelInfoList.dActiveStatus = dOldStatus;
		return -8;
	}
	else
		log_print(LOGN_DEBUG, LH"SUCCESS IN dSendROAM(MID_FLT_MODEL[%d])", LT, MID_FLT_MODEL);

	return 0;
}

int dInit_IRMInfo(void)
{
	int		dRet;


	if( (dRet = dMakeIRMHash()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dMakeIRMHash() dRet[%d]", LT, dRet);
		return -1;
	}

	if( (dRet = dSendROAM(MID_FLT_IRM)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendROAM() dRet[%d]", LT, dRet);
		return -2;
	}
	else
		log_print(LOGN_DEBUG, LH"SUCCESS IN dSendROAM(MID_FLT_IRM[%d])", LT, MID_FLT_IRM);

	if( (dRet = dSendMTRACE(MID_FLT_IRM)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMTRACE() dRet[%d]", LT, dRet);
		return -3;
	}
	else
		log_print(LOGN_DEBUG, LH"SUCCESS IN dSendMTRACE(MID_FLT_IRM[%d])", LT, MID_FLT_IRM);

	return 0;
}

int dInit_WatchInfoAccess(void)
{
	int						dCount, dRet;
	st_WatchPCFList			stPCFEquip;
	st_WatchBSCList			stBSCEquip;
	st_WatchBTSList			stBTSEquip;

	memset(&stPCFEquip, 0x00, sizeof(st_WatchPCFList));
	if( (dRet = dGetInfoAccessCount(&stMySQL, SYSTEM_TYPE_PCF, &dCount)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetInfoAccessCount(SYSTEM_TYPE_PCF) dRet[%d]", LT, dRet);
		return -1;
	}

	if(dCount > MAX_MON_PCF_CNT)
		log_print(LOGN_CRI, LH"TB_MACCESSC PCF COUNT[%d] is over MAX_MON_PDSN_CNT[%d]", LT, dCount, MAX_MON_PCF_CNT);

	if( (dRet = dGetInfoAccessPCF(&stMySQL, &stPCFEquip)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetInfoAccessPCF() dRet[%d]", LT, dRet);
		return -2;
	}
	memcpy(&(gWatchFilter->stWatchPCFList), &stPCFEquip, sizeof(st_WatchPCFList));

	memset(&stBSCEquip, 0x00, sizeof(st_WatchBSCList));
	if( (dRet = dGetInfoAccessCount(&stMySQL, SYSTEM_TYPE_BSC, &dCount)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetInfoAccessCount(SYSTEM_TYPE_BSC) dRet[%d]", LT, dRet);
		return -3;
	}

	if(dCount > MAX_MON_BSC_CNT)
		log_print(LOGN_CRI, LH"TB_MACCESSC BSC COUNT[%d] is over MAX_MON_BSC_CNT[%d]", LT, dCount, MAX_MON_BSC_CNT);

	if( (dRet = dGetInfoAccessBSC(&stMySQL, &stBSCEquip)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetInfoAccessBSC() dRet[%d]", LT, dRet);
		return -4;
	}
	memcpy(&(gWatchFilter->stWatchBSCList), &stBSCEquip, sizeof(st_WatchBSCList));

	memset(&stBTSEquip, 0x00, sizeof(st_WatchBTSList));
	if( (dRet = dGetInfoAccessCount(&stMySQL, SYSTEM_TYPE_BTS, &dCount)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetInfoAccessCount(SYSTEM_TYPE_BTS) dRet[%d]", LT, dRet);
		return -5;
	}

	if(dCount > MAX_MON_BTS_CNT)
		log_print(LOGN_CRI, LH"TB_MACCESSC BTS COUNT[%d] is over MAX_MON_BTS_CNT[%d]", LT, dCount, MAX_MON_BTS_CNT);

	if( (dRet = dGetInfoAccessBTS(&stMySQL, &stBTSEquip)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetInfoAccessBTS() dRet[%d]", LT, dRet);
		return -6;
	}
	memcpy(&(gWatchFilter->stWatchBTSList), &stBTSEquip, sizeof(st_WatchBTSList));

	if( (dRet = dSendMsg_O_SVCMON(MID_FLT_ACCESS)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg_O_SVCMON() dRet[%d]", LT, dRet);
		return -7;
	}

	if( (dRet = dSendRPPI(MID_FLT_ACCESS)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendRPPI() dRet[%d]", LT, dRet);
		return -8;
	}

	if( (dRet = dSendROAM(MID_FLT_ACCESS)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendROAM() dRet[%d]", LT, dRet);
		return -9;
	}

	return 0;
}

int dInit_FltIPPool_Info(void)
{
	int			i, dRet, dCount;
	pst_NAS		pstNAS;
	st_NAS_db	stNASMmc[MAX_MNIP_COUNT];;

	memset(&stNASMmc, 0x00, sizeof(st_NAS_db)*MAX_MNIP_COUNT);
	if( (dRet = dGetNAS(&stMySQL, stNASMmc, &dCount, NULL)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetNAS() dRet[%d]", LT, dRet);
		return -1;
	}

	flt_info->stNASShm.dCount = dCount;
	log_print(LOGN_CRI, LH"FLT IPPOOL SELECT CNT=%d", LT, dCount);

	for(i = 0; i < dCount; i++)
	{
		pstNAS		= &flt_info->stNASShm.stNAS[i];
		pstNAS->dIdx		= i+1;
		pstNAS->uMNIP		= stNASMmc[i].uMNIP;
		pstNAS->usNetMask	= stNASMmc[i].usNetMask;
		pstNAS->cFlag		= stNASMmc[i].cFlag;
		pstNAS->cSysType	= stNASMmc[i].cSysType;
		strcpy(pstNAS->szDesc, stNASMmc[i].szDesc);
		log_print(LOGN_CRI, "Idx[%d] uMNIP[%u] usNetMask[%hu] cFlag[%hu] cSysType[%hu] szDesc=[%s]",
			pstNAS->dIdx, pstNAS->uMNIP, pstNAS->usNetMask, pstNAS->cFlag, pstNAS->cSysType, pstNAS->szDesc);
	}

	return 0;
}

int dInit_FltThres_Info(void)
{
	int				i, dRet, dCount;
	st_ThresMMC		stThresMMC[MAX_DEFECT_THRES];
	pst_Thres		pstThres;

	memset(&stThresMMC, 0x00, sizeof(st_ThresMMC)*MAX_DEFECT_THRES);

	if( (dRet = dGetThres(&stMySQL, stThresMMC, &dCount, NULL)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetThres() dRet[%d]", LT, dRet);
		return -1;
	}

	flt_info->stThresShm.dCount = dCount;
	log_print(LOGN_CRI, LH"FLT DEFECT THRESHOLD SELECT CNT=%d", LT, flt_info->stThresShm.dCount);

	for(i = 0; i < dCount; i++)
	{
		pstThres					= &flt_info->stThresShm.stThres[i];
		pstThres->dIdx				= i+1;
		pstThres->cSvcType			= stThresMMC[i].cSvcType;
		pstThres->uTCPSetupTime		= stThresMMC[i].uTCPSetupTime;
		pstThres->uResponseTime		= stThresMMC[i].uResponseTime;
		pstThres->uUpThroughput		= stThresMMC[i].uUpThroughput;
		pstThres->uDnThroughput		= stThresMMC[i].uDnThroughput;
		pstThres->uUpRetransCount	= stThresMMC[i].uUpRetransCount;
		pstThres->uDnRetransCount	= stThresMMC[i].uDnRetransCount;
		pstThres->uUpJitter			= stThresMMC[i].uUpJitter;
		pstThres->uDnJitter			= stThresMMC[i].uDnJitter;
		pstThres->uUpPacketLoss		= stThresMMC[i].uUpPacketLoss;
		pstThres->uDnPacketLoss		= stThresMMC[i].uDnPacketLoss;
		strcpy(pstThres->szDesc, stThresMMC[i].szDesc);
		log_print(LOGN_CRI, "dIdx[%d] cSvcType[%u] uTCPSetupTime[%u] uResponseTime[%u] uUpThroughput[%u] uDnThroughput[%u] "
			"uUpRetransCount[%u] uDnRetransCount[%u] uUpJitter[%u] uDnJitter[%u] uUpPacketLoss[%u] uDnPacketLoss[%u] szDesc[%s]",
			pstThres->dIdx, pstThres->cSvcType, pstThres->uTCPSetupTime, pstThres->uResponseTime,
			pstThres->uUpThroughput, pstThres->uDnThroughput, pstThres->uUpRetransCount, pstThres->uDnRetransCount,
			pstThres->uUpJitter, pstThres->uDnJitter, pstThres->uUpPacketLoss, pstThres->uDnPacketLoss, pstThres->szDesc);
	}

	return 0;
}

int dInit_User_Info(void)
{
	int				i, dRet, dCnt;
	st_UserAdd		stUserInfo[MAX_USER];
	pst_User_Add	pstUserAdd;

	memset(stUserInfo, 0x00, sizeof(st_UserAdd)*MAX_USER);
	if( (dRet = dGetUserInfo(&stMySQL, stUserInfo, &dCnt)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetUserInfo() dRet[%d]", LT, dRet);
		return -1;
	}

	flt_info->stUserAddShm.dCount = dCnt;
	log_print(LOGN_CRI, LH"USER LIST SELECT CNT=%d", LT, flt_info->stUserAddShm.dCount);

	for(i = 0; i < dCnt; i++)
	{
		pstUserAdd	= &flt_info->stUserAddShm.stUserAdd[i];
		strcpy(pstUserAdd->szUserName, stUserInfo[i].szUserName);
		strcpy(pstUserAdd->szPassword, stUserInfo[i].szPassword);
		strcpy(pstUserAdd->szLocalName, stUserInfo[i].szLocalName);
		strcpy(pstUserAdd->szContact, stUserInfo[i].szContact);
		pstUserAdd->sSLevel			= stUserInfo[i].sSLevel;
		pstUserAdd->usLogin			= stUserInfo[i].usLogin;
		pstUserAdd->uConnectIP		= stUserInfo[i].uConnectIP;
		pstUserAdd->uLastLoginTime	= stUserInfo[i].uLastLoginTime;
		pstUserAdd->uCreateTime		= stUserInfo[i].uCreateTime;

		pstUserAdd->dIdx = i;

		log_print(LOGN_DEBUG, "I=%d USER=%s] PASS=%s] NM=%s] CT=%s] LVL=%d LOGIN=%d IP=%d LAST=%d CRE=%d",
			pstUserAdd->dIdx, pstUserAdd->szUserName, pstUserAdd->szPassword, pstUserAdd->szLocalName,
			pstUserAdd->szContact, pstUserAdd->sSLevel, pstUserAdd->usLogin, pstUserAdd->uConnectIP,
			pstUserAdd->uLastLoginTime, pstUserAdd->uCreateTime);
	}

	return 0;
}

int dInit_Equip_Info(void)
{
	int					i, dCount, dRet;
	pst_Info_Equip		pstEquip;
	st_InfoEquip_MMC	stInfoEquip[MAX_EQUIP_INFO];

	memset(stInfoEquip, 0x00, (sizeof(st_InfoEquip_MMC)*MAX_EQUIP_INFO));
	if( (dRet = dGetEquipInfo(&stMySQL, stInfoEquip, &dCount)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetEquipInfo() dRet[%d]", LT, dRet);
		return -1;
	}

	flt_info->stInfoEquipShm.dCount = dCount;
	log_print(LOGN_CRI, LH"Equip LIST SELECT COUNT=%d", LT, dCount);

	for(i = 0; i < dCount; i++)
	{
		pstEquip				= &flt_info->stInfoEquipShm.stInfoEquip[i];

		pstEquip->dIdx			= i+1;
		pstEquip->uEquipIP		= stInfoEquip[i].uEquipIP;
		pstEquip->cEquipType	= stInfoEquip[i].cEquipType;
		strcpy(pstEquip->szEquipTypeName, stInfoEquip[i].szEquipTypeName);
		strcpy(pstEquip->szEquipName, stInfoEquip[i].szEquipName);
		strcpy(pstEquip->szDesc, stInfoEquip[i].szDesc);
		log_print(LOGN_CRI, "dIdx[%d] uEquipIP[%u] cEquipType[%hu] szEquipTypeName[%s] szEquipName[%s] szDesc[%s]",
			pstEquip->dIdx, pstEquip->uEquipIP, pstEquip->cEquipType, pstEquip->szEquipTypeName, pstEquip->szEquipName, pstEquip->szDesc);
	}

	return 0;
}

int dWriteWatchFilter(char *FilePath)
{

	int dRet;
	if( ( dRet = write_file(FilePath, (char*)gWatchFilter, sizeof(st_WatchFilter), 0) ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN write_file(FILE=%s), dRet=%d", LT, FilePath, dRet);
		return -1;
	}
	return 0;
}

int dReadWatchFilter(void)
{

	int 			dRet, size;
	st_WatchFilter	gstWatchFilter;

	size = sizeof(st_WatchFilter);
	if( (dRet = read_file( FILE_WATCHFILTER, (char*)&gstWatchFilter, size,0 )) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN read_file(%s), dRet=%d", LT, FILE_WATCHFILTER, dRet);
		return -1;
	}

	memcpy(gWatchFilter, &gstWatchFilter, size);

	return 0;
}

void InitTamSvclist(void)
{
	/* INYOUNG */
	memset(&stTafSvclist, 0x00,sizeof(st_Taf_Svcinfo));

	stTafSvclist.stSvcStat[0].dSvcID = 1000;	/* SVC_MENU */
	stTafSvclist.stSvcStat[0].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[1].dSvcID = 3000;	/* SVC_DN */
	stTafSvclist.stSvcStat[1].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[2].dSvcID = 4000;	/* SVC_STREAM */
	stTafSvclist.stSvcStat[2].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[3].dSvcID = 5000;	/* SVC_MMS */
	stTafSvclist.stSvcStat[3].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[4].dSvcID = 13000;	/* SVC_WIDGET */
	stTafSvclist.stSvcStat[4].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[5].dSvcID = 15000;	/* SVC_PHONE */
	stTafSvclist.stSvcStat[5].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[6].dSvcID = 9000;	/* SVC_EMS */
	stTafSvclist.stSvcStat[6].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[7].dSvcID = 12000;	/* SVC_BANK */
	stTafSvclist.stSvcStat[7].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[8].dSvcID = 10000;	/* SVC_FV */
	stTafSvclist.stSvcStat[8].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[9].dSvcID = 11000;	/* SVC_IM */
	stTafSvclist.stSvcStat[9].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[10].dSvcID = 14000;	/* SVC_VT */
	stTafSvclist.stSvcStat[10].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[11].dSvcID = 6000;	/* SVC_ETC */
	stTafSvclist.stSvcStat[11].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[12].dSvcID = 16000;	/* SVC_CORP */
	stTafSvclist.stSvcStat[12].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[13].dSvcID = 60000;	/* SVC_INET */
	stTafSvclist.stSvcStat[13].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[14].dSvcID = 7000;	/* SVC_FB */
	stTafSvclist.stSvcStat[14].dOnOff = 1;		/* ON */
	stTafSvclist.stSvcStat[15].dSvcID = 8000;	/* SVC_IV */
	stTafSvclist.stSvcStat[15].dOnOff = 1;		/* ON */

	dReadSvcStat();

}

int dReadSvcStat(void)
{
	int dRet;

	if( (dRet = read_file(FILE_SVC_LIST, (char*)&stTafSvclist, sizeof(st_Taf_Svcinfo), 0)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN read_file(%s), dRet=%d", LT, FILE_SVC_LIST, dRet);
		return -1;
	}

	return 0;
}

int dWriteSvcStat(void)
{
	int dRet;

	if( (dRet = write_file(FILE_SVC_LIST, (char*)&stTafSvclist, sizeof(st_Taf_Svcinfo), 0)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN write_file(%s), dRet=%d", LT, FILE_SVC_LIST, dRet);
		return -1;
	}

	return 0;
}
