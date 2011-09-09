#include "stmd_proto.h"
#include "sfm_msgtypes.h"

extern  int     stmdQid, ixpcQid, condQid, fimdQid, nmsifQid, rlegQid;
extern  char    mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogId, trcErrLogId, trcLogFlag;
extern  int     txStaticCnt;
extern  int     sysCnt;
extern  int     numMmcHdlr;
extern  short   printTIME [STMD_PERIOD_TYPE_NUM];
extern  short   delTIME [STMD_PERIOD_TYPE_NUM];
extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
//extern  char    updateJOB [MAX_CRONJOB_NUM];
extern  char    strITEM[STMD_MASK_ITEM_NUM][14];
extern  int    max_sts_count;
extern  STMD_StatisticProcessInfo   txStaticsProcInfo[SYSCONF_MAX_ASSO_SYS_NUM*SYSCONF_MAX_APPL_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  OnDemandList    onDEMAND[MAX_ONDEMAND_NUM];
extern  StmdMmcHdlrVector   mmcHdlrVector[STMD_MAX_MMC_HANDLER];
extern  CronList    cronJOB[MAX_CRONJOB_NUM];
extern  MYSQL       sql, *conn;
extern  MYSQL       sql_Rm, *conn_Rm;

extern SFM_sfdb     *sfdb;
//extern SFM_SysCommMsgType   *loc_sadb;
extern int			SCE_CNT;
extern SCE_t		g_stSCE[MAX_SCE_NUM];

char    ver[8] = "R2.0.0";    // BEFORE: R1.2.0 -> R2.0.0 (2011-05-05)

char	rsFname[32];

extern 	RuleSetList 	g_stSCERule[MAX_SCE_NUM];
extern int				g_ruleIdBuf[MAX_SCE_NUM][MAX_RULE_NUM];
extern int				g_ruleItemCnt;

extern 	RuleEntryList 	g_stSCEEntry[MAX_SCE_NUM];
extern int				g_ruleEntryBuf[MAX_ENTRY_NUM];
extern char				g_ruleEntryName[MAX_ENTRY_NUM][256];
extern int				g_ruleEntryCnt;

extern PDSN_LIST		g_stPdsn;
extern int				g_PdsnCnt;
extern SMSC_LIST		g_stSmsc[MAX_SMSC_NUM];
extern int				g_SmscCnt;

extern unsigned int	mysql_timeout;

int stmd_init_INI_table(void);
int stmd_init_Local_INI_table(void);

int	initPdsn(void);
int	initSmsc(void);
int writeDelTime(void);
int readDelTime(void);

int InitSys()
{
    GeneralQMsgType rxGenQMsg;
    char    *env, fname[256], tmp[64];
    int     key;
    int     i, ret;
	int		RETRY = 5;
	int		shmId = -1;
//	int		dup = 0;
	

    if ((env = getenv(MY_SYS_NAME)) == NULL) {
        fprintf(stderr,"[stmd_init] not found %s environment name\n", MY_SYS_NAME);
        return -1;
    }

    strcpy (mySysName, env);
    strcpy (myAppName, "STMD");
    commlib_setupSignals(NULL);

    if(set_proc_version(OMP_VER_INX_STMD, ver) < 0){
        fprintf(stderr, "[InitSys] setting process version failed\n");
        return -1;
    }

    if (stmd_initLog() < 0)
        return -1;

    // Keepalive를 공유메모리를 할당한다.
    if (keepalivelib_init (myAppName) < 0)
        return -1;

    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"[InitSys] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	sprintf (rsFname, "%s/%s", env, RULE_SET_FILE);

    // sysconfig에서 자신의 message queue key를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", myAppName, 1, tmp) < 0) {
        sprintf(trcBuf, "My System message queue Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    key = strtol(tmp,0,0);
    if ((stmdQid = msgget (key, IPC_CREAT|0666)) < 0) {
        sprintf(trcBuf,"[stmd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    fprintf(stdout, "stmdQid = %x, key = %x\n", stmdQid, key);

    // sysconfig에서 IXPC의 message queue key를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "IXPC", 1, tmp) < 0) {
        sprintf(trcBuf, "IXPC message queue Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    key = strtol(tmp,0,0);
    if ((ixpcQid = msgget (key, IPC_CREAT|0666)) < 0) {
        sprintf(trcBuf,"[stmd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }

    // sysconfig에서 COND의 message queue key를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "COND", 1, tmp) < 0) {
        sprintf(trcBuf, "COND message queue Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    key = strtol(tmp,0,0);
    if ((condQid = msgget (key, IPC_CREAT|0666)) < 0) {
        sprintf(trcBuf,"[stmd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
#if 0
    // sysconfig에서 COND의 message queue key를 구한다.
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "RLEG", 1, tmp) < 0) {
        sprintf(trcBuf, "RLEG message queue Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    key = strtol(tmp,0,0);
    if ((rlegQid = msgget (key, IPC_CREAT|0666)) < 0) {
        sprintf(trcBuf,"[stmd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
#endif

#if 0 
    /* INIT_SHM: SHM_LOC_SADB */
    if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LOC_SADB", 1, tmp) < 0) {
        sprintf( trcBuf, "CAN'T GET SHM KEY OF S_SSHM_FIDB err=%s", strerror(errno));
        trclib_writeErr( FL, trcBuf );
        return -1;
    }
    else
        key = strtol(tmp, 0, 0);
    ret = shmget( key, sizeof(SFM_SysCommMsgType), 0666|IPC_CREAT|IPC_EXCL);
    if( ret < 0 )
    {
        if(errno == EEXIST)
        {
            ret = shmget( key, sizeof(SFM_SysCommMsgType), 0666|IPC_CREAT);
            if(ret < 0)
                return -2;

            if((long)(loc_sadb = (SFM_SysCommMsgType *)shmat( ret, (char*)0, 0)) == -1)
                return -3;
        }
        else
            return -4;
    }
    if((long)(loc_sadb = (SFM_SysCommMsgType *)shmat( ret, (char*)0, 0)) == -1)
        return -5;

#endif

    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "NMSIF", 1, tmp) < 0) {
        sprintf(trcBuf, "NMSIF message queue Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    key = strtol(tmp,0,0);
    if ((nmsifQid = msgget (key, IPC_CREAT|0666)) < 0) {
        sprintf(trcBuf,"[stmd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }

    // Get FIMD Qid -- 03.09.17
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "FIMD", 1, tmp) < 0) {
        sprintf(trcBuf, "FIMD message queue Get Fail = %s[%d]\n", strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    key = strtol(tmp,0,0);
    if ((fimdQid = msgget (key, IPC_CREAT|0666)) < 0) {
        sprintf(trcBuf,"[stmd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }

    
    // 상태,장애 정보를 관리하는 shared memory(sfdb)를 attach한다.
    //
    if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SFDB", 1, tmp) < 0)
        return -1;
    key = strtol(tmp,0,0);
    
    // attach
    //
    if ((shmId = (int)shmget (key, SFM_SFDB_SIZE, 0666)) < 0) 
    {
        if (errno != ENOENT) 
        {
            fprintf(stderr,"1.[stmd_getSfdb] SFDB shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
            return -1;
        }
    }
    if ((sfdb = (SFM_sfdb*) shmat (shmId,0,0)) == (SFM_sfdb*)-1) 
    {
        fprintf(stderr,"2.[stmd_getSfdb] SFDB shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

	dInit_SvcAlmConf();
    

    // STMD가 주기적 통계를 위한 정보를 수집한다.
	readDelTime();
    readPrintTime();
    readPrintMaskValue();
    readCronJobInFile();

    // STMD가 5분 통계 요구를 하기 위한 시스템과 프로세스를 정리한다.
    if ( stmd_initStatisticReq_ProcInfo() < 0 ) {
        sprintf(trcBuf, "stmd_initStatisticReq_ProcInfo error\n");
        trclib_writeErr(FL, trcBuf);
        return -1;
    }

    for ( i=0 ; i<MAX_ONDEMAND_NUM; i++)
        onDEMAND[i].statisticsType = NOT_REGISTERED;

//    memset(updateJOB, 0, sizeof(updateJOB));

	// local DB Connection FAIL이 발생하면, 프로세스 종료 added by uamyd
	if( stmd_mysql_init(LOCAL_DB) < 0 ){
		keepalivelib_increase();

		sprintf(trcBuf,">>> mysql_real_connect local fail; err=%s\n", mysql_error(&sql));
		trclib_writeLogErr (FL,trcBuf);

		sprintf(trcBuf,"msgq cleaning...\n"); trclib_writeLogErr (FL,trcBuf);
		while (msgrcv(stmdQid, &rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT) > 0);

		sprintf(trcBuf,"stmd terminate.\n");  trclib_writeLogErr (FL,trcBuf);
		return -1;
	}



/****** changed by uamyd 20110303 ******************************************
	mysql_timeout = KEEPALIVE_DEADLINE;
    
    mysql_init (&sql);
	mysql_options(&sql, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&mysql_timeout);

	while( (conn = mysql_real_connect(&sql, "localhost", "root", "mysql", 
			STM_STATISTIC_DB_NAME, 0, 0, 0)) == NULL )
	{
	        keepalivelib_increase();
		while (msgrcv(stmdQid, &rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT) > 0);
        	sprintf(trcBuf,">>> mysql_real_connect local fail; err=%s\n", mysql_error(&sql));
        	trclib_writeLogErr (FL,trcBuf);
		sleep(5);
	}
****************************************************************************/
/*
    if ((conn = mysql_real_connect (&sql, "localhost", "root", "mysql",
        STM_STATISTIC_DB_NAME, 0, 0, 0)) == NULL) 
    {
        sprintf(trcBuf,">>> mysql_real_connect local fail; err=%s\n", mysql_error(&sql));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
*/

	// ACTIVE SYSTEM을 찾아서 직접 접속하자. 
	if( (sfdb->sys[SCMA].commInfo.systemDup.myStatus == ACTIVE) && 
		(sfdb->sys[SCMA].commInfo.systemDup.yourStatus == STANDBY) && 
		(sfdb->sys[SCMB].commInfo.systemDup.myStatus != ACTIVE) )
	{
		SYS_MODE = ACT_STANDBY_MODE;
		ACT_SYS	= ACTIVE_A;
	}
	else if( (sfdb->sys[SCMA].commInfo.systemDup.myStatus != ACTIVE) && 
			 (sfdb->sys[SCMB].commInfo.systemDup.myStatus == ACTIVE) &&
			 (sfdb->sys[SCMB].commInfo.systemDup.yourStatus == STANDBY) )

	{
		SYS_MODE = ACT_STANDBY_MODE;
		ACT_SYS	= ACTIVE_B;
	}
	else
	{
		SYS_MODE = DUAL_STANDBY_MODE;
	}

#if 0
	if( SYS_MODE == DUAL_STANDBY_MODE )
	{
        sprintf(trcBuf,">>> Dual Standby Mode...Keepalive...Not Available service.\n");
        trclib_writeLogErr (FL,trcBuf);
		while( SYS_MODE == DUAL_STANDBY_MODE )
		{
			if( (sfdb->sys[1].commInfo.systemDup.myStatus == 1 && sfdb->sys[2].commInfo.systemDup.myStatus != 1) || 
					(sfdb->sys[1].commInfo.systemDup.myStatus != 1 && sfdb->sys[2].commInfo.systemDup.myStatus == 1) )
			{
				SYS_MODE = ACT_STANDBY_MODE;
				sprintf(trcBuf,">>> Dual Standby Mode -> Active Standby Mode Available service.\n");
				trclib_writeLogErr (FL,trcBuf);
				break;
			}
			else
			{
				SYS_MODE = DUAL_STANDBY_MODE;
			}
	        keepalivelib_increase();
			sleep(1);
		}
	}
#endif

	/*
	if( (sfdb->sys[1].commInfo.systemDup.myStatus == 1 && sfdb->sys[2].commInfo.systemDup.myStatus != 1) || 
			(sfdb->sys[1].commInfo.systemDup.myStatus != 1 && sfdb->sys[2].commInfo.systemDup.myStatus == 1) )
	{
		dup = ACT_STANDBY_MODE;
	}
	else
	{
		dup = DUAL_STANDBY_MODE;
	}
	
	if( dup == DUAL_STANDBY_MODE )
	{
        sprintf(trcBuf,">>> Dual Standby Mode...Keepalive...Not Available service.\n");
        trclib_writeLogErr (FL,trcBuf);
		while( dup == DUAL_STANDBY_MODE )
		{
			if( (sfdb->sys[1].commInfo.systemDup.myStatus == 1 && sfdb->sys[2].commInfo.systemDup.myStatus != 1) || 
					(sfdb->sys[1].commInfo.systemDup.myStatus != 1 && sfdb->sys[2].commInfo.systemDup.myStatus == 1) )
			{
				dup = ACT_STANDBY_MODE;
				sprintf(trcBuf,">>> Dual Standby Mode -> Active Standby Mode Available service.\n");
				trclib_writeLogErr (FL,trcBuf);
			}
			else
			{
				dup = DUAL_STANDBY_MODE;
			}

	        keepalivelib_increase();
			sleep(1);
		}
	}
	*/

	// remote DB Connection FAIL 발생하는 경우에는 실패를 하더라도, Next Process added by uamyd
	if( stmd_mysql_init(REMOTE_DB) < 0 ){
        sprintf(trcBuf,">>> mysql_real_connect Remote fail; err=%s\n", mysql_error(&sql_Rm));
        trclib_writeLogErr (FL,trcBuf);
		keepalivelib_increase();
		for( i = 0; i< RETRY; i++ ){
			if( stmd_mysql_init(REMOTE_DB) < 0 ){
				sprintf(trcBuf,">>> Retry Remote Connection[%dth]; err=%s\n", i, mysql_error(&sql_Rm));
				trclib_writeLogErr (FL,trcBuf);
        		keepalivelib_increase();
			} else {
				sprintf(trcBuf,">>> Retry Remote Connection Success[%dth].\n", i);
				trclib_writeLogErr (FL,trcBuf);
				break;
			}
		}
	}
/**** changed by uamyd 20110303
    mysql_init (&sql_Rm);
	mysql_options(&sql_Rm, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&mysql_timeout);
 	if ((conn_Rm = mysql_real_connect (&sql_Rm, DSC_IP, "root", "mysql",
        CM_DB_NAME, 0, 0, 0)) == NULL) 
    {
        sprintf(trcBuf,">>> mysql_real_connect Remote fail; err=%s\n", mysql_error(&sql_Rm));
        trclib_writeLogErr (FL,trcBuf);
        keepalivelib_increase();
		for( i = 0; i < RETRY; i++ )
		{
        	keepalivelib_increase();
			if( conn_Rm != NULL )
				mysql_close(conn_Rm);

    		mysql_init (&sql_Rm);
			mysql_options(&sql_Rm, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&mysql_timeout);
			if ((conn_Rm = mysql_real_connect (&sql_Rm, DSC_IP, "root", "mysql",
							CM_DB_NAME, 0, 0, 0)) == NULL) 
			{
				sprintf(trcBuf,">>> Retry Remote Connection[%dth]; err=%s\n", i, mysql_error(&sql_Rm));
				trclib_writeLogErr (FL,trcBuf);
        		keepalivelib_increase();
			}
			else
			{
				sprintf(trcBuf,">>> Retry Remote Connection Success[%dth].\n", i);
				trclib_writeLogErr (FL,trcBuf);
				break;
			}
		}
    }
*******/
	if( i != RETRY )
 		logPrint(trcLogId, FL, "mysql connection success : local : %x, remote : %x\n", conn, conn_Rm);
	else
	{
		sprintf(trcBuf,">>> Retry Remote Connection All Fail.\n");
		trclib_writeLogErr (FL,trcBuf);

		sprintf(trcBuf,"stmd terminate.\n"); trclib_writeLogErr (FL,trcBuf);
		return -1; // if fail process terminate. added by uamyd 20110303
	}

	ret = getSceIP();
	
	if( i != RETRY )
	{
		ret = stmd_init_INI_table();
		if( ret < 0 )
		{
			sprintf(trcBuf, "stmd_init_INI_table() fail\n");
			trclib_writeErr (FL,trcBuf);
			return -1;
		}
	}
	else
	{
		// Entry 초기화 
		stmd_init_Local_INI_table();
	}

	initPdsn();
	initSmsc();
	
	// Rule Set 초기화 
    memset(g_stSCERule, 0x00, sizeof(RuleSetList)*MAX_SCE_NUM);
    memset(g_ruleIdBuf,0x00, sizeof(int)*MAX_SCE_NUM*MAX_RULE_NUM);
    g_ruleItemCnt = readRuleConfFile(rsFname);

    // MMC 처리 function들을 bsearch로 찾기 위해 sort한다.
    qsort ((void*)mmcHdlrVector,
        numMmcHdlr,
        sizeof(StmdMmcHdlrVector),
        stmd_mmcHdlrVector_qsortCmp);

    sprintf(trcBuf, "stmd initiation successfully end...\n");
    trclib_writeLogErr(FL, trcBuf);

    return 1;
}

int getSceIP(void)
{
	char query[1024] = {0,};
	MYSQL_RES *res;
	MYSQL_ROW row;
	int i = 0;

	snprintf(query, 1024, "select se_ip, value_key, value from INI_VALUES where value_type = 5  "
			" order by value asc");

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "select fail Local : err = %s\n", mysql_error(conn));
        trclib_writeErr (FL,trcBuf);
		return -1;
	}

	res = mysql_store_result(conn);

	while( ( row = mysql_fetch_row(res)) != NULL )
	{
		if(i > 2)
		{
			sprintf(trcBuf, "NUMBER OF SCE is Over 2... Abnormal INI_VALUES table.\n");
			trclib_writeLogErr (FL,trcBuf);
			break;
		}
		strcpy(g_stSCE[i].sce_ip, row[0]);
		g_stSCE[i].sce_id = atoi(row[2]);
		i++;

	}

	SCE_CNT = 2;
	for( i = 0; i < SCE_CNT; i++)
	{
		if(i == 0)
			strcpy(g_stSCE[i].sce_name, "SCEA");
		else
			strcpy(g_stSCE[i].sce_name, "SCEB");
	}

	mysql_free_result(res);

	return 0;
}

int stmd_init_Local_INI_table(void)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[1024] = {0,};
	int	entry = 0, i, cnt = 0;

	// ENTRY 구조체 initial 
	memset(query,0x00,sizeof(query));
	snprintf(query,1024,"SELECT distinct value_key, VALUE FROM INI_VALUES "
						" WHERE value_type=1 order by value " );

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}
	res = mysql_store_result(conn);
	cnt = 0;
	while( (row = mysql_fetch_row(res)) != NULL)
	{
		entry = atoi(row[1]);
		for(i = 0; i < SCE_CNT; i++ )
		{
				g_stSCEEntry[i].stEntry[cnt].real = 1;
                g_stSCEEntry[i].stEntry[cnt].eFlag = 0;
				g_stSCEEntry[i].stEntry[cnt].eId = entry;
				strcpy(g_stSCEEntry[i].stEntry[cnt].eName, row[0]);
				g_stSCEEntry[i].ruleEntryCnt++;
				
/*
				g_ruleEntryBuf[cnt] = entry;
				sprintf(g_ruleEntryName[cnt], "%s", row[0]);
				g_ruleEntryCnt++;
*/
		}
		cnt++;
	}
	mysql_free_result(res);

	return 0;
}


int stmd_init_INI_table(void)
{
	char query[1024] = {0,};
	MYSQL_RES *res, *res_rm;
	MYSQL_ROW row, row_rm;
	char	sce_ip[24] = {0,}, time_stamp[32] = {0,};
	int	entry = 0, i, cnt = 0;

	snprintf(query, 1024, "select count(*) from INI_VALUES ");

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "select fail Local : err = %s\n", mysql_error(conn));
        trclib_writeErr (FL,trcBuf);
	}

	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);
	if( atoi(row[0]) == 0 )
	{
		memset(query, 0x00, sizeof(query));
		snprintf(query, 1024, "select * from INI_VALUES order by se_ip, value_type, value");

		if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
        	sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn_Rm));
        	trclib_writeErr (FL,trcBuf);
			mysql_free_result(res);
			return -1;
		}

		res_rm = mysql_store_result(conn_Rm);
		while( ( row_rm = mysql_fetch_row(res_rm)) != NULL )
		{
			if( sce_ip[0] == 0 && time_stamp[0] == 0)
			{
				strcpy(time_stamp, row_rm[0]);
				strcpy(sce_ip, row_rm[1]);
			}
			memset(query, 0x00, sizeof(query));
			snprintf(query, 1024, "INSERT INTO INI_VALUES values ('%s', '%s', '%s', '%s', '%s')",\
									row_rm[0], row_rm[1], row_rm[2], row_rm[3], row_rm[4]);
			if (stmd_mysql_query (query) < 0) {
        		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
        		trclib_writeErr (FL,trcBuf);
				mysql_free_result(res_rm);
				return -2;
			}
		}
		mysql_free_result(res_rm);
	}
	mysql_free_result(res);

	// ENTRY 구조체 initial 
	memset(query,0x00,sizeof(query));
	snprintf(query,1024,"SELECT distinct value_key, VALUE FROM INI_VALUES "
						" WHERE value_type=1 order by value " );

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}
	res = mysql_store_result(conn);
	cnt = 0;
	while( (row = mysql_fetch_row(res)) != NULL)
	{
		entry = atoi(row[1]);
		for(i = 0; i < SCE_CNT; i++ )
		{
				g_stSCEEntry[i].stEntry[cnt].real = 1;
                g_stSCEEntry[i].stEntry[cnt].eFlag = 0;
				g_stSCEEntry[i].stEntry[cnt].eId = entry;
				strcpy(g_stSCEEntry[i].stEntry[cnt].eName, row[0]);
				g_stSCEEntry[i].ruleEntryCnt++;
				
/*
				g_ruleEntryBuf[cnt] = entry;
				sprintf(g_ruleEntryName[cnt], "%s", row[0]);
				g_ruleEntryCnt++;
*/
		}
		cnt++;
	}
	mysql_free_result(res);

	return 0;
}

int initSmsc(void)
{
	char query[1024] = {0,};
	MYSQL_RES *res;
	MYSQL_ROW row;
	int	 i, cnt;

	for(i = 0; i < 2; i++ )
	{
		for(cnt = 0; cnt < MAX_SMSC_NUM; cnt++)
		{
			// SMSC
			g_stSmsc[i].stItem[cnt].eFlag = 0;
			g_stSmsc[i].stItem[cnt].req = 0;
			g_stSmsc[i].stItem[cnt].succ = 0;
			g_stSmsc[i].stItem[cnt].fail = 0;
			g_stSmsc[i].stItem[cnt].smpp_err = 0;
			g_stSmsc[i].stItem[cnt].etc_err = 0;

			memset(g_stSmsc[i].stItem[cnt].ip, 0x00, sizeof(g_stSmsc[i].stItem[cnt].ip));
		}
		g_stSmsc[i].smscCnt = 0;
	}

	// SMS LIST initial
	memset(query,0x00,sizeof(query));
	sprintf(query,"select code from ip_code_tbl where type = 2 order by type, code asc ");

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}

	res = mysql_store_result(conn);
	cnt = 0;
	while( (row = mysql_fetch_row(res)) != NULL)
	{
		for(i = 0; i < 2; i++ )
		{
			// SMSC
			g_stSmsc[i].stItem[cnt].eFlag = 0;
			g_stSmsc[i].stItem[cnt].req = 0;
			g_stSmsc[i].stItem[cnt].succ = 0;
			g_stSmsc[i].stItem[cnt].fail = 0;
			g_stSmsc[i].stItem[cnt].smpp_err = 0;
			g_stSmsc[i].stItem[cnt].etc_err = 0;

			strcpy(g_stSmsc[i].stItem[cnt].ip, row[0]);
			g_stSmsc[i].smscCnt++;

		}
		cnt++;
	}
	mysql_free_result(res); // MEM MUST Leak 10.07

	
	return cnt;
}

int initPdsn(void)
{
	char query[1024] = {0,};
	MYSQL_RES *res;
	MYSQL_ROW row;
	int	 i, cnt;

	for(cnt = 0; cnt < MAX_PDSN_NUM; cnt++ )
	{
		// PDSN
		g_stPdsn.stItem[cnt].eFlag = 0;
		g_stPdsn.stItem[cnt].rx_cnt = 0;
		g_stPdsn.stItem[cnt].start = 0;
		g_stPdsn.stItem[cnt].interim = 0;
		g_stPdsn.stItem[cnt].stop = 0;
		g_stPdsn.stItem[cnt].start_logon_cnt = 0;
		g_stPdsn.stItem[cnt].int_logon_cnt = 0;
		g_stPdsn.stItem[cnt].logout_cnt = 0;

		g_stPdsn.stItem[cnt].log_req = 0;
		g_stPdsn.stItem[cnt].log_succ = 0;
		g_stPdsn.stItem[cnt].log_fail = 0;
		g_stPdsn.stItem[cnt].HBIT_0 = 0;
		g_stPdsn.stItem[cnt].HBIT_1 = 0;
		g_stPdsn.stItem[cnt].HBIT_2 = 0;
		g_stPdsn.stItem[cnt].HBIT_3 = 0;
		g_stPdsn.stItem[cnt].HBIT_etc = 0;

		g_stPdsn.stItem[cnt].sm_int_err = 0;
		g_stPdsn.stItem[cnt].op_err = 0;
		g_stPdsn.stItem[cnt].op_timeout = 0;
		g_stPdsn.stItem[cnt].etc_fail = 0;

		memset(g_stPdsn.stItem[cnt].ip, 0x00, sizeof(g_stPdsn.stItem[cnt].ip));
		memset(g_stPdsn.stItem[cnt].desc, 0x00, sizeof(g_stPdsn.stItem[cnt].desc));
	}
	g_stPdsn.pdsnCnt = 0;


	// PDSN LIST initial
	memset(query,0x00,sizeof(query));
	sprintf(query,"select code, name from ip_code_tbl where type = 1 order by type, code asc ");

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}


	res = mysql_store_result(conn);
	cnt = 0;
	while( (row = mysql_fetch_row(res)) != NULL)
	{
			// PDSN
		g_stPdsn.stItem[cnt].eFlag = 0;
		g_stPdsn.stItem[cnt].rx_cnt = 0;
		g_stPdsn.stItem[cnt].start = 0;
		g_stPdsn.stItem[cnt].interim = 0;
		g_stPdsn.stItem[cnt].stop = 0;
		g_stPdsn.stItem[cnt].start_logon_cnt = 0;
		g_stPdsn.stItem[cnt].int_logon_cnt = 0;
		g_stPdsn.stItem[cnt].logout_cnt = 0;

		g_stPdsn.stItem[cnt].log_req = 0;
		g_stPdsn.stItem[cnt].log_succ = 0;
		g_stPdsn.stItem[cnt].log_fail = 0;
		g_stPdsn.stItem[cnt].HBIT_0 = 0;
		g_stPdsn.stItem[cnt].HBIT_1 = 0;
		g_stPdsn.stItem[cnt].HBIT_2 = 0;
		g_stPdsn.stItem[cnt].HBIT_3 = 0;
		g_stPdsn.stItem[cnt].HBIT_etc = 0;

		g_stPdsn.stItem[cnt].sm_int_err = 0;
		g_stPdsn.stItem[cnt].op_err = 0;
		g_stPdsn.stItem[cnt].op_timeout = 0;
		g_stPdsn.stItem[cnt].etc_fail = 0;

		strcpy(g_stPdsn.stItem[cnt].ip, row[0]);
		strcpy(g_stPdsn.stItem[cnt].desc, row[1]);
		g_stPdsn.pdsnCnt++;
		cnt++;
	}

	for(i = 0; i < 2; i++ )
	{
		for(cnt = 0; cnt < MAX_SMSC_NUM; cnt++)
		{
			// SMSC
			g_stSmsc[i].stItem[cnt].eFlag = 0;
			g_stSmsc[i].stItem[cnt].req = 0;
			g_stSmsc[i].stItem[cnt].succ = 0;
			g_stSmsc[i].stItem[cnt].fail = 0;
			g_stSmsc[i].stItem[cnt].smpp_err = 0;
			g_stSmsc[i].stItem[cnt].etc_err = 0;

			memset(g_stSmsc[i].stItem[cnt].ip, 0x00, sizeof(g_stSmsc[i].stItem[cnt].ip));
		}
		g_stSmsc[i].smscCnt = 0;
	}
	mysql_free_result(res); // MEM MUST Leak 10.07

	// SMS LIST initial
	memset(query,0x00,sizeof(query));
	sprintf(query,"select code from ip_code_tbl where type = 2 order by type, code asc ");

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}

	res = mysql_store_result(conn);
	cnt = 0;
	while( (row = mysql_fetch_row(res)) != NULL)
	{
		for(i = 0; i < 2; i++ )
		{
			// SMSC
			g_stSmsc[i].stItem[cnt].eFlag = 0;
			g_stSmsc[i].stItem[cnt].req = 0;
			g_stSmsc[i].stItem[cnt].succ = 0;
			g_stSmsc[i].stItem[cnt].fail = 0;
			g_stSmsc[i].stItem[cnt].smpp_err = 0;
			g_stSmsc[i].stItem[cnt].etc_err = 0;

			strcpy(g_stSmsc[i].stItem[cnt].ip, row[0]);
			g_stSmsc[i].smscCnt++;

		}
		cnt++;
	}
	mysql_free_result(res); // MEM MUST Leak 10.07

	
	return cnt;
}

int stmd_initLog (void)
{
    char    *env, fname[256];
	char	path[256];

    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"[stmd_initLog] not found %s environment name\n", IV_HOME);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", env, STMD_TRCLOG_FILE, mySysName);
    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[stmd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", env, STMD_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[stmd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf( path, "%s/%s",env, STATD_MAIN_PATH );
	
	if( 0 != access(path, R_OK | W_OK | F_OK) )
		mkdir(path, DIRMODE);

    return 1;
}

int stmd_initStatisticReq_ProcInfo()
{
    char    *env, fname[256], getBuf[256], token[5][64];
    FILE    *fp;
    int     lNum=0;
    int     i, j, proc_cnt;
    char    procname[SYSCONF_MAX_APPL_NUM][CONFLIB_MAX_TOKEN_LEN];

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, SYSCONF_FILE);

    if ((fp = fopen(fname,"r")) == NULL) {
        sprintf(trcBuf,"%s openerr[%d] = %s\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }

    if ((lNum = conflib_seekSection (fp, "VRTS_IP")) < 0) {
        sprintf(trcBuf,"%s seek_VRTS_IP_err[%d] = %s\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
	if( (lNum = conflib_getStringInSection(fp, "IP", DSC_IP)) < 0 )
	{
        sprintf(trcBuf,"%s DSC_IP GET Fail[%d] = %s\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
	/* 07. 17 : IP_POOL 로 관리하는 것으로 변경함. 
	// IP_POOL[0] : VRTS IP 
	if( (lNum = conflib_getStringInSection(fp, "IP", IP_POOL[0])) < 0 )
	{
        sprintf(trcBuf,"%s VRTS IP GET Fail[%d] = %s\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
	*/


    if ((lNum = conflib_seekSection (fp, "ASSOCIATE_SYSTEMS")) < 0) {
        sprintf(trcBuf,"%s seek_ASSOCIATE_SYSTEMS_err[%d] = %s\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }

    while ( (fgets (getBuf, sizeof(getBuf), fp) != NULL) &&
        (sysCnt < SYSCONF_MAX_ASSO_SYS_NUM) ) {
        lNum++;
        if (getBuf[0] == '[') /* end of section */
            break;
        if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
            continue;

		/* 07.17 ASSOCIATE SYSTEM에서 MP의 Secondary IP을 추가로 셋팅한다. */ 
		sscanf (getBuf, "%s%s%s%s%s%s", token[0], token[1], token[2], token[3], token[4], token[5]);
		/* VRTS VIRTUAL IP Setting */
		strcpy (StatisticSystemInfo[sysCnt].sysType, token[2]);
		strcpy (StatisticSystemInfo[sysCnt].sysGroup, token[3]);
		strcpy (StatisticSystemInfo[sysCnt].sysName, token[0]);
		/*
		if( !strcmp(token[0], "SCMA") )
			strcpy (SCMA_IP, token[4]);
		else if( !strcmp(token[0], "SCMB") )
			strcpy (SCMB_IP, token[4]);
			*/
		if( !strcmp(token[0], "SCMA") )
		{
			// IP_POOL[1] : SCMA Primary IP, IP_POOL[2] : SCMA Secondary IP
//			strcpy (IP_POOL[1], token[4]);
//			strcpy (IP_POOL[2], token[5]);
			strcpy (SCMA_PRI_IP, token[4]);
			strcpy (SCMA_SCD_IP, token[5]);
		}
		else if( !strcmp(token[0], "SCMB") )
		{
			// IP_POOL[3] : SCMB Primary IP, IP_POOL[4] : SCMB Secondary IP
//			strcpy (IP_POOL[3], token[4]);
//			strcpy (IP_POOL[4], token[5]);
			strcpy (SCMB_PRI_IP, token[4]);
			strcpy (SCMB_SCD_IP, token[5]);
		}
		sysCnt++;

    }
    fclose(fp);

    for(i = 0 ; i < sysCnt; i++) {
        proc_cnt = conflib_getTokenCntInFileSection(fname, "STMD_CONFIG", StatisticSystemInfo[i].sysType);
        conflib_getNTokenInFileSection(fname, "STMD_CONFIG", StatisticSystemInfo[i].sysType , proc_cnt, procname);
        for(j = 0 ; j < proc_cnt; j++) {
            strcpy(txStaticsProcInfo[txStaticCnt].sysName, StatisticSystemInfo[i].sysName);
            strcpy(txStaticsProcInfo[txStaticCnt].prcName, procname[j]);
            txStaticCnt++;
        }
    }

    for(i = 0 ; i < txStaticCnt ; i++) {
        sprintf(trcBuf, " %d: sysName[%s], prcName[%s]\n",
            i, txStaticsProcInfo[i].sysName, txStaticsProcInfo[i].prcName);
        trclib_writeLog(FL, trcBuf);
    }

	return 0;
}

int readPrintTime()
{
    char    *env, fname[256], c_temp[64];
    FILE    *fp;
    int     i;

    for (i=0; i<STMD_PERIOD_TYPE_NUM; i++)
        printTIME[i]  = 0;

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, STMD_PRINT_TIME_FILE);

    if ((fp = fopen(fname,"r")) == NULL) {
        writePrintTime();
        return -1;
    }
    fclose(fp);

    if (conflib_getNthTokenInFileSection(fname, "PRINT_TIME", STMD_STR_HOUR, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_HOUR);
        trclib_writeErr(FL, trcBuf);
    }
    printTIME[STMD_HOUR] = atoi(c_temp);

    if (conflib_getNthTokenInFileSection(fname, "PRINT_TIME", STMD_STR_DAY, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_DAY);
        trclib_writeErr(FL, trcBuf);
    }
    printTIME[STMD_DAY] = atoi(c_temp);

    if (conflib_getNthTokenInFileSection(fname, "PRINT_TIME", STMD_STR_WEEK, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_WEEK);
        trclib_writeErr(FL, trcBuf);
    }
    printTIME[STMD_WEEK] = atoi(c_temp);

    if (conflib_getNthTokenInFileSection(fname, "PRINT_TIME", STMD_STR_MONTH, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_MONTH);
        trclib_writeErr(FL, trcBuf);
    }
    printTIME[STMD_MONTH] = atoi(c_temp);

    return 1;
}

int readDelTime() // table별 보관 주기를 설정. 
{
    char    *env, fname[256], c_temp[64];
    FILE    *fp;
    int     i;

    for (i=0; i<STMD_PERIOD_TYPE_NUM; i++)
        delTIME[i]  = 0;

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, STMD_DEL_TIME_FILE);

    if ((fp = fopen(fname,"r")) == NULL) {
        writeDelTime();
        return -1;
    }
    fclose(fp);

    if (conflib_getNthTokenInFileSection(fname, "DELETE_DURATION", STMD_STR_MIN, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_MIN);
        trclib_writeErr(FL, trcBuf);
    }
    delTIME[STMD_MIN] = atoi(c_temp);


    if (conflib_getNthTokenInFileSection(fname, "DELETE_DURATION", STMD_STR_HOUR, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_HOUR);
        trclib_writeErr(FL, trcBuf);
    }
    delTIME[STMD_HOUR] = atoi(c_temp);

    if (conflib_getNthTokenInFileSection(fname, "DELETE_DURATION", STMD_STR_DAY, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_DAY);
        trclib_writeErr(FL, trcBuf);
    }
    delTIME[STMD_DAY] = atoi(c_temp);

    if (conflib_getNthTokenInFileSection(fname, "DELETE_DURATION", STMD_STR_WEEK, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_WEEK);
        trclib_writeErr(FL, trcBuf);
    }
    delTIME[STMD_WEEK] = atoi(c_temp);

    if (conflib_getNthTokenInFileSection(fname, "DELETE_DURATION", STMD_STR_MONTH, 1, c_temp) < 0) {
        sprintf(trcBuf, "get %s error\n", STMD_STR_MONTH);
        trclib_writeErr(FL, trcBuf);
    }
    delTIME[STMD_MONTH] = atoi(c_temp);

    return 1;
}

int writeDelTime(void)
{
    char    *env, fname[256];
    FILE    *fp;
    char    printBuf[256], printTmp[64];

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, STMD_DEL_TIME_FILE);

    if ((fp = fopen(fname,"w")) == NULL) {
        sprintf(trcBuf, " %s open error = %d[%s]\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf(printBuf, "\n[DELETE_DURATION]\n");
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_MIN, delTIME[STMD_MIN]);
    strcat(printBuf, printTmp);
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_HOUR, delTIME[STMD_HOUR]);
    strcat(printBuf, printTmp);
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_DAY, delTIME[STMD_DAY]);
    strcat(printBuf, printTmp);
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_WEEK, delTIME[STMD_WEEK]);
    strcat(printBuf, printTmp);
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_MONTH, delTIME[STMD_MONTH]);
    strcat(printBuf, printTmp);

    fprintf(fp, "%s", printBuf);
    fflush(fp);
    fclose(fp);

	return 0;
}


int writePrintTime()
{
    char    *env, fname[256];
    FILE    *fp;
    char    printBuf[256], printTmp[64];

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, STMD_PRINT_TIME_FILE);

    if ((fp = fopen(fname,"w")) == NULL) {
        sprintf(trcBuf, " %s open error = %d[%s]\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf(printBuf, "\n[PRINT_TIME]\n");
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_HOUR, printTIME[STMD_HOUR]);
    strcat(printBuf, printTmp);
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_DAY, printTIME[STMD_DAY]);
    strcat(printBuf, printTmp);
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_WEEK, printTIME[STMD_WEEK]);
    strcat(printBuf, printTmp);
    sprintf(printTmp, "%-7s = %d\n", STMD_STR_MONTH, printTIME[STMD_MONTH]);
    strcat(printBuf, printTmp);

    fprintf(fp, "%s", printBuf);
    fflush(fp);
    fclose(fp);

	return 0;
}

int readPrintMaskValue()
{
    char    *env, fname[256], getBuf[256], token[6][CONFLIB_MAX_TOKEN_LEN];
    FILE    *fp;
    int     i, j, lNum, ret_cnt;

    for (i=0 ; i<STMD_MASK_ITEM_NUM; i++) {
        for (j=0; j<STMD_PERIOD_TYPE_NUM; j++)
            maskITEM[i][j]  = UNMASK;
    }

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, STMD_PRINT_MASK_FILE);

    if ((fp = fopen(fname,"r")) == NULL) {
        writePrintMaskValue();
        return -1;
    }

    if ((lNum = conflib_seekSection(fp,"[PRINT_MASK]")) < 0) {
        sprintf(trcBuf, "PRINT MASK seek error in %s\n", fname);
        trclib_writeErr(FL, trcBuf);
    }

    while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) ) {
        lNum++;
        if (getBuf[0] == '[') /* end of section */
            break;
        if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
            continue;

        if ((ret_cnt = sscanf(getBuf,"%s%s%s%s%s%s",token[0],token[1],token[2],token[3],token[4],token[5])) < 6) {
            sprintf(trcBuf, "syntax error; file=%s, lNum=%d, ret_cnt = %d\n", fname, lNum, ret_cnt);
            trclib_writeErr (FL,trcBuf);
            fclose(fp);
            return -1;
        }
        if ( !strcasecmp(token[0], STMD_STR_FAULT)) {
            maskITEM[STMD_FAULT][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_FAULT][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_FAULT][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_FAULT][STMD_MONTH] = atoi(token[5]);
        } 
		else if ( !strcasecmp(token[0], STMD_STR_LOAD)) {
            maskITEM[STMD_LOAD][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_LOAD][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_LOAD][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_LOAD][STMD_MONTH] = atoi(token[5]);
        }
		else if ( !strcasecmp(token[0], STMD_STR_LINK)) {
            maskITEM[STMD_LINK][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_LINK][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_LINK][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_LINK][STMD_MONTH] = atoi(token[5]);
        }
		else if ( !strcasecmp(token[0], STMD_STR_LEG)) {
            maskITEM[STMD_LEG][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_LEG][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_LEG][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_LEG][STMD_MONTH] = atoi(token[5]);
        }
		else if ( !strcasecmp(token[0], STMD_STR_LOGON)) {
            maskITEM[STMD_LOGON][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_LOGON][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_LOGON][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_LOGON][STMD_MONTH] = atoi(token[5]);
        }
		else if ( !strcasecmp(token[0], STMD_STR_FLOW)) {
            maskITEM[STMD_FLOW][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_FLOW][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_FLOW][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_FLOW][STMD_MONTH] = atoi(token[5]);
        }
		else if ( !strcasecmp(token[0], STMD_STR_RULE_SET)) {
            maskITEM[STMD_RULE_SET][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_RULE_SET][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_RULE_SET][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_RULE_SET][STMD_MONTH] = atoi(token[5]);
        }
		else if ( !strcasecmp(token[0], STMD_STR_RULE_ENT)) {
            maskITEM[STMD_RULE_ENT][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_RULE_ENT][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_RULE_ENT][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_RULE_ENT][STMD_MONTH] = atoi(token[5]);
        }
		else if ( !strcasecmp(token[0], STMD_STR_SMS)) {
            maskITEM[STMD_SMS][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_SMS][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_SMS][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_SMS][STMD_MONTH] = atoi(token[5]);
        }
		else if ( !strcasecmp(token[0], STMD_STR_DEL)) {
            maskITEM[STMD_DEL][STMD_HOUR]  = atoi(token[2]);
            maskITEM[STMD_DEL][STMD_DAY]   = atoi(token[3]);
            maskITEM[STMD_DEL][STMD_WEEK]  = atoi(token[4]);
            maskITEM[STMD_DEL][STMD_MONTH] = atoi(token[5]);
        }
    }
    fclose(fp);
    return 0;
}

