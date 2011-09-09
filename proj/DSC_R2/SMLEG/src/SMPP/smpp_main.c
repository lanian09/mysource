/**********************************************************
   Author   :
   Revision : 
   Description:
   Copyright (c) , Inc.
***********************************************************/
#include <smpp.h>
#include <unistd.h>

extern 	SMPP_FD_TBL client;
extern 	MYSQL	sql, *conn;
extern int  check_my_run_status(char *procname);

char	module_conf[256];
char    sysLabel[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
int		smppQid, ixpcQid;

int		JiSTOPFlag;
int		FinishFlag;
// TRACE_INFO.conf ¡¾¢¬A¢ÒA¨ù 
st_SESSInfo			*gpTrcList[DEF_SET_CNT];
st_SESSInfo			*gpCurTrc; // CURRENT OPERATION pointer
// TIMEOUT.conf ±¸Á¶Ã¼ 
MPTimer            	*gpMPTimer[DEF_SET_CNT];
MPTimer            	*gpCurMPTimer;

st_NOTI				gstIdx;
st_NOTI				*gpIdx = &gstIdx;

int 	dReadFLTIDXFile(void);
int 	InitSHM_TIMER(void);
int 	InitSHM_TRACE(void);
void 	dSetCurTIMEOUT(NOTIFY_SIG *pNOTISIG);
void dSetCurTrace(NOTIFY_SIG *pNOTISIG);

MmcHdlrVector mmcHdlrVector[MML_MAX_MMC_HANDLER] = 
{
		{"set-smc-info",	smpp_mmc_set_smc_info},
		{"dis-smc-info",	smpp_mmc_dis_smc_info},
		{"del-smc-info",	smpp_mmc_del_smc_info},
};

int     pid;
int		numMmcHdlr=10;
char	resBuf[4096], resHead[1024];
char    vERSION[7] = "R2.0.0";		// R1.0.0 -> R2.0.0

SMS_DB_INFO 	sms_db;
SMS_HIS     	sms_his;
SMPP_MSG_INFO	smpp_msg_info;


////////////////////////////////////////////////////////////////////////////
int main (void)
{
    time_t  now, old = 0;
	int     dRet;
	int     check_Index;

	pid = getpid();

	/*## APPLICATION INIT ##*/
	if (smpp_init(pid) < 0) {
		exit(1);
	}

#if 1
	if( (check_Index = check_my_run_status("SMPP")) < 0) {
		dAppLog(LOG_CRI, "[SMPP INIT] %s is already running as pid %d", myAppName, pid);
		exit(0);
	}
#endif

	/*## SET VERSION ##*/
	if( (dRet = set_version(SEQ_PROC_SMPP, vERSION)) < 0) {   
		dAppLog( LOG_CRI, "[SMPP INIT] set_version error(ret=%d,idx=%d,ver=%s)"
				, dRet, SEQ_PROC_SMPP, vERSION);                                                       
	}

	dRet = dReadFLTIDXFile();
	if( dRet < 0 ) {
		dAppLog( LOG_CRI, "dReadFLTIDXFile() FAIL RET:%d", dRet);
	}

	/*## DB CONNECTION ##*/
	if ((dRet = smpp_connectDB()) < 0) {
		dAppLog(LOG_CRI, "[SMPP INIT] mysql connection fail, err=%s", mysql_error(&sql));
		exit(1);
	}

	dAppLog(LOG_CRI, "[### PROCESS INITIAL SUCCESS - SMPP START]:[PID:%d] ###", pid);

	/*## MAIN ROOF ##*/
    while (JiSTOPFlag)
	{
		now = time(&now);
		if (now != old) {

			keepalivelib_increase ();
			old = now;
			manage_con_sts();
			audit_call(now);
		}

		proc_msgQ_data();
		proc_socket_data();
	}

	smpp_finProc();
	return 0;
}

int InitSHM_TRACE(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TRACE", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(st_SESSInfo), (void **)&gpTrcList[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TRACE1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(st_SESSInfo), (void **)&gpTrcList[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}


int InitSHM_TIMER(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* INIT_SHM: TIMER0 */
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TIMER", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(MPTimer), (void **)&gpMPTimer[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	/* INIT_SHM: TIMER1 */
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TIMER1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(MPTimer), (void **)&gpMPTimer[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}


int dReadFLTIDXFile(void)
{
	FILE *fa;
	char szBuf[1024];
	char szType[64];
	int i = 0, dIdx = 0;

	fa = fopen(DEF_NOTI_INDEX_FILE, "r");
	if(fa == NULL)
	{
		dAppLog(LOG_CRI,"dReadFLTIDXFile : %s FILE OPEN FAIL (%s)",
		DEF_NOTI_INDEX_FILE, strerror(errno));
		return -1;
	}

	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#')
		{
			dAppLog(LOG_CRI,"dReadFLTIDXFile : %s File [%d] row format error",
			DEF_NOTI_INDEX_FILE, i);
			fclose(fa);
			return -1;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{

			if(sscanf(&szBuf[2],"%s %d",szType, &dIdx) == 2)
			{
				if(strcmp(szType,"TRACE") == 0)
				{
					gpIdx->dTrcIdx = dIdx;
					dAppLog(LOG_CRI, "TRACE READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "PDSN") == 0 )
				{
					gpIdx->dPdsnIdx = dIdx;
					dAppLog(LOG_CRI, "PDSN READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "RULESET_LIST") == 0 )
				{
					gpIdx->dRsetListIdx = dIdx;
					dAppLog(LOG_CRI, "RULESET_LIST READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "RULESET_USED") == 0 )
				{
					gpIdx->dRsetUsedIdx = dIdx;
					dAppLog(LOG_CRI, "RULESET_USED READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "CPS") == 0 )
				{
					gpIdx->dCpsIdx = dIdx;
					dAppLog(LOG_CRI, "CPS OVLD CONTROL READ IDX[%d]", dIdx);
				}
				else if( strcmp(szType, "TIMEOUT") == 0 )
				{
					gpIdx->dTimeIdx = dIdx;
					gpCurMPTimer = (dIdx == 0) ? gpMPTimer[0] : gpMPTimer[1];
					dAppLog(LOG_CRI, "TIMEOUT READ IDX[%d]", dIdx);
				}
			}
		}
		dIdx = 0; i++;
	}

	fclose(fa);

	return i;
} 

void dSetCurTrace(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dTrcIdx < 0 || pNOTISIG->stNoti.dTrcIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dTrcIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dTrcIdx);
		return;
		//gpCurTrc = NULL;
	}

	gpIdx->dTrcIdx = pNOTISIG->stNoti.dTrcIdx;
	gpCurTrc = gpTrcList[gpIdx->dTrcIdx];

	dAppLog(LOG_CRI, "NOTI] TRACE ACTIVE IDX[%d]", pNOTISIG->stNoti.dTrcIdx);
}

void dSetCurTIMEOUT(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dTimeIdx < 0 || pNOTISIG->stNoti.dTimeIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dTimeIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dTimeIdx);
		return;
		//gpCurMPTimer = NULL;
	}

	gpIdx->dTimeIdx = pNOTISIG->stNoti.dTimeIdx;
	gpCurMPTimer = gpMPTimer[gpIdx->dTimeIdx];

	dAppLog(LOG_CRI, "NOTI] TIMEOUT ACTIVE IDX[%d]", pNOTISIG->stNoti.dTimeIdx);
}

