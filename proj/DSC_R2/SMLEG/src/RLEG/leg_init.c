/* File Include */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#ifdef SM_MUTEX
#include <pthread.h>
#endif 
#include "init_shm.h"
#include "define.h"
#include "ipaf_stat.h"
#include "ipaf_svc.h"
#include "ipaf_define.h"
#include "sfm_msgtypes.h"
#include "utillib.h"
#include "conflib.h"
#include "leg.h"
#include "leg_sm_sess.h"
#include "ipaf_sem.h"

/* Definition of New Constants */

/* Declaration of Global Variable */

/* Declaration of Extern Global Variable */

extern char		logfilepath[256];
extern LEG_DATA_SUM        *gpstCallInfo[DEF_STAT_SET_CNT];
extern LEG_TOT_STAT_t      *gpstTotStat[DEF_STAT_SET_CNT];
// TIMEOUT.conf 구조체 
extern MPTimer            	*gpMPTimer[DEF_SET_CNT];
extern MPTimer            	*gpCurMPTimer;
extern st_NOTI				*gpIdx;
extern _mem_check			*gpShmem;
extern char					g_szProcName[32];

extern unsigned int			gMyIdx;

/* Declaration of Function Definition */
extern int conflib_getNthTokenInFileSection ( char *fname, char *section, char *keyword, int  n, char *string );

int dReadFLTIDXFile(void);
int	InitSHM_TIMER(void);
void dSetCurTIMEOUT(NOTIFY_SIG *pNOTISIG);
        
int init_ipcs(void)
{
	int     dRet, key, shmId;
	char    tmp[64], fname[256], sm_sess_name[64];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* GET SYSTEM LABEL */
	if (conflib_getNthTokenInFileSection (fname, "[GENERAL]", "SYSTEM_LABEL", 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[init_ipcs] CAN'T GET SYSTEM LABEL err=%s", strerror(errno));
		return -1;
	}
	else {
		strcpy(sysLable, tmp);
		strcpy(mySysName, sysLable);
		strcpy(myAppName, g_szProcName);
		dAppLog(LOG_INFO, "SYSTEM LABEL=[%s]", sysLable);
	}

	/* RLEG MSGQ */
	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", g_szProcName, 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[init_ipcs] can't get queue key of RLEG err=%s", strerror(errno));
		return -3;
	} else
		key = strtol(tmp, 0, 0);

	if((dMyQid = msgget(key, 0666|IPC_CREAT)) < 0) {
		dAppLog(LOG_CRI, "[init_ipcs] [FAIL] MSGGET of RLEG: [%d][%s]", errno, strerror(errno) );
		return -4;
	}

	/* IXPC MSGQ */
    if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "IXPC", 1, tmp) < 0) {
        dAppLog( LOG_CRI, "[init_ipcs] CAN'T GET MSGQ KEY OF IXPC err=%s", strerror(errno));
        return -5;
    } else 
        key = strtol(tmp, 0, 0);

    if ((dIxpcQid = msgget(key,IPC_CREAT|0666)) < 0) {
            dAppLog( LOG_CRI, "[init_ipcs] IXPC msgget fail; key=0x%x,err=%d(%s)", key, errno, strerror(errno));
        return -6;
    }
	dAppLog(LOG_INFO, "[init_ipcs] dMyQid = %d", dMyQid);


	/* INIT_SHM: SHM_LOC_SADB */
	if (conflib_getNthTokenInFileSection(fname, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
		return -1;
	key = strtol(tmp, 0, 0);

	if((shmId = (int)shmget(key, sizeof(SFM_SysCommMsgType), 0666 | IPC_CREAT)) < 0) {
		if(errno != ENOENT) {
			dAppLog(LOG_CRI, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)"
					, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -1;
		}
	}
	if((loc_sadb = (SFM_SysCommMsgType*)shmat(shmId, 0, 0)) == (SFM_SysCommMsgType*)-1) {
		dAppLog(LOG_CRI, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)"
				, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}
	loc_sadb->cps_over_alm_flag = 0;
	loc_sadb->smConn[gMyIdx].dConn = 0;

	/* TIMER SHM - timeout value  */
	if( InitSHM_TIMER() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_TIMER() FAIL");
		return -1;
	}

	/** LEG_TOT_STAT 0 / 1 **/
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_STAT", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_LEG_STAT err=%s", strerror(errno));
		return -1;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_TOT_STAT_SIZE, (void **)&gpstTotStat[0]);
	if( dRet < 0 ) {
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_LEG_STAT) dRet=%d", dRet);
		return -1;
	}
	else if( dRet == 1 )
		memset(gpstTotStat[0], 0, DEF_LEG_TOT_STAT_SIZE);
	
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_STAT_1", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_LEG_STAT_1 err=%s", strerror(errno));
		return -1;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_TOT_STAT_SIZE, (void **)&gpstTotStat[1]);
	if( dRet < 0 ) {
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_LEG_STAT_1) dRet=%d", dRet);
		return -1;
	}
	else if( dRet == 1 )
		memset(gpstTotStat[1], 0, DEF_LEG_TOT_STAT_SIZE);

	/** SHM_LEG_CPS 0 / 1 **/
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_CPS", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_LEG_CPS err=%s", strerror(errno));
		return -1;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_DATA_SUM_SIZE, (void **)&gpstCallInfo[0]);
	if( dRet < 0 ) {
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_LEG_CPS) dRet=%d", dRet);
		return -1;
	}
	else if( dRet == 1 )
		memset(gpstCallInfo[0], 0, DEF_LEG_DATA_SUM_SIZE);

	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_CPS_1", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_LEG_CPS_1 err=%s", strerror(errno));
		return -1;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_DATA_SUM_SIZE, (void **)&gpstCallInfo[1]);
	if( dRet < 0 ) {
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_LEG_CPS) dRet=%d", dRet);
		return -1;
	}
	else if( dRet == 1 )
		memset(gpstCallInfo[1], 0, DEF_LEG_DATA_SUM_SIZE);

	/* INIT_SHM: SHM_LEG_SESS_CNT */
	if (conflib_getNthTokenInFileSection(fname, "SHARED_MEMORY_KEY", "SHM_LEG_SESS_CNT", 1, tmp) < 0)
		return -1;

	key = strtol(tmp, 0, 0);
	if((shmId = (int)shmget(key, sizeof(_mem_check), 0666 | IPC_CREAT)) < 0) {
		if(errno != ENOENT) {
			dAppLog(LOG_CRI, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)"
					, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -1;
		}
	}

	if((gpShmem = (_mem_check *)shmat(shmId, 0, 0)) == (_mem_check *)-1) {
		dAppLog(LOG_CRI, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)"
				, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}

	/* INIT_SHM: SHM_LEG_SESS */
	sprintf(sm_sess_name, "SHM_SM_SESS%d", gMyIdx);
	if (conflib_getNthTokenInFileSection(fname, "SHARED_MEMORY_KEY", sm_sess_name, 1, tmp) < 0)
		return -1;

	key = strtol(tmp, 0, 0);
	init_sm_sess (key);

	return 0;
}

