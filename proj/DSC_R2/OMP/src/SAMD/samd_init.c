#include "samd.h"

extern int		ixpcQID, samdQID, mcdmQID, trcLogId, trcErrLogId, queCNT;
extern long		oldSumOfPIDs, newSumOfPIDs;
extern char		*iv_home, l_sysconf[256];
extern char		trcBuf[4096], trcTmp[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern char		l3pdFileName[256], sceFileName[256], szl2swFileName[256];
extern SAMD_ProcessInfo		ProcessInfo[SYSCONF_MAX_APPL_NUM];
extern SFM_SysCommMsgType	*loc_sadb;
extern STM_LoadOMPStatMsgType		system_statistic;
extern T_keepalive  *keepalive;
extern SFM_sfdb     *sfdb;
extern SFM_L3PD		*l3pd;
SFM_SCE      *g_pstSCEInfo;
SFM_L2Dev	*g_pstL2Dev;
char l3pd_IPaddr[MAX_PROBE_DEV_NUM][20];

char         g_szL2_IPaddr[MAX_PROBE_DEV_NUM][20];
char     	g_szSCE_IPAddr[MAX_PROBE_DEV_NUM][20];
int          PDCOUNT;
int          L2COUNT;
int      	SCECOUNT;
char         g_szRDR_IPAddr[MAX_PROBE_DEV_NUM][20];

#ifdef DATA_SCP
#include "system.h"
extern int		shmKey_dscp_cliTbl, shmKey_dscp_srvTbl;
#endif

char    ver[8] = "R2.0.0";	// R1.2.0 (20110302) -> R2.0.0 (2011-05-05)

//NTP
extern unsigned char ucRmtNTPSvrStatus;
extern unsigned char ucLclNTPDmnStatus;
extern st_NTP_STS   stNTPSTS;
extern st_NTP_STS   oldNTPSTS;

DB_INFO_t   g_stDBInfo;
extern int init_L2_snmp();

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/

void FinishProgram()
{
	int  process_idx;
	for( process_idx = 0; process_idx < loc_sadb->processCount; process_idx++ ){
		if( !strncasecmp( loc_sadb->loc_process_sts[process_idx].processName, "SAMD", 4 ) ){
			loc_sadb->loc_process_sts[process_idx].status = SFM_STATUS_DEAD;
		}
	}

	report_sadb2FIMD ();

    if( !g_dStopFlag ){                                                                                                                        
        sprintf (trcBuf,">>> DON'T WANT PROGRAM FINISHED, cause=%d\n",g_dStopFlag);                                                            
    }else{                                                                                                                                     
        sprintf (trcBuf," >>> PROGRAM IS NORMALLY TERMINATED, cause=%d\n", g_dStopFlag);                                                       
    }                                                                                                                                          
    trclib_writeLogErr (FL,trcBuf);                                                                                                            
                                                                                                                                               
    exit(0);                                                                                                                                   
}     


/*------------------------------------------------------------------------------                                                               
------------------------------------------------------------------------------*/

void UserControlledSignal(int sign)
{
    g_dStopFlag = 1;
	sprintf (trcBuf, "[%s] RECEIVED TERMINATED SIGNAL, sig=%d, when=%ld\n"
			,__FUNCTION__, sign, time(0));
	trclib_writeLogErr (FL,trcBuf);
}


/*******************************************************************************
    
*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM){
		sprintf (trcBuf, "[%s] UNWANTED SIGNAL IS RECEIVED, sig=%d\n"
				,__FUNCTION__, sign);
		trclib_writeLogErr (FL,trcBuf);
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
	/* 20110228 by dcham, SIGCHLD/SIGCLD default 처리 */
	//signal(SIGCHLD, handleChildProcess);
	//signal(SIGCLD,  handleChildProcess);
} 