int writePrintMaskValue()
{
    char    *env, fname[256];
    FILE    *fp;
    char    maskBuf[512], maskTmp[64];

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, STMD_PRINT_MASK_FILE);

    if ((fp = fopen(fname,"w")) == NULL) {
        sprintf(trcBuf, " %s open error = %d[%s]\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf(maskBuf, "\n[PRINT_MASK]\n");
    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_FAULT, 
        maskITEM[STMD_FAULT][STMD_HOUR], maskITEM[STMD_FAULT][STMD_DAY],
        maskITEM[STMD_FAULT][STMD_WEEK], maskITEM[STMD_FAULT][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_LOAD, 
        maskITEM[STMD_LOAD][STMD_HOUR], maskITEM[STMD_LOAD][STMD_DAY],
        maskITEM[STMD_LOAD][STMD_WEEK], maskITEM[STMD_LOAD][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_LINK, 
        maskITEM[STMD_LINK][STMD_HOUR], maskITEM[STMD_LINK][STMD_DAY],
        maskITEM[STMD_LINK][STMD_WEEK], maskITEM[STMD_LINK][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_LEG, 
        maskITEM[STMD_LEG][STMD_HOUR], maskITEM[STMD_LEG][STMD_DAY],
        maskITEM[STMD_LEG][STMD_WEEK], maskITEM[STMD_LEG][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_LOGON, 
        maskITEM[STMD_LOGON][STMD_HOUR], maskITEM[STMD_LOGON][STMD_DAY],
        maskITEM[STMD_LOGON][STMD_WEEK], maskITEM[STMD_LOGON][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_FLOW, 
        maskITEM[STMD_FLOW][STMD_HOUR], maskITEM[STMD_FLOW][STMD_DAY],
        maskITEM[STMD_FLOW][STMD_WEEK], maskITEM[STMD_FLOW][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_RULE_SET, 
        maskITEM[STMD_RULE_SET][STMD_HOUR], maskITEM[STMD_RULE_SET][STMD_DAY],
        maskITEM[STMD_RULE_SET][STMD_WEEK], maskITEM[STMD_RULE_SET][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_RULE_ENT, 
        maskITEM[STMD_RULE_ENT][STMD_HOUR], maskITEM[STMD_RULE_ENT][STMD_DAY],
        maskITEM[STMD_RULE_ENT][STMD_WEEK], maskITEM[STMD_RULE_ENT][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_SMS, 
        maskITEM[STMD_SMS][STMD_HOUR], maskITEM[STMD_SMS][STMD_DAY],
        maskITEM[STMD_SMS][STMD_WEEK], maskITEM[STMD_SMS][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_DEL, 
        maskITEM[STMD_DEL][STMD_HOUR], maskITEM[STMD_DEL][STMD_DAY],
        maskITEM[STMD_DEL][STMD_WEEK], maskITEM[STMD_DEL][STMD_MONTH]);
    strcat(maskBuf, maskTmp);

#if 0
    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_DB, 
        maskITEM[STMD_DB][STMD_HOUR], maskITEM[STMD_DB][STMD_DAY],
        maskITEM[STMD_DB][STMD_WEEK], maskITEM[STMD_DB][STMD_MONTH]);
    strcat(maskBuf, maskTmp);
    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_SCIB, 
        maskITEM[STMD_SCIB][STMD_HOUR], maskITEM[STMD_SCIB][STMD_DAY],
        maskITEM[STMD_SCIB][STMD_WEEK], maskITEM[STMD_SCIB][STMD_MONTH]);
    strcat(maskBuf, maskTmp);
    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_RCIF, 
        maskITEM[STMD_RCIF][STMD_HOUR], maskITEM[STMD_RCIF][STMD_DAY],
        maskITEM[STMD_RCIF][STMD_WEEK], maskITEM[STMD_RCIF][STMD_MONTH]);
    strcat(maskBuf, maskTmp);
    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_SCPIF, 
        maskITEM[STMD_SCPIF][STMD_HOUR], maskITEM[STMD_SCPIF][STMD_DAY],
        maskITEM[STMD_SCPIF][STMD_WEEK], maskITEM[STMD_SCPIF][STMD_MONTH]);
    strcat(maskBuf, maskTmp);
    sprintf(maskTmp, "%-7s = %d  %d  %d  %d\n", STMD_STR_WISE, 
        maskITEM[STMD_WISE][STMD_HOUR], maskITEM[STMD_WISE][STMD_DAY],
        maskITEM[STMD_WISE][STMD_WEEK], maskITEM[STMD_WISE][STMD_MONTH]);
    strcat(maskBuf, maskTmp);
#endif

    fprintf(fp, "%s", maskBuf);
    fflush(fp);
    fclose(fp);

    return 0;
}

int readCronJobInFile()
{
    char    *env, fname[256], getBuf[256], token[5][CONFLIB_MAX_TOKEN_LEN];
    FILE    *fp;
    int     i, j=0, k=0, lNum, ret_cnt;

    for ( i=0 ; i < MAX_CRONJOB_NUM; i++) {
        cronJOB[i].statisticsType = NOT_REGISTERED;
        cronJOB[i].period = 0;
        cronJOB[i].sysName[0] = 0;
        cronJOB[i].measureTime[0] = 0;
    }

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, STMD_CRONJOB_FILE);

    if ((fp = fopen(fname,"r")) == NULL) {
        return -1;
    }

    if ((lNum = conflib_seekSection(fp,"[CRONJOB]")) < 0) {
        sprintf(trcBuf, "CRONJOB seek error in %s\n", fname);
        trclib_writeErr(FL, trcBuf);
    }

    while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) ) {
        lNum++;
        if (getBuf[0] == '[') /* end of section */
            break;
        if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
            continue;

        if ((ret_cnt = sscanf(getBuf,"%s%s%s%s%s",token[0],token[1],token[2],token[3],token[4])) < 5) {
            sprintf(trcBuf, "syntax error; file=%s, lNum=%d, ret_cnt = %d\n", fname, lNum, ret_cnt);
            trclib_writeErr (FL,trcBuf);
            fclose(fp);
            
            return -1;
        }
        
        for(j=0 ; j<STMD_MASK_ITEM_NUM ; j++) {
            if (!strcasecmp(strITEM[j], token[0])) {
                break;
            }
        }
        cronJOB[k].statisticsType = j;
        cronJOB[k].period = atoi(token[1]);
        strcpy(cronJOB[k].sysName, token[2]) ;
//      sprintf(cronJOB[k].measureTime, "%s %s", token[3], token[4]); 
        sprintf(cronJOB[k].measureTime, "%s", get_ondemand_time(cronJOB[k].period*60));
 		logPrint(trcLogId, FL, "cronJOB[%d].statisticsType : %d\n", k, cronJOB[k].statisticsType);
 		logPrint(trcLogId, FL, "cronJOB[%d].period : %d\n", k, cronJOB[k].period);
 		logPrint(trcLogId, FL, "cronJOB[%d].sysName : %s\n", k, cronJOB[k].sysName);
 		logPrint(trcLogId, FL, "cronJOB[%d].measureTime : %s\n", k, cronJOB[k].measureTime);
        k++;
    }
    fclose(fp);
    return 0;
}

int writeCronJobInFile()
{
    char    *env, fname[256];
    FILE    *fp;
    char    cronTmp[64];
    char    tmp[2][32];
    int     i;

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    sprintf (fname, "%s/%s", env, STMD_CRONJOB_FILE);

    if ((fp = fopen(fname,"w")) == NULL) {
        sprintf(trcBuf, " %s open error = %d[%s]\n", fname, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }

    fprintf(fp, "\n[CRONJOB]\n");

    for ( i=0 ; i < MAX_CRONJOB_NUM; i++) {
        if ( cronJOB[i].statisticsType != NOT_REGISTERED) {
            strncpy (tmp[0], (char *)&cronJOB[i].measureTime[0],10);
            tmp[0][10] = 0;
            strncpy (tmp[1], (char *)&cronJOB[i].measureTime[11],5);
            tmp[1][5] = 0;
            sprintf(cronTmp, "%-7s  %2d  %-7s  %10s  %5s\n",
                strITEM[cronJOB[i].statisticsType],
                cronJOB[i].period, 
                cronJOB[i].sysName,
                tmp[0], tmp[1]);
            fprintf(fp,"%s", cronTmp);
        }
    }

    fflush(fp);
    fclose(fp);

    return 0;
}

int getQid(char* qname)
{
    int     qid;
    char    *env, fname[256], tmp[64];
    int     key;
    
    if ((env = getenv(IV_HOME)) == NULL) 
    {
        fprintf(stderr,"[getQid] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf (fname, "%s/%s", env, SYSCONF_FILE);
    
    if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", qname, 1, tmp) < 0) 
    {
        sprintf(trcBuf, "%s message queue Get Fail = %s[%d]\n",qname, strerror(errno), errno);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    key = strtol(tmp,0,0);
    if ((qid = msgget (key, IPC_CREAT|0666)) < 0) 
    {
        sprintf(trcBuf,"[getQid] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    
    return qid;
}
