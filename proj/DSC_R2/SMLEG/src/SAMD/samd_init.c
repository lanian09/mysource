#include "samd.h"

DiskConfig	disk_conf;

extern int		ixpcQID, samdQID, trcLogId, trcErrLogId, queCNT;
extern long		oldSumOfPIDs, newSumOfPIDs;
extern char		iv_home[64], l_sysconf[256], R_sysconf[256], systemModel[5];
extern char		trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern SAMD_ProcessInfo		ProcessInfo[SYSCONF_MAX_APPL_NUM];
//extern SAMD_DaemonInfo     DaemonInfo[SYSCONF_MAX_DAEMON_NUM];
extern SAMD_AuxilaryInfo   AuxilaryInfo[SYSCONF_MAX_AUXILARY_MSGQ_NUM];

extern SFM_SysCommMsgType	*loc_sadb;
extern SFM_L3PD				*l3pd;
extern STM_LoadMPStatMsgType		system_statistic;
extern T_keepalive  *keepalive;

extern	int		appCnt, daemonCnt, auxilaryCnt;
extern OpticalLan	optLan[SFM_MAX_DEV_CNT];
extern off_t           gdMsgOldSize;

//NTP
extern unsigned char ucRmtNTPSvrStatus;
extern unsigned char ucLclNTPDmnStatus;
extern st_NTP_STS   stNTPSTS;
extern st_NTP_STS   oldNTPSTS;
extern unsigned int     pingInterval;

////extern int duiaValue;
extern char l_sysconf2[256];

char vERSION[8] = "R2.0.0"; // BEFORE: R1.2.0 (2011-03-02) -> R2.0.0 (2011-05-05)
DB_INFO_t	g_stDBInfo;

extern int	set_version(int prc_idx, char *ver);
int init_db_info(void);
void print_db_info(void);

/* 20100608 by dcham */
char volumeName[1][20];
char diskNameList1[1][20];
char diskNameList2[1][20];

char lanConfigName[20][20];

/* hjjung_20100607 */
int lan_startIndex, lan_endIndex;

