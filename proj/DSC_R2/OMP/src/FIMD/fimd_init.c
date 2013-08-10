#include "fimd_proto.h"

extern int	fimdQid, ixpcQid, condQid, nmsifQid, eqSysCnt;
extern int	dataPortNum, eventPortNum, dupChkTime;
extern int	wtEndIdx;
extern int	WTtable[24];
extern time_t	currentTime;
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern char	trcBuf[4096], trcTmp[1024], sfdbFileName[256], audioFileName[256], l3pdFileName[256], sceFileName[256], szl2swFileName[256], callFileName[256],legFileName[256]; //tpsFileName[256];
extern char	logonFileName[256], smChFileName[256];
extern char	systemModel[5];
extern int	trcLogId, trcErrLogId, trcFlag, trcLogFlag;
extern SFM_sfdb	    *sfdb;
extern SFM_L3PD	    *l3pd;
extern SFM_L2Dev   	*g_pstL2Dev;
extern SFM_SCE		*g_pstSCEInfo;
/* hjjung_20100823 */
//extern SFM_LEG		*g_pstLEGInfo;
extern SFM_CALL		*g_pstCALLInfo;
extern SFM_LOGON    *g_pstLogonRate;
extern SFM_LOGON    g_stLogonRate[LOG_MOD_CNT][2];
extern int	numMmcHdlr;
extern int	*sound_flag;
extern int	minRate[5];
extern FimdMmcHdlrVector        mmcHdlrVector[FIMD_MAX_MMC_HANDLER];
extern FimdKeepAlive            fimdKeepAlive[MAX_KEEPALIVE];

/* 20040921-mnpark */
extern int 	dbSyncFailCnt[SYSCONF_MAX_ASSO_SYS_NUM];
extern int 	dbDeadCnt[SYSCONF_MAX_ASSO_SYS_NUM];

extern int	chkCnt2[SYSCONF_MAX_ASSO_SYS_NUM][RADIUS_IP_CNT];
extern int 	g_hwswType[SFM_MAX_HPUX_HW_COM];
extern LANIF_CONFIG   lanConf;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

char    ver[8] = "R2.0.0";	// BEFORE: R1.2.0 (2011-03-02) -> R2.0.0 (2011-05-05)

int wtTable_cmp (const void *a, const void *b)
{
	//fprintf(stderr,"a=%d, b=%d\n",*(int *)a,*(int *)b);
	return (int)(*(int *)a - *(int *)b);
}

void FinishProgram()
{
	GeneralQMsgType	rxGenQMsg;
	int             sysIndex;

	commlib_microSleep(1000000);
		
	while(msgrcv(fimdQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0) {
		fimd_exeRxQMsg (&rxGenQMsg);
		memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));
	}

	for(sysIndex = 0; sysIndex < eqSysCnt; sysIndex++){
		fimd_broadcastSfdb2Client (sysIndex);
	}

    if( !g_dStopFlag ){                                                                                                                        
        sprintf (trcBuf,">>> DON'T WANT PROGRAM FINISHED, cause=%d\n",g_dStopFlag);                                                            
    }else{                                                                                                                                     
        sprintf (trcBuf," >>> PROGRAM IS NORMALLY TERMINATED, cause=%d\n", g_dStopFlag);                                                       
    }                                                                                                                                          
    trclib_writeLogErr (FL,trcBuf);                                                                                                            
                                                                                                                                               
    exit(0);                                                                                                                                   
}     

void UserControlledSignal(int sign)
{
    g_dStopFlag = 1;
	logPrint (trcErrLogId,FL, "[%s] RECEIVED TERMINATED SIGNAL, sig=%d, when=%ld\n"
			,__FUNCTION__, sign, time(0));
}


/*******************************************************************************
    
*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM){
		logPrint (trcErrLogId,FL, "[%s] UNWANTED SIGNAL IS RECEIVED, sig=%d\n"
				,__FUNCTION__, sign);
	}
    signal(sign, IgnoreSignal);
}


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
}


/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
                                                                                                                                               
/*------------------------------------------------------------------------------                                                               
------------------------------------------------------------------------------*/                                                               