int initProc (void)
{
	int 	ret = 0;

	SetUpSignal();

	if ((ret = init_ipcs ()) < 0) {
		return ret;
	}

	/* SCM INFO */
	if ((ret = dGetConfig_LEG ()) < 0) {
		return ret;
	}
	
	/* 초기화에 실패해도 프로세스는 기동한다
	 * 단, 실패하면 RULESET_LIST.conf 에 Rule이 있더라도 
	 * Rule 적용 여부를 판단하는 RULESET_USED.conf 가 OFF로 초기화 되기 때문에
	 * MMC 명령을 이용하여 사용할 Rule에 대해서 등록 시켜야 한다. **/

	return ret;
}


void finProc (void)
{
#if 0
	int i;
	for (i=0; i<MAX_SM_CONN_COUNT; i++)
	{
    	dAppLog(LOG_CRI, "FIN PROC][while cancelling with SM connections ...(%d) ", i);
		SMNB_disconnect(gSMConn[i].hdl_nbSceApi);
		SMNB_release(gSMConn[i].hdl_nbSceApi);
	}
#endif
    dAppLog(LOG_CRI, "### finProc #1 (cause = %d) ###", FinishFlag);
	loc_sadb->smConn[gMyIdx].dConn = DISCONNECTED;
	//disconnSCE();

    dAppLog(LOG_CRI, "### PROGRAM IS NORMALLY TERMINATED(cause = %d) ###", FinishFlag);
    exit(0);
}

/************************************************************************************
	SIGNAL FUNCTION
***********************************************************************************/
void UserControlledSignal(int sign)
{
    dAppLog(LOG_CRI, "DEFINED SIGNAL IS RECEIVED, signal = %d", sign);
    JiSTOPFlag = 0;
    FinishFlag = sign;
	finProc();
}

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        dAppLog(LOG_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

void SetUpSignal()
{
//	int i;
    JiSTOPFlag = 1;

#if 0
	for (i=0; i<20; i++)
	{
    	signal(i, UserControlledSignal);

	}
#endif
    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGKILL, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);
    dAppLog(LOG_DEBUG, "SIGNAL HANDLER WAS INSTALLED");
}