void FinishProgram(void)
{
	int  process_idx;
	for( process_idx = 0; process_idx < loc_sadb->processCount; process_idx++ ){
		if( !strncasecmp( loc_sadb->loc_process_sts[process_idx].processName, "SAMD", 4 ) ){
			loc_sadb->loc_process_sts[process_idx].status = SFM_STATUS_DEAD;
		}
	}

	report_sadb2FIMD();

	sprintf(trcBuf, "PROGRAM IS NORMALLY TERMINATED, when=%ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
	exit(0);
}


void UserControlledSignal(int sign)
{
	g_dStopFlag = 1;
	sprintf(trcBuf, "RECEIVED TERMINATED SIGNAL, sign=%d, when=%ld\n", sign, time(0));
	trclib_writeLogErr(FL, trcBuf);
	signal(sign, UserControlledSignal);
}

void IgnoreSignal(int sign) 
{   
    if (sign != SIGALRM) {
		sprintf(trcBuf, "UNWANTED SIGNAL IS RECEIVED, signal=%d, when=%ld\n", sign, time(0));
		trclib_writeLogErr(FL, trcBuf);
	}
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
	signal(SIGCHLD, handleChildProcess);
	signal(SIGCLD,  handleChildProcess);
} 

void handleChildProcess(int sign)
{
//	int status;
   
#if 0
	sprintf(trcBuf1, "SIGCHLD SIGNAL IS RECEIVED, signal=%d\n", sign);
	trclib_writeLogErr(FL, trcBuf1);
#endif
	signal(SIGCHLD, (void *)handleChildProcess);
	signal(SIGCLD,  (void *)handleChildProcess);
    /* 좀비 때문에 **/
    //while (wait3 (&status, WNOHANG, (struct rusage *)0) > 0);
}

int InitSys(void)
{
	char	*env, diskNameList[16][CONFLIB_MAX_TOKEN_LEN], tmp[32];
	int		i, key, ret, cnt, shmId;
	struct stat	msgstat;
#if 0
	int			notMaskSig[8];
	char 	tmp2[32];
#endif

	if( (env = getenv(MY_SYS_NAME)) == NULL)
	{
		fprintf(stderr,"[%s:%s:%d] not found %s environment name\n", __FILE__, __FUNCTION__, __LINE__, MY_SYS_NAME);
		return -1;
	}
	strcpy(mySysName, env);
	strcpy(myAppName, "SAMD");

#if 0
	notMaskSig[0]	= SIGCHLD;
	notMaskSig[1]	= 0;
	commlib_setupSignals(notMaskSig);
#endif
	//commlib_setupSignals(NULL);
	SetUpSignal();

	// ping_test할때 wait로 child 프로세스의 종료 조건을 확인해야하는데,
	//	commlib_setupSignals에서 SIGCHLD를 catch하도록 되어 있는 것을 release한다.
//	sigrelse(SIGCHLD);
	UnBlockSignal(SIGCHLD);

	if( (env = getenv(IV_HOME)) == NULL)
	{
		fprintf(stderr, "[%s:%s:%d] not found %s environment name\n", __FILE__, __FUNCTION__, __LINE__, IV_HOME);
		return -1;
	}
	strcpy(iv_home, env);

	if(samd_initLog() < 0)
		return -1;

	sprintf(l_sysconf, "%s/%s", iv_home, SYSCONF_FILE);

	if( (ret = keepalivelib_init("SAMD")) < 0)
		return -1;

	memset((char*)keepalive, 0x00, sizeof(T_keepalive));

	/* IXPC MSG Qid를 구한다 */
	if(conflib_getNthTokenInFileSection(l_sysconf, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if( (ixpcQID = msgget(key, IPC_CREAT | 0666)) < 0)
	{
		fprintf(stderr, "[%s:%s:%d] msgget fail; key=%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}

	/* SAMD MSG Qid를 구한다 */
	if(conflib_getNthTokenInFileSection(l_sysconf, "APPLICATIONS", "SAMD", 1, tmp) < 0)
		return -1;
	key = strtol(tmp, 0, 0);
	if( (samdQID = msgget(key, IPC_CREAT|0666)) < 0)
	{
		fprintf(stderr, "[%s:%s:%d] msgget fail; key=%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}

	/* watch-dog 메시지를 구성할 때 사용할 SYSTEM_LABEL 이름 구한다 */
	if (conflib_getNthTokenInFileSection(l_sysconf, "GENERAL", "SYSTEM_LABEL", 1, sysLabel) < 0)
		return -1;

	if (conflib_getNthTokenInFileSection(l_sysconf, "GENERAL", "SYSTEM_MODEL", 1, systemModel) < 0)
		return -1;

	// by helca 07.31
	memset(&system_statistic, 0x00, sizeof(STM_LoadMPStatMsgType)); /* system_statistic를 초기화 한다 */

	if(conflib_getNthTokenInFileSection(l_sysconf, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
		return -1;

	key = strtol(tmp, 0, 0);

	if( (shmId = (int)shmget(key, sizeof(SFM_SysCommMsgType), 0666 | IPC_CREAT)) < 0)
	{
		if(errno != ENOENT)
		{
			fprintf(stderr, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -1;
		}
	}

	if( (loc_sadb = (SFM_SysCommMsgType*)shmat(shmId, 0, 0)) == (SFM_SysCommMsgType*)-1)
	{
		fprintf(stderr, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}

	if(set_version(SEQ_PROC_SAMD, vERSION) < 0)
	{
		fprintf(stderr, "[%s:%s:%d] set_version failed\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
/////////////////////////////////////////////////////////////
/////Probing Device INIT
	if(conflib_getNthTokenInFileSection(l_sysconf, "SHARED_MEMORY_KEY", "SHM_L3PD", 1, tmp) < 0)
		return -1;
	key = strtol(tmp, 0, 0);

	if( (shmId = (int)shmget(key, SFM_L3PD_SIZE, 0666 | IPC_CREAT)) < 0)
	{
		fprintf(stderr, "[%s:%s:%d] SNMP PD shmget fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}

	if( (l3pd = (SFM_L3PD*)shmat(shmId, 0, 0)) == (SFM_L3PD*)-1)
	{
		fprintf(stderr, "[%s:%s:%d] SFDB shmat fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -1;
	}

	/* Because many processes reference this memory,
	 * it is not good to initialize it. */
#if 0
	memset (loc_sadb, 0, sizeof(SFM_SysCommMsgType)); /* loc_sadb를 초기화 한다 */
#endif

	/*	ping test해야하는 다른 시스템들의 이름과 ipAddress를 저장하는 table을 만든다.	*/
	if(samd_initPingAddrTbl(l_sysconf) < 0)
	{
		fprintf(stderr, "[%s:%s:%d] samd_initPingAddrTbl\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if(samd_opticalLanInfo(l_sysconf) < 0)
	{
		fprintf(stderr, "[%s:%s:%d] samd_opticalLanInfo\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	/*	loc_sadb->loc_process_sts와 ProcessInfo에 관리대상 프로세스의 정보를 저장한다.	*/
	if( (cnt = samd_initProcessTbl(l_sysconf)) < 0)
	{
		fprintf(stderr, "[%s:%s:%d] samd_initProcessTbl\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	/*	프로세스 상태관리를 위한 기동 시 초기 상태정보를 수집한다.							*/
	newSumOfPIDs = oldSumOfPIDs = getSumOfPIDs();
	if(newSumOfPIDs < 0)
	{
		sprintf(trcBuf, "[%s:%s:%d] newSumOfPIDs error\n", __FILE__, __FUNCTION__, __LINE__);
		trclib_writeLogErr(FL, trcBuf);
	}

	if( (ret = getProcessStatus()) < 0)
	{
		sprintf(trcBuf, "[%s:%s:%d] process status is wrong\n", __FILE__, __FUNCTION__, __LINE__);
		trclib_writeLogErr(FL, trcBuf);
	}

	for(i = 0; i < appCnt; i++)
		ProcessInfo[i].old_status = ProcessInfo[i].new_status;

	/*	add by helca 2008.12.01	*/
	sdmdForPrcSts_init();

	/*	disk partition 갯수와 이름을 저장한다.	*/
	cnt = conflib_getTokenCntInFileSection(l_sysconf, "SAMD_CONFIG", "DISK");
	loc_sadb->diskCount = cnt;
	conflib_getNTokenInFileSection(l_sysconf, "SAMD_CONFIG", "DISK", cnt, diskNameList);
	for(i = 0; i < cnt; i++)
		strcpy(loc_sadb->loc_disk_sts[i].diskName, diskNameList[i]);

    /* set file size of "/var/log/messages"	*/
    if(stat(MESSAGE_FILE, &msgstat) < 0)
	{
		fprintf(stderr, "[%s:%s:%d] %s stat error\n", __FILE__, __FUNCTION__, __LINE__, MESSAGE_FILE);
		return -1;
    }

	gdMsgOldSize		= msgstat.st_size;

	/*	NTP's status value initial	*/
	ucLclNTPDmnStatus	= NTP_INITIAL;
	ucRmtNTPSvrStatus	= NTP_INITIAL;
	memset(&stNTPSTS, 0x00, sizeof(st_NTP_STS));
	memset(&oldNTPSTS, 0x00, sizeof(st_NTP_STS));

	/*	ping interval	*/
	if(conflib_getNthTokenInFileSection(l_sysconf, "NETWORK_PROBE", "PING_INTERVAL", 1, tmp) < 0)
		pingInterval = 3;		/*	ping interval default	*/
	else
		pingInterval = strtol(tmp, 0, 0);

#if 0
	/*	DUIA Service Type	*/
	loc_sadb->duia.svcType = 0;
	sprintf(l_sysconf2, "%s/%s", iv_home, DUIA_FILE);

	if(conflib_getNthTokenInFileSection(l_sysconf2, "SERVICE_TYPE", "TYPE", 1, tmp2) < 0)
		return -1;
	duiaValue = strtol(tmp2, 0, 0);
#endif
	loc_sadb->duia.svcType	= 0; // duia는 DSC에서 사용되지 않으나 
								 // shared memory 구조에서 빼지못해 0으로 초기화 함.

	/*	disk partition 정보를 초기화한다.	*/
	init_disk_info();

	// 20100608 by dcham 
	/*disk, volume의 이름을 저장한다.*/
	/*
	conflib_getNTokenInFileSection(l_sysconf, "DISK_INFO", "VOLUME", 1, volumeName);
	conflib_getNTokenInFileSection(l_sysconf, "DISK_INFO", "DISK1", 1, diskNameList1);
	conflib_getNTokenInFileSection(l_sysconf, "DISK_INFO", "DISK2", 1, diskNameList2);
	*/
	
	if(conflib_getNthTokenInFileSection(l_sysconf, "DISK_INFO", "VOLUME", 1, (char *)&volumeName[0]) < 0) {
		fprintf(stderr, "[%s:%s:%d] DISK_INFO VOLUME error\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if(conflib_getNthTokenInFileSection(l_sysconf, "DISK_INFO", "DISK1", 1, (char *)&diskNameList1[0]) < 0) {
		fprintf(stderr, "[%s:%s:%d] DISK_INFO DISK1 error\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if(conflib_getNthTokenInFileSection(l_sysconf, "DISK_INFO", "DISK2", 1, (char *)&diskNameList2[0]) < 0) {
		fprintf(stderr, "[%s:%s:%d] DISK_INFO DISK2 error\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	/* hjjung_20100607 lan의 startKey와 endKey를 얻어온다. */
    char lan_startKey[16], lan_endKey[16];
//	extern int lan_startIndex, lan_endIndex;

	if(conflib_getNthTokenInFileSection(l_sysconf, "LANCARD_INFO", "STARTKEY", 1, (char *)lan_startKey) < 0) {
		fprintf(stderr, "[%s:%s:%d] lan_startKey error\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
    if(conflib_getNthTokenInFileSection(l_sysconf, "LANCARD_INFO", "ENDKEY", 1, (char *)lan_endKey) < 0) {
		fprintf(stderr, "[%s:%s:%d] lan_endKey error\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
       sprintf(trcBuf, "[lan_startKey] =%s, [lan_endKey] =%s \n", lan_startKey, lan_endKey);
       trclib_writeLogErr(FL, trcBuf);

        

    /**< HW Link Info **/
    /**  HW link ip address´A  hw_loc_lan[]¿¡ AuAa.
      **/
	char loc_hw [16], loc_eth[16];
	int k = 0;
	// lan_startIndex, lan_endIndex SETTING
    for (i = 0; i< SFM_MAX_HPUX_HW_COM; i++) {
        sprintf (loc_hw, "HARDWARE%d", i);

		/* hjjung_20100607 */
		if(strstr(lan_startKey,loc_hw)){
            lan_startIndex = i;
			sprintf(trcBuf, "[lan_startIndex] =%d \n", lan_startIndex);
			trclib_writeLogErr(FL, trcBuf);
		}
		if(strstr(lan_endKey,loc_hw)){
            lan_endIndex = i;
			sprintf(trcBuf, "[lan_EndIndex] =%d \n", lan_endIndex);
			trclib_writeLogErr(FL, trcBuf);
			break;
		}
	}
	
	// HARDWARE INFO SETTING & LINK INFO SETTING
    for (i = 0; i< SFM_MAX_HPUX_HW_COM; i++) {
        sprintf (loc_hw, "HARDWARE%d", i);
        if (conflib_getNthTokenInFileSection(l_sysconf, "HW_LOCAL_INFO", loc_hw, 1, (char *)loc_eth) < 0) {
			sprintf(trcBuf, "[samd_init] HARDWARE COUNT=%d \n", i-1);
			trclib_writeLogErr(FL, trcBuf);
            break;
        }
        sprintf (loc_sadb->sysHW[i].StsName, "%s", loc_eth);
       	if (i <= lan_endIndex) { 
			sprintf(loc_sadb->sysSts.linkSts[i].StsName, "%s", loc_eth);
			strncpy(lanConfigName[k], loc_eth, 8); //20100910 by dcham
			sprintf(trcBuf, "[samd_init] lanConfigName[%d]=%s, loc_eth=%s \n",k, lanConfigName[k], loc_eth);
			trclib_writeLogErr(FL, trcBuf);
			k++;
		}

        if (conflib_getNthTokenInFileSection(l_sysconf, "HW_LOCAL_INFO", loc_hw, 2, (char *)loc_eth) < 0) {
            continue;
        }

		if (loc_eth[0] == '0')
        	loc_sadb->sysHW[i].stsType = 0;
		else if (loc_eth[0] == '1')
        	loc_sadb->sysHW[i].stsType = 1;
		else  {
			fprintf (stderr, "unknown hw type, %c\n", loc_eth[0]);
			return -9;
		}

        if (conflib_getNthTokenInFileSection(l_sysconf, "HW_LOCAL_INFO", loc_hw, 3, (char *)loc_eth) < 0) {
            continue;
        }

        if (!strcmp(loc_eth, "0.0.0.0")) {
            loc_sadb->sysHW[i].status = SFM_LAN_NOT_EQUIP;
        }
        sprintf (loc_sadb->sysHW[i].StsInfo, "%s", loc_eth);

        logPrint(trcLogId, FL, "LOC LAN IP %d. NAME:%s.TYPE:%d, STS:%d, INFO:%s\n", i,
                loc_sadb->sysHW[i].StsName, 
                loc_sadb->sysHW[i].stsType, 
				loc_sadb->sysHW[i].status, 
				loc_sadb->sysHW[i].StsInfo);
    }

	logPrint(trcLogId, FL, "%s startup...\n", myAppName);
	logPrint(trcErrLogId, FL, "%s startup...\n", myAppName);

	// 07.22 jjinri	init_db_info();
	// print_db_info();

	return 1;
}

void print_db_info(void)
{
	int j = 0;

	printf("SYS:%s\n",g_stDBInfo.sysName);
	printf("IP:%s\n",g_stDBInfo.dbIp);
	printf("ID:%s\n",g_stDBInfo.dbId);
	printf("PASS:%s\n",g_stDBInfo.dbPass);
	printf("DB:%s\n",g_stDBInfo.dbName);
	printf("BakupFile:%s\n",g_stDBInfo.backFile);
	printf("table count : %d\n", g_stDBInfo.tblCnt);

	for(j = 0; j < g_stDBInfo.tblCnt; j++ )
	{
		printf("TBL:%s\n", g_stDBInfo.tblName[j]);
	}
}

int init_db_info(void)
{
	char	fname[256] = {0,};
	FILE	*fp = NULL;
	char	buf[512] = {0,};
	int		tCnt = 0;
	char	sSys[32] = {0,}, sDb[32] = {0,}, sTbl1[32] = {0,}, sTbl2[32] = {0,}, sBkFile[128] = {0,};
	char	sIp[20] = {0,}, sId[8] = {0,}, sPass[8] = {0,};

	sprintf(fname, "%s/%s", iv_home, SAMD_CONF_FILE);

	if( (fp = fopen(fname, "r")) != NULL )
	{
		while( fgets(buf, sizeof(buf), fp) )
		{
			if( buf[0] == '#' )
				continue;
			else if(!strncmp(buf, mySysName, 4))
			{
				sscanf(buf, "%s %s %s %s %s %s %s %s", sSys, sIp, sId, sPass, sDb, sBkFile, sTbl1, sTbl2);
				strcpy(g_stDBInfo.sysName, sSys);
				strcpy(g_stDBInfo.dbIp, sIp);
				strcpy(g_stDBInfo.dbId, sId);
				strcpy(g_stDBInfo.dbPass, sPass);
				strcpy(g_stDBInfo.dbName, sDb);
				strcpy(g_stDBInfo.backFile, sBkFile);
				if( sTbl1 != NULL )
				{
					strcpy(g_stDBInfo.tblName[tCnt], sTbl1);
					tCnt++;
				}
				if( sTbl2 != NULL )
				{
					strcpy(g_stDBInfo.tblName[tCnt], sTbl2);
					tCnt++;
				}

				g_stDBInfo.tblCnt = tCnt;
				break;
			}
			tCnt = 0;
		}
	}
	else
	{
		sprintf(trcBuf, "[init_db_info()] file open fail=%s\n", fname);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	fclose(fp);

	return 0;
}


int samd_initLog(void)
{
	char	fname[256];

	sprintf(fname, "%s/%s.%s", iv_home, SAMD_LOG_FILE, mySysName);

	if( (trcLogId = loglib_openLog(fname, LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE | LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0)
	{
		fprintf(stderr,"[samd_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	sprintf(fname,"%s/%s.%s", iv_home, SAMD_ERRLOG_FILE, mySysName);
	if( (trcErrLogId = loglib_openLog(fname, LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE | LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0)
	{
		fprintf(stderr,"[samd_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	return 1;
} /** End of samd_initLog **/


int samd_initProcessTbl(char *fname)
{
	char	getBuf[256], token[9][64];
	int		i, lNum, queCnt;
//	int		retTok;
	FILE	*fp;

	queCnt	= 0;

	memset(&ProcessInfo, 0x00, sizeof(ProcessInfo));

	if( (fp = fopen(fname,"r")) == NULL)
	{
		fprintf(stderr,"[samd_initProcessTbl] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/*	[APPLICATIONS] section으로 이동			*/
	if( (lNum = conflib_seekSection(fp, "APPLICATIONS")) < 0)
		return -1;

	/*	등록된 application들의 정보를 저장한다.		*/
	while( (fgets(getBuf, sizeof(getBuf), fp) != NULL) && (appCnt < SYSCONF_MAX_APPL_NUM))
	{
		lNum++;
		if(getBuf[0] == '[')						/*	end of section			*/
			break;

		if( (getBuf[0]=='#') || (getBuf[0]=='\n'))	/*	comment line or empty	*/
			continue;

		/*	config 파일에 "name = Qkey 실행파일위치level" format으로 들어있다.		*/
		if(sscanf(getBuf, "%s%s%s%s%s", token[0], token[1], token[2], token[3], token[4]) < 5)
		{
			sprintf(trcBuf, "[samd_initProcessTbl] syntax error; file=%s, lNum=%d\n", fname, lNum);
			trclib_writeLogErr(FL, trcBuf);
			return -1;
		}

		/*	name, 소문자로 바꿔서 넣는다.	*/
		for(i = 0; i < strlen(token[0]); i++)
			token[0][i] = tolower(token[0][i]);
		strcpy(loc_sadb->loc_process_sts[appCnt].processName, token[0]);

		/*	죽었을때 OMP-FIMD에서 설정할 장애 등급	*/
		loc_sadb->loc_process_sts[appCnt].level		= strtol(token[4], 0, 0);

		/* 상태관리에 필요한 초기값 설정 */
		loc_sadb->loc_process_sts[appCnt].status	= SFM_STATUS_DEAD;
		ProcessInfo[appCnt].new_status				= SFM_STATUS_DEAD;
	#if 0
		ProcessInfo[appCnt].msgQkey[0]				= strtol(token[2], 0, 0);	//	APPLICATIONS에서 읽는 것은 무조건 처음으로 구성한다.
	#endif
		ProcessInfo[appCnt].msgQkey					= strtol(token[2], 0, 0);	//	by helca
		strcpy(ProcessInfo[appCnt].procName, loc_sadb->loc_process_sts[appCnt].processName);
		if(!strcasecmp(ProcessInfo[appCnt].procName, "CM") || !strcasecmp(ProcessInfo[appCnt].procName, "SMSERVER"))
			sprintf(ProcessInfo[appCnt].exeFile, "%s", token[3]);
		else
			sprintf(ProcessInfo[appCnt].exeFile, "%s/%s", iv_home, token[3]);

		/*	QUEUE 초기 정보 수집	*/
		if(ProcessInfo[appCnt].msgQkey != 0)
		{
			loc_sadb->loc_que_sts[queCnt].qKEY		= (int)ProcessInfo[appCnt].msgQkey;
			strcpy(loc_sadb->loc_que_sts[queCnt].qNAME, loc_sadb->loc_process_sts[appCnt].processName);
			loc_sadb->loc_que_sts[queCnt].qNUM		= 0;
			loc_sadb->loc_que_sts[queCnt].cBYTES	= 0;
			loc_sadb->loc_que_sts[queCnt].qBYTES	= 0;
			queCnt++;
		}
		appCnt++;
	}
	fclose(fp);
#if 0
	loc_sadb->processCount = appCnt + daemonCnt; /* 관리하는 process 갯수 */
#endif
	loc_sadb->processCount	= appCnt;		/*	관리하는 process 갯수		*/
	loc_sadb->queCount		= queCnt;		/*	관리하는 queue 갯수		*/
	queCNT					= queCnt;		/*	등록된 놈					*/

	return (loc_sadb->processCount);
} /** End of samd_initProcessTbl **/


int samd_initPingAddrTbl (char *fname)
{
	char    getBuf[256],token[8][CONFLIB_MAX_TOKEN_LEN];
	int     lanCnt=0,lNum;
	int		ret;
	FILE    *fp;

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[samd_initPingAddrTbl] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* [ASSOCIATE_SYSTEMS] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"ASSOCIATE_SYSTEMS")) < 0) {
		fclose(fp);
		return -1;
	}

    /* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(lanCnt < SFM_MAX_LAN_CNT) )
	{
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((ret = sscanf(getBuf,"%s%s%s%s%s%s",token[0],token[1],token[2],token[3],token[4],token[5])) < 6) {
			fprintf(stderr,"[samd_initPingAddrTbl] syntax error; file=%s, lNum=%d, ret = %d\n", fname, lNum, ret);
			fclose(fp);
			return -1;
		}
		if (!strcasecmp(token[0], mySysName)) /* 자기 시스템은 제외하기 위해서 */
			continue;

		/* primary를 위해 */
		strcpy(loc_sadb->loc_lan_sts[lanCnt].target_SYSName, token[0]);
//		strcpy(loc_sadb->loc_lan_sts[lanCnt].target_IPaddress, token[4]);
		loc_sadb->loc_lan_sts[lanCnt].target_IPaddress = inet_addr(token[4]); // 06.11.20 samuel
		loc_sadb->loc_lan_sts[lanCnt].status = SFM_LAN_CONNECTED;

		lanCnt++;

		if (strcasecmp(token[5], "NULL")) { // secondary를 위해
			strcpy(loc_sadb->loc_lan_sts[lanCnt].target_SYSName, token[0]);
//			strcpy(loc_sadb->loc_lan_sts[lanCnt].target_IPaddress, token[5]);
			loc_sadb->loc_lan_sts[lanCnt].target_IPaddress = inet_addr(token[5]); // 06.11.20 samuel
			loc_sadb->loc_lan_sts[lanCnt].status = SFM_LAN_CONNECTED;
			lanCnt++;
		}
	}
	fclose(fp);

	//

	loc_sadb->lanCount = lanCnt; /* 관리하는 lan connection 갯수 */
	//fprintf(stderr, "loc_sadb->lanCount: %d\n", loc_sadb->lanCount);
	return (lanCnt);

} /** End of samd_initPingAddrTbl **/


int init_disk_info(void)
{
#ifndef __LINUX__
	int		i, cnt;
	char	diskNameList[16][CONFLIB_MAX_TOKEN_LEN];

	cnt	= 0;

	/*	disk partition 갯수와 이름을 저장한다.	*/
	cnt = conflib_getTokenCntInFileSection(l_sysconf, "SAMD_CONFIG", "DISK");
	loc_sadb->diskCount = cnt;
	if(loc_sadb->diskCount > 16)
	{
		sprintf(trcBuf, "[init_disk_info] MAXIMUM DISK Count(%d) is over in configuration file(%s)\n", cnt, l_sysconf);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	conflib_getNTokenInFileSection(l_sysconf, "SAMD_CONFIG", "DISK", cnt, diskNameList);
	for(i = 0; i < cnt; i++)
		strcpy(loc_sadb->loc_disk_sts[i].diskName, diskNameList[i]);
#else
	int				ret, idx;
	FILE			*fp;
	struct mnttab	mnt_tab;

	idx = 0;

	if( (fp = fopen("/etc/mnttab", "r")) == NULL)
	{
		sprintf(trcBuf, "[init_disk_info] can't open file [/etc/mnttab] :%s\n", strerror (errno));
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	while( (ret = getmntent(fp, &mnt_tab)) == 0)
	{
		strcpy(disk_conf.name[idx], mnt_tab.mnt_mountp);
		printf("[*] partition_%d=%s\n", idx, disk_conf.name[idx]);
		idx++;
	}
	disk_conf.cnt = idx;
	if(fp) fclose(fp);
#endif
	return 1;
}

int UnBlockSignal(int SIGNAL)
{
	sigset_t	set;

	/*	initalize	*/
	sigemptyset(&set);

	/*	add the SIGNAL to the set	*/
	sigaddset(&set, SIGNAL);

	/*	block it	*/
	if(sigprocmask(SIG_UNBLOCK, &set, NULL) < 0)
	{
		fprintf(stderr, "sigprocmask(): %s\n", strerror(errno));
		exit(-1);
	}

	/*	done	*/
	return 0;
}

int samd_opticalLanInfo(char *fname)
{
	char	getBuf[256], token[3][20], *pUnder;
	int		devCnt, lNum, ret;
	FILE	*fp;

	devCnt = 0;

	if( (fp = fopen(fname,"r")) == NULL)
	{
		fprintf(stderr, "[samd_opticalLanInfo] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/*	rmt_sysconfig의 [OPTICAL_LAN] 으로 이동	*/
	if( (lNum = conflib_seekSection(fp, "OPTICAL_LAN")) < 0)
		return -1;

	/* get optical mapping information */
	while( (fgets(getBuf,sizeof(getBuf),fp) != NULL) && (devCnt < SFM_MAX_DEV_CNT))
	{
		lNum++;
		if(getBuf[0] == '[') /* end of section */
			break;
		if( (getBuf[0]=='#') || (getBuf[0]=='\n')) /* comment line or empty */
			continue;

		if( (ret = sscanf(getBuf, "%s%s%s", token[0], token[1], token[2])) < 3)
		{
			fprintf(stderr,"[samd_opticalLanInfo] syntax error; file=%s, lNum=%d, ret = %d\n", fname, lNum, ret);
			fclose(fp);
			return -1;
		}

		strcpy(optLan[devCnt].name, token[0]);			/* PD-A, PD-B */
		if( (pUnder = strchr(token[2], '_')) == NULL)	/* Port_A, Port_B */
			return -1;
		pUnder++;
		optLan[devCnt].side = *(pUnder);				/* A or B */
		strcpy(loc_sadb->loc_link_sts[devCnt].linkName, optLan[devCnt].name);
		devCnt++;
	}
	fclose(fp);

	return devCnt;
}

void samd_Duplication_init(void){
	int i=2;
	loc_sadb->loc_system_dup.myLocalDupStatus = SYS_STATE_STANDBY;
	loc_sadb->loc_system_dup.yourDupStatus = SYS_STATE_STANDBY;
	loc_sadb->loc_system_dup.heartbeatAlarm = htonl(i);
	loc_sadb->loc_system_dup.OosAlarm = htonl(i);
}

void sdmdForPrcSts_init(void)
{
	char	prcName[COMM_MAX_NAME_LEN];
	int		i,/*j,*/k,ret,rst,uniq;

	//not used : sjjeon
	//memset(loc_sadb->sdmd_Prc_sts, 0x00, (sizeof(SFM_SysCommSdmdPrcSts)*SFM_MAX_PROC_CNT));

	for(k = 0, i = 0; k < loc_sadb->processCount && i < loc_sadb->processCount; i++)
	{
		if(!strcasecmp(loc_sadb->loc_process_sts[i].processName, "IXPC") ||
			!strcasecmp(loc_sadb->loc_process_sts[i].processName,"SAMD") ||
			!strcasecmp(loc_sadb->loc_process_sts[i].processName,"MMCR") ||
			!strcasecmp(loc_sadb->loc_process_sts[i].processName,"STMM") ||
			!strcasecmp(loc_sadb->loc_process_sts[i].processName,"SDMD") ||
			!strcasecmp(loc_sadb->loc_process_sts[i].processName,"LOGM"))
		{
			continue;
		}

		ret	= strlen(loc_sadb->loc_process_sts[i].processName);
		rst	= isdigit(loc_sadb->loc_process_sts[i].processName[ret-1]);
		if(rst)
		{
			uniq = 1;
			memset(prcName, 0x00, COMM_MAX_NAME_LEN);

			strncpy(prcName, loc_sadb->loc_process_sts[i].processName, ret-1);
/*
   not used : sjjeon
			for(j = 0; j < loc_sadb->processCount; j++)
			{
				if(!strcasecmp(loc_sadb->sdmd_Prc_sts[j].processName, prcName))
					uniq = 0;
			}

			if(uniq)
			{
				strcpy(loc_sadb->sdmd_Prc_sts[k].processName, prcName);
				loc_sadb->sdmd_Prc_sts[k].status = SFM_STATUS_DEAD;
				k++;
			}
			else
				continue;
		}
		else
		{
			strcpy(loc_sadb->sdmd_Prc_sts[k].processName, loc_sadb->loc_process_sts[i].processName);
			loc_sadb->sdmd_Prc_sts[k].status = SFM_STATUS_DEAD;
			k++;
*/
		}
	}
} /* End of sdmdForPrcSts_init */
