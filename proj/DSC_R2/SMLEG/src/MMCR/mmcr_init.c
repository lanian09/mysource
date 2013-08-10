/**A.1* FILE INCLUSION ********************************************************/
#include "mmcr.h"
#include "ipaf_names.h"
#include "ipaf_svc.h"
#include "ipaf_stat.h"
#include "hash_pdsn.h"
#include "init_shm.h"
#include "comm_trace.h"
#include "nifo.h"
#include "mems.h"
#include "common_ana.h"
#include "sm_subs_info.h"
#include "comm_session.h"

/**B.1* DEFINITION OF NEW CONSTANTS *******************************************/

/**C.1* DECLARATION OF VARIABLES **********************************************/
extern int      msgqTable[MSGQ_MAX_SIZE];
extern int      trcLogId, trcErrLogId;
extern char		iv_home[64], l_sysconf[256];
extern char		trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern T_keepalive  	*keepalive;

extern stLogLevel   	*logLevel;
extern int				numMmcHdlr;
extern MmcFuncTable		mmcFuncTable[MMCR_MAX_MMC_HANDLER];
extern int      		numMmcMemberHdlr;
extern MmcMemberTable 	mmcMemberTable[MMCR_MAX_MMC_HANDLER];

extern stMEMSINFO			*pstMEMSINFO;
// TRACE_INFO.conf 구조체 
extern st_SESSInfo			*gpTrcList[DEF_SET_CNT];
// PDSN.conf 구조체  
extern PDSN_LIST        	*gpPdsnList[DEF_SET_CNT];
// PDSN HASH
extern stHASHOINFO         *gpPdsnHash[DEF_SET_CNT];
// RULESET_LIST.conf 구조체 
extern ST_PBTABLE_LIST  	*gpRsetList[DEF_SET_CNT];
// RULESET_USED.conf 구조체 
extern RULESET_USED_FLAG	*gpRSetUsedList[DEF_SET_CNT];
// CALL_OVER_CTRL.conf 구조체 
extern CPS_OVLD_CTRL		*gpCpsOvldCtrl[DEF_SET_CNT];
// TIMEOUT.conf 구조체 
extern MPTimer            	*gpMPTimer[DEF_SET_CNT];

extern LEG_DATA_SUM			*gpstCallInfo[DEF_STAT_SET_CNT];

extern LEG_CALL_DATA        *gpstCallDataPerSec; //이전 5sec 동안의 cps와 tps값을 저장하고 있는 변수.
// RLEG SESSION COUNT STRUCTURE
_mem_check           *gpShmem; 

char vERSION[7] = "R2.0.0"; 	// R1.0.0 -> R2.0.0 (2011-05-05)

/**D.1*  DEFINITION OF FUNCTIONS  *********************************************/
extern void mmc_yh_init ();
extern int dLoad_TrcConf(void);

/******************************************************************************

*******************************************************************************/
int InitSHM_LEG_CPS(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/** SHM_LEG_CPS 0 / 1 **/
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_CPS", 1, tmp) < 0) {
		logPrint(trcErrLogId, FL, "CAN'T GET SHM KEY OF SHM_LEG_CPS err=%s\n", strerror(errno));
		return -11;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_DATA_SUM_SIZE, (void **)&gpstCallInfo[0]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL Init_shm(SHM_LEG_CPS) dRet=%d\n", dRet);
		return -12;
	}
	else if( dRet == 1 )
		memset(gpstCallInfo[0], 0, DEF_LEG_DATA_SUM_SIZE);

	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_CPS_1", 1, tmp) < 0) {
		logPrint(trcErrLogId, FL, "CAN'T GET SHM KEY OF SHM_LEG_CPS_1 err=%s\n", strerror(errno));
		return -13;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_DATA_SUM_SIZE, (void **)&gpstCallInfo[1]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL Init_shm(SHM_LEG_CPS) dRet=%d\n", dRet);
		return -14;
	}
	else if( dRet == 1 )
		memset(gpstCallInfo[1], 0, DEF_LEG_DATA_SUM_SIZE);

	// 이전 5sec 동안의 CPS와 TPS값을 저장해 놓기 위한 공유 메모리 설정.
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_CALL_PER_SEC", 1, tmp) < 0) {
		logPrint(trcErrLogId, FL, "CAN'T GET SHM KEY OF SHM_CALL_PER_SEC err=%s\n", strerror(errno));
		return -15;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_CALL_DATA_SIZE, (void **)&gpstCallDataPerSec);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL Init_shm(SHM_CALL_PER_SEC) dRet=%d\n", dRet);
		return -16;
	}
	else if( dRet == 1 )
		memset(gpstCallDataPerSec, 0, DEF_LEG_CALL_DATA_SIZE);

	return 0;
}