int InitSys (void)
{
	char	*env, tmp[32], diskNameList[16][CONFLIB_MAX_TOKEN_LEN];
	int		i, key, ret, cnt, shmId;

	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[samd_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);
	strcpy (myAppName, "SAMD");
	SetUpSignal();

	// ping_test할때 wait로 child 프로세스의 종료 조건을 확인해야하는데,
	//	commlib_setupSignals에서 SIGCHLD를 catch하도록 되어 있는 것을 release한다.
	sigrelse (SIGCHLD);

	if ((iv_home = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[samd_init] not found %s environment name\n", IV_HOME);
		return -1;
	}

	if(set_proc_version(OMP_VER_INX_SAMD, ver) < 0){
		fprintf(stderr, "[InitSys] setting process version failed\n");
		return -1;
	}

	if (samd_initLog() < 0)
		return -1;

	sprintf(l_sysconf, "%s/%s", iv_home, SYSCONF_FILE);

	if ((ret = keepalivelib_init("SAMD")) < 0)
		return -1;
	memset ((char*)keepalive, 0, sizeof(T_keepalive));

	/* IXPC MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[samd_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	/* SAMD MSG Qid를 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "SAMD", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((samdQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[samd_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	/* MCDM MSG Qid를 구한다 : sjjeon*/
	if (conflib_getNthTokenInFileSection (l_sysconf, "APPLICATIONS", "MCDM", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((mcdmQID = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[samd_init] msgget fail; key=%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	/* watch-dog 메시지를 구성할 때 사용할 SYSTEM_LABEL 이름 구한다 */
	if (conflib_getNthTokenInFileSection (l_sysconf, "GENERAL", "SYSTEM_LABEL", 1, sysLabel) < 0)
		return -1;

    // by helca 07.31
	memset (&system_statistic, 0, sizeof(STM_LoadOMPStatMsgType)); /* system_statistic를 초기화 한다 */

    //dis-lan-sts 의 모든 시스템의 랜 정보를 보여줘야 하기 때문에 SFM_sfdb 를 
    //연결 하였다
	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_SFDB", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, SFM_SFDB_SIZE, 0666)) < 0) {
		fprintf(stderr,"SFDB shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	if ((sfdb = (SFM_sfdb*) shmat (shmId,0,0)) == (SFM_sfdb*)-1) {
		fprintf(stderr,"SFDB shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	if ((shmId = (int)shmget (key, sizeof(SFM_SysCommMsgType), 0666|IPC_CREAT)) < 0) {
		if (errno != ENOENT) {
			fprintf (stderr,"[samd_init] shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
	}
	if ((loc_sadb = (SFM_SysCommMsgType*) shmat (shmId,0,0)) == (SFM_SysCommMsgType*)-1) {
		fprintf (stderr,"[samd_init] shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	memset (loc_sadb, 0, sizeof(SFM_SysCommMsgType)); /* loc_sadb를 초기화 한다 */
    /* TAP INIT */
	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_L3PD", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	sprintf (l3pdFileName, "%s/%s", iv_home, SFM_L3PD_FILE); // by helca 09.11
	if (samd_getL3pd (key) < 0)
		return -1;

#if 0 
	/* by june : 일단 막아둔다 필요시 오픈하되 함수에 대한 검증을 하고 쓸것 */
	// sjjeon 기존의 snmp 함수를 초기하여 셋팅하는 부분이나. L2 snmp는 쉘로 얻어오는 것이므로 할 필요없다.
	if(init_L2_snmp() < 0){
		return -1;
	}
#endif
    /* SCE Device INIT */
	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_SCE", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	sprintf (sceFileName, "%s/%s", iv_home, SFM_SCE_FILE); // by helca 09.11
	if (samd_getSCE(key) < 0)
		return -1;

	if(samd_get_snmp_sce_ipaddress(l_sysconf) < 0){
		fprintf(stderr,"[samd_init] samd_get_snmp_sce_ipaddress fails\n");
		return -1;
	}

	if(samd_get_snmp_rdr_ipaddress(l_sysconf) < 0){
		fprintf(stderr,"[samd_init] samd_get_snmp_rdr_ipaddress fails\n");
		return -1;
	}

    /* L2SW Device INIT */
	if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_L2SW", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);

	sprintf (szl2swFileName, "%s/%s", iv_home, SFM_L2SW_FILE); // by helca 09.11
	if (samd_getL2SW(key) < 0)
		return -1;

	if(samd_get_snmp_L2_ipaddress(l_sysconf) < 0){
		fprintf(stderr,"[samd_init] samd_get_snmp_L2_ipaddress fails\n");
		return -1;
	}
	/* ping test해야하는 다른 시스템들의 이름과 ipAddress를 저장하는 table을 만든다.
	*/
	if (samd_initPingAddrTbl (l_sysconf) < 0) {
		fprintf(stderr,"[samd_init] samd_initPingAddrTbl\n");
		return -1;
	}

	/* loc_sadb->loc_process_sts와  ProcessInfo에 관리대상 프로세스의 정보를 저장한다.
	*/
	if ((cnt = samd_initProcessTbl (l_sysconf)) < 0) {
		fprintf(stderr,"[samd_init] samd_initProcessTbl\n");
		return -1;
	}

	/* 프로세스 상태관리를 위한 기동 시 초기 상태정보를 수집한다.
	*/
	newSumOfPIDs = oldSumOfPIDs = getSumOfPIDs();
    if (newSumOfPIDs < 0 ) {
        sprintf(trcBuf,"[InitSys] newSumOfPIDs error\n");
        trclib_writeLogErr (FL,trcBuf);
    }
    if ((ret = getProcessStatus()) < 0) {
		sprintf(trcBuf,"[InitSys] process status is wrong\n");
		trclib_writeLogErr (FL,trcBuf);
	}
	for (i=0; i < loc_sadb->processCount; i++) {
		ProcessInfo[i].old_status = ProcessInfo[i].new_status;
	}

	// NTP's status value initial
	ucLclNTPDmnStatus = NTP_INITIAL;
	ucRmtNTPSvrStatus = NTP_INITIAL;
	memset( &stNTPSTS, 0x00, sizeof(st_NTP_STS));
	memset( &oldNTPSTS, 0x00, sizeof(st_NTP_STS));

#if TRU64
	loc_sadb->cpuCount = 1; /* 일단 무조건 1로 설정한다.*/
#endif

	/* disk partition 갯수와 이름을 저장한다.
	*/
	cnt = conflib_getTokenCntInFileSection (l_sysconf, "SAMD_CONFIG", "DISK");
	loc_sadb->diskCount = cnt;
	conflib_getNTokenInFileSection (l_sysconf, "SAMD_CONFIG", "DISK", cnt, diskNameList);
	for (i=0; i<cnt; i++) {
		strcpy(loc_sadb->loc_disk_sts[i].diskName, diskNameList[i]);
	}

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);

	return 1;
}

int samd_initLog (void)
{
    char    fname[256];

    sprintf(fname,"%s/%s.%s", iv_home, SAMD_LOG_FILE, mySysName);

    if ((trcLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[samd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    sprintf(fname,"%s/%s.%s", iv_home, SAMD_ERRLOG_FILE, mySysName);
    if ((trcErrLogId = loglib_openLog (fname,
            LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
            LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
        fprintf(stderr,"[samd_initLog] openLog fail[%s]\n", fname);
        return -1;
    }

    return 1;

} /** End of samd_initLog **/


int samd_initProcessTbl (char *fname)
{
	char    getBuf[256],token[8][64];
	int     appCnt=0,queCnt=0,lNum;
	FILE    *fp;
	int		i;

	memset(&ProcessInfo, 0, sizeof(ProcessInfo));

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[samd_initProcessTbl] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* [APPLICATIONS] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"APPLICATIONS")) < 0) {
		if(fp)fclose(fp);
		return -1;
	}

	/*
	* 등록된 application들의 정보를 저장한다.
	*/
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(appCnt < SYSCONF_MAX_APPL_NUM) )
	{
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		/* config 파일에 "name = Qkey 실행파일위치level" format으로 들어있다.  */
		if (sscanf(getBuf,"%s%s%s%s%s",token[0],token[1],token[2],token[3],token[4]) < 5) {
			sprintf(trcBuf,"[samd_initProcessTbl] syntax error; file=%s, lNum=%d\n", fname, lNum);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}

		/* name, 소문자로 바꿔서 넣는다. */
		for (i=0; i<strlen(token[0]); i++) token[0][i] = tolower(token[0][i]);
		strcpy (loc_sadb->loc_process_sts[appCnt].processName, token[0]);

		/* 죽었을때 OMP-FIMD에서 설정할 장애 등급 */
		loc_sadb->loc_process_sts[appCnt].level = strtol(token[4],0,0);

		/* 상태관리에 필요한 초기값 설정 */
		loc_sadb->loc_process_sts[appCnt].status = SFM_STATUS_DEAD;
		ProcessInfo[appCnt].new_status = SFM_STATUS_DEAD;
		ProcessInfo[appCnt].msgQkey = strtol(token[2],0,0);
		strcpy (ProcessInfo[appCnt].procName, loc_sadb->loc_process_sts[appCnt].processName);
		sprintf (ProcessInfo[appCnt].exeFile, "%s/%s", iv_home, token[3]);

		/* QUEUE 초기 정보 수집 */
		if(ProcessInfo[appCnt].msgQkey != 0) {
			loc_sadb->loc_que_sts[queCnt].qKEY = (int)ProcessInfo[appCnt].msgQkey;
			strcpy(loc_sadb->loc_que_sts[queCnt].qNAME, loc_sadb->loc_process_sts[appCnt].processName);
			queCnt++;
		}
		appCnt++;
	}
	fclose(fp);
	loc_sadb->processCount = appCnt; /* 관리하는 process 갯수 */
	loc_sadb->queCount = queCnt; /* 관리하는 queue 갯수 */

	queCNT = queCnt;			/* 등록된 놈 */

	return (appCnt);

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
		if(fp)fclose(fp);
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
		/* add by mnpark - 20040109*/
		loc_sadb->loc_lan_sts[lanCnt].target_IPaddress = inet_addr(token[4]);
		loc_sadb->loc_lan_sts[lanCnt].status = SFM_LAN_DISCONNECTED;
		lanCnt++;
		
		if (strcasecmp(token[5], "NULL")) { /* secondary를 위해 */
			strcpy(loc_sadb->loc_lan_sts[lanCnt].target_SYSName, token[0]);
			loc_sadb->loc_lan_sts[lanCnt].target_IPaddress = inet_addr(token[5]);
			loc_sadb->loc_lan_sts[lanCnt].status = SFM_LAN_DISCONNECTED;
			lanCnt++;
		}
	}
	fclose(fp);
	loc_sadb->lanCount = lanCnt; /* 관리하는 lan connection 갯수 */

	return (lanCnt);

} /** End of samd_initPingAddrTbl **/

//int samd_get_snmp_ipaddress(char *fname) // name 변경 ..sjjeon
int samd_get_TAP_ipaddress(char *fname)
{
	char    getBuf[256],IPAddr[20];
	int     IPcnt=0,lNum, i, j;
	FILE    *fp;

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[%s] fopen fail[%s]; err=%d(%s)\n",__FUNCTION__, fname, errno, strerror(errno));
		return -1;
	}

	/* ["L3_IP_ADDRESS"] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"L3PD_IP_ADDRESS")) < 0) {
		if(fp)fclose(fp);
		return -1;
	}

	PDCOUNT = 0;

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(IPcnt < MAX_PROBE_DEV_NUM) )
	{
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if(inet_addr(getBuf) < 0){
			fprintf(stderr,"[%s] Wrong l3pd IP Address %s\n",__FUNCTION__, getBuf);
			fclose(fp);
			return -1;
		}

		memset(IPAddr, 0x00, sizeof(IPAddr));
		for(i = 0; getBuf[i] == ' ';i++);
		for(i = i, j = 0; (isdigit(getBuf[i]) || getBuf[i] == '.'); i++, j++)
			IPAddr[j] = getBuf[i];

		IPAddr[j] = 0x00;

		strncpy(l3pd_IPaddr[IPcnt], IPAddr, 20); /* by dhkim */
		IPcnt++;
	}
	fclose(fp);
	PDCOUNT = IPcnt;

	return 0;
}

/* 2009.04.14 by dhkim */
int samd_get_snmp_sce_ipaddress(char *fname)
{
	char    getBuf[256],IPAddr[20];
	int     IPcnt=0,lNum, i, j;
	FILE    *fp;


	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[samd_get_snmp_sce_ipaddress] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* ["SCE_IP_ADDRESS"] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"SCE_IP_ADDRESS")) < 0) {
		if(fp)fclose(fp);
		return -1;
	}

	memset(g_szSCE_IPAddr, 0x00, sizeof(g_szSCE_IPAddr));
	SCECOUNT = 0;

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(IPcnt < MAX_PROBE_DEV_NUM) )
	{
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if(inet_addr(getBuf) < 0){
			fprintf(stderr,"[samd_get_snmp_sce_ipaddress] Wrong SCE IP Address %s\n", getBuf);
			fclose(fp);
			return -1;
		}

		memset(IPAddr, 0x00, sizeof(IPAddr));
		for(i = 0; getBuf[i] == ' ';i++);
		for(i = i, j = 0; (isdigit(getBuf[i]) || getBuf[i] == '.'); i++, j++)
			IPAddr[j] = getBuf[i];

		IPAddr[j] = 0x00;

		strncpy(g_szSCE_IPAddr[IPcnt], IPAddr, 20); /* by dhkim */
		IPcnt++;
	}
	fclose(fp);
	SCECOUNT = IPcnt;

	return 0;
}

int samd_get_snmp_L2_ipaddress(char *fname)
{
	char    getBuf[256], IPAddr[20];
	int     IPcnt=0, lNum, i, j;
	FILE    *fp;

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[samd_get_snmp_L2_ipaddress] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* ["L2_IP_ADDRESS"] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"L2_IP_ADDRESS")) < 0) {
		if(fp)fclose(fp);
		return -1;
	}

	memset(g_szL2_IPaddr, 0x00, sizeof(g_szL2_IPaddr));
	L2COUNT = 0;

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(IPcnt < MAX_PROBE_DEV_NUM) )
	{
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if(inet_addr(getBuf) < 0){
			fprintf(stderr,"[samd_get_snmp_L2_ipaddress] Wrong L2 IP Address %s\n", getBuf);
			fclose(fp);
			return -1;
		}

		memset(IPAddr, 0x00, sizeof(IPAddr));
		for(i = 0; getBuf[i] == ' ';i++);
		for(i = i, j = 0; (isdigit(getBuf[i]) || getBuf[i] == '.'); i++, j++)
			IPAddr[j] = getBuf[i];

		IPAddr[j] = 0x00;

		strncpy(g_szL2_IPaddr[IPcnt], IPAddr, 20);
		IPcnt++;
	}
	fclose(fp);
	L2COUNT = IPcnt;

	return 0;
}