int dGetConfig_LEG (void)
{
	int     lNum, rowCnt=0, ret;
	char    *env;
	char    iv_home[64];
	char    module_conf[64];
	char    getBuf[256], token[9][64];
	FILE    *fp=NULL;

	if( (env = getenv(IV_HOME)) == NULL) {
		dAppLog(LOG_CRI, "[%s:%s:%d] not found %s environment name", __FILE__, __FUNCTION__, __LINE__, IV_HOME);
		return -1;
	}
	strcpy(iv_home, env);
	sprintf(module_conf, "%s", DEF_SYSCONF_FILE);

	if ((fp = fopen(module_conf, "r")) == NULL) {
		dAppLog(LOG_CRI,"[dGetConfig_LEG] fopen fail[%s]; err=%d(%s)", module_conf, errno, strerror(errno));
		return -1;
	}

	/* [RLEG_CONFIG] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"RLEG_CONFIG")) < 0) {
		dAppLog(LOG_CRI,"[dGetConfig_LEG] Secsion not found");
		return -1;
	}

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL)
	{
		if (rowCnt >= MAX_SM_NUM) break;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((ret = sscanf(getBuf,"%s%s%s%s%s%s%s%s%s"
						, token[0],token[1],token[2],token[3],token[4],token[5],token[6],token[7],token[8])) < 9)
		{
			dAppLog(LOG_CRI,"[dGetConfig_LEG] config syntax error; file=%s, lNum=%d, ret = %d"
					, module_conf , lNum, ret);
			fclose(fp);
			return -1;
		}

		strcpy(gSCM[rowCnt].ip, token[2]);
		gSCM[rowCnt].port = atoi(token[3]);

		gSCM[rowCnt].svc_type[0] = atoi(token[4]);
		gSCM[rowCnt].svc_type[1] = atoi(token[5]);
		gSCM[rowCnt].svc_type[2] = atoi(token[6]);
		gSCM[rowCnt].svc_type[3] = atoi(token[7]);
		gSCM[rowCnt].svc_type[4] = atoi(token[8]);

		dAppLog(LOG_INFO, "[dGetConfig_LEG] gSCM[%d] [IP:%s][PORT:%d][SVC_OPT1:%d][SVC_OPT2:%d][SVC_OPT3:%d][SVC_OPT4:%d][SVC_OPT5:%d]"
				, rowCnt, gSCM[rowCnt].ip, gSCM[rowCnt].port
				, gSCM[rowCnt].svc_type[0], gSCM[rowCnt].svc_type[1], gSCM[rowCnt].svc_type[2], gSCM[rowCnt].svc_type[3], gSCM[rowCnt].svc_type[4]);

		rowCnt++;
	}
	fclose(fp);
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
					dAppLog(LOG_CRI, " * TRACE READ        : IDX[%d", dIdx);
				}
				else if( strcmp(szType, "PDSN") == 0 )
				{
					gpIdx->dPdsnIdx = dIdx;
					dAppLog(LOG_CRI, " * PDSN READ         : IDX[%d", dIdx);
				}
				else if( strcmp(szType, "RULESET_LIST") == 0 )
				{
					gpIdx->dRsetListIdx = dIdx;
					dAppLog(LOG_CRI, " * RULESET_LIST READ : IDX[%d", dIdx);
				}
				else if( strcmp(szType, "RULESET_USED") == 0 )
				{
					gpIdx->dRsetUsedIdx = dIdx;
					dAppLog(LOG_CRI, " * RULESET_USED READ : IDX[%d", dIdx);
				}
				else if( strcmp(szType, "CPS") == 0 )
				{
					gpIdx->dCpsIdx = dIdx;
					dAppLog(LOG_CRI, " * CPS OVLD READ     : IDX[%d", dIdx);
				}
				else if( strcmp(szType, "TIMEOUT") == 0 )
				{
					gpIdx->dTimeIdx = dIdx;
					gpCurMPTimer = (dIdx == 0) ? gpMPTimer[0] : gpMPTimer[1];
					dAppLog(LOG_CRI, " * TIMEOUT READ      : IDX[%d", dIdx);
					dAppLog(LOG_CRI, "   - SM TIMEOUT      : %d", gpCurMPTimer->sm_sess_timeout);
				}
			}
		}
		dIdx = 0; i++;
	}

	fclose(fa);

	return i;
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