int InitSHM_LEG_SESS_CNT(void)
{
	int     dRet;
	int     key; 
	char    tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* INIT_SHM: SHM_LEG_SESS_CNT */
	if (conflib_getNthTokenInFileSection(fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_SESS_CNT", 1, tmp) < 0) { 
		return -17; 
	}    
	else 
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, sizeof(_mem_check), (void **)&gpShmem);
	if( dRet < 0 )
	{    
		logPrint(trcErrLogId, FL, "FAIL %s dRet=%d\n", __FUNCTION__, dRet);
		return -14; 
	}    
	else if( dRet == 1 ) {
		gpShmem->rad_sess=0;
		logPrint(trcErrLogId, FL, ">>> SHM_LEG_SESS_CNT [%d] [key=%d : addr=%p\n", dRet, key, gpShmem);
	}
	else {
		logPrint(trcErrLogId, FL, ">>> SHM_LEG_SESS_CNT FAIL [%d] [key=%d : addr=%p\n", dRet, key, gpShmem);
	}

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
		logPrint(trcErrLogId, FL, "FAIL %s(0) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TRACE1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(st_SESSInfo), (void **)&gpTrcList[1]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(1) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_PDSN_LIST(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_LIST", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(PDSN_LIST), (void **)&gpPdsnList[0]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(0) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_LIST1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(PDSN_LIST), (void **)&gpPdsnList[1]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(1) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_PDSN_HASH(void)
{
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_HASH", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	if((gpPdsnHash[0] = hasho_init(key, sizeof(st_Pdsn_HashKey), sizeof(st_Pdsn_HashKey), sizeof(st_Pdsn_HashData), MAX_HASH_PDSN_CNT, NULL, 0)) == NULL) 
	{
		logPrint(trcErrLogId, FL, "PDSN HASH hasho_init() NULL\n");
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_HASH1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	if((gpPdsnHash[1] = hasho_init(key, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_DATA_SIZE, MAX_HASH_PDSN_CNT, NULL, 0)) == NULL) 
	{
		logPrint(trcErrLogId, FL, "PDSN HASH hasho_init() NULL\n");
		return -1;
	}

	return 0;
}

int InitSHM_RSET_LIST(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_RSET_LIST", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(ST_PBTABLE_LIST), (void **)&gpRsetList[0]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(0) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_RSET_LIST1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(ST_PBTABLE_LIST), (void **)&gpRsetList[1]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(1) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_RSET_USED(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_RSET_USED", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(RULESET_USED_FLAG), (void **)&gpRSetUsedList[0]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(0) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_RSET_USED1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(RULESET_USED_FLAG), (void **)&gpRSetUsedList[1]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(1) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_CPS_OVLD(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_CPS_OVLD", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(CPS_OVLD_CTRL), (void **)&gpCpsOvldCtrl[0]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(0) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_CPS_OVLD1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(CPS_OVLD_CTRL), (void **)&gpCpsOvldCtrl[1]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(1) dRet=%d\n", __FUNCTION__, dRet);
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
		logPrint(trcErrLogId, FL, "FAIL %s(0) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}

	/* INIT_SHM: TIMER1 */
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TIMER1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(MPTimer), (void **)&gpMPTimer[1]);
	if( dRet < 0 )
	{
		logPrint(trcErrLogId, FL, "FAIL %s(1) dRet=%d\n", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}


int InitSys()
{
	char	*env, tmp[32], tmp2[10];
	int		i, key, ret, shmId;
/*	int		pid; */

	if( (pstMEMSINFO = nifo_init_zone("MMCR", SEQ_PROC_MMCR, DEF_NIFO_ZONE_CONF_FILE)) == NULL )
	{
		logPrint(trcErrLogId, FL, "ERROR nifo_init_zone NULL\n");
		return -15;
	}

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[mmcr_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);
	strcpy (myAppName, "MMCR");
	
	commlib_setupSignals (NULL);

	// ping_test할때 wait로 child 프로세스의 종료 조건을 확인해야하는데,
	//	commlib_setupSignals에서 SIGCHLD를 catch하도록 되어 있는 것을 release한다.
	//sigrelse (SIGCHLD);
#if 0 // jjinri 06.12
	UnBlockSignal(SIGCHLD);
#endif	

	// MMC 처리 function들을 bsearch로 찾기 위해 sort한다.
	qsort ( (void*)mmcFuncTable,
			numMmcHdlr,
			sizeof(MmcFuncTable),
			mmcr_mmcHdlrVector_qsortCmp );

	qsort ( (void*)mmcMemberTable,
                numMmcMemberHdlr,
                sizeof(MmcMemberTable),
                mmcr_mmcMemberHdlrVector_qsortCmp );

	// MMC 처리 function들을 bsearch로 찾기 위해 sort한다.
	mmc_yh_init();
#if 0
	/* add by sjjeon : 이 기능을 사용해야 하는지 고려. 기존 로그 방식 우선 사용.*/
	pid = getpid();
	/*## APPLICATION LOG INIT ##*/ 
	Init_logdebug( pid, "MMCR", "/DSC/APPLOG");

	dAppLog(LOG_CRI, "[PROCESS INITIAL SUCCESS - SAMD START]:[PID:%d]", pid); 
#endif
	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[mmcr_init] not found %s environment name\n", IV_HOME);
		return -1;
	}
	strcpy (iv_home, env);

	if (mmcr_initLog() < 0)
		return -1;

	sprintf(l_sysconf, "%s/%s", iv_home, SYSCONF_FILE);
	
	if ((ret = keepalivelib_init("MMCR")) < 0)
		return -1;

	memset ((char*)keepalive, 0, sizeof(T_keepalive));

	/* IXPC MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((msgqTable[0] = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[mmcr_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
        sprintf(trcBuf,"[mmcr_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	/* MMCR MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "MMCR", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((msgqTable[1] = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[mmcr_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	/* PANA MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "PANA", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((msgqTable[2] = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[mmcr_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	/* RANA MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "RANA", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((msgqTable[3] = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[mmcr_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	/* RLEG MSG Qid를 구한다 */
	for( i = 0; i < MAX_RLEG_CNT; i++ ){
		sprintf(tmp2,"RLEG%d", i);
		if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", tmp2, 1, tmp) < 0)
			return -1;
		key = strtol(tmp,0,0);
		if ((msgqTable[i+7] = msgget (key, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[mmcr_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
	}

	/* RDRANA MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "RDRANA", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((msgqTable[5] = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[mmcr_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	/* SMPP MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "SMPP", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((msgqTable[6] = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[mmcr_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	
    /* Shared Memory */
    /* Log level */
    if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_LOG_LEVEL", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);

//	if ((shmId = (int)shmget (key, sizeof(stLogLevel), 0644|IPC_CREAT)) < 0) {
	if ((shmId = (int)shmget (key, sizeof(stLogLevel), 0666|IPC_CREAT)) < 0) {
		if (errno != ENOENT) {
			fprintf (stderr,"[mmcr_init] shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
	}

	if ((logLevel = (stLogLevel*) shmat (shmId,0,0)) == (stLogLevel*)-1) {
		fprintf (stderr,"[mmcr_init] shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
    }
    loadLogLevel();

	if( InitSHM_LEG_CPS() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_LEG_CPS() FAIL\n");
		return -4;
	}
	if( InitSHM_LEG_SESS_CNT() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_LEG_SESS_CNT() FAIL\n");
		return -5;
	}
	/* MMCR NOTI Shared Memory Init */
	if( InitSHM_TRACE() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_TRACE() FAIL\n");
		return -7;
	}

	if( InitSHM_PDSN_LIST() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_PDSN_LIST() FAIL\n");
		return -8;
	}

	if( InitSHM_PDSN_HASH() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_PDSN_HASH() FAIL\n");
		return -9;
	}

	if( InitSHM_RSET_LIST() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_RSET_LIST() FAIL\n");
		return -10;
	}

	if( InitSHM_RSET_USED() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_RSET_USED() FAIL\n");
		return -11;
	}

	if( InitSHM_CPS_OVLD() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_CPS_OVLD() FAIL\n");
		return -12;
	}

	if( InitSHM_TIMER() < 0 ) {
		logPrint (trcErrLogId,FL,"ERROR InitSHM_TIMER() FAIL\n");
		return -13;
	}

	if(set_version(SEQ_PROC_MMCR,vERSION) < 0 )
	{
		fprintf (stderr,"[mmcr_init] set_version failed\n");
        return -14;
    }

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);	
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);
	return 1;
} /* End of InitSys */


int mmcr_initLog (void)
{
    char    fname[FILESIZE];

    sprintf(fname,"%s/%s.%s", iv_home, MMCR_LOG_FILE, mySysName);

    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[samd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", iv_home, MMCR_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[samd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }
    return 1;

} /** End of samd_initLog **/


void handleChildProcess(int sign)
{
	int status;

	while (wait3 (&status, WNOHANG, (struct rusage *)0) > 0);
}
    

void UserControlledSignal(int sign)
{
	//    dAppLog( LOG_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = 0");
	exit(0);
}


/*******************************************************************************

 *******************************************************************************/
void IgnoreSignal(int sign)
{
	//    if (sign != SIGALRM)
	//       dAppLog( LOG_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
	signal(sign, IgnoreSignal);
}

/*******************************************************************************

 *******************************************************************************/
void SetUpSignal()
{
	/* WANTED SIGNALS   */
	signal(SIGTERM, UserControlledSignal);
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
	//    signal(SIGCLD, SIG_IGN);
	signal(SIGCHLD, handleChildProcess);
}