int fimd_initial (void)
{
	char	*env, tmp[64], fname[256];
	int	key,num,i;
	char	tmpStr[24];

	
	if ((env = getenv(MY_SYS_NAME)) == NULL) {
		fprintf(stderr,"[fimd_init] not found %s environment name\n", MY_SYS_NAME);
		return -1;
	}
	strcpy (mySysName, env);
	strcpy (myAppName, "FIMD");

	if(set_proc_version(OMP_VER_INX_FIMD, ver) < 0){
		fprintf(stderr, "[fimd_init] setting process version failed\n");
		return -1;
	}
	
	SetUpSignal ();
	currentTime = time(0);

	// MMC 처리 function들을 bsearch로 찾기 위해 sort한다.
	//
	qsort ((void*)mmcHdlrVector,
			numMmcHdlr,
			sizeof(FimdMmcHdlrVector),
			fimd_mmcHdlrVector_qsortCmp);


	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[fimd_init] not found %s environment name\n", IV_HOME);
		return -1;
	}
	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	// config file에서 message queue key를 읽어, attach
	//
	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "FIMD", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((fimdQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[fimd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "IXPC", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((ixpcQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[fimd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "COND", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((condQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[fimd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// by helca 08.17
	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "NMSIF", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	if ((nmsifQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"[fimd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	

	// 상태,장애 정보를 관리하는 shared memory(sfdb)를 attach한다.
	//
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SFDB", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	sprintf (sfdbFileName, "%s/%s", env, SFM_SFDB_FILE);

	if (fimd_getSfdb (key) < 0)
		return -1;
	
	// 상태,장애 정보를 관리하는 shared memory(sfdb)를 attach한다.
	//
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SOUND", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	sprintf (audioFileName, "%s/%s", env, SFM_AUDIO_FILE);
	if (fimd_getAudio (key) < 0)
		return -1;
	//--------------------------------------------------------------------
	// shared memory(l3pd)를 attach한다.
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_L3PD", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	sprintf (l3pdFileName, "%s/%s", env, SFM_L3PD_FILE); // by helca 09.11
	
	if (fimd_getL3pd (key) < 0)
		return -1;

	alm_lmt_pd_output(); // limit 값을 파일에서 가져온다.
	//--------------------------------------------------------------------
	// SCE Device, shared memory attach.
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_SCE", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	sprintf (sceFileName, "%s/%s", env, SFM_SCE_FILE); // by helca 09.11
	
	if (fimd_getSCE(key) < 0)
		return -1;

	if(alm_lmt_sce_readInfoFile()<0) // limit 값을 파일에서 가져온다. rename....sjjeon
		return -1;
	//--------------------------------------------------------------------
	/* hjjung_20100823 */
	// CALL Device, shared memory attach.
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_CALL", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	sprintf (callFileName, "%s/%s", env, SFM_CALL_FILE);
	if (fimd_getCALL(key) < 0)
		return -1;
	if(alm_lmt_sess_readInfoFile()<0) // session limit 값을 파일에서 가져온다.
		return -1;
	if(alm_lmt_tps_readInfoFile()<0)  // tps limit 값을 파일에서 가져온다.
		return -1;
#if 0
	//--------------------------------------------------------------------
	// added by dcham 20110525 for TPS
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_CALL", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	sprintf (tpsFileName, "%s/%s", env, SFM_TPS_FILE);
	if (fimd_getTPS(key) < 0)
		return -1;
#endif

	//--------------------------------------------------------------------
	// L2 Device, shared memory attach.
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_L2SW", 1, tmp) < 0)
		return -1;
	key = strtol(tmp,0,0);
	sprintf (szl2swFileName, "%s/%s", env, SFM_L2SW_FILE); // by helca 09.11
	
	if (fimd_getL2Dev(key) < 0)
		return -1;

	if(alm_lmt_l2sw_readInfoFile()<0) // limit 값을 파일에서 가져온다. rename....sjjeon
		return -1;

	/* LOGON 성공율 감시를 위한 초기값 loading. by uamyd 20110210 */
	sprintf (logonFileName, "%s/%s", env, SFM_LOGON_FILE);
	if( fimd_getLogonSuccessRate() < 0 )
		return -1;
	if( alm_lmt_logon_success_rate_readInfoFile() < 0 )
		return -1;

	sprintf (smChFileName, "%s/%s", env, SFM_SM_CONN_FILE);
	if( fimd_getSMChSts() < 0 )
		return -1;
	/* SM Connection Status 를 위한 threadhold 초기값 loading. added by uamyd 20110425 */
	if( alm_lmt_sm_ch_sts_readInfoFile() < 0 )
		return -1;

	//--------------------------------------------------------------------

	// 장애통계 수집을 위한 영역에 시스템 type과 name을 setting한다.
	//
	fimd_initSysAlmStat ();


	// GUI가 접속할 자신의 bind port를 읽어 binding한다.
	// - 주기적으로 sfdb를 전송할 data port와
	// - 장애 발생/해지 시 current alarm list를 alarm_history DB에서 다시 읽어가도록
	//	통보하는 event port를 각각 binding한다.
	//
	if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "FIMD_DATA", 1, tmp) < 0)
		return -1;
	dataPortNum = strtol(tmp,0,0);
	if (socklib_initTcpBind (dataPortNum) < 0)
		return -1;

	if (conflib_getNthTokenInFileSection (fname, "SOCKET_PORT", "FIMD_EVENT", 1, tmp) < 0)
		return -1;
	eventPortNum = strtol(tmp,0,0);
	if (socklib_initTcpBind (eventPortNum) < 0)
		return -1;

	// 장애메시지 header에 붙일 system_label을 읽는다.
	//
	if (conflib_getNthTokenInFileSection (fname, "GENERAL", "SYSTEM_LABEL", 1, sysLabel) < 0)
		return -1;

	if (conflib_getNthTokenInFileSection (fname, "GENERAL", "SYSTEM_MODEL", 1, systemModel) < 0)
		return -1;

	/* Duplication status alarm check time add by helca 2009.01.09 */
	if (conflib_getNthTokenInFileSection (fname, "DUP_CHECK_TIME", "TIME", 1, tmp) < 0)
		return -1;
	dupChkTime = strtol(tmp,0,0);
	/**/

	// log file들을 open한다.
	//
	if (fimd_initLog () < 0)
		return -1;

	fimd_hwInfo_init (); // add by helca 2008.07.21

	//
	if (keepalivelib_init (myAppName) < 0)
		return -1;

	//
	memset(fimdKeepAlive, 0x0, sizeof(fimdKeepAlive));
	// COMM STATUS
	num=0;
	fimdKeepAlive[num].category = MSGID_SYS_COMM_STATUS_REPORT;
	fimdKeepAlive[num].sysIdx   = fimd_getSysIndexByName (SYSCONF_SYSTYPE_MPA);
	num++;
	fimdKeepAlive[num].category = MSGID_SYS_COMM_STATUS_REPORT;
	fimdKeepAlive[num].sysIdx   = fimd_getSysIndexByName (SYSCONF_SYSTYPE_MPB);
	num++;
	fimdKeepAlive[num].category = MSGID_SYS_COMM_STATUS_REPORT;
	fimdKeepAlive[num].sysIdx   = fimd_getSysIndexByName (SYSCONF_SYSTYPE_OMP);
	num++;

	// HW STATUS
/*  fimdKeepAlive[num].category = MSGID_SYS_SPEC_HW_STATUS_REPORT;
	fimdKeepAlive[num].sysIdx   = fimd_getSysIndexByName (SYSCONF_SYSTYPE_MPA);
	num++;
	fimdKeepAlive[num].category = MSGID_SYS_SPEC_HW_STATUS_REPORT;
	fimdKeepAlive[num].sysIdx   = fimd_getSysIndexByName (SYSCONF_SYSTYPE_MPB);
	num++;
	*/

/*	// TCP CONN STATUS
	fimdKeepAlive[num].category = MSGID_SYS_SPEC_CONN_STATUS_REPORT;
	fimdKeepAlive[num].sysIdx   = fimd_getSysIndexByName (SYSCONF_SYSTYPE_MPA);
	num++;
	fimdKeepAlive[num].category = MSGID_SYS_SPEC_CONN_STATUS_REPORT;
	fimdKeepAlive[num].sysIdx   = fimd_getSysIndexByName (SYSCONF_SYSTYPE_MPB);
	num++;
*/

	/* 20040921-mnpark DB Replication Fail Count */
	memset(dbSyncFailCnt, 0x00, sizeof(dbSyncFailCnt));
	memset(dbDeadCnt, 0x00, sizeof(dbDeadCnt));	/* 20041129.mnpark */

	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
		fimd_updateSysAlmInfo (i);
	}

	// mysql (alarm_history DB)에 접속한다.
	//
	if (fimd_mysql_init () < 0){
		sprintf(trcBuf,"init fail, fimd terminate.\n"); trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	memset(chkCnt2, 0x00, sizeof(chkCnt2));

	SFM_HpUxHWInfo  *hwInfo_mpa = &sfdb->sys[1].specInfo.u.sms.hpuxHWInfo;
	SFM_HpUxHWInfo  *hwInfo_mpb = &sfdb->sys[2].specInfo.u.sms.hpuxHWInfo;

    char loc_hw [16], loc_eth[16], loc_type[3];

	for (i = 0; i< SFM_MAX_HPUX_HW_COM; i++) 
	{
		sprintf (loc_hw, "HARDWARE%d", i);
		if (conflib_getNthTokenInFileSection(fname, "HW_LOCAL_INFO", loc_hw, 1, (char *)loc_eth) < 0) {
			//fprintf(stderr,"loc error\n");
			break;
		}
		sprintf (hwInfo_mpa->hwcom[i].name, "%s", loc_eth);
		sprintf (hwInfo_mpb->hwcom[i].name, "%s", loc_eth);
		//logPrint (trcLogId,FL,"HWINFO name: %s\n", hwInfo_mpb->hwcom[i].name);
	}

	for (i = 0; i< SFM_MAX_HPUX_HW_COM; i++) {
		/* 
		   Alarm Type에서 HW/SW/Etc 등의 설정.
			hw : 0, sw : 1, hw(mirror) : 2
		 */
		sprintf (loc_hw, "HARDWARE%d", i);
		if (conflib_getNthTokenInFileSection(fname, "HW_LOCAL_INFO", loc_hw, 2, (char *)loc_type) < 0) {
			//fprintf(stderr,"loc error\n");
			break;
		}

		g_hwswType[i] = atoi(loc_type);
		//logPrint (trcLogId,FL,"HWINFO type: %d\n", g_hwswType[i]);
	}


	/* ADD BY JUNE, 2010-12-21 */                                                                                                              
	memset(&lanConf, 0x00, sizeof(LANIF_CONFIG));
	if (conflib_getNthTokenInFileSection (fname, "[LAN_INFO]", "IF_TYPE_COUNT", 1, tmp) < 0) {
		logPrint (trcLogId,FL, "CAN'T GET COUNT of LAN DEVICE err=%s\n", strerror(errno));
		return -1;
	}
	else
		lanConf.count = strtol(tmp,0,0);
		logPrint (trcLogId,FL, "CAP IF_TYPE_COUNT[%d]\n", lanConf.count);

	for (i=0 ; i < lanConf.count ; i++)
	{
		if (i >= MAX_LAN_IF_TYPE_NUM) break;
		sprintf(tmpStr, "IF_TYPE_NAME%d", i);
		if (conflib_getNthTokenInFileSection (fname, "[LAN_INFO]", tmpStr, 1, tmp) < 0) {
			logPrint (trcLogId,FL, "CAN'T GET NAME of LAN IF err=%s\n", strerror(errno));
			return -1;
		}
		else {
			strcpy(&lanConf.lanif[i].name[0], tmp);
			lanConf.lanif[i].name_size = strlen(&lanConf.lanif[i].name[0]);
			logPrint (trcLogId,FL, "IF_TYPE_NAME[%d]=[%s(%d)]\n", i, &lanConf.lanif[i].name[0], lanConf.lanif[i].name_size);
		}
	}

	logPrint (trcLogId,FL,"%s startup...\n", myAppName);
	logPrint (trcErrLogId,FL,"%s startup...\n", myAppName);

	return 1;
	
} //----- End of fimd_initial -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_initLog (void)
{
	char	*env, fname[256];

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"[fimd_initLog] not found %s environment name\n", IV_HOME);
		return -1;
	}

	sprintf (fname, "%s/%s.%s", env, FIMD_TRCLOG_FILE, mySysName);

	if ((trcLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[fimd_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	sprintf (fname, "%s/%s.%s", env, FIMD_ERRLOG_FILE, mySysName);
	if ((trcErrLogId = loglib_openLog (fname,
			LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE |
			LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) {
		fprintf(stderr,"[fimd_initLog] openLog fail[%s]\n", fname);
		return -1;
	}

	return 1;

} //----- End of fimd_initLog -----//



//------------------------------------------------------------------------------
int fimd_getL3pd (int key)
{
	int		ret, fd, shmId, l3pdLoadFlag=0;

	
	// attach
	//
	if ((shmId = (int)shmget (key, SFM_L3PD_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[fimd_getL3pd] L3PD shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_L3PD_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[fimd_getL3pd] L3PD shmget fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		
		l3pdLoadFlag = 1;
	}
	if ((l3pd = (SFM_L3PD*) shmat (shmId,0,0)) == (SFM_L3PD*)-1) {
		fprintf(stderr,"[fimd_getL3pd] L3PD shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	
	if (l3pdLoadFlag) {
		
		if ((fd = open (l3pdFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)l3pd, SFM_L3PD_SIZE)) < 0) {
				fprintf(stderr,"[fimd_getL3pd] open fail[%s]; err=%d(%s)\n", l3pdFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
//			fprintf(stderr, "l3pd read size: %d\n", ret);
			close(fd);
		} else {
			
			if (errno != ENOENT) {
				fprintf(stderr,"[fimd_getL3pd] open fail[%s]; err=%d(%s)\n", l3pdFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( l3pd, 0 ,sizeof(SFM_L3PD) );
			}
		}
	}
	
	return 0;
}
//----- End of fimd_getL3pd -----//

int fimd_getSCE (int key)
{
	int		ret, fd, shmId, loadFlag=0;



	// attach
	//
	if ((shmId = (int)shmget (key, SFM_SCE_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[fimd_getSCE] SCE shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_SCE_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[fimd_getSCE] SCE shmget fail..; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		
		loadFlag = 1;
	}
	if ((g_pstSCEInfo = (SFM_SCE*) shmat (shmId,0,0)) == (SFM_SCE*)-1) {
		fprintf(stderr,"[fimd_getSCE] SCE shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	
	if (loadFlag) {
		
		if ((fd = open (sceFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)g_pstSCEInfo, SFM_SCE_SIZE)) < 0) {
				fprintf(stderr,"[fimd_getSCE] open fail[%s]; err=%d(%s)\n", l3pdFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
//			fprintf(stderr, "g_pstSCEInfo read size: %d\n", ret);
			close(fd);
		} else {
			
			if (errno != ENOENT) {
				fprintf(stderr,"[fimd_getSCE] open fail.[%s]; err=%d(%s)\n", l3pdFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( g_pstSCEInfo, 0x00 ,sizeof(SFM_SCE) );
			}
		}
	}
	
	return 0;
}
//----- End of fimd_getSCE -----//

/* added by dcham 20110530 */ 
int fimd_getCALL (int key)
{
	int		ret, fd, shmId, loadFlag=0;

	// attach
	//
	if ((shmId = (int)shmget (key, SFM_CALL_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[fimd_getCALL] CALL shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_CALL_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[fimd_getCALL] CALL shmget fail..; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		
		loadFlag = 1;
	}
	if ((g_pstCALLInfo = (SFM_CALL*) shmat (shmId,0,0)) == (SFM_CALL*)-1) {
		fprintf(stderr,"[fimd_getCALL] CALL shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	g_pstCALLInfo->cps.uiLogOnSumCps=0;
	g_pstCALLInfo->cps.uiLogOutSumCps=0;


	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	
	if (loadFlag) {
		
		if ((fd = open (callFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)g_pstCALLInfo, SFM_CALL_SIZE)) < 0) {
				fprintf(stderr,"[fimd_getCALL] legFile open fail[%s]; err=%d(%s)\n", callFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
			close(fd);
		} else {
			
			if (errno != ENOENT) {
				fprintf(stderr,"[fimd_getCALL] open fail.[%s]; err=%d(%s)\n", callFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( g_pstCALLInfo, 0x00 ,sizeof(SFM_CALL) );
			}
		}
	}
	
	return 0;
}
//----- End of fimd_getCALL -----//
#if 0
int fimd_getTPS (int key)
{
	int	ret, fd, shmId, loadFlag=0;

	if ((shmId = (int)shmget (key, SFM_TPS_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[fimd_getTPS] TPS shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_TPS_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[fimd_getTPS] LEG shmget fail..; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		
		loadFlag = 1;
	}
	if ((g_pstCALLInfo->tpsInfo = (SFM_CALL*) shmat (shmId,0,0)) == (SFM_CALL*)-1) {
		fprintf(stderr,"[fimd_getTPS] TPS shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	if (loadFlag) {
		
		if ((fd = open (tpsFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)g_pstCALLInfo->tpsInfo, SFM_TPS_SIZE)) < 0) {
				fprintf(stderr,"[fimd_getTPS] open fail[%s]; err=%d(%s)\n", tpsFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
			close(fd);
		} else {
			
			if (errno != ENOENT) {
				fprintf(stderr,"[fimd_getTPS] open fail.[%s]; err=%d(%s)\n", tpsFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( g_pstCALLInfo->tpsInfo, 0x00 ,sizeof(TPS_INFO) );
			}
		}
	}
	
	return 0;
}
#endif
int fimd_getL2Dev (int key)
{
	int		ret, fd, shmId, loadFlag=0;


	
	// attach
	//
	if ((shmId = (int)shmget (key, SFM_L2DEV_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[fimd_getL2Dev] SCE shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_L2DEV_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[fimd_getL2Dev] SCE shmget fail..; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		
		loadFlag = 1;
	}
	if ((g_pstL2Dev = (SFM_L2Dev *) shmat (shmId,0,0)) == (SFM_L2Dev *)-1) {
		fprintf(stderr,"[fimd_getL2Dev] SCE shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	
	if (loadFlag) {
		
		if ((fd = open (szl2swFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)g_pstL2Dev, SFM_L2DEV_SIZE)) < 0) {
				fprintf(stderr,"[fimd_getL2Dev] open fail[%s]; err=%d(%s)\n", szl2swFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
//			fprintf(stderr, "g_pstSCEInfo read size: %d\n", ret);
			close(fd);
		} else {
			
			if (errno != ENOENT) {
				fprintf(stderr,"[fimd_getL2Dev] open fail.[%s]; err=%d(%s)\n", szl2swFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( g_pstL2Dev, 0 ,sizeof(SFM_L2Dev) );
			}
		}
	}
	
	return 0;
}
//----- End of fimd_getL2Dev -----//


//------------------------------------------------------------------------------
int fimd_getSfdb (int key)
{
	int  i, ret, fd, shmId, sfdbLoadFlag=0;
	//char rate[24][8], fname[256], lineBuf[1024];
	//char *token, *env;
	char *env;
	//char seps[] = " ";
	//FILE *fp;
	
	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr,"[fimd_getSfdb] getenv fail.\n");
        return -1;
	}
	// attach
	//
	if ((shmId = (int)shmget (key, SFM_SFDB_SIZE, 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[fimd_getSfdb] SFDB shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.
		if ((shmId = (int)shmget (key, SFM_SFDB_SIZE, IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[fimd_getSfdb] SFDB shmget fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		
		sfdbLoadFlag = 1;
	}
	if ((sfdb = (SFM_sfdb*) shmat (shmId,0,0)) == (SFM_sfdb*)-1) {
		fprintf(stderr,"[fimd_getSfdb] SFDB shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}
	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	if (sfdbLoadFlag) {
		if ((fd = open (sfdbFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)sfdb, SFM_SFDB_SIZE)) < 0) {
				fprintf(stderr,"[fimd_getSfdb] open fail[%s]; err=%d(%s)\n", sfdbFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
//			fprintf(stderr, "sfdb_file read size: %d\n", ret);
			close(fd);
		} else {
			if (errno != ENOENT) {
				fprintf(stderr,"[fimd_getSfdb] open fail[%s]; err=%d(%s)\n", sfdbFileName, errno, strerror(errno));
				return -1;
			} else {
				memset ( sfdb, 0 ,sizeof(SFM_sfdb) );
				if (fimd_setDefaultSfdb () < 0)
					return -1;
			}
		}
	}
	// 실장되어 있는 시스템의 총 갯수를 eqSysCnt에 setting한다.
	//
	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
		if (strcasecmp (sfdb->sys[i].commInfo.name, ""))
			eqSysCnt++;
	}

	if ( eqSysCnt == 0 ) eqSysCnt = 3; /* MPA, MPB, OMP */
#if 0
	// by sjjeon 사용하지 않음.. rate_file이 없으므로 return 됨.
	// dsc/scma/scmb 의 limit_dsc_file 을 읽지 못함. 

	/* SuccRate init*/
	sprintf (fname, "%s/DATA/rate_file", env); // by helca 07.29
	
	if ((fp = fopen(fname, "r+"))==NULL){
	
		sprintf (trcBuf, "[disprc] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	
	}
	// rate_file에서 설정값을 읽어 온다.
	//
	fgets (lineBuf, sizeof(lineBuf), fp);
	if(fp)fclose(fp);
	token = strtok(lineBuf, seps);
	for(i=0; i<24; i++){
		strcpy(rate[i], token);
		token = strtok(NULL, seps);
					
	}
	for(i=1; i<3; i++){
		strcpy(sfdb->sys[i].commInfo.succRate[0].name, rate[0]);
		sfdb->sys[i].commInfo.succRate[0].cnt = atoi(rate[1]);
		sfdb->sys[i].commInfo.succRate[0].rate = atoi(rate[2]);

		strcpy(sfdb->sys[i].commInfo.succRate[1].name, rate[3]);
		sfdb->sys[i].commInfo.succRate[1].cnt = atoi(rate[4]);
		sfdb->sys[i].commInfo.succRate[1].rate = atoi(rate[5]);

		strcpy(sfdb->sys[i].commInfo.succRate[2].name, rate[6]);
		sfdb->sys[i].commInfo.succRate[2].cnt = atoi(rate[7]);
		sfdb->sys[i].commInfo.succRate[2].rate = atoi(rate[8]);

		strcpy(sfdb->sys[i].commInfo.succRate[3].name, rate[9]);
		sfdb->sys[i].commInfo.succRate[3].cnt = atoi(rate[10]);
		sfdb->sys[i].commInfo.succRate[3].rate = atoi(rate[11]);

		strcpy(sfdb->sys[i].commInfo.succRate[4].name, rate[12]);
		sfdb->sys[i].commInfo.succRate[4].cnt = atoi(rate[13]);
		sfdb->sys[i].commInfo.succRate[4].rate = atoi(rate[14]);
	
       	strcpy(sfdb->sys[i].commInfo.succRate[5].name, rate[15]);
		sfdb->sys[i].commInfo.succRate[5].cnt = atoi(rate[16]);
		sfdb->sys[i].commInfo.succRate[5].rate = atoi(rate[17]);

 		strcpy(sfdb->sys[i].commInfo.succRate[6].name, rate[18]);
		sfdb->sys[i].commInfo.succRate[6].cnt = atoi(rate[19]);
		sfdb->sys[i].commInfo.succRate[6].rate = atoi(rate[20]);
	
		strcpy(sfdb->sys[i].commInfo.succRate[7].name, rate[21]);
		sfdb->sys[i].commInfo.succRate[7].cnt = atoi(rate[22]);
		sfdb->sys[i].commInfo.succRate[7].rate = atoi(rate[23]);	
	}
#endif 

	// by helca 08.17
	// limit file의 값을 읽어 set
	//alm_lmt_bsd_output();
	alm_lmt_dsc_output();

	return 1;

} //----- End of fimd_getSfdb -----//

int fimd_getAudio (int key)
{
	int	ret, fd, shmId, audioLoadFlag=0;
	// attach
	//
	if ((shmId = (int)shmget (key, sizeof(int), 0666)) < 0) {
		if (errno != ENOENT) {
			fprintf(stderr,"[fimd_getAudio] SOUND shmget fail.; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		// shared memory가 아직 만들어 지지 않은 경우이면 shared memory를 create한 후
		//	이전에 file로 backup해둔 놈에서 정보를 읽어 들인다.

		if ((shmId = (int)shmget (key, sizeof(int), IPC_CREAT|0666)) < 0) {
			fprintf(stderr,"[fimd_getAudio] SFDB shmget fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
			return -1;
		}
		audioLoadFlag = 1;
	}
	if ((sound_flag = (int *) shmat (shmId,0,0)) == (int *)-1) {
		fprintf(stderr,"[fimd_getAudio] SFDB shmat fail...; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
		return -1;
	}

	// share memory가 새로 만들어졌으면 backup file에서 읽어 들인다.
	// - backup file이 없으면 default 값으로 setting한다.
	//
	if (audioLoadFlag) {
		if ((fd = open (audioFileName, O_RDONLY, 0666)) >= 0) {
			lseek(fd,0,0);
			if ((ret = read (fd, (void*)sound_flag, sizeof(int))) < 0) {
				fprintf(stderr,"[fimd_getAudio] open fail[%s]; err=%d(%s)\n", audioFileName, errno, strerror(errno));
				close(fd);
				return -1;
			}
			close(fd);
		} else {
			if (errno != ENOENT) {
				fprintf(stderr,"[fimd_getAudio] open fail[%s]; err=%d(%s)\n", audioFileName, errno, strerror(errno));
				return -1;
			} else {
				*sound_flag = 1;
			}
		}
	}

	return 1;

} //----- End of fimd_getSfdb -----//


#define FIMD_DEFAULT_MINOR_LIMIT		70
#define FIMD_DEFAULT_MAJOR_LIMIT		80
#define FIMD_DEFAULT_CRITICAL_LIMIT		90
#define FIMD_DEFAULT_DURATION			100

#define FIMD_DEFAULT_MINOR_LIMIT_STAT		1
#define FIMD_DEFAULT_MAJOR_LIMIT_STAT		30
#define FIMD_DEFAULT_CRITICAL_LIMIT_STAT 	50

#define FIMD_DEFAULT_MINOR_LIMIT_DBSYNC 	10 
#define FIMD_DEFAULT_MAJOR_LIMIT_DBSYNC 	50
#define FIMD_DEFAULT_CRITICAL_LIMIT_DBSYNC 	90

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_setDefaultSfdb (void)
{
	char	*env, fname[256], getBuf[256], token[4][64];
	int		i, j, sysCnt=0, groupCnt=0, memberCnt, lNum;
	FILE	*fp;
	//
	// 관리대상 시스템들의 type과 이름을 설정한다.
	//
	env = getenv(IV_HOME);
	sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	if ((fp = fopen(fname,"r")) == NULL) {
		fprintf(stderr,"[fimd_setDefault] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		return -1;
	}
	if ((lNum = conflib_seekSection (fp, "ASSOCIATE_SYSTEMS")) < 0) {
		fprintf(stderr,"[fimd_setDefault] not found section[ASSOCIATE_SYSTEMS] in [%s]\n", fname);
		return -1;
	}

	while ( (fgets (getBuf, sizeof(getBuf), fp) != NULL) &&
			(sysCnt < SYSCONF_MAX_ASSO_SYS_NUM) ) {
		lNum++;
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		sscanf (getBuf, "%s%s%s%s", token[0], token[1], token[2], token[3]);

		strcpy (sfdb->sys[sysCnt].commInfo.name,  token[0]);
		strcpy (sfdb->sys[sysCnt].commInfo.type,  token[2]);
		strcpy (sfdb->sys[sysCnt].commInfo.group, token[3]);
		sprintf(trcBuf,"[fimd_setDefaultSfdb] loading sysName=%s type=%s group=%s\n",
				sfdb->sys[sysCnt].commInfo.name, sfdb->sys[sysCnt].commInfo.type, sfdb->sys[sysCnt].commInfo.group);
		trclib_writeLogErr (FL,trcBuf);
		sysCnt++;
	}
	if(fp)fclose(fp);

	//
	// 각종 장애 등급 기준값에 default를 넣는다.
	//

	//
	// sfdb->sys를 뒤져서 sfdb->group정보를 setting한다.
	//
	// 각 시스템 이름을 차례로 검색하여 sfdb->group에 그룹이름들을 넣는다.
	for (i=0; i<sysCnt; i++) {
		for (j=0; j<groupCnt; j++) {
			if (!strcasecmp (sfdb->sys[i].commInfo.group, sfdb->group[j].name))
				break;
		}
		if (j >= groupCnt) {  // 새로운 group이면
			strcpy (sfdb->group[groupCnt++].name, sfdb->sys[i].commInfo.group);
		}
	}

	// sfdb->group에 각 그룹에 속한 시스템 리스트를 구성한다.
	for (i=0; i<groupCnt; i++) {
		memberCnt = 0;
		for (j=0; j<sysCnt; j++) {
			if (!strcasecmp (sfdb->group[i].name, sfdb->sys[j].commInfo.group)) {
				strcpy (sfdb->group[i].memberName[memberCnt++], sfdb->sys[j].commInfo.name);
				//group에 type을 설정한다.
				strcpy (sfdb->group[i].type, sfdb->sys[j].commInfo.type);  
			}
		}
		sfdb->group[i].memberCnt = memberCnt;
	}

	strcpy(sfdb->active_sys_name, "ACTIVE");

	return 1;

} //----- End of fimd_setDefaultSfdb -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void fimd_backupSfdb2File (void)
{
	int		fd, ret;
	if ((fd = open (sfdbFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[fimd_backupSfdb2File] open fail[%s]; err=%d(%s)\n", sfdbFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
	if ((ret = write (fd, sfdb, SFM_SFDB_SIZE)) < 0) {
		sprintf(trcBuf,"[fimd_backupSfdb2File] write fail[%s]; err=%d(%s)\n", sfdbFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
//	fprintf(stderr, "sfdb_file backup size: %d\n", ret);
	close(fd);

	return;

} //----- End of fimd_backupSfdb2File -----//


//------------------------------------------------------------------------------
// by helca 09.11
void fimd_backupL3pd2File (void)
{
	int	fd, ret;
	
	if ((fd = open (l3pdFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[%s] open fail[%s]; err=%d(%s)\n",__FUNCTION__, l3pdFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
	if ((ret = write (fd, (void*)l3pd, SFM_L3PD_SIZE)) < 0) {
		sprintf(trcBuf,"[%s] write fail[%s]; err=%d(%s)\n",__FUNCTION__, l3pdFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
	//fprintf(stderr, "l3pd backup size: %d\n", ret);
	close(fd);

	return;

} //----- End of fimd_backupL3pd2File -----//
//------------------------------------------------------------------------------
/* by june '09.04.15 */
void fimd_backupSCE2File (void)
{
	int	fd, ret;
	
	if ((fd = open (sceFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[fimd_backupSCE2File] open fail[%s]; err=%d(%s)\n", sceFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
	if ((ret = write (fd, (void*)g_pstSCEInfo, SFM_SCE_SIZE)) < 0) {
		sprintf(trcBuf,"[fimd_backupSCE2File] write fail[%s]; err=%d(%s)\n", sceFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
//	fprintf(stderr, "g_pstSCEInfo backup size: %d\n", ret);
	close(fd);

	return;

} //----- End of fimd_backupSCE2File -----//

void fimd_backupCALLFile (void)
{
	int	fd, ret;
	
	if ((fd = open (callFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[fimd_backupCALLFile] open fail[%s]; err=%d(%s)\n", legFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
	if ((ret = write (fd, (void*)g_pstCALLInfo, SFM_CALL_SIZE)) < 0) {
		sprintf(trcBuf,"[fimd_backupCALLFile] write fail[%s]; err=%d(%s)\n", legFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
	close(fd);

	return;

} //----- End of fimd_backupCALLFile -----//
#if 0
/* added by dcham 2011.05.25 */
void fimd_backupTPSFile (void)
{
	int	fd, ret;
	
	if ((fd = open (tpsFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[fimd_backupCALLFile] open fail[%s]; err=%d(%s)\n", tpsFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
	if ((ret = write (fd, (void*)g_pstCALLInfo, SFM_TPS_SIZE)) < 0) {
		sprintf(trcBuf,"[fimd_backupCALLFile] write fail[%s]; err=%d(%s)\n", tpsFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
	close(fd);

	return;

} //----- End of fimd_backupCALLFile -----//
#endif
//------------------------------------------------------------------------------
void fimd_backupL2sw2File (void)
{
	int		fd, ret;
	
	if ((fd = open (szl2swFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[fimd_backupL3pd2File] open fail[%s]; err=%d(%s)\n", szl2swFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
	if ((ret = write (fd, (void*)g_pstL2Dev, SFM_L2DEV_SIZE)) < 0) {
		sprintf(trcBuf,"[fimd_backupL3pd2File] write fail[%s]; err=%d(%s)\n", szl2swFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
	close(fd);

	return;

} //----- End of fimd_backupL3pd2File -----//



void fimd_backupAudio2File (void)
{
	int		fd;

	if ((fd = open (audioFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[fimd_backupAudio2File] open fail[%s]; err=%d(%s)\n", audioFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}

	if (write (fd, sound_flag, sizeof(int)) < 0) {
		sprintf(trcBuf,"[fimd_backupAudio2File] write fail[%s]; err=%d(%s)\n", audioFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
	close(fd);

	return;

} //----- End of fimd_backupSfdb2File -----//


void init_l3pd_status_shm(void)
{
	int i;
	for(i=0; i<2; i++){
		l3pd->l3ProbeDev[i].cpuInfo.minLimit = 70;
		l3pd->l3ProbeDev[i].cpuInfo.majLimit = 80;
		l3pd->l3ProbeDev[i].cpuInfo.criLimit = 90;

		l3pd->l3ProbeDev[i].cpuInfo.minDurat = FIMD_DEFAULT_DURATION;
		l3pd->l3ProbeDev[i].cpuInfo.majDurat = FIMD_DEFAULT_DURATION;
		l3pd->l3ProbeDev[i].cpuInfo.criDurat = FIMD_DEFAULT_DURATION;

		l3pd->l3ProbeDev[i].memInfo.minLimit = 70;
		l3pd->l3ProbeDev[i].memInfo.majLimit = 80;
		l3pd->l3ProbeDev[i].memInfo.criLimit = 90;

		l3pd->l3ProbeDev[i].memInfo.minDurat = FIMD_DEFAULT_DURATION;
		l3pd->l3ProbeDev[i].memInfo.majDurat = FIMD_DEFAULT_DURATION;
		l3pd->l3ProbeDev[i].memInfo.criDurat = FIMD_DEFAULT_DURATION;
	}
}

/* by june '09.04.15 */
void init_sce_status_shm(void)
{
	int i, j;

	for(i=0; i<2; i++){
		for(j=0; j<3; j++){
			g_pstSCEInfo->SCEDev[i].cpuInfo[j].minLimit = 70;
			g_pstSCEInfo->SCEDev[i].cpuInfo[j].majLimit = 80;
			g_pstSCEInfo->SCEDev[i].cpuInfo[j].criLimit = 90;

			g_pstSCEInfo->SCEDev[i].cpuInfo[j].minDurat = FIMD_DEFAULT_DURATION;
			g_pstSCEInfo->SCEDev[i].cpuInfo[j].majDurat = FIMD_DEFAULT_DURATION;
			g_pstSCEInfo->SCEDev[i].cpuInfo[j].criDurat = FIMD_DEFAULT_DURATION;
			
			g_pstSCEInfo->SCEDev[i].memInfo[j].minLimit = 70;
			g_pstSCEInfo->SCEDev[i].memInfo[j].majLimit = 80;
			g_pstSCEInfo->SCEDev[i].memInfo[j].criLimit = 90;

			g_pstSCEInfo->SCEDev[i].memInfo[j].minDurat = FIMD_DEFAULT_DURATION;
			g_pstSCEInfo->SCEDev[i].memInfo[j].majDurat = FIMD_DEFAULT_DURATION;
			g_pstSCEInfo->SCEDev[i].memInfo[j].criDurat = FIMD_DEFAULT_DURATION;
		}

		g_pstSCEInfo->SCEDev[i].diskInfo.minLimit = 70;
		g_pstSCEInfo->SCEDev[i].diskInfo.majLimit = 80;
		g_pstSCEInfo->SCEDev[i].diskInfo.criLimit = 90;

		g_pstSCEInfo->SCEDev[i].diskInfo.minDurat = FIMD_DEFAULT_DURATION;
		g_pstSCEInfo->SCEDev[i].diskInfo.majDurat = FIMD_DEFAULT_DURATION;
		g_pstSCEInfo->SCEDev[i].diskInfo.criDurat = FIMD_DEFAULT_DURATION;

		/* hjjung */
		g_pstSCEInfo->SCEDev[i].userInfo.minLimit = 70;
		g_pstSCEInfo->SCEDev[i].userInfo.majLimit = 80;
		g_pstSCEInfo->SCEDev[i].userInfo.criLimit = 90;

		g_pstSCEInfo->SCEDev[i].userInfo.minDurat = FIMD_DEFAULT_DURATION;
		g_pstSCEInfo->SCEDev[i].userInfo.majDurat = FIMD_DEFAULT_DURATION;
		g_pstSCEInfo->SCEDev[i].userInfo.criDurat = FIMD_DEFAULT_DURATION;

	}
}

/* hjjung */
void init_leg_status_shm(void)
{
	int i;

	for(i=0; i<2; i++){
		
		g_pstCALLInfo->legInfo[i].minLimit = 70;
		g_pstCALLInfo->legInfo[i].majLimit = 80;
		g_pstCALLInfo->legInfo[i].criLimit = 90;

		g_pstCALLInfo->legInfo[i].minDurat = FIMD_DEFAULT_DURATION;
		g_pstCALLInfo->legInfo[i].majDurat = FIMD_DEFAULT_DURATION;
		g_pstCALLInfo->legInfo[i].criDurat = FIMD_DEFAULT_DURATION;

	}
}

// by helca 08.17
//int alm_lmt_bsd_output(void)
int alm_lmt_dsc_output(void)
{
	int i, sysIndex;
	char fname[256], lineBuf[1024] ;
	char token[25][5];
	char *env;
	FILE *fp;

	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
        return -1;
	}
	
	sprintf(fname, "%s/%s", env,FILE_DSC_LIMIT); // by helca 07.31

	if((fp = fopen(fname, "r+"))== NULL){
		sprintf (trcBuf, "[disprc] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	}
		
	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;
		sscanf(lineBuf, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", 
				token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7], token[8], 
				token[9], token[10], token[11], token[12], token[13], token[14], token[15], token[16], 
			    token[17], token[18], token[19], token[20], token[21], token[22], token[23], token[24]
			    );
		
		if(!strncasecmp(token[0], "DSCM", 4)){
			sysIndex = 0;
		}else if(!strncasecmp(token[0], "SCMA", 4)){
			sysIndex = 1;
		}else if(!strncasecmp(token[0], "SCMB", 4)){
			sysIndex = 2;
		}
				
		sfdb->sys[sysIndex].commInfo.cpuInfo.minLimit = atoi(token[1]);
		sfdb->sys[sysIndex].commInfo.cpuInfo.majLimit = atoi(token[2]);
		sfdb->sys[sysIndex].commInfo.cpuInfo.criLimit = atoi(token[3]);

		sfdb->sys[sysIndex].commInfo.cpuInfo.minDurat = atoi(token[4]);
		sfdb->sys[sysIndex].commInfo.cpuInfo.majDurat = atoi(token[5]);
		sfdb->sys[sysIndex].commInfo.cpuInfo.criDurat = atoi(token[6]);

		sfdb->sys[sysIndex].commInfo.memInfo.minLimit = atoi(token[7]);
		sfdb->sys[sysIndex].commInfo.memInfo.majLimit = atoi(token[8]);
		sfdb->sys[sysIndex].commInfo.memInfo.criLimit = atoi(token[9]);

		sfdb->sys[sysIndex].commInfo.memInfo.minDurat = atoi(token[10]);
		sfdb->sys[sysIndex].commInfo.memInfo.majDurat = atoi(token[11]);
		sfdb->sys[sysIndex].commInfo.memInfo.criDurat = atoi(token[12]);

		for(i=0; i < SFM_MAX_DISK_CNT; i++){
			sfdb->sys[sysIndex].commInfo.diskInfo[i].minLimit = atoi(token[13]);
			sfdb->sys[sysIndex].commInfo.diskInfo[i].majLimit = atoi(token[14]);
			sfdb->sys[sysIndex].commInfo.diskInfo[i].criLimit = atoi(token[15]);
		}

		for(i=0; i < SFM_MAX_QUE_CNT; i++){
			sfdb->sys[sysIndex].commInfo.queInfo[i].minLimit = atoi(token[16]);
			sfdb->sys[sysIndex].commInfo.queInfo[i].majLimit = atoi(token[17]);
			sfdb->sys[sysIndex].commInfo.queInfo[i].criLimit = atoi(token[18]);
		
		}

		for(i=0; i < SFM_MAX_RSRC_LOAD_CNT; i++){
			sfdb->sys[sysIndex].commInfo.rsrcSts[i].minLimit = atoi(token[19]);
			sfdb->sys[sysIndex].commInfo.rsrcSts[i].majLimit = atoi(token[20]);
			sfdb->sys[sysIndex].commInfo.rsrcSts[i].criLimit = atoi(token[21]);

			sfdb->sys[sysIndex].commInfo.rsrcSts[i].minDurat = atoi(token[22]);
			sfdb->sys[sysIndex].commInfo.rsrcSts[i].majDurat = atoi(token[23]);
			sfdb->sys[sysIndex].commInfo.rsrcSts[i].criDurat = atoi(token[24]);
		}
		
	}
	if(fp)fclose(fp);
	return 0;
}

int alm_lmt_pd_output(void)
{
	int  sysIndex;
	char fname[256], lineBuf[1024] ;
	char token[13][5];
	char *env;
	FILE *fp;
	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
        return -1;
	}
	
	sprintf(fname, "%s/%s", env,FILE_TAP_LIMIT); // by helca 07.31

	if((fp = fopen(fname, "r+"))==NULL){
		sprintf (trcBuf, "[disprc] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	
	}
	
	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;
		sscanf(lineBuf, "%s %s %s %s %s %s %s %s %s %s %s %s %s", 
				token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7], token[8], 
				token[9], token[10], token[11], token[12]
			    );
		//if(!strncasecmp(token[0], "PDA", 3)){
		if(!strncasecmp(token[0], "TAPA", 4)){
			sysIndex = 0;
		//}else if(!strncasecmp(token[0], "PDB", 3)){
		}else if(!strncasecmp(token[0], "TAPB", 4)){
			sysIndex = 1;
		}

		l3pd->l3ProbeDev[sysIndex].cpuInfo.minLimit = atoi(token[1]);
		l3pd->l3ProbeDev[sysIndex].cpuInfo.majLimit = atoi(token[2]);
		l3pd->l3ProbeDev[sysIndex].cpuInfo.criLimit = atoi(token[3]);
		
		l3pd->l3ProbeDev[sysIndex].cpuInfo.minDurat = atoi(token[4]);
		l3pd->l3ProbeDev[sysIndex].cpuInfo.majDurat = atoi(token[5]);
		l3pd->l3ProbeDev[sysIndex].cpuInfo.criDurat = atoi(token[6]);
		
		l3pd->l3ProbeDev[sysIndex].memInfo.minLimit = atoi(token[7]);
		l3pd->l3ProbeDev[sysIndex].memInfo.majLimit = atoi(token[8]);
		l3pd->l3ProbeDev[sysIndex].memInfo.criLimit = atoi(token[9]);
		
		l3pd->l3ProbeDev[sysIndex].memInfo.minDurat = atoi(token[10]);
		l3pd->l3ProbeDev[sysIndex].memInfo.majDurat = atoi(token[11]);
		l3pd->l3ProbeDev[sysIndex].memInfo.criDurat = atoi(token[12]);

		
	}
	if(fp)fclose(fp);
	return 0;
}

int alm_lmt_sce_readInfoFile(void)   /*alm_lmt_sce_output -> alm_lmt_sce_readInfoFile rename : sjjeon*/
{
	int i, sysIndex;
	char fname[256], lineBuf[1024] ;
	char token[50][7];
	char sysname[5];
	char *env;
	FILE *fp;
	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
        return -1;
	}
	
	sprintf(fname, "%s/%s", env,FILE_SCE_LIMIT); // by helca 07.31

	if((fp = fopen(fname, "r+"))==NULL){
		sprintf (trcBuf, "[alm_lmt_sce_output] fopen fail[%s]; err=%d(%s)\n"
				, fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	
	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;

		/* hjjung_20100823 42~47 추가 */
		sscanf(lineBuf, "%s" 
						"%s %s %s %s %s %s "
						"%s %s %s %s %s %s "
						"%s %s %s %s %s %s " 
						"%s %s %s %s %s %s "
						"%s %s %s %s %s %s "
						"%s %s %s %s %s %s "
						"%s %s %s %s %s %s "
						"%s %s %s %s %s %s ", 
				sysname,
				token[0],  token[1],  token[2],  token[3],  token[4],  token[5],
				token[6],  token[7],  token[8],  token[9],  token[10], token[11],
				token[12], token[13], token[14], token[15], token[16], token[17],
				token[18], token[19], token[20], token[21], token[22], token[23],
				token[24], token[25], token[26], token[27], token[28], token[29],
				token[30], token[31], token[32], token[33], token[34], token[35],
				token[36], token[37], token[38], token[39], token[40], token[41],
				token[42], token[43], token[44], token[45], token[46], token[47]
			    );
		if(!strncasecmp(sysname, "SCEA", 4)){
			sysIndex = 0;
		}else if(!strncasecmp(sysname, "SCEB", 4)){
			sysIndex = 1;
		}
//printf("sys : %s\n",sysname);
		for(i=0; i<3; i++){
			/* CPU Limit */
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].minLimit = atoi(token[6*i+0]);
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].majLimit = atoi(token[6*i+1]);
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].criLimit = atoi(token[6*i+2]);
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].minDurat = atoi(token[6*i+3]);
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].majDurat = atoi(token[6*i+4]);
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].criDurat = atoi(token[6*i+5]);
/*
			printf("cpuInfo[%d] set : %d, %d, %d, %d, %d, %d\n ",i,
					atoi(token[6*i+0]), atoi(token[6*i+1]), atoi(token[6*i+2]),
					atoi(token[6*i+3]), atoi(token[6*i+4]), atoi(token[6*i+5]));
*/
		}
		for(i=0; i<3; i++){
			/* Memory Limit */
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].minLimit = atoi(token[18+(6*i)+0]);
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].majLimit = atoi(token[18+(6*i)+1]);
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].criLimit = atoi(token[18+(6*i)+2]);
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].minDurat = atoi(token[18+(6*i)+3]);
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].majDurat = atoi(token[18+(6*i)+4]);
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].criDurat = atoi(token[18+(6*i)+5]);
/*
			printf("memInfo[%d] set : %d, %d, %d, %d, %d, %d\n ",i,
					atoi(token[18+(6*i)+0]), atoi(token[18+(6*i)+1]), atoi(token[18+(6*i)+2]),
					atoi(token[18+(6*i)+3]), atoi(token[18+(6*i)+4]), atoi(token[18+(6*i)+5]));
*/
		}

		/* Disk Limit */
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minLimit = atoi(token[36]);
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majLimit = atoi(token[37]);
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criLimit = atoi(token[38]);
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minDurat = atoi(token[39]);
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majDurat = atoi(token[40]);
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criDurat = atoi(token[41]);
/*
			printf("diskInfo[%d] set : %d, %d, %d, %d, %d, %d\n ",
					atoi(token[36]), atoi(token[37]), atoi(token[38]),
					atoi(token[39]), atoi(token[40]), atoi(token[41]));
*/
		/* hjjung_20100823 */
		/* User Limit */
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.minLimit = atoi(token[42]);
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.majLimit = atoi(token[43]);
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.criLimit = atoi(token[44]);
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.minDurat = atoi(token[45]);
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.majDurat = atoi(token[46]);
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.criDurat = atoi(token[47]);

	}
	if(fp)fclose(fp);
	return 0;
}

int alm_lmt_tps_readInfoFile(void) // added by dcham 20110525 for TPS
{
	int  sysIndex;
	char fname[256], lineBuf[1024] ;
	char token[50][7];
	char sysname[5];
	char *env;
	FILE *fp;

	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
        return -1;
	}
	
	sprintf(fname, "%s/%s", env,FILE_TPS_LIMIT);

	if((fp = fopen(fname, "r+"))==NULL){
		sprintf (trcBuf, "[alm_lmt_tps_output] fopen fail[%s]; err=%d(%s)\n"
				, fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	
	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;

		sscanf(lineBuf, "%s" 
						"%s %s %s %s %s %s ", 
				sysname,
				token[0],  token[1],  token[2],  token[3],  token[4],  token[5]
			    );
		if(!strncasecmp(sysname, "SCMA", 5)){
			sysIndex = 0;
		}else if(!strncasecmp(sysname, "SCMB", 5)){
			sysIndex = 1;
		}

		g_pstCALLInfo->tpsInfo[sysIndex].minLimit = atoi(token[0]);
		g_pstCALLInfo->tpsInfo[sysIndex].majLimit = atoi(token[1]);
		g_pstCALLInfo->tpsInfo[sysIndex].criLimit = atoi(token[2]);
		g_pstCALLInfo->tpsInfo[sysIndex].minDurat = atoi(token[3]);
		g_pstCALLInfo->tpsInfo[sysIndex].majDurat = atoi(token[4]);
		g_pstCALLInfo->tpsInfo[sysIndex].criDurat = atoi(token[5]);

	}
	if(fp)fclose(fp);
	return 0;
}

int alm_lmt_sess_readInfoFile(void) 
{
	int  sysIndex;
	char Fname[256], lineBuf[1024];
	char token[50][7];
	char sysname[5];
	char *env;
	FILE *fp;

	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
        return -1;
	}
	
	sprintf(Fname, "%s/%s", env,FILE_SESS_LIMIT);

	if((fp = fopen(Fname, "r+"))==NULL){
		sprintf (trcBuf, "[alm_lmt_sess_output] Sesssion File fopen fail[%s]; err=%d(%s)\n"
				, Fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;

		sscanf(lineBuf, "%s" 
						"%s %s %s %s %s %s ", 
				sysname,
				token[0],  token[1],  token[2],  token[3],  token[4],  token[5]
			    );
		if(!strncasecmp(sysname, "SCMA", 5)){
			sysIndex = 0;
		}else if(!strncasecmp(sysname, "SCMB", 5)){
			sysIndex = 1;
		}

		g_pstCALLInfo->legInfo[sysIndex].minLimit = atoi(token[0]);
		g_pstCALLInfo->legInfo[sysIndex].majLimit = atoi(token[1]);
		g_pstCALLInfo->legInfo[sysIndex].criLimit = atoi(token[2]);
		g_pstCALLInfo->legInfo[sysIndex].minDurat = atoi(token[3]);
		g_pstCALLInfo->legInfo[sysIndex].majDurat = atoi(token[4]);
		g_pstCALLInfo->legInfo[sysIndex].criDurat = atoi(token[5]);

	}
	if(fp)fclose(fp);
        return 0;
}

/*
	limit_dsc_file limit 값을 설정한다. 
 */
int alm_lmt_l2sw_readInfoFile(void) 
{
	int  sysIndex;
	char fname[256], lineBuf[1024] ;
	char token[13][5];
	char *env;
	FILE *fp;
	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
        return -1;
	}
	
	sprintf(fname, "%s/%s", env, FILE_L2SW_LIMIT); // by helca 07.31

	if((fp = fopen(fname, "r+"))==NULL){
		sprintf (trcBuf, "[disprc] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	
	}
	
	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;
		sscanf(lineBuf, "%s %s %s %s %s %s %s %s %s %s %s %s %s"
				, token[0]
				, token[1], token[2], token[3], token[4],  token[5],  token[6]
				, token[7], token[8], token[9], token[10], token[11], token[12]);

		if(!strncasecmp(token[0], "L2SWA", 5)){
			sysIndex = 0;
		}else if(!strncasecmp(token[0], "L2SWB", 5)){
			sysIndex = 1;
		}

		g_pstL2Dev->l2Info[sysIndex].cpuInfo.minLimit = atoi(token[1]);
		g_pstL2Dev->l2Info[sysIndex].cpuInfo.majLimit = atoi(token[2]);
		g_pstL2Dev->l2Info[sysIndex].cpuInfo.criLimit = atoi(token[3]);
		
		g_pstL2Dev->l2Info[sysIndex].cpuInfo.minDurat = atoi(token[4]);
		g_pstL2Dev->l2Info[sysIndex].cpuInfo.majDurat = atoi(token[5]);
		g_pstL2Dev->l2Info[sysIndex].cpuInfo.criDurat = atoi(token[6]);
		
		g_pstL2Dev->l2Info[sysIndex].memInfo.minLimit = atoi(token[7]);
		g_pstL2Dev->l2Info[sysIndex].memInfo.majLimit = atoi(token[8]);
		g_pstL2Dev->l2Info[sysIndex].memInfo.criLimit = atoi(token[9]);
		
		g_pstL2Dev->l2Info[sysIndex].memInfo.minDurat = atoi(token[10]);
		g_pstL2Dev->l2Info[sysIndex].memInfo.majDurat = atoi(token[11]);
		g_pstL2Dev->l2Info[sysIndex].memInfo.criDurat = atoi(token[12]);

		
	}
	if(fp)fclose(fp);
	return 0;
}

int fimd_hwInfo_init(void)
{
        int i;
        char fname[64], lineBuf[32] ;
        char token[2][10];
        char *env;
        FILE *fp;
        
	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
		return -1;
	} 

#if 0
	if ( !strcasecmp(systemModel, "G3")){
		sprintf(fname, "%s/DATA/HW_CONF_G3", env); // by helca 2008.07.21
	}else if ( !strcasecmp(systemModel, "G5")){
		sprintf(fname, "%s/DATA/HW_CONF_G5", env); // by helca 2008.07.21
	} 
#endif

	sprintf(fname, "%s/DATA/HW_INFO_SUN10", env); // by helca 2008.07.21

	if((fp = fopen(fname, "r+"))==NULL){
		sprintf (trcBuf, "[fimd_hwInfo_init] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
 		trclib_writeLogErr (FL,trcBuf);
		return 0;
	}
	
	/* H/W name init */
	/* /DSC/DATA/HW_INFO_SUN10 파일에 등록된 H/W 정보만 체크한다. sjjeon*/
	for( i=0; i< SFM_MAX_HPUX_HW_COM; i++) {
//		sprintf (sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[i].name, "%s%d", "LINK", i);
//		sprintf (sfdb->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[i].name, "%s%d", "LINK", i);
		strcpy(sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[i].name, "noname");
		strcpy(sfdb->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[i].name, "noname");
	}
	
	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;

		sscanf(lineBuf, "%s %s ", token[0], token[1] );

		strcpy(sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[atoi(token[0])].name, token[1]);
		strcpy(sfdb->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[atoi(token[0])].name, token[1]);
//printf("[%s|%s][link] num=%d, name=%s\n",atoi(token[0]), token[1]);

	}
	/*dscm 은 hwlink 정보에 db name 정보를 넣는다. sjjeon*/
	strcpy(sfdb->sys[0].specInfo.u.sms.hpuxHWInfo.hwcom[atoi(token[0])].name, "mysql");
        
	if(fp)fclose(fp);
	return 0;
}

/*
* Logon 통계 감시를 위한 alarm status memory 값을 파일로 남기는 함수
* added by uamyd 20110208
*/
void fimd_backupLogon2File (void)
{
	int	fd, ret;
	
	if ((fd = open (logonFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[fimd_backupLogon2File] open fail[%s]; err=%d(%s)\n", logonFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
	if ((ret = write (fd, (void*)&g_stLogonRate[0], sizeof(SFM_LOGON) *2 *LOG_MOD_CNT)) < 0) { //pre 0,1 - Log On, post 2,3 - Log Out
		sprintf(trcBuf,"[fimd_backupLogon2File] write fail[%s]; err=%d(%s)\n", logonFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
	close(fd);

	return;

} //----- End of fimd_backupLogon2File -----//

/*
* LOGON 성공율 threshold 값을 읽는 부분. 
* added by uamyd 20110210
*/
int alm_lmt_logon_success_rate_readInfoFile(void) 
{
	char fname[256], lineBuf[1024] ;
	char logMod[5], token[4][5];
	char *env;
	int  log_mode;
	FILE *fp;
	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
        return -1;
	}
	
	sprintf(fname, "%s/%s", env, FILE_LOGON_LIMIT);

	if((fp = fopen(fname, "r+"))==NULL){
		sprintf (trcBuf, "[fimd] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	
	}
	
	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;

		if(!strncasecmp(logMod, "LOGON", 5)){
			log_mode = 0;
			sprintf(logMod,"LOGON");
		} else {
			log_mode = 1; //log-out
			sprintf(logMod,"LOGOUT");
		}

		sscanf(lineBuf, "%s %s %s %s %s",
				logMod, token[0] , token[1], token[2], token[3] );

		if(!strncasecmp(token[0], "SCMA", 4)){
			g_stLogonRate[log_mode][0].minLimit = atoi(token[1]);
			g_stLogonRate[log_mode][0].majLimit = atoi(token[2]);
			g_stLogonRate[log_mode][0].criLimit = atoi(token[3]);
		} else {
			//if(!strncasecmp(token[0], "SCMB", 4)){
		
			g_stLogonRate[log_mode][1].minLimit = atoi(token[1]);
			g_stLogonRate[log_mode][1].majLimit = atoi(token[2]);
			g_stLogonRate[log_mode][1].criLimit = atoi(token[3]);
		}
		
	}
	if(fp)fclose(fp);
	return 0;
}

/*
* LOGON 성공율 관리를 위한 구조체 저장 파일 읽는 함수
* added by uamyd 20110210
*/
int fimd_getLogonSuccessRate (void)
{
	int		ret, fd;

	// initial
	g_pstLogonRate = &g_stLogonRate[0][0];
	
	if ((fd = open (logonFileName, O_RDONLY, 0666)) >= 0) {
		lseek(fd,0,0);
		if (( ret = read (fd, (void*)g_pstLogonRate, sizeof(SFM_LOGON) *2 *LOG_MOD_CNT) ) < 0) {
			fprintf(stderr,"[fimd_getLogonSuccessRate] open fail[%s]; err=%d(%s)\n", logonFileName, errno, strerror(errno));
			close(fd);
			return -1;
		}
		close(fd);
	} else {
		
		if (errno != ENOENT) {
			fprintf(stderr,"[fimd_getLogonSuccessRate] open fail.[%s]; err=%d(%s)\n", logonFileName, errno, strerror(errno));
			return -1;
		} else {
			memset( g_pstLogonRate, 0, sizeof(SFM_LOGON) *2 *LOG_MOD_CNT);
		}
	}
	
	return 0;
}

/*
* SM Connetion Status 의 threadhold 값을 읽는 부분. 
* added by uamyd 20110425
*/
int alm_lmt_sm_ch_sts_readInfoFile(void) 
{
	char fname[256], lineBuf[1024] ;
	char token[4][5];
	char *env;
	FILE *fp;
	if ((env = getenv(IV_HOME)) == NULL){
		fprintf(stderr, "[%s] getenv fail\n",__FUNCTION__);
        return -1;
	}
	
	sprintf(fname, "%s/%s", env, FILE_SMCONN_LIMIT);

	if((fp = fopen(fname, "r+"))==NULL){
		sprintf (trcBuf, "[fimd] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	
	}
	
	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
		memset(token, 0x00, sizeof(token));
		if(lineBuf[0] == '#')
			continue;

		sscanf(lineBuf, "%s %s %s %s",
				token[0] , token[1], token[2], token[3] );

		if(!strncasecmp(token[0], "SCMA", 4)){
			sfdb->sys[1].commInfo.smChSts.minLimit = atoi(token[1]);
			sfdb->sys[1].commInfo.smChSts.majLimit = atoi(token[2]);
			sfdb->sys[1].commInfo.smChSts.criLimit = atoi(token[3]);
		} else {
		
			sfdb->sys[2].commInfo.smChSts.minLimit = atoi(token[1]);
			sfdb->sys[2].commInfo.smChSts.majLimit = atoi(token[2]);
			sfdb->sys[2].commInfo.smChSts.criLimit = atoi(token[3]);
		}
		
	}
	if(fp)fclose(fp);
	return 0;
}

void fimd_backupSMChSts2File (void)
{
	int	 fd, ret;

	if ((fd = open (smChFileName,  O_RDWR | O_CREAT, 0666)) < 0) {
		sprintf(trcBuf,"[fimd_backupSMChSts2File] open fail[%s]; err=%d(%s)\n", smChFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	}
	
	if ((ret = write (fd, (void*)&sfdb->sys[1].commInfo.smChSts, sizeof(SFM_SMChInfo))) < 0) {
		sprintf(trcBuf,"[fimd_backupSMChSts2File] write.SCMA fail[%s]; err=%d(%s)\n", smChFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
	if ((ret = write (fd, (void*)&sfdb->sys[2].commInfo.smChSts, sizeof(SFM_SMChInfo))) < 0) {
		sprintf(trcBuf,"[fimd_backupSMChSts2File] write.SCMB fail[%s]; err=%d(%s)\n", smChFileName, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
	}
	close(fd);

	return;

}

int fimd_getSMChSts (void)
{
    int  ret, fd;

    if ((fd = open (smChFileName, O_RDONLY, 0666)) >= 0) {
        lseek(fd,0,0);
		if (( ret = read (fd, (void*)&sfdb->sys[1].commInfo.smChSts, sizeof(SFM_SMChInfo)) ) < 0) {
			fprintf(stderr,"[fimd_getSMChSts] open fail(SCMA)[%s]; err=%d(%s)\n", smChFileName, errno, strerror(errno));
            memset( &sfdb->sys[1].commInfo.smChSts, 0, sizeof(SFM_SMChInfo)); //SCMA
            memset( &sfdb->sys[2].commInfo.smChSts, 0, sizeof(SFM_SMChInfo)); //SCMB
		}
		if (( ret = read (fd, (void*)&sfdb->sys[2].commInfo.smChSts, sizeof(SFM_SMChInfo)) ) < 0) {
			fprintf(stderr,"[fimd_getSMChSts] open fail(SCMB)[%s]; err=%d(%s)\n", smChFileName, errno, strerror(errno));
            memset( &sfdb->sys[2].commInfo.smChSts, 0, sizeof(SFM_SMChInfo)); //SCMB
		}
        if( fd > 0 ) close(fd);
    } else {

        if (errno != ENOENT) {
            fprintf(stderr,"[fimd_getSMChSts] open fail.[%s]; err=%d(%s)\n", smChFileName, errno, strerror(errno));
            return -1;
        } else {
            memset( &sfdb->sys[1].commInfo.smChSts, 0, sizeof(SFM_SMChInfo)); //SCMA
            memset( &sfdb->sys[2].commInfo.smChSts, 0, sizeof(SFM_SMChInfo)); //SCMB
        }
    }

    return 0;
}