int samd_getL3pd (int key)
{
	int	 ret, fd, shmId, l3pdLoadFlag=0;
	char *env;

	if ((env = getenv(IV_HOME)) == NULL)
        	return -1;
	// attach
	if ((shmId = (int)shmget (key, SFM_L3PD_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[samd_getL3pd] L3PD shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_L3PD_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[samd_getL3pd] L3PD shmget fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		l3pdLoadFlag = 1;
	}
	if ((l3pd = (SFM_L3PD*) shmat (shmId,0,0)) == (SFM_L3PD*)-1) {
		fprintf(stderr,"[samd_getL3pd] L3PD shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	
	if (l3pdLoadFlag) {
		if ((fd = open (l3pdFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)l3pd, SFM_L3PD_SIZE)) < 0) {
				fprintf(stderr,"[samd_getL3pd] open fail[%s]; err=%d(%s)\n", l3pdFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
			close(fd);
		} else {
			if (errno != ENOENT) {
				fprintf(stderr,"[samd_getL3pd] open fail[%s]; err=%d(%s)\n", l3pdFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( l3pd, 0 ,sizeof(SFM_L3PD) );
				
			}
		}
	}
	return 0;
}
//----- End of samd_getL3pd -----//

int samd_getSCE (int key)
{
	int	 ret, fd, shmId, SCELoadFlag=0;
	char	*env;

	if ((env = getenv(IV_HOME)) == NULL)
		return -1;
	// attach
	if ((shmId = (int)shmget (key, SFM_SCE_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[samd_getSCE] SCE shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_SCE_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[samd_getSCE] SCE shmget fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		SCELoadFlag = 1;
	}
	if ((g_pstSCEInfo = (SFM_SCE*) shmat (shmId,0,0)) == (SFM_SCE*)-1) {
		fprintf(stderr,"[samd_getSCE] SCE shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	if (SCELoadFlag) {
		if ((fd = open (sceFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)g_pstSCEInfo, SFM_SCE_SIZE)) < 0) {
				fprintf(stderr,"[samd_getSCE] open fail[%s]; err=%d(%s)\n", sceFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
			close(fd);
		} else {
			if (errno != ENOENT) {
				fprintf(stderr,"[samd_getSCE] open fail[%s]; err=%d(%s)\n", sceFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( g_pstSCEInfo, 0x00 ,sizeof(SFM_SCE) );

			}
		}
	}
	return 0;
}

int samd_getL2SW (int key)
{
	int	ret, fd, shmId, dLoadFlag=0;
	char	*env;

	if ((env = getenv(IV_HOME)) == NULL)
        return -1;
	// attach
	if ((shmId = (int)shmget (key, SFM_L2DEV_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[samd_getL2SW] L2SW shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_L2DEV_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[samd_getL2SW] L2SW shmget fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		dLoadFlag = 1;
	}
	if ((g_pstL2Dev = (SFM_L2Dev*) shmat (shmId,0,0)) == (SFM_L2Dev*)-1) {
		fprintf(stderr,"[samd_getL2SW] L2SW shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	if (dLoadFlag) {
		if ((fd = open (l3pdFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)g_pstL2Dev, SFM_L2DEV_SIZE)) < 0) {
				fprintf(stderr,"[samd_getL2SW] open fail[%s]; err=%d(%s)\n", l3pdFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
			close(fd);
		} else {
			if (errno != ENOENT) {
				fprintf(stderr,"[samd_getL2SW] open fail[%s]; err=%d(%s)\n", l3pdFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( g_pstL2Dev, 0 ,sizeof(SFM_L2Dev) );
				
			}
		}
	}
	return 1;
}

/* 2009.04.16 by dhkim */
int samd_get_snmp_rdr_ipaddress(char *fname)
{
	char    getBuf[256],IPAddr[20];
	int     IPcnt=0,lNum, i, j;
	FILE    *fp;

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[samd_get_snmp_rdr_ipaddress] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}

	/* ["RDR_IP_ADDRESS"] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"RDR_IP_ADDRESS")) < 0) {
		if(fp)fclose(fp);
		return -1;
	}

	memset(g_szRDR_IPAddr, 0x00, sizeof(g_szRDR_IPAddr));

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while ( (fgets(getBuf,sizeof(getBuf),fp) != NULL) &&
			(IPcnt < MAX_PROBE_DEV_NUM) )
	{
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if(inet_addr(getBuf) < 0){
			fprintf(stderr,"[samd_get_snmp_rdr_ipaddress] Wrong RDR IP Address %s\n", getBuf);
			fclose(fp);
			return -1;
		}

		memset(IPAddr, 0x00, sizeof(IPAddr));
		for(i = 0; getBuf[i] == ' ';i++);
		for(i = i, j = 0; (isdigit(getBuf[i]) || getBuf[i] == '.'); i++, j++)
			IPAddr[j] = getBuf[i];

		IPAddr[j] = 0x00;

		strncpy(g_szRDR_IPAddr[IPcnt], IPAddr, 20);
		IPcnt++;
	}
	fclose(fp);

	return 0;
}

