#include "samd.h"
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <signal.h>
#include <conf.h>
extern char     trcBuf[4096], trcTmp[1024];

#pragma pack(1)
typedef struct _idx{
	int	 linkNum;
    char idxName[20];
    char idxValue[20];
    char idxValue2[20];
} st_idx, *pst_idx;
#pragma pack()



extern int		ixpcQID, samdQID, mcdmQID;
extern int		trcFlag, trcLogFlag;
extern long		oldSumOfPIDs, newSumOfPIDs;
extern char		l_sysconf[256];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern SFM_SysCommMsgType	*loc_sadb;
extern SAMD_ProcessInfo		ProcessInfo[SYSCONF_MAX_APPL_NUM];
extern T_keepalive	*keepalive;
extern int          trcLogId, trcErrLogId;
extern SFM_sfdb		*sfdb;
extern SFM_L3PD		*l3pd;
extern SFM_SCE      *g_pstSCEInfo;

void sendWatchdogMsg2COND (int procIdx);
//void sendApplySCERst2COND ();
void sendApplySCERst2COND (int stsNum);
//int mmcReqSyncRuleFile(int sysIdx, int rule);
int mmcReqSyncRuleFile(int sysIdx, int rule, int bkOpt);

/* DB Backup */
// db 삭제 extern DB_INFO_t    g_stDBInfo;

extern void handleChildProcess (void);
int findIndex(char *syntax, char *idex);
extern int findCaseIndex(char *syntax, char *idex);
//extern int  samd_send2mmcd (char*, char, int);

int runSetRuleSce(int index, char *argSts, char *argFName, int flag);
void getCurTimeStr(char *outTimeStr);
void shellRunDisSceMode(IxpcQMsgType *rxIxpcMsg, int index);
int getShellInfo(char *path, st_idx *stidx, int stidxCnt);
void doDisSceMode (IxpcQMsgType *rxIxpcMsg);
void doReloadSce(IxpcQMsgType *rxIxpcMsg);
void doSetSceMode(IxpcQMsgType *rxIxpcMsg);
void shellRunReloadSce(IxpcQMsgType *rxIxpcMsg, int index);
void shellRunSetSceMode(IxpcQMsgType *rxIxpcMsg, int sysType, int mode, int stat);
int mmc_makePDStsOutputMsg (char *trcBuf_loc, int devIndex, char *argSys);
//------------------------------------------------------------------------------
// /proc디렉토리 밑의 모든 프로세스 ID의 합을 구한다.
//------------------------------------------------------------------------------
long getSumOfPIDs (void)
{
	DIR	*dirp;
	long	totalPid=0;
	struct dirent   *direntp;
	char	trcBuf_loc[1024];

	if ((dirp = opendir (PROC_PATH)) == NULL) {
		sprintf(trcBuf_loc,"[getSumOfPIDs] opendir err\n");
		trclib_writeLogErr (FL,trcBuf_loc);
		return -1;
	}

	while ((direntp = readdir(dirp)) != NULL ) {
		totalPid += strtol (direntp->d_name, 0, 10);
	}
	closedir (dirp);

	return totalPid;
}


//------------------------------------------------------------------------------
// /proc를 검색하여 관리대상 프로세스들의 상태를 ProcessInfo에 저장한다.
//------------------------------------------------------------------------------
int getProcessStatus(void)
{
	int	i, fd, listID;
	DIR	*dirp;
	char	pathName[256];
	struct dirent   *direntp;
	prpsinfo_t      psInfo;
	char	trcBuf_loc[1024];

	if ((dirp = opendir(PROC_PATH)) == (DIR *)NULL) {
		sprintf(trcBuf_loc,"[getProcessStatus] opendir err\n");
		trclib_writeLogErr (FL,trcBuf_loc);
		return -1;
	}

	/** 모든 상태를 dead로 바꾼다 **/
	for (i=0; i < loc_sadb->processCount; i++ ){
		loc_sadb->loc_process_sts[i].status = SFM_STATUS_DEAD;
		ProcessInfo[i].new_status = SFM_STATUS_DEAD;
		ProcessInfo[i].runCnt = 0;
		ProcessInfo[i].pid = -1;
	}

	while ((direntp = readdir (dirp)) != NULL)
	{
		if ( !strcasecmp (direntp->d_name, PARENT_PATH) ||
			 !strcasecmp (direntp->d_name, HOME_PATH ))
			continue;

		sprintf (pathName, "%s/%s", PROC_PATH, direntp->d_name);

		if ((fd = open(pathName, O_RDONLY)) < 0) {
			if (errno != EACCES && errno != ENOENT) { /* Permission denied 는 skip하기 위해 */
				sprintf(trcBuf_loc,"[getProcessStatus] open fail; (%s) err=%d[%s]\n", pathName, errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf_loc);
			}
			continue;
		}

		if ( ioctl (fd, PIOCPSINFO, &psInfo) < 0 ) {
			sprintf(trcBuf_loc,"[getProcessStatus] pathName=%s ioctl err=%d[%s]\n", pathName, errno, strerror(errno));
			trclib_writeLogErr (FL,trcBuf_loc);
			close (fd);
			continue;
		}
		close (fd);

		if (strlen(psInfo.pr_psargs) == 0)
			continue;

		/* 관리대상 프로세스인지 확인하고, 내부적으로 관리하는 해당 프로세스의 index가
		*	return한다.
		*/
		if ((listID = getListID (psInfo.pr_psargs)) < 0)
			continue;

		loc_sadb->loc_process_sts[listID].status = SFM_STATUS_ALIVE;

		ProcessInfo[listID].new_status = SFM_STATUS_ALIVE;
		//startprc 에서 메세지ㅈ� 수신한 경우, mask 처리하도록 수정. by uamyd 20110504
		//ProcessInfo[listID].mask = SFM_STATUS_ALIVE; // killprc로 죽인놈이 살아나면 mask가 자동으로 풀린다.
		ProcessInfo[listID].runCnt++;
		ProcessInfo[listID].pid = psInfo.pr_pid;
		strftime (ProcessInfo[listID].startTime, 32, "%m-%d %H:%M",
		localtime((time_t*)&(psInfo.pr_start.tv_sec)));
		loc_sadb->loc_process_sts[listID].pid = psInfo.pr_pid;
		loc_sadb->loc_process_sts[listID].uptime = psInfo.pr_start.tv_sec;
//		loc_sadb->loc_process_sts[listID].uptime = htons(psInfo.pr_start.tv_sec);
//		printf("uptime: %d\n", psInfo.pr_start.tv_sec);
	}
	closedir ( dirp );

	return 1;
}


//------------------------------------------------------------------------------
// 관리대상 프로세스인지 확인하고 해당 프로세스의 index를 return한다.
//------------------------------------------------------------------------------
int getListID (char *name)
{
	int     i;

	for (i=0; i<loc_sadb->processCount; i++) {
		if (!strcasecmp (name, loc_sadb->loc_process_sts[i].processName) ){
			return i;
		}
	}
	return -1;
}


//------------------------------------------------------------------------------
// 프로세스의 상태을 확인하여 죽은 상태로 변경된 놈을 자동으로 기동시킨다.
// - killprc로 죽여 mask로 설정된 경우는 살리지 않는다.
//------------------------------------------------------------------------------
void checkProcessStatus_OLD(void)
{
	int     i, restartCnt;
	char	trcBuf_loc[1024];

	/* 전체 pid의 합이 같으면 한개씩 확인할 필요가 없다.
	*/
	if ((newSumOfPIDs = getSumOfPIDs()) < 0)
		return;
	if ( newSumOfPIDs == oldSumOfPIDs )
		return;

	oldSumOfPIDs = newSumOfPIDs;

	/* 관리대상 프로세스들의 현재 상태 정보를 ProcessInfo table과
	*	loc_sadb->loc_process_sts[i].status에 넣는다.
	*/
	if (getProcessStatus() < 0)
		return;

	/* killprc 에 의해 종료되는 경우, kill 전에 mask signal을 전송한다.
	*  이를 해결하기 위해 이 위치(재기동 프로세스 감시 모듈)에서 msgq를 읽는다.
	*  added by uamyd 20110427
	*/
	//HandleRxMsg();

	/*
	* dead 상태로 변경된 놈들에 대해 killprc로 죽인 경우가 아니면 auto_restart 시킨다.
	*/
	for (i=0, restartCnt = 0; i<loc_sadb->processCount; i++)
	{
		if (ProcessInfo[i].new_status == SFM_STATUS_ALIVE || // 살아있는 경우
			ProcessInfo[i].new_status == ProcessInfo[i].old_status || // 변경 없는 경우
			ProcessInfo[i].mask == SFM_ALM_MASKED) // killprc로 죽인 경우
		{
			ProcessInfo[i].old_status = ProcessInfo[i].new_status;
			continue;
		}

        	logPrint (trcLogId,FL,"check down process %s is dead\n", ProcessInfo[i].procName);
        	logPrint (trcErrLogId,FL,"check down process %s is dead\n", ProcessInfo[i].procName);

		if (runProcess (i, AUTO_FLOW) < 0) {
			sprintf(trcBuf_loc,"process auto restart fail\n");
			trclib_writeLogErr (FL,trcBuf_loc);
		}

		sprintf(trcBuf_loc,"process auto restart %s\n", ProcessInfo[i].procName);
		trclib_writeLogErr (FL,trcBuf_loc);
		restartCnt++;
	}

	/* 단 한건응鵑瓚繭捉� Process restart가 발생한다면, FIMD로 noti를 날린다. by uamyd 20110422 */
	if( restartCnt ){
		report_sadb2FIMD();
		getProcessStatus();
	}
}

void checkProcessStatus(void)
{
	int     i, flag;
	char	trcBuf_loc[1024];

	/* 전체 pid의 합이 같으면 한개씩 확인할 필요가 없다.
	*/
	if ((newSumOfPIDs = getSumOfPIDs()) < 0)
		return;
	if ( newSumOfPIDs == oldSumOfPIDs )
		return;

	oldSumOfPIDs = newSumOfPIDs;

	/* 관리대상 프로세스들의 현재 상태 정보를 ProcessInfo table과
	*	loc_sadb->loc_process_sts[i].status에 넣는다.
	*/
	if (getProcessStatus() < 0)
		return;

	for (i=0, flag = 0; i<loc_sadb->processCount; i++)
	{
		if (ProcessInfo[i].new_status == SFM_STATUS_ALIVE || // 살아있는 경우
			ProcessInfo[i].new_status == ProcessInfo[i].old_status ) // 변경 없는 경우
		{
			ProcessInfo[i].old_status = ProcessInfo[i].new_status;
			continue;
		}
		else{
			flag++;
			logPrint (trcErrLogId,FL,"check process %s(%d:%d) alive -> dead\n", 
						ProcessInfo[i].procName, ProcessInfo[i].new_status, loc_sadb->loc_process_sts[i].status);
		}
	}

	HandleRxMsg();
	if( flag ){

		for (i=0; i<loc_sadb->processCount; i++)
		{
			if (ProcessInfo[i].new_status != SFM_STATUS_ALIVE && // 살아있는 경우
				ProcessInfo[i].new_status != ProcessInfo[i].old_status && // 변경 없는 경우
				ProcessInfo[i].mask != SFM_ALM_MASKED) // killprc로 죽인 경우
			{

				logPrint (trcLogId,FL,"check down process %s is dead\n", ProcessInfo[i].procName);
				logPrint (trcErrLogId,FL,"check down process %s is dead\n", ProcessInfo[i].procName);

				if (runProcess (i, AUTO_FLOW) < 0) {
					sprintf(trcBuf_loc,"process auto restart fail %s\n", ProcessInfo[i].procName);
				}else{
					sprintf(trcBuf_loc,"process auto restarted %s\n", ProcessInfo[i].procName);
				}
				trclib_writeLogErr (FL,trcBuf_loc);
			}
		}

		report_sadb2FIMD();
		getProcessStatus();
	}
}

/** 세어드 메모리의 와치도그 기능을 위해 **/
void checkKeepAlive (void)
{
	int     i;
	char	trcBuf_loc[1024];

	for ( i=0; i<loc_sadb->processCount; i++)
	{
		if (!strcasecmp(loc_sadb->loc_process_sts[i].processName, myAppName))
			continue;
		if (loc_sadb->loc_process_sts[i].status != SFM_STATUS_ALIVE)
			continue;
		if (keepalive->cnt[i] == keepalive->oldcnt[i]) {
			(keepalive->retry[i])++;
		} else {
			keepalive->retry[i] = 0;
		}
		keepalive->oldcnt[i] = keepalive->cnt[i];

		if (keepalive->retry[i] < KEEPALIVE_CHECK_TIME)
			continue;

		if (strcasecmp(loc_sadb->loc_process_sts[i].processName, "UDRCOL")){	
			sprintf(trcBuf_loc,"[checkKeepAlive] watchdog kill; proc= %s`\n",
				loc_sadb->loc_process_sts[i].processName);
			trclib_writeLogErr (FL,trcBuf_loc);
		}	
		
		/* UDRCOL은 그 특성상 checkKeepAlive에서 Auto kill, restart가 맞지 않아 제외한다.*/	
		/* 가져와야 할 백업 Data가 많으면 KEEPALIVE_CHECK_TIME안에 처리 할 Data list를 못가져 온다. */	
		if (strcasecmp(loc_sadb->loc_process_sts[i].processName, "UDRCOL")){
			kill (ProcessInfo[i].pid, SIGKILL);
			keepalive->retry[i] = -10;
			sendWatchdogMsg2COND (i);

		}
		
		// OMP-FIMD에서 장애메시지를 만들수 있도록 하기 위해 프로세스 상태정보를
		//	다시한번 읽어 ProcessInfo에 setting한 후 즉시 report한다.
		//
		getProcessStatus ();
		report_sadb2FIMD ();
		report_l3pd2FIMD ();
		report_SCE2FIMD ();
		
	}

	return;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int runProcess (int procIdx, int isAuto)
{
	int         i, pid;
	struct stat	statBuff;
	char	trcBuf_loc[1024];

	/* 혹시 이미 다시 살아났는지 한번더 확인한다.
	*/
	if ((pid = getProcessID (ProcessInfo[procIdx].procName)) > 1)
		return pid;
   
	/* 기동시킬 파일이 있는지 확인한다.
	*/
	if (stat (ProcessInfo[procIdx].exeFile, &statBuff) < 0) {
		sprintf(trcBuf_loc,"[runProcess] stat fail=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf_loc);
        return -1;
	}

	//sigignore (SIGCHLD);
	signal(SIGCHLD, (void *)handleChildProcess);
	if ((pid = fork()) < 0)
		return -1;

	if ( pid == 0) {
		/* child process는 execl로 기동 시킨 후 종료
		*/
		freopen ("/dev/null", "w", stdout);
		execl (ProcessInfo[procIdx].exeFile, ProcessInfo[procIdx].procName, NULL);
		exit(0);
	}

	/* auto_restart인 경우에는 새로 기동시킨 놈의 pid가 필요 없다.
	*/
	if (isAuto == AUTO_FLOW){
        logPrint (trcLogId,FL,"auto startup process %s\n", ProcessInfo[procIdx].procName);
        logPrint (trcErrLogId,FL,"auto startup process %s\n", ProcessInfo[procIdx].procName);
        return pid;
    }

	/* start-prc 명령인 경우에는 새로 기동시킨 놈의 pid를 결과 메시지에 출력하기
	*	위해 필요하므로 pid를 찾아 return한다.
	*	- 자신의 child 프로세스는 execl로 실행시키고 종료되었으므로 위에서 fork()에서
	*		넘어온 pid는 이미 사라졌다.
	*/
	for (i=0; i<5; i++) {
		commlib_microSleep (100000);
		if ((pid = getProcessID (ProcessInfo[procIdx].procName)) > 1){
            logPrint (trcLogId,FL,"[runProcess] mmc startup process %s\n", ProcessInfo[procIdx].procName);
            logPrint (trcErrLogId,FL,"[runProcess] mmc startup process %s\n", ProcessInfo[procIdx].procName);
			return pid;
		}
	}
	return -1;
}


//------------------------------------------------------------------------------
// 특정 프로세스의 PID를 찾는다.
//------------------------------------------------------------------------------
int getProcessID (char *procName)
{
	int	fd;
	DIR	*dirp;
	char	pathName[256];
	struct dirent   *direntp;
	prpsinfo_t      psInfo;
	char	trcBuf_loc[1024];

	if ((dirp = opendir(PROC_PATH)) == NULL) {
		sprintf(trcBuf_loc,"[getProcessID] opendir fail; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf_loc);
		return -1;
	}

	while ((direntp = readdir (dirp)) != NULL)
	{
		if ( !strcasecmp ( direntp->d_name, PARENT_PATH) ||
			 !strcasecmp ( direntp->d_name, HOME_PATH ) ) continue;

		sprintf (pathName, "%s/%s", PROC_PATH, direntp->d_name);

		if ( (fd = open ( pathName, O_RDONLY )) < 0 )
			continue;
		if ( ioctl ( fd, PIOCPSINFO, &psInfo ) < 0) {
			close ( fd );
			continue;
		}
		close (fd);

		if (!strcasecmp (procName, psInfo.pr_psargs)) {
#ifdef DEBUG
			fprintf (stderr,"[SFMD:GET_PROC_ID] pid = %ld\n",psInfo.pr_pid);
#endif
			closedir (dirp);
			return psInfo.pr_pid;
		}
	}
	closedir (dirp);

	return -1;
}



void doDisPrcSts(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	int		i, ret;
	int		alive=0, dead=0;
	char	argSys[COMM_MAX_NAME_LEN];
	char    tmpbuf[128], *pVer;
	char	trcBuf_loc[1024];
	char	trcTmp_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input paramters */
	strcpy (argSys, rxReqMsg->head.para[0].paraVal);

	if(strlen(argSys) == 0)
		strcpy(argSys, mySysName);

	for(i=0; i<strlen(argSys); i++) argSys[i] = toupper(argSys[i]);

	if (strcasecmp(mySysName, argSys)) { /* 내시스템과 일치하지 않으면 */
		for (i=0 ; i < loc_sadb->lanCount; i++) { /* 시스템 이름과 일치하는 것이 있으면 */
			if(!strncasecmp(loc_sadb->loc_lan_sts[i].target_SYSName, argSys, strlen(argSys))) {
				strcpy(rxIxpcMsg->head.dstSysName, argSys);
				MMCReqBypassSnd(rxIxpcMsg);
				return;
			}
		}
		sprintf(trcBuf_loc, "\n    RESULT = FAIL\n    REASON = UNRECOGNIZED SYSTEM\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	} else {
		if ((ret = getProcessStatus()) < 0 ) {
			sprintf(trcBuf_loc, "\n    RESULT = FAIL\n    REASON = ERR GET PROCESS STATUS\n\n");
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			return;
		}
		sprintf(trcBuf_loc, "    SYSTEM = %s\n    RESULT = SUCCESS\n", mySysName);
		strcat(trcBuf_loc, "    ==============================================\n");
		sprintf(trcTmp_loc, "    Process  PID    STATUS    START-TIME   VERSION\n");
		strcat(trcBuf_loc, trcTmp_loc);
		strcat(trcBuf_loc, "    ==============================================\n");
		for ( i = 0 ; i < loc_sadb->processCount ; i++) {
			strcpy(tmpbuf, ProcessInfo[i].procName);
			strtoupper(tmpbuf);
			pVer = (char *)get_proc_version(ProcessInfo[i].procName);
			if(!pVer)
				pVer = "UNKNOWN";
			if ( loc_sadb->loc_process_sts[i].status == SFM_STATUS_ALIVE ) {
				if (ProcessInfo[i].runCnt == 1 ) {
					 sprintf(trcTmp_loc,"    %-8s %-4d   ALIVE     %-12s %-10s\n",
									tmpbuf, (int)ProcessInfo[i].pid, ProcessInfo[i].startTime, (char*)pVer);
					strcat(trcBuf_loc, trcTmp_loc);
				} else {
					sprintf(trcTmp_loc,"    %-8s %-4d   ALIVE(%d)  %-12s %-10s\n",
									tmpbuf, (int)ProcessInfo[i].pid, ProcessInfo[i].runCnt, ProcessInfo[i].startTime, (char*)pVer);
					strcat(trcBuf_loc, trcTmp_loc);
				}
				alive += ProcessInfo[i].runCnt;
			} else {
				sprintf(trcTmp_loc,"    %-8s -      DEAD      -            %-10s\n", tmpbuf,(char*) pVer);
				strcat(trcBuf_loc, trcTmp_loc);
				dead++;
			}
		}
		strcat(trcBuf_loc, "    ---------------------------------------------\n");
		sprintf(trcTmp_loc, "    TOTAL:%d (ALIVE:%d, DEAD:%d)\n", alive+dead, alive, dead);
		strcat(trcBuf_loc, trcTmp_loc);
		strcat(trcBuf_loc, "    =============================================\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
	}
}


void doKillPrc(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	int		i, procIdx;
	char	argSysName[COMM_MAX_NAME_LEN], argProcName[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[1024];
	char	trcTmp_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input parameters
	*/
	strcpy (argSysName, rxReqMsg->head.para[0].paraVal);
	strcpy (argProcName, rxReqMsg->head.para[1].paraVal);

	/* 대문자로 출력하기 위해 
	*/
	for(i=0 ; i<strlen(argSysName); i++)  argSysName[i]  = toupper(argSysName[i]);
	for(i=0 ; i<strlen(argProcName); i++) argProcName[i] = toupper(argProcName[i]);

	/* 자신의 시스템 것이 아니면 해당 시스템으로 bypass한다.
	*/
	if (strcasecmp(mySysName, argSysName)) {
		for (i=0 ; i < loc_sadb->lanCount ; i++) { /* 시스템 이름과 일치하는 것이 있으면 */
			/* modify by mnpark - 20040112 */
			if(!strncasecmp(loc_sadb->loc_lan_sts[i].target_SYSName, argSysName, strlen(argSysName))) {
				strcpy(rxIxpcMsg->head.dstSysName, argSysName);
				MMCReqBypassSnd(rxIxpcMsg);
				return;
			}
		}
		sprintf(trcBuf_loc, "\n      RESULT = FAIL\n      REASON = UNRECOGNIZED SYSTEM\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}


	/* 등록되지 않은 프로세스인지 확인한다.
	*/
	for ( i=0 ; i < loc_sadb->processCount ; i++) {
		if(!strcasecmp(ProcessInfo[i].procName, argProcName)) {
			procIdx = i;
			break;
		}
	}
	if (i >= loc_sadb->processCount) {
		sprintf (trcBuf_loc, "\n     RESULT = FAIL\n      REASON = NOT EXIST APPNAME\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	/* samd, ixpc, mmcd는 죽이지 못한다.
	*/
	if (!strcasecmp (argProcName, "SAMD") ||
		!strcasecmp (argProcName, "IXPC") ||
		!strcasecmp (argProcName, "MMCD")) {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON = CAN'T KILL PROCESS\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	/* 이미 죽어 있는지 확인한다.
	*/
	if (ProcessInfo[procIdx].pid < 2 || ProcessInfo[procIdx].mask == SFM_ALM_MASKED) {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON = NOT RUNNING STATE\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	/*
	*/
	if (kill (ProcessInfo[procIdx].pid, SIGTERM) < 0 ) {
		if (kill (ProcessInfo[procIdx].pid, 9) < 0 ) {
			sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON = CAN'T KILL(%s)\n\n", strerror(errno));
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			return;
		}
	}

	ProcessInfo[procIdx].mask = SFM_ALM_MASKED;

	sprintf (trcBuf_loc, "\n      RESULT = SUCCESS\n\n");
	sprintf (trcTmp_loc, "      SYSTEM  = %s\n      APPNAME = %s\n\n", argSysName, argProcName);
	strcat (trcBuf_loc, trcTmp_loc);
	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
	return;
}


void doRunPrc(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	int		i, procIdx;
	char		argSysName[COMM_MAX_NAME_LEN], argProcName[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[1024];
	char	trcTmp_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input parameters
	*/
	strcpy (argSysName, rxReqMsg->head.para[0].paraVal);
	strcpy (argProcName, rxReqMsg->head.para[1].paraVal);

	/* 대문자로 출력하기 위해 
	*/
	for(i=0 ; i<strlen(argSysName); i++)  argSysName[i]  = toupper(argSysName[i]);
	for(i=0 ; i<strlen(argProcName); i++) argProcName[i] = toupper(argProcName[i]);

	/* 자신의 시스템 것이 아니면 해당 시스템으로 bypass한다.
	*/
	if (strcasecmp(mySysName, argSysName)) {
		for (i=0 ; i < loc_sadb->lanCount ; i++) { /* 시스템 이름과 일치하는 것이 있으면 */
			if(!strncasecmp(loc_sadb->loc_lan_sts[i].target_SYSName, argSysName, strlen(argSysName))) {
				strcpy(rxIxpcMsg->head.dstSysName, argSysName);
				MMCReqBypassSnd(rxIxpcMsg);
				return;
			}
		}
		sprintf(trcBuf_loc, "\n      RESULT = FAIL\n      REASON = UNRECOGNIZED SYSTEM\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	/* 등록되지 않은 프로세스인지 확인한다.
	*/
	for ( i=0 ; i < loc_sadb->processCount ; i++) {
		if(!strcasecmp(ProcessInfo[i].procName, argProcName)) {
			procIdx = i;
			break;
		}
	}
	if (i >= loc_sadb->processCount) {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON = NOT EXIST APPNAME\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}


	/* 이미 살아 있는지 확인한다.
	*/
	if (ProcessInfo[procIdx].pid > 0) {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON = ALREADY RUNNING STATE\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	/*
	*/
	if (runProcess (procIdx, NORMAL_FLOW) < 0 ) {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON = RUN FAIL\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	sprintf (trcBuf_loc, "\n      RESULT = SUCCESS\n\n");
	sprintf (trcTmp_loc, "      SYSTEM  = %s\n      APPNAME = %s\n\n", argSysName, argProcName);
	strcat (trcBuf_loc, trcTmp_loc);
	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);

	return;

}


void doDisLoadSts(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	int		i,j, queUsage;
	//char	argSys[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[1024*8];
	char	trcTmp_loc[1024*4];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	i = sprintf(trcBuf_loc, "    =========================================================================\n");
	sprintf(trcBuf_loc+i, "    SYSTEM = %s\n    RESULT = SUCCESS\n", mySysName);
	strcat(trcBuf_loc, "    ========================================================================\n"); 	
	for ( i = 0 ; i < loc_sadb->cpuCount ; i++) {
		sprintf(trcTmp_loc, "    CPU[%d] USED = %d.%d%%\n", i, (loc_sadb->cpu_usage[i]/10), (loc_sadb->cpu_usage[i]%10));
		strcat(trcBuf_loc, trcTmp_loc);
	}
	sprintf(trcTmp_loc, "    MEM USED    = %d.%d%%\n", (loc_sadb->mem_usage/10), (loc_sadb->mem_usage%10));
	strcat(trcBuf_loc, trcTmp_loc);
	for ( i = 0 ; i < loc_sadb->diskCount ; i++) {
		//sjjeon
		sprintf(trcTmp_loc, "    DISK [ %-10s] USED = %d.%d%%\n", 
				loc_sadb->loc_disk_sts[i].diskName, 
				(loc_sadb->loc_disk_sts[i].disk_usage/10), (loc_sadb->loc_disk_sts[i].disk_usage%10));
		strcat(trcBuf_loc, trcTmp_loc);
	}

	for ( i = 0 ; i < loc_sadb->queCount ; i++) {
		if((!loc_sadb->loc_que_sts[i].cBYTES)||(!loc_sadb->loc_que_sts[i].qBYTES))
			queUsage = 0;
		else
			queUsage = ((loc_sadb->loc_que_sts[i].cBYTES*100)/loc_sadb->loc_que_sts[i].qBYTES);
		sprintf(trcTmp_loc, "    QUEUE[ %-10s] LOAD = [%3d%%] , NUM = %5d , USED = %5d/%5d(Bytes)\n",
				loc_sadb->loc_que_sts[i].qNAME,
				queUsage,	
				loc_sadb->loc_que_sts[i].qNUM,
				loc_sadb->loc_que_sts[i].cBYTES,
				loc_sadb->loc_que_sts[i].qBYTES
			   );
		strcat(trcBuf_loc, trcTmp_loc);
	}			

	// SCE 상태 정보를 넣는다. sjjeon 20090604
	// SCE 정보는 시스템으로 mmc를 확인할 수 없는 관계로 OMP에서 처리 하기로 한다.
	for(j=0;j<MAX_SCE_DEV_NUM;j++)
	{
		strcat(trcBuf_loc, "    =========================================================================\n");
		if(j==0) 
			sprintf(trcTmp_loc, "    SYSTEM = SCEA\n");
		else
			sprintf(trcTmp_loc, "    SYSTEM = SCEB\n");
		strcat(trcTmp_loc, "    =========================================================================\n"); 	
		strcat(trcBuf_loc, trcTmp_loc);

		for(i=0; i<MAX_SCE_CPU_CNT; i++){
			sprintf(trcTmp_loc, "    CPU[%d] USED = %d %%\n", i,  g_pstSCEInfo->SCEDev[j].cpuInfo[i].usage);
			strcat(trcBuf_loc, trcTmp_loc);
		}

		for(i=0; i<MAX_SCE_MEM_CNT; i++){
			sprintf(trcTmp_loc, "    MEM[%d] USED = %d %%\n", i,  g_pstSCEInfo->SCEDev[j].memInfo[i].usage);
			strcat(trcBuf_loc, trcTmp_loc);
		}

		sprintf(trcTmp_loc, "    DISK[%d] USED = %d %%\n", i,  g_pstSCEInfo->SCEDev[j].diskInfo.usage);
		strcat(trcBuf_loc, trcTmp_loc);
		
		/* hjjung_20100823 */
		sprintf(trcTmp_loc, "    USER[%d] USED = %d %%\n", i,  g_pstSCEInfo->SCEDev[j].userInfo.num);
		strcat(trcBuf_loc, trcTmp_loc);
	}

#ifdef DEBUG
sprintf(trcBuf,"[%s] DIS_LOAD_STS RESULT >>> \n%s\n", __FUNCTION__, trcBuf_loc);
trclib_writeLogErr (FL,trcBuf);	
#endif
	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);

}

void doDisLanSts (IxpcQMsgType *rxIxpcMsg)
{
	int		i, j;
	MMLReqMsgType   *rxReqMsg;
	char    argSys[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[1024];
	char	trcTmp_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input paramters */
	strcpy (argSys, rxReqMsg->head.para[0].paraVal);


	for(i=0; i<strlen(argSys); i++) argSys[i] = toupper(argSys[i]);

	for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){
		if(!strncasecmp(sfdb->sys[i].commInfo.name, argSys, strlen(argSys))){
			sprintf (trcBuf_loc, "    SYSTEM = %s\n", mySysName);
			sprintf (trcTmp_loc, "    RESULT = SUCCESS\n");
			strcat (trcBuf_loc, trcTmp_loc);
			strcat (trcBuf_loc, "    ============================================\n");
			sprintf (trcTmp_loc, "    %-12s  %-16s  %-12s\n", "DESTINATION", "IP_ADDRESS", "STATUS");
			strcat (trcBuf_loc, trcTmp_loc);
			strcat (trcBuf_loc, "    ============================================\n");

			// Local Lan 정보
			for(j = 0; j < sfdb->sys[i].commInfo.lanCnt; j++){
				if(sfdb->sys[i].commInfo.lanInfo[j].status == SFM_LAN_CONNECTED){
					sprintf (trcTmp_loc, "    %-12s  %-16s  %-12s\n",
							sfdb->sys[i].commInfo.lanInfo[j].name,
							sfdb->sys[i].commInfo.lanInfo[j].targetIp,
							"CONNECTED");
				}
				else {
					sprintf (trcTmp_loc, "    %-12s  %-16s  %-12s\n",
							sfdb->sys[i].commInfo.lanInfo[j].name,  
							sfdb->sys[i].commInfo.lanInfo[j].targetIp,
							"DISCONNECTED");
				}
				strcat (trcBuf_loc, trcTmp_loc);
			}
			strcat (trcBuf_loc, "    ============================================\n");
			MMCResSnd (rxIxpcMsg, trcBuf_loc, 0, 0);
#ifdef DEBUG
sprintf(trcBuf,"[%s] DIS_LAN_STS RESULT >>> \n%s\n", __FUNCTION__, trcBuf_loc);
trclib_writeLogErr (FL,trcBuf);	
#endif
			return;
		}
	}

	sprintf(trcBuf_loc, "\n    RESULT = FAIL\n      REASON = INPUT PARA ERR\n\n");
	MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);

	return;
}

void doDisPdSts (IxpcQMsgType *rxIxpcMsg){
	
	MMLReqMsgType	*rxReqMsg;
	int		i,  devIndex;
	char	SysName[2][5] = {"TAPA", "TAPB"};
	char	argSys[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[4098];
	int len =0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	
	memset(trcBuf_loc, 0x00, sizeof(trcBuf_loc));
	strcpy (argSys, rxReqMsg->head.para[0].paraVal);
	for(i=0; i<strlen(argSys); i++) argSys[i] = toupper(argSys[i]);

	if (!strcasecmp(argSys, "")){
		strcpy(argSys, "ALL");
	}

	if ((strcasecmp(argSys, "TAPA") || strcasecmp(argSys, "TAPB") || strcasecmp(argSys, "ALL")) < 0) {
			sprintf(trcBuf_loc,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			return;
	}

	if(!strcasecmp(argSys, "TAPA")){
		devIndex = 0;
		mmc_makePDStsOutputMsg(trcBuf_loc, devIndex, argSys);
	}
	else if(!strcasecmp(argSys, "TAPB")){
		devIndex = 1;
		mmc_makePDStsOutputMsg(trcBuf_loc, devIndex, argSys);
	}
	else if(!strcasecmp(argSys, "ALL")){
		/* IXPC는 두번 연속으로 전송 시 한번만 받는다. sjjeon */
#if 0
		for(i=0; i<2; i++){
			fprintf(stderr,"i : %d, name : %s\n",i, SysName[i]);
			mmc_makePDStsOutputMsg(rxIxpcMsg, i, SysName[i]);
		}
#endif
		len = mmc_makePDStsOutputMsg(trcBuf_loc, 0, SysName[0]);
		mmc_makePDStsOutputMsg(trcBuf_loc+len, 1, SysName[1]);
	}

#ifdef DEBUG
sprintf(trcBuf_loc,"[%s] DIS_PD_STS RESULT >>> \n%s\n", __FUNCTION__, trcBuf_loc);
trclib_writeLogErr (FL,trcBuf_loc);	
#endif

	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
}

int mmc_makePDStsOutputMsg (char *trcBuf_loc, int devIndex, char *argSys)
{
	int		i;
	char	rcBuf[256], hcBuf[256], link_sts[52][5], power_sts[5][5];
	//char	trcBuf_loc[4098];
	//char	trcTmp_loc[1024];

	for(i=0; i<23; i++){
		if(l3pd->l3ProbeDev[devIndex].gigaLanInfo[i].mask == SFM_ALM_MASKED) strcpy(link_sts[i], "MASK");
		else if(l3pd->l3ProbeDev[devIndex].gigaLanInfo[i].status == 0) strcpy(link_sts[i], "UP");
		else strcpy(link_sts[i], "DOWN");
	}
	// dcham 20110502
	for(i=0; i<3; i++){
		if(l3pd->l3ProbeDev[devIndex].powerInfo[i].mask == SFM_ALM_MASKED) strcpy(power_sts[i], "MASK");
		else if(l3pd->l3ProbeDev[devIndex].powerInfo[i].status == 0) strcpy(power_sts[i], "UP");
		else strcpy(power_sts[i], "DOWN");
	}

	sprintf(hcBuf, "     SYSTEM = %s\n     RESULT = SUCCESS\n", argSys);
	strcat(hcBuf, "     ======================================================================\n");
	// TAP 의 CPU/MEM 상태 감시 기능 배제 2009/10/14
//	sprintf(trcTmp_loc,"     CPU USED = %d%%\n", l3pd->l3ProbeDev[devIndex].cpuInfo.usage);
	strcat(trcBuf_loc, hcBuf);
//	strcat(trcBuf_loc, trcTmp_loc);
//	sprintf(rcBuf, "     MEM USED = %d%%\n", l3pd->l3ProbeDev[devIndex].memInfo.usage);
//	strcat(trcBuf_loc, rcBuf);
//	strcat(trcBuf_loc, "    ----------------------------------------------------------------------\n");
	strcat(trcBuf_loc, "     TYPE    STATUS    TYPE    STATUS    TYPE    STATUS    TYPE    STATUS\n");
	strcat(trcBuf_loc, "    ======================================================================\n");
	strcat(trcBuf_loc, "     [IN-LINE PORT]\n");
	sprintf(rcBuf, "     PORT1   %-5s     PORT2   %-5s     PORT3   %-5s     PORT4   %-5s\n", link_sts[0], link_sts[1], link_sts[2], link_sts[3]);
	strcat(trcBuf_loc, rcBuf);
	sprintf(rcBuf, "     PORT5   %-5s     PORT6   %-5s     PORT7   %-5s     PORT8   %-5s\n", link_sts[4], link_sts[5], link_sts[6], link_sts[7]);
	strcat(trcBuf_loc, rcBuf);
	sprintf(rcBuf, "     PORT9   %-5s     PORT10  %-5s     PORT11  %-5s     PORT12  %-5s\n", link_sts[8], link_sts[9], link_sts[10], link_sts[11]);
	strcat(trcBuf_loc, rcBuf);
	strcat(trcBuf_loc, "    ----------------------------------------------------------------------\n");
	strcat(trcBuf_loc, "     [MONITOR PORT]\n");
	sprintf(rcBuf, "     PORT13  %-5s     PORT14  %-5s     PORT15  %-5s     PORT16  %-5s\n", link_sts[12], link_sts[13], link_sts[14], link_sts[15]);
	strcat(trcBuf_loc, rcBuf);
	sprintf(rcBuf, "     PORT17  %-5s     PORT18  %-5s     PORT19  %-5s     PORT20  %-5s\n", link_sts[16], link_sts[17], link_sts[18], link_sts[19]);
	strcat(trcBuf_loc, rcBuf);
	sprintf(rcBuf, "     PORT21  %-5s     PORT22  %-5s\n", link_sts[20], link_sts[21]);
	strcat(trcBuf_loc, rcBuf);
	strcat(trcBuf_loc, "    ----------------------------------------------------------------------\n");
	strcat(trcBuf_loc, "     [MANAGE PORT]\n");
	sprintf(rcBuf, "     PORT23  %-5s \n", link_sts[22]);
	strcat(trcBuf_loc, rcBuf);
	strcat(trcBuf_loc, "    ----------------------------------------------------------------------\n");
	strcat(trcBuf_loc, "     [POWER]\n");
	sprintf(rcBuf, "     POWER1  %-5s \n", power_sts[0]);
	strcat(trcBuf_loc, rcBuf);
	sprintf(rcBuf, "     POWER2  %-5s \n", power_sts[1]);
	strcat(trcBuf_loc, rcBuf);
	strcat(trcBuf_loc, "    ======================================================================\n");
	strcat(trcBuf_loc, "\n");

//	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0); // ALL 호출시 함수상 두번 IXPC는 실패 , 함수 외부에서 보낸다. 
	memset(hcBuf, 0x00, sizeof(hcBuf));
	memset(rcBuf, 0x00, sizeof(rcBuf));

	return strlen(trcBuf_loc);
//	return 1;
}

void doSetAutoMode(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	char			mode [COMM_MAX_NAME_LEN];
	int				idx;
	char	trcBuf_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input parameters
	*/
	strcpy (mode, rxReqMsg->head.para[0].paraVal);


	idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
	idx += sprintf(trcBuf_loc+idx, "    =======================================================\n");

	if (!strcmp (mode, "on") || !strcmp(mode, "ON")) {

		if (sfdb->auto_sce_mode == 0) {			//Already sce mode auto 
			idx += sprintf(trcBuf_loc+idx,"    *** Alreay SCE Mode [AUTO]\n");
		} else {
			sfdb->auto_sce_mode = 0;
			idx += sprintf(trcBuf_loc+idx,"    *** Change SCE Mode [AUTO]\n");
		}
	} else {

		if (sfdb->auto_sce_mode == 1) {			//Already sce mode manual 
			idx += sprintf(trcBuf_loc+idx,"    *** Alreay SCE Mode Manual!\n");
		} else {
			idx += sprintf(trcBuf_loc+idx,"    *** Change SCE Mode Manual!\n");
			sfdb->auto_sce_mode = 1;
		}
	}

	idx += sprintf(trcBuf_loc+idx, "    =======================================================\n");

	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
}

void doSetDUPSts(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	int			i, idx;
	char		argSysName[COMM_MAX_NAME_LEN], otherSysName[COMM_MAX_NAME_LEN];
	char		statusA_Old[COMM_MAX_NAME_LEN],statusB_Old[COMM_MAX_NAME_LEN] ;

	char		cmdSwitch[256] = {0,}; 
	char	trcBuf_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input parameters
	*/
	strcpy (argSysName, rxReqMsg->head.para[0].paraVal);

	/* 대문자로 출력하기 위해 
	*/
	for(i=0 ; i<strlen(argSysName); i++)  argSysName[i]  = toupper(argSysName[i]);

	switch( sfdb->sys[1].commInfo.systemDup.myStatus) {
		case 1:
			strcpy(statusA_Old, "ACTIVE");
			break;
		case 2:
			strcpy(statusA_Old, "STANDBY");
			break;
		default:
			strcpy(statusA_Old, "UNKNOWN");
			break;
	}

	switch( sfdb->sys[2].commInfo.systemDup.myStatus) {
		case 1:
			strcpy(statusB_Old, "ACTIVE");
			break;
		case 2:
			strcpy(statusB_Old, "STANDBY");
			break;
		default:
			strcpy(statusB_Old, "UNKNOWN");
			break;
	}

	if( !strcmp(argSysName, "SCMA") )
	{
		strcpy(otherSysName, "SCMB");

		if( !strcmp(statusA_Old, "STANDBY") && !strcmp(statusB_Old, "ACTIVE"))
		{
			sprintf(cmdSwitch, "rsh -l root %s /opt/VRTSvcs/bin/hagrp -switch SCM_SG -to %s", argSysName,argSysName);
			system(cmdSwitch);

			sleep (1);

			idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
			idx += sprintf(trcBuf_loc+idx,"	SCMA : 'STANDBY' -> 'ACTIVE'\n");
			idx += sprintf(trcBuf_loc+idx,"	SCMB : 'ACTIVE' -> 'STANDBY'\n");

			/** 절체 명령 이후 다시 확인하는 절차 없이 MML 결과는 성공 메시지로 출력해주는 걸로 바꿈. 
			// 절체 후 다시 상태 확인을 해서 변경되었는지 확인한다. 
			sprintf(cmdSts, "rsh -l root %s /opt/VRTSvcs/bin/hastatus -sum > %s",
					argSysName, filename);
			system(cmdSts);

			if( (fp=fopen(filename,"r")) != NULL )
			{
				while(fgets(buf,sizeof(buf),fp))
				{
					if( strstr(buf,"SCM_SG") && strstr(buf,"SCMA") )
					{
						sscanf(buf,"%s %s %s %s %s %s",tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);
						if( !strncmp(tmp6,"ONLINE",6))
						{
							ret1 = 1; 
						}
						else
						{
							ret1 = 0;
						}
						if( ret1 == 1 )
							strcpy(statusA_New, "ACTIVE");
						else
							strcpy(statusA_New, "STANDBY");

						memset(tmp6,0x00,sizeof(tmp6));
					}
					else if( strstr(buf,"SCM_SG") && strstr(buf,"SCMB") )
					{
						sscanf(buf,"%s %s %s %s %s %s",tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);
						if( !strncmp(tmp6,"OFFLINE",7))
						{
							ret2 = 1; 
						}
						else
						{
							ret2 = 0;
						}
						if(ret2 == 1)
							strcpy(statusB_New, "STANDBY");
						else
							strcpy(statusB_New, "ACTIVE");

						break;
					}
				}
			}
			fclose(fp);
			remove(filename);
			*/

			/**
			if( ret1*ret2 == 1 )
			{
				idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
				idx += sprintf(trcBuf_loc+idx,"	SCMA : %s -> %s\n", statusA_Old, statusA_New);
				idx += sprintf(trcBuf_loc+idx,"	SCMB : %s -> %s\n", statusB_Old, statusB_New);
			}
			else
			{
				idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
				idx += sprintf(trcBuf_loc+idx,"	Command Fail. Abnormal Change.\n");
				idx += sprintf(trcBuf_loc+idx,"	SCMA : %s -> %s\n", statusA_Old, statusA_New);
				idx += sprintf(trcBuf_loc+idx,"	SCMB : %s -> %s\n", statusB_Old, statusB_New);
			}
			*/
		}
		else
		{
			idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
			idx += sprintf(trcBuf_loc+idx,"	Aleady %s is ACTIVE. \n",argSysName);
			idx += sprintf(trcBuf_loc+idx,"	SCMA : %s\n", statusA_Old);
			idx += sprintf(trcBuf_loc+idx,"	SCMB : %s\n", statusB_Old);
		}
	}

	else if( !strcmp(argSysName, "SCMB") )
	{
		strcpy(otherSysName, "SCMA");

		if( !strcmp(statusA_Old, "ACTIVE") && !strcmp(statusB_Old, "STANDBY"))
		{
			sprintf(cmdSwitch, "rsh -l root %s /opt/VRTSvcs/bin/hagrp -switch SCM_SG -to %s", argSysName,argSysName);
			system(cmdSwitch);

			sleep (1);

			idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
			idx += sprintf(trcBuf_loc+idx,"	SCMA : 'ACTIVE' -> 'STANDBY'\n");
			idx += sprintf(trcBuf_loc+idx,"	SCMB : 'STANDBY' -> 'ACTIVE'\n");
			/** 절체 명령어에 대한 결과는 다시 확인하지 않고 성공으로 판단해서 MML 결과를 보여주는것으로 변경 */
			/**
			// 절체 후 다시 상태 확인을 해서 변경되었는지 확인한다. 
			sprintf(cmdSts, "rsh -l root %s /opt/VRTSvcs/bin/hastatus -sum > %s",
					argSysName, filename);
			system(cmdSts);

			if( (fp=fopen(filename,"r")) != NULL )
			{
				while(fgets(buf,sizeof(buf),fp))
				{
					if( strstr(buf,"SCM_SG") && strstr(buf,"SCMA") )
					{
						sscanf(buf,"%s %s %s %s %s %s",tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);
						if( !strncmp(tmp6,"OFFLINE",7))
						{
							ret1 = 1; 
						}
						else
						{
							ret1 = 0;
						}
						if(ret1 == 1) // bug 07.05 : ret2 --> ret1 으로 바꿈....
							strcpy(statusA_New, "STANDBY");
						else
							strcpy(statusA_New, "ACTIVE");
						memset(tmp6,0x00,sizeof(tmp6));
					}
					else if( strstr(buf,"SCM_SG") && strstr(buf,"SCMB") )
					{
						sscanf(buf,"%s %s %s %s %s %s",tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);
						if( !strncmp(tmp6,"ONLINE",6))
						{
							ret2 = 1; 
						}
						else
						{
							ret2 = 0;
						}
						if(ret2 == 1)
							strcpy(statusB_New, "ACTIVE");
						else
							strcpy(statusB_New, "STANDBY");
						break;
					}
				}
			}
			fclose(fp);
			remove(filename);

			if( ret1*ret2 == 1 )
			{
				idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
				idx += sprintf(trcBuf_loc+idx,"	SCMA : %s -> %s\n", statusA_Old, statusA_New);
				idx += sprintf(trcBuf_loc+idx,"	SCMB : %s -> %s\n", statusB_Old, statusB_New);
			}
			else
			{
				idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
				idx += sprintf(trcBuf_loc+idx,"	Command Fail. Abnormal Change.\n");
				idx += sprintf(trcBuf_loc+idx,"	SCMA : %s -> %s\n", statusA_Old, statusA_New);
				idx += sprintf(trcBuf_loc+idx,"	SCMB : %s -> %s\n", statusB_Old, statusB_New);
			}
			*/
		}
		else
		{
			idx = sprintf(trcBuf_loc, "\n	RESULT = SUCCESS\n\n");
			idx += sprintf(trcBuf_loc+idx,"	Aleady %s is ACTIVE.\n",argSysName);
			idx += sprintf(trcBuf_loc+idx,"	SCMA : %s\n", statusA_Old);
			idx += sprintf(trcBuf_loc+idx,"	SCMB : %s\n", statusB_Old);
		}
	}

	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);

	return;

}

// 20100927 by dcham
void doClrScmFaultSts(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType   *rxReqMsg;
	int         i, idx;
	char        argSysName[COMM_MAX_NAME_LEN];
	char        cmdSwitch[256] = {0,};
	char        trcBuf_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	// get input parameters
	strcpy (argSysName, rxReqMsg->head.para[0].paraVal);

	// 대문자로 출력 
	for(i=0 ; i<strlen(argSysName); i++)
		argSysName[i] = toupper(argSysName[i]);

	if( strcmp(argSysName, "ALL") )
		sprintf(cmdSwitch, "rsh -l root %s /opt/VRTSvcs/bin/hagrp -clear SCM_SG -sys %s", argSysName, argSysName);
	else
		sprintf(cmdSwitch, "rsh -l root %s /opt/VRTSvcs/bin/hagrp -clear SCM_SG", "SCMA");

	if( strcmp(argSysName, "ALL") ){
		if((!strcmp(argSysName, "SCMA") && sfdb->sys[1].commInfo.systemDup.myStatus == 3) ||
				(!strcmp(argSysName, "SCMB") && sfdb->sys[2].commInfo.systemDup.myStatus == 3))
		{
			system(cmdSwitch);
			sleep(1);
			idx = sprintf(trcBuf_loc, "\n    RESULT = SUCCESS\n\n");
			idx += sprintf(trcBuf_loc+idx,"    %s : FAULTED => STANDBY STATUS CHANGE\n", argSysName); // 설명형식으로 20101003 by dcham
		}else {
			idx = sprintf(trcBuf_loc, "\n    RESULT = FAILED\n\n");
			idx += sprintf(trcBuf_loc+idx,"    %s : NOT FAULTED STATUS\n", argSysName);
		}
	}else{                                                                                                                                                                                   
		if(sfdb->sys[1].commInfo.systemDup.myStatus == 3 || sfdb->sys[2].commInfo.systemDup.myStatus == 3)
		{
			system(cmdSwitch);
			sleep(1);
			idx = sprintf(trcBuf_loc, "\n    RESULT = SUCCESS\n\n");
			idx += sprintf(trcBuf_loc+idx,"    SCM_SG : FAULTED STATUS CLEARED IN SCM_SG\n"); // 설명형식으로 20101003 by dcham
		}else{                                                                                                                                                                               
			idx = sprintf(trcBuf_loc, "\n    RESULT = FAILED\n\n");                                                                                                                          
			idx += sprintf(trcBuf_loc+idx,"    SCM_SG : NOT FAULTED STATUS IN SCM_SG\n");                                                                                                    
		}                                                                                                                                                                                    
	}                                                                                                                                                                                        

	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);                                                                                                                                                  

	return;                                                                                                                                                                                  
}

/**< yhshin 
  	Duplication 정보 출력 
**/
void doDisDUPSts(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	int				i, idx;
	char			argSysName[COMM_MAX_NAME_LEN];
	char			status_strA [COMM_MAX_NAME_LEN], status_strB [COMM_MAX_NAME_LEN];

	char    currentStamp2[32], currentStamp1[32];
	struct tm   *pLocalTime;
	char	trcBuf_loc[1024];


	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input parameters
	*/
	strcpy (argSysName, rxReqMsg->head.para[0].paraVal);

	/* 대문자로 출력하기 위해 
	*/
	for(i=0 ; i<strlen(argSysName); i++)  argSysName[i]  = toupper(argSysName[i]);

	// sys[1] SCMA?, sys[2] SCMB?
	switch (sfdb->sys[1].commInfo.systemDup.myStatus) {
	case 1:  // ACTIVE 
		sprintf(status_strA, "ACTIVE");
		break;
	case 2:
		sprintf(status_strA, "STANDBY");
		break;
	case 3:
		sprintf(status_strA, "FAULTED");
		break;
	default:
		sprintf(status_strA, "UNKNOWN");
		break;
	}

	switch (sfdb->sys[2].commInfo.systemDup.myStatus) {
	case 1:  // ACTIVE 
		sprintf(status_strB, "ACTIVE");
		break;
	case 2:
		sprintf(status_strB, "STANDBY");
		break;
	case 3:
		sprintf(status_strB, "FAULTED");
		break;
	default:
		sprintf(status_strB, "UNKNOWN");
		break;
	}

	// SCMA SAMD run-time
	pLocalTime = (struct tm*)localtime((time_t*)&sfdb->sys[1].commInfo.procInfo[1].uptime);
	strftime (currentStamp1, 32, "%Y-%m-%d %H:%M:%S", pLocalTime);

	// SCMB SAMD run-time
	pLocalTime = (struct tm*)localtime((time_t*)&sfdb->sys[2].commInfo.procInfo[1].uptime);
	strftime (currentStamp2, 32, "%Y-%m-%d %H:%M:%S", pLocalTime);

	idx = sprintf (trcBuf_loc, "\n      RESULT = SUCCESS\n\n");
	idx += sprintf(trcBuf_loc+idx, "    ============================================================\n");
	idx += sprintf(trcBuf_loc+idx, "    %6s  %10s	%15s\n", "SYSTEM", "STATUS", "RUN TIME(SAMD)");
	idx += sprintf(trcBuf_loc+idx, "    ============================================================\n");
	idx += sprintf(trcBuf_loc+idx, "    %6s  %10s	%15s\n", "SCMA", status_strA, currentStamp1);
	idx += sprintf(trcBuf_loc+idx, "    %6s  %10s	%15s\n", "SCMB", status_strB, currentStamp2);
	idx += sprintf(trcBuf_loc+idx, "    ============================================================\n");

	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);

	return;

}

#if 0 // db 삭제 
int db_backup(char *backupdir)
{
	int     i = 0;
	char    cmd[1024] = {0,};
	char    backFile[256] = {0,};

	for( i = 0; i < MAX_SYS_CNT; i++ )
	{
		memset(cmd, 0x00, sizeof(cmd));
		memset(backFile, 0x00, sizeof(backFile));
		sprintf(backFile, "%s/%s", backupdir, g_stDBInfo.backFile);

		if( !strncmp(g_stDBInfo.sysName, "DSCM",4 ))
		{
			sprintf(cmd, "mysqldump -u%s -p%s -h%s %s > %s", g_stDBInfo.dbId,
					g_stDBInfo.dbPass,
					g_stDBInfo.dbIp,
					g_stDBInfo.dbName,
					backFile);
			system(cmd);
			break;

		}
		else if( !strncmp(g_stDBInfo.sysName, "SCMA",4 ) || !strncmp(g_stDBInfo.sysName, "SCMB",4))
		{
			sprintf(cmd, "mysqldump -u%s -p%s -h%s %s %s %s > %s", g_stDBInfo.dbId,
					g_stDBInfo.dbPass,
					g_stDBInfo.dbIp,
					g_stDBInfo.dbName,                                            
					g_stDBInfo.tblName[0],
					g_stDBInfo.tblName[1],
					backFile);
			system(cmd);
			break;
		}
	}
	return 0;
}
#endif


void doBkupPkg (IxpcQMsgType *rxIxpcMsg)
{
	char    oldpath[256], newpath[256], tarname[256];
	char    cmd[1024], curdir[256], ivhome[256], tmp[128];
	MMLReqMsgType   *rxReqMsg;
	int             i;
	char            option[COMM_MAX_NAME_LEN];
	struct tm       tmnow;
	time_t          now = time(NULL);
	int             ret = 0;/*, pid = -1;*/
	char	trcBuf_loc[1024];
	char	trcTmp_loc[1024];
	struct stat file_info;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input parameters
	 */
	strcpy (option, rxReqMsg->head.para[0].paraVal);

	/* ´e¹®AU·I Aa·ACI±a A§CØ
	 */
	for(i=0 ; i<strlen(option); i++) option[i] = toupper(option[i]);

	if(!localtime_r(&now, &tmnow)){
		sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(local_time_r) CALL FAIL\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	strcpy(ivhome, getenv(IV_HOME));
	strftime(tmp, sizeof(tmp), "%m%d", &tmnow);
	sprintf(oldpath, "%s/OLD/%s", ivhome, tmp);

	if (stat(oldpath, &file_info) == -1)
	{
#ifdef __LINUX__
		if(mkdir(oldpath, DEFFILEMODE) < 0){
#else
		if(mkdir(oldpath, 0755) < 0){
#endif
			if(errno != EEXIST){
				sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = BACKUP DIRECTORY CREATE FAIL\n\n");
				MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
				return;
			}
		}
	}

	if(!getcwd(curdir, sizeof(curdir))){
		sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(getcwd) CALL FAIL\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	sprintf(newpath, "%s/", ivhome);
	if(chdir(newpath) < 0){                                                              
		sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(chdir) CALL FAIL\n\n");        
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);                                             
		return;                                                                          
	}                                                                                    

	//BSDA_APP_0707181230.tar                                                            
	/* jjinri 2009-05-23                                                                 
	   sprintf(tarname, "%s/%s_%s_%.02d%.02d%.02d%.02d%.02d.tar",                           
	   oldpath, mySysName, option, (tmnow.tm_year-100), (tmnow.tm_mon+1),           
	   tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);                                 
	 */                                                                                   
// db 삭제 	if( strcasecmp(option, "DB") != 0){                                                  
	sprintf(tarname, "%s/%s_%s_%.02d%.02d%.02d%.02d%.02d.tar",                       
			oldpath, mySysName, option, (tmnow.tm_year-100), (tmnow.tm_mon+1),       
			tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);                             
//	}                                                                                    

	if(!strcasecmp(option, "ALL")){                                                      
		sprintf(cmd, "/bin/tar cvf %s BIN DATA > /dev/null", tarname);                   
#if 0 // db 삭제 
		if( (pid = fork()) < 0 )                                                         
		{                                                                                
			sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(fork) CALL FAIL\n\n");     
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);                                         
			return ;                                                                     
		}                                                                                
		if( pid == 0 ) // child is db backup.                                            
		{                                                                                
			ret = db_backup(oldpath);                                                    
			if( ret == -1 )                                                              
			{                                                                            
				sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = DB BACKUP FAIL\n\n");   
				MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);                                     
				return ;                                                                 
			}                                                                            

			exit(0);                                                                     
		}                                                                                
#endif
	}else if(!strcasecmp(option, "APP")){                                                
		sprintf(cmd, "/bin/tar cvf %s BIN > /dev/null", tarname);                        
	}else if(!strcasecmp(option, "DATA")){                                               
		sprintf(cmd, "/bin/tar cvf %s DATA > /dev/null", tarname);                       
	}
#if 0 // db 삭제 
	else if(!strcasecmp(option, "DB")){                                                 
		if( (pid = fork()) < 0 )                                                         
		{                                                                                
			sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(fork) CALL FAIL\n\n");     
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);                                         
			return ;                                                                     
		}                    
		if( pid == 0 ) // child is db backup.                                            
		{                                                                                
			ret = db_backup(oldpath);                                                    
			if( ret == -1 )                                                              
			{                                                                            
				sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = DB BACKUP FAIL\n\n");   
				MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);                                     
				return ;                                                                 
			}                                                                            

			exit(0);                                                                     
		}                                                                                
	}                                                                                    
#endif

// db 삭제 	if( strcasecmp(option, "DB") != 0){                                                  
	if(system(cmd) < 0){                                                             
		sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(system) CALL FAIL\n\n");   
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);                                         
		return;                                                                      
	}                                                                                
//	}                                                                                    

	if(chdir(curdir) < 0){                                                               
		sprintf(trcBuf_loc, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(chdir) CALL FAIL\n\n");        
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);                                             
		return;                                                                          
	}                                                                                    

	if( ret == 0 )                                                                       
	{                                                                                    
		memset(trcBuf_loc,0x00,sizeof(trcBuf_loc));
		memset(trcTmp_loc,0x00,sizeof(trcTmp_loc));
// db 삭제 		if( strcasecmp(option, "DB") != 0){                                              
		sprintf (trcBuf_loc, "    RESULT = SUCCESS\n    BACKUP FILE = %s\n", tarname);   
//		}                                                                                

#if 0 // db 삭제 
		if( !strcasecmp(option, "ALL") || !strcasecmp(option, "DB") ) {                  
			sprintf (trcTmp_loc, "    DB BACKUP FILE = %s\n", g_stDBInfo.backFile);      
			strcat(trcBuf_loc, trcTmp_loc);                                                      
		}                                                                                
#endif
		MMCResSnd (rxIxpcMsg, trcBuf_loc, 0, 0);                                             
		return;                                                                          
	}                                                                                    
}

/*
sjjeon
룰셋 적용..
 */
#define RULE_SET_APPLY 1
#define RULE_SET_NOT_APPLY 0
void doSetRuleSce (IxpcQMsgType *rxIxpcMsg){
	
	MMLReqMsgType	*rxReqMsg;
	int		i, devIndex;
	char	argSys[COMM_MAX_NAME_LEN];
	char 	argSts[10];
	char 	argFName[30];
	char 	command[100], command2[100];
	int		pid ;
	char	trcBuf_loc[4098];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	
	memset(trcBuf_loc, 0x00, sizeof(trcBuf_loc));
	memset( argFName, 0, sizeof( argFName ) );
	memset( command, 0, sizeof( command ) );
	memset( command2, 0, sizeof( command2 ) );

	strcpy (argSys, rxReqMsg->head.para[0].paraVal);
	strcpy( argSts, rxReqMsg->head.para[1].paraVal );
	if( !strcasecmp( argSts, "recovery" ) )
	{
		strcpy( argFName, rxReqMsg->head.para[2].paraVal );
	}

	for(i=0; i<strlen(argSys); i++) 
		argSys[i] = toupper(argSys[i]);

	// SCEA 일때...
	if(!strcasecmp(argSys, "SCEA")) {
		devIndex = 0;

		if ((pid = fork()) < 0){
			logPrint (trcErrLogId,FL,"[%s] fork() fail....\n", __FUNCTION__);
			return;
		}

		if (pid == 0) {
			sendApplySCERst2COND(APPLY_RULESET_SCEA_PROGRESS_ALARM);
			runSetRuleSce(devIndex, argSts, argFName, RULE_SET_APPLY);
			exit(0);
		}

	// SCEB 일때...
	}else if(!strcasecmp(argSys, "SCEB")) {
		devIndex = 1;
	
		if ((pid = fork()) < 0){
			logPrint (trcErrLogId,FL,"[%s] fork() fail....\n", __FUNCTION__);
			return;
		}

		if (pid == 0) {
			sendApplySCERst2COND(APPLY_RULESET_SCEB_PROGRESS_ALARM);
			runSetRuleSce(devIndex, argSts, argFName, RULE_SET_APPLY);
			exit(0);
		}

	} else if (!strcasecmp(argSys, "ALL")) {

		if ((pid = fork()) < 0){
			logPrint (trcErrLogId,FL,"[%s] fork() fail....\n", __FUNCTION__);
			return;
		}

		if (pid == 0) {
			devIndex=2;   // ALL-SCEA
			sendApplySCERst2COND(APPLY_RULESET_ALL_PROGRESS_ALARM);  // 시작하는 메시지 .....
			runSetRuleSce(devIndex, argSts, argFName, RULE_SET_APPLY);  // flag 0 : 쉘만 수행한다.SCE에 적용한다.
			exit(0);
		}
 
		if ((pid = fork()) < 0){
			logPrint (trcErrLogId,FL,"[%s] fork()2 fail....\n", __FUNCTION__);
			return;
		}

		if (pid == 0) {
			devIndex=3;
			runSetRuleSce(devIndex, argSts, argFName, RULE_SET_NOT_APPLY); //  쉘 수행과 동시에 BACKUP도 한다.
			exit(0);
		}

	} else{
		sprintf(trcBuf_loc,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	sprintf (trcBuf_loc, "    Appling RULE SET to SCE :%s\n", argSys);
	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);

}


/*
sjjeon : SCE Rule 적용. 
0  : OK
-1 : FAIL
*/

#define SYNC_RULE_RLEG 0
#define SYNC_RULE_RDRANA 1
int runSetRuleSce(int index, char *argSts, char *argFName, int flag)
{
	int rv;
	char    command[100];
	char	trcBuf_loc[1024];

	if(argSts==NULL||argFName==NULL){
		sprintf(trcBuf_loc,"[%s] para input error. argSts : %s, argFName : %s\n",
						__FUNCTION__,  argSts, argFName);
		trclib_writeLogErr (FL,trcBuf_loc);	
		return -1;
	}

	// 1. RuleEditor --> apply 
	//	1) 사용자 가 새로 입력한 Rule
	//    /DSC/RULE/temp_sce_conf.pqb 
	//	  /DSC/RULE/TEMP_RULE_LIST.conf 
	//  
	//	2) ruleApply.sh 실행. 
	//		: SCEA에 temp...pqb file을 적용하는 shelle
	//  3) ruleApply2.sh 실행. 
	//		: SCEB에 temp...pqb file을 적용하는 shelle

	/* SCEA 적용 */
	//if(index==0) {
	if(index==0||index==2) {
		sprintf( command, "/usr/bin/bash /DSC/RULE/EDITOR/ruleApply.sh %s %s", argSts, argFName );
		sprintf (trcBuf_loc,"SCEA Run shall======>>>>  %s\n", command);
		trclib_writeLogErr (FL,trcBuf_loc);

	/* SCEB */
	//} else if(index==1){
	} else if(index==1||index==3){
		sprintf( command, "/usr/bin/bash /DSC/RULE/EDITOR/ruleApply2.sh %s %s", argSts, argFName );
		sprintf (trcBuf_loc,"SCEB Run shall======>>>>  %s\n", command);
		trclib_writeLogErr (FL,trcBuf_loc);

	// ALL 기능은 막아둔다.
	} else {
	}

	/*Shell 구동..*/
	rv = runShellProgress(command);
	if(rv<0)  // Shell 구동 실패..
	{
		if(index==0||index==2){
			sprintf(trcBuf_loc,"[%s] runShellProgress(). SCEA Shell execute fail...!!\n",__FUNCTION__);
			sendApplySCERst2COND(APPLY_RULESET_SCEA_FAIL_ALARM);
		}
		else if(index==1||index==3){
			sprintf(trcBuf_loc,"[%s] runShellProgress(). SCEB Shell execute fail...!!\n",__FUNCTION__);
			sendApplySCERst2COND(APPLY_RULESET_SCEB_FAIL_ALARM);
		}
		trclib_writeLogErr (FL,trcBuf_loc);
		return -1;
	}

	/*동기화 및 백업, sync */
	// SCEA, SCEB, ALL-SCEB 만 작업 수행..
	//if (flag == 1) {
	if(index==0||index==1||index==3){
		//mmcReqSyncRuleFile(index,1,1); /*0:SCMA/1:SCMB, 0:RULE-FILE-1, 1:BACKUP*/
		mmcReqSyncRuleFile(index,SYNC_RULE_RLEG,1); /*0:SCMA/1:SCMB, 0:RULE-FILE-1, 1:BACKUP*/
		sleep(1);
		mmcReqSyncRuleFile(index,SYNC_RULE_RDRANA,0); /*0:SCMA/1:SCMB, 1:RULE-FILE-2, 0:NO BACKUP*/
	}

	if(index==0 || index==2){
		sendApplySCERst2COND(APPLY_RULESET_SCEA_COMPLETE_ALARM);
		sprintf(trcBuf_loc,"[%s] scea rule set success !!\n",__FUNCTION__);
	} else if(index==1 || index==3){
		sendApplySCERst2COND(APPLY_RULESET_SCEB_COMPLETE_ALARM);
		sprintf(trcBuf_loc,"[%s] sceb rule set success !!\n",__FUNCTION__);
	}

	trclib_writeLogErr (FL,trcBuf_loc);
	
	return 0;

} /*End fo runSetRuleSce()*/


/*
//by sjjeon : RULESET_LIST.conf 파일을 rcopy 해준 뒤, MP 에 sync-rule-file 적용한다.
sysIdx 	: 0: SCMA,     1: SCMB,   
rule 	: 0: rule 1,   1: rule 2	 
bkOpt  : 0: no backup 1: backup : 백업 여부...
*/
int mmcReqSyncRuleFile(int sysIdx, int rule, int bkOpt)
{
    GeneralQMsgType     txGenQMsg;
    IxpcQMsgType        *txIxpcMsg;
	MMLReqMsgType   	*txReqMsg;
	int					jobNo=15;
    int                 txLen;
	char				sysName[5];
	char				filePath[2][64];
	char				curTM[15];
	char				cmd[100];
	struct passwd 		*u_info;
	struct group 		*g_info;
	char	trcBuf_loc[1024];

	memset(sysName,0,sizeof(sysName));
	memset(cmd,0,sizeof(cmd));
	memset(filePath,0,sizeof(filePath));
	
	if(sysIdx == 0 || sysIdx ==2){
		strcpy(sysName,"SCMA");
	}else if(sysIdx==1||sysIdx==3){
		strcpy(sysName,"SCMB");
	}

	// 중복 Backup을 막기위한 option
	if(sysIdx==0||sysIdx==1||sysIdx==3){

		getCurTimeStr(curTM); // 현재 시간을 String으로 얻어온다.
		u_info = getpwnam("tomcat"); //권한설정 owner
		g_info = getgrnam("cvs");   //권한설정 group

		/*1. 이전 RULESET_LIST.conf 파일을 /DSC/RULE/BACKUP_RULESET/_YYYYMMDDHHMMSS.conf 
		     위치에 BACKUP 한다. */
//		sprintf(filePath[0], "/DSC/RULE/BACKUP_RULESET/_%s.conf", curTM);
//		sprintf(cmd,"/usr/bin/cp /DSC/RULE/RULESET_LIST.conf %s",filePath[0]);
//		system(cmd); bzero(cmd,sizeof(cmd));

		/*2. 백업 파일의 권한을 준다.(tomcat:cvs계정) */
//		chown((char*)filePath[0], u_info->pw_uid, g_info->gr_gid);

		/*3. 이전 sce_conf.pqb 파일을 /DSC/RULE/BACKUP_RULESET/_YYYYMMDDHHMMSS.pqb
		     위치에 BACKUP 한다. */
//		sprintf(filePath[1], "/DSC/RULE/BACKUP_RULESET/_%s.pqb", curTM);
//		sprintf(cmd,"/usr/bin/cp /DSC/RULE/sce_conf.pqb %s",filePath[1]);
//		system(cmd);

		/*4. 백업 파일의 권한을 준다.(tomcat:cvs계정) */
//		chown((char*)filePath[1], u_info->pw_uid, g_info->gr_gid);

		/*5. TEMP_RULESET_LIST.conf => RULESET_LIST.conf 로 복사한다.*/
		system("/usr/bin/cp /DSC/RULE/TEMP_RULESET_LIST.conf /DSC/RULE/RULESET_LIST.conf");

		/*6. temp_sce_conf.pqb => sce_conf.pqb 로 복사한다. */
		system("/usr/bin/cp /DSC/RULE/temp_sce_conf.pqb /DSC/RULE/sce_conf.pqb");
	
		/*7. RULESET_LIST.conf를 SCMA/SCMB에 RCOPY 한다.*/
		system("/usr/bin/rcp /DSC/RULE/RULESET_LIST.conf SCMA:/DSC/NEW/DATA/RULESET_LIST.conf");
		system("/usr/bin/rcp /DSC/RULE/RULESET_LIST.conf SCMB:/DSC/NEW/DATA/RULESET_LIST.conf");

	}

	txGenQMsg.mtype = MTYPE_MMC_REQUEST;
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	txReqMsg = (MMLReqMsgType*)txIxpcMsg->body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));
	txIxpcMsg->head.msgId= htonl(10);

    strcpy (txIxpcMsg->head.srcSysName, "DSCM");
    strcpy (txIxpcMsg->head.srcAppName, "SAMD");
    strcpy (txIxpcMsg->head.dstSysName, "DSCM");
    strcpy (txIxpcMsg->head.dstAppName, "MCDM");

	/*"sync-rule-file" "sync-rule-file2"*/
	txReqMsg->head.paraCnt = (1);
	txReqMsg->head.mmcdJobNo = (jobNo+sysIdx+rule);
	if(rule == 0) {
		strcpy(txReqMsg->head.cmdName,"sync-rule-file");
	}else{
		strcpy(txReqMsg->head.cmdName,"sync-rule-file2");
	}

#if 0
	strcpy (txReqMsg->head.para[0].paraName, "SYS");
	if(sysIdx==0||sysIdx==1)
		strcpy (txReqMsg->head.para[0].paraVal, sysName);
	else 
		strcpy (txReqMsg->head.para[0].paraVal, " ");
	//strcpy (txReqMsg->head.para[0].paraVal, "ALL");
#endif

    txIxpcMsg->head.bodyLen = sizeof(txReqMsg->head) - sizeof(txReqMsg->head.para)
								+ txReqMsg->head.paraCnt*sizeof(CommPara)+30;
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if(sysIdx!=2)   // ALL-SCEA 일때는 전송하지 않는다.
	{
		//8. SEND
		if (msgsnd(mcdmQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
			sprintf(trcBuf_loc,"[%s] msgsnd(report_SCEFIMD) fail; err=%d(%s)\n"
					, __FUNCTION__
					, errno
					, strerror(errno));
			trclib_writeLogErr (FL,trcBuf_loc);                                              
			return -1;                                                                   
		}                                                                                
		else {                                                                           
//			printf(" type: %d\n", txGenQMsg.mtype);
//			printf(" msgsnd : len %d\n", txLen);
//			printf("%s::%s->%s::%s->%s\n"                                   
					//, __FUNCTION__
					//, txIxpcMsg->head.srcSysName                                         
					//, txIxpcMsg->head.srcAppName
					//, txIxpcMsg->head.dstSysName                                         
					//, txIxpcMsg->head.dstAppName);                                       
		}
	}
                                                                                   
//printf("========>> Output %s Function <<============\n",__FUNCTION__);
    return 1;                                                                        
                                           
}


/*
sjjeon
shell 구동 중, 진행 상태 전송하며, 해당 Index를 찾으면 완료한다.
0 : OK
-1 : FAIL
*/
int runShellProgress(char *cmd)
{

    FILE *pp = NULL, *fp = NULL;
    int rv=0;
    int _BUFSIZ=512;
	char index[2][10]={"CM","completed"};
    char buf[_BUFSIZ], curTimeStr[15], logPath[64];
    time_t cur_tm;/*, old_tm;*/
	char	trcBuf_loc[1024];

    if(!cmd) {
		sprintf(trcBuf_loc,"[%s] cmd is null..\n",__FUNCTION__);
		trclib_writeLogErr (FL,trcBuf_loc);
        return -1;
    }

	bzero(logPath,sizeof(logPath));	
	bzero(buf,sizeof(buf));	

	getCurTimeStr(curTimeStr);
	sprintf(logPath,"/DSC/LOG/RULE_SET_LOG/%s.log", curTimeStr);

    memset(buf,0x00,_BUFSIZ);

	fp = fopen(logPath,"a+");
    if(fp==NULL){
		sprintf(trcBuf_loc,"[%s] fopen() err.(path=%s)\n",__FUNCTION__, logPath);
		trclib_writeLogErr (FL,trcBuf_loc);
        rv=-1;
		goto OUT;
    }

    pp = popen(cmd, "r");
    if(pp==NULL){
		sprintf(trcBuf_loc,"[%s] popen() err.(cmd=%s)\n",__FUNCTION__, cmd);
		trclib_writeLogErr (FL,trcBuf_loc);
        rv=-1;
		goto OUT;
    }

    while(fgets(buf, _BUFSIZ, pp) != NULL)
    {
        time(&cur_tm);
//        printf("%s\n",buf);
		fputs(buf,fp); /* LOG */
		/*2개의 Index를 만족하는 문장을 찾는다.*/
		if(findIndex(buf,index[0])){
			if(findIndex(buf,index[1]))
			{
//printf("RULE-SET ..... OK : %s, %s\n", index[0],index[1]);
				while(fgets(buf,_BUFSIZ,pp)){;};
				rv = 0;
				goto OUT;
			}
		}
/*
		if(!(cur_tm%2)){
			if(old_tm != cur_tm){
				old_tm = cur_tm;
				printf("###################################################\n");
				printf("진행중.......................\n");                                               
				printf("###################################################\n");                         
			}
		}
*/
	}
	rv = -1;

OUT:
	if(fp)fclose(fp);
    if(pp)pclose(pp);
    return rv;
}

/*
by sjjeon
current time string output. size 15
 */
void getCurTimeStr(char *outTimeStr)
{
    time_t	the_time;
    struct	tm *tm_ptr;

	if(!outTimeStr)
		return;

    time(&the_time);
//    tm_ptr = gmtime(&the_time); /*GMT Time*/
    tm_ptr = localtime(&the_time);
    sprintf(outTimeStr, "%04d%02d%02d%02d%02d%02d",
            tm_ptr->tm_year + 1900, tm_ptr->tm_mon +1,
            tm_ptr->tm_mday, tm_ptr->tm_hour,
            tm_ptr->tm_min, tm_ptr->tm_sec);
	
}

/*
by sjjeon
display sce mode information
 */
void doDisSceMode (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char	argSys[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	
	strcpy(argSys, rxReqMsg->head.para[0].paraVal);

	//printf("mml test : %s \n",argSys );

	if(!strcasecmp(argSys, "SCEA")){
		shellRunDisSceMode(rxIxpcMsg, 0); // SCEA

	}else if(!strcasecmp(argSys, "SCEB")){
		shellRunDisSceMode(rxIxpcMsg, 1); // SCEB
	

	}else if(!strcasecmp(argSys, "ALL")||!strcasecmp(argSys,"")){
		shellRunDisSceMode(rxIxpcMsg, 2); // ALL

	}else {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  Unknown System.(%s)\n\n",argSys );
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
	}

}

/*
by sjjeon
shell excute dis-sce-mode 
index :
0 : scea
1 : sceb
2 : all
 */

void shellRunDisSceMode(IxpcQMsgType *rxIxpcMsg, int index)
{

	int pos=0;
	const int MAX_ST_IDX=10;
	char cmd[128], automode[16];
	st_idx  idxdb[MAX_ST_IDX], idxdb2[MAX_ST_IDX];
	char	trcBuf_loc[1024];
	
	bzero(cmd,sizeof(cmd));
	memset(idxdb,0,sizeof(idxdb));

	pos += sprintf(trcBuf_loc+pos, "    SYSTEM = %s\n", mySysName);
	pos += sprintf(trcBuf_loc+pos, "    RESULT = SUCCESS\n");
	pos += sprintf(trcBuf_loc+pos, "    ============================================================================\n");
	pos += sprintf(trcBuf_loc+pos, "    %4s %-12s %-12s %-12s %-10s\n",  "SCE", "ACTIVE-MODE", "FAILURE-MODE", "FAILURE-RECOVERY-MODE", "AUTO-MODE");
	pos += sprintf(trcBuf_loc+pos, "    ============================================================================\n");


	if (sfdb->auto_sce_mode == 0) {
		sprintf (automode, "AUTO");
	} else {
		sprintf (automode, "MANUAL");
	}

	if(index == 0)
	{
		strcpy(cmd, "/DSC/SCRIPT/dis_scea_mode.sh");
		if(getShellInfo(cmd, (st_idx*)&idxdb[0],MAX_ST_IDX) !=0){
			//fail
			return ;
		}
	pos += sprintf(trcBuf_loc+pos, "     SCEA %-12s %-12s %-12s %10s\n", idxdb[0].idxValue, idxdb[1].idxValue, idxdb[0].idxValue2, automode);
		
	}
	else if(index == 1){
		strcpy(cmd, "/DSC/SCRIPT/dis_sceb_mode.sh");
		if(getShellInfo(cmd, (st_idx*)&idxdb[0],MAX_ST_IDX) !=0){
			//fail
			return ;
		}

	pos += sprintf(trcBuf_loc+pos, "     SCEB %-12s %-12s %-12s %10s\n", idxdb[0].idxValue, idxdb[1].idxValue, idxdb[0].idxValue2, automode);

	}
	else {
		memset(idxdb,0,sizeof(idxdb2));
		strcpy(cmd, "/DSC/SCRIPT/dis_scea_mode.sh");
		if(getShellInfo(cmd, (st_idx*)&idxdb[0],MAX_ST_IDX) !=0){
			//fail
			return ;
		}
	pos += sprintf(trcBuf_loc+pos, "     SCEA %-12s %-12s %-12s %10s\n", idxdb[0].idxValue, idxdb[1].idxValue, idxdb[0].idxValue2, automode);
	pos += sprintf(trcBuf_loc+pos, "    ----------------------------------------------------------------------------\n");
		
		strcpy(cmd, "/DSC/SCRIPT/dis_sceb_mode.sh");
		if(getShellInfo(cmd, (st_idx*)&idxdb2[0],MAX_ST_IDX) !=0){
			//fail
			return ;
		}
	pos += sprintf(trcBuf_loc+pos, "     SCEB %-12s %-12s %-12s %10s\n", idxdb2[0].idxValue, idxdb2[1].idxValue, idxdb[0].idxValue2, automode);

	}

	//pos += sprintf(trcBuf_loc+pos, "     TEST MODE 진행중..........\n");
	pos += sprintf(trcBuf_loc+pos, "    ============================================================================\n");
	
	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
	return;
}


/*
by sjjeon
shell을 구동 후에 결과값을 얻어온다.
path : shell path
stidx : stidx structure
stidxCnt : stidx structure Max Count
*/
int getShellInfo(char *path, st_idx *stidx, int stidxCnt)
{
    FILE *fp;
    const int _BUFSIZE =1024;
    char buf[_BUFSIZE];
    char tmp[10][20];
	int stcnt=0;
	int mode=0, linkNum=0;
	char *pValue=NULL;
	int i=0;
	char	trcBuf_loc[1024];

    bzero(tmp,sizeof(tmp));
    fp = popen(path,"r");
    if(fp==NULL){
		sprintf(trcBuf_loc,"[%s] popen() err.(path=%s)\n",__FUNCTION__, path);
		trclib_writeLogErr (FL,trcBuf_loc);
        return -1;
    }

    /*
	   System Operation mode on failure recovery is: non-operational

        Actual  link mode on active is : forwarding
        Actual  link mode on failure is: cutoff
    */
    while(fgets(buf, _BUFSIZE, fp) != NULL)
    {
		if(stcnt >= stidxCnt){
    		while(fgets(buf, _BUFSIZE, fp) != NULL){;};
			if(fp) pclose(fp);
			return 0;
		}


		if (strstr(buf, "non-operational")) {
			strcpy(stidx->idxValue2,"NON-OPERATIONAL");
		}  else if (strstr(buf, "operational")) {
			strcpy(stidx->idxValue2,"OPERATIONAL");
		}


		if(findCaseIndex(buf,"GBE1")) linkNum = 1;
		else if(findCaseIndex(buf,"GBE3")) linkNum = 2;

        if(findCaseIndex(buf,"Actual"))
        {
			if(!mode) stidx->linkNum = 2;
            /*active mode*/
            if(findCaseIndex(buf,"active")){
                strcpy(stidx->idxName, "ACTIVE");

				/*":"로 구분된 곳 뒤의 value 값을 넣는다. */
				pValue= strstr(buf,":")+1;
                if(NULL != pValue){
					while(*pValue==' '){ pValue++;};
					strcpy(stidx->idxValue, pValue);
					if(strlen(pValue)<3) 
						strcpy(stidx->idxValue,"UNKNOWN");
					else{
						if(stidx->idxValue[strlen(pValue)-1]=='\n');
							stidx->idxValue[strlen(pValue)-1]='\0';
						for(i=0; i<strlen(pValue)-1; i++) 
							stidx->idxValue[i] = toupper(stidx->idxValue[i]);
					}

					stidx->linkNum = linkNum;

                }else{
					strcpy(stidx->idxValue,"UNKNOWN" );
				}
            }
            /*failure mode*/
            else if(findCaseIndex(buf,"failure")){
                strcpy(stidx->idxName, "FAILURE");

				/*":"로 구분된 곳 뒤의 value 값을 넣는다. */
				pValue= strstr(buf,":")+1;
                if(NULL != pValue){
					while(*pValue==' '){ pValue++;};
					strcpy(stidx->idxValue, pValue);

					if(strlen(pValue)<3) 
						strcpy(stidx->idxValue,"UNKNOWN");
					else{
						if(stidx->idxValue[strlen(pValue)-1]=='\n');
							stidx->idxValue[strlen(pValue)-1]='\0';
						for(i=0; i<strlen(pValue)-1; i++) 
							stidx->idxValue[i] = toupper(stidx->idxValue[i]);
					}


                }
            }

            stidx++; stidxCnt++;
		}
		memset(buf,0,_BUFSIZE);
	}
	if(fp)pclose(fp);
	return 0;
}


/*
by sjjeon
display sce mode information
 */
void doReloadSce(IxpcQMsgType *rxIxpcMsg)
{

	MMLReqMsgType   *rxReqMsg;
	char	argSys[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	strcpy(argSys, rxReqMsg->head.para[0].paraVal);

	if(!strcasecmp(argSys, "SCEA")){
		shellRunReloadSce(rxIxpcMsg, 0); // SCEA
//		shellRunDisSceMode(rxIxpcMsg, 0); // SCEA

	}else if(!strcasecmp(argSys, "SCEB")){
		shellRunReloadSce(rxIxpcMsg, 1); // SCEA

	}else if(!strcasecmp(argSys, "ALL")||!strcasecmp(argSys,"")){
		shellRunReloadSce(rxIxpcMsg, 2); // SCEA

	}else {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  UNKNOWN SYSTEM.(%s)\n\n",argSys );
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
	}
}


/*
by sjjeon
shell excute reload-sce
index :
0 : scea
1 : sceb
2 : all
 */
#define RELOAD_SCEA_CMD "/usr/bin/bash /DSC/SCRIPT/reload_scea.sh" 
#define RELOAD_SCEB_CMD "/usr/bin/bash /DSC/SCRIPT/reload_sceb.sh" 
void shellRunReloadSce(IxpcQMsgType *rxIxpcMsg, int index)
{

	char cmd[128];
	int pos=0, pid=0;
	char	trcBuf_loc[1024];

	bzero(cmd,sizeof(cmd));

	switch(index)
	{
		case 0:
			/*SCEA*/
			strcpy(cmd, RELOAD_SCEA_CMD);
			pos += sprintf(trcBuf_loc+pos, "      SYSTEM = %s\n", "SCEA");
			break;

		case 1:
			/*SCEB*/
			strcpy(cmd, RELOAD_SCEB_CMD);
			pos += sprintf(trcBuf_loc+pos, "      SYSTEM = %s\n", "SCEB");
			break;

		case 2:
			/*ALL*/
			strcpy(cmd, RELOAD_SCEA_CMD);
			pos += sprintf(trcBuf_loc+pos, "      SYSTEM = %s|%s\n", "SCEA","SCEB");
			break;

		default:
			break;		
		
	}

	if ((pid = fork()) < 0){
		logPrint (trcErrLogId,FL,"[%s] fork() fail....\n", __FUNCTION__);
		return;
	}

	if(pid == 0){
		system(cmd);
		
		/* reload-sce not all */
		if(index !=2){
			pos += sprintf(trcBuf_loc+pos, "      RESULT = SUCCESS\n");
			MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
		}
		exit(0);
	}

	/* reload-sce all */
	if(index == 2){
		if ((pid = fork()) < 0){
			logPrint (trcErrLogId,FL,"[%s] fork() fail....\n", __FUNCTION__);
			return;
		}

		if(pid == 0){
			strcpy(cmd, RELOAD_SCEB_CMD);
			system(cmd);
			pos += sprintf(trcBuf_loc+pos, "      RESULT = SUCCESS\n");
			MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
			exit(0);
		}
	}
	return;
}

/*
by sjjeon
Set SCE Mode 
 */
void doSetSceMode(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char	argSys[COMM_MAX_NAME_LEN];
	char	argMode[COMM_MAX_NAME_LEN];
	char	argStat[COMM_MAX_NAME_LEN];
	int sysType, modeType, statType, idx= 0;
	char	trcBuf_loc[1024];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	strcpy(argSys, rxReqMsg->head.para[0].paraVal);
	strcpy(argMode, rxReqMsg->head.para[1].paraVal);
	strcpy(argStat, rxReqMsg->head.para[2].paraVal);


	if (sfdb->auto_sce_mode == 0) {			// 0 - Auto
		idx += sprintf (trcBuf_loc+idx, "\n      RESULT = SUCCESS\n      현재 설정이 자동 모드로 되어 있습니다.");
		idx += sprintf (trcBuf_loc+idx, "\n      AUTO-SCE-MODE 명령을 통해 수동 설정으로 변경 후 사용하세요.. \n\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}


	if(!strcasecmp(argSys, "SCEA")){
		sysType = 0;		
	}else if(!strcasecmp(argSys, "SCEB")){
		sysType = 1;		
	}else if(!strcasecmp(argSys, "ALL")||!strcasecmp(argSys,"")){
		sysType = 2;		
	}else {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  UNKNOWN SYSTEM.(%s)\n\n",argSys );
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		return;
	}

	/*Active mode*/
	if(!strcasecmp(argMode, "ACTIVE")){
		modeType = 0;		
		if(!strcasecmp(argStat, "FORWARDING")) statType = 0;
		else if(!strcasecmp(argStat, "BYPASS")) statType = 1;
		else if(!strcasecmp(argStat, "CUTOFF")) statType = 2;
		else {
			sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  UNKNOWN PARAMETER.(%s)\n\n",argStat );
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			return;
		}

	/*Failure mode*/
	}else if(!strcasecmp(argMode, "FAILURE")){
		modeType = 1;		
		if(!strcasecmp(argStat, "BYPASS")) statType = 0;
		else if(!strcasecmp(argStat, "CUTOFF")) statType = 1;
		else {
			sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  UNKNOWN PARAMETER.(%s)\n\n",argStat );
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			return;
		}

	/*Recovery mode*/
	}else if(!strcasecmp(argMode, "RECOVERY")){
		modeType = 2;		
		if(!strcasecmp(argStat, "OPERATIONAL")) statType = 0;
		else if(!strcasecmp(argStat, "NON-OPERATIONAL")) statType = 1;
		else {
			sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  UNKNOWN PARAMETER.(%s)\n\n",argStat );
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			return;
		}
	}else {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  UNKNOWN PARAMETER.(%s)\n\n",argMode );
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
	}

	//run
	shellRunSetSceMode(rxIxpcMsg, sysType, modeType, statType);

}

/*
by sjjeon
shell excute set-sce-mode

sysType : 0 : scea , 1 : sceb, 2 : all
mode : 0 : active, 1: failure, 2: recovery
active stat : 0: forwarding, 1: bypass, 2: cutoff
failure stat : 0 : bypass, 1: cutoff
recovery stat : 0 : operational, 1: non-operational
 */
#define SET_SCEA_FORWARD_MODE_CMD "/usr/bin/bash /DSC/SCRIPT/set-scea-forword-mode.sh" 
#define SET_SCEA_BYPASS_MODE_CMD "/usr/bin/bash /DSC/SCRIPT/set-scea-bypass-mode.sh" 
#define SET_SCEA_CUTOFF_MODE_CMD "/usr/bin/bash /DSC/SCRIPT/set-scea-cutoff-mode.sh" 
#define SET_SCEB_FORWARD_MODE_CMD "/usr/bin/bash /DSC/SCRIPT/set-sceb-forword-mode.sh" 
#define SET_SCEB_BYPASS_MODE_CMD "/usr/bin/bash /DSC/SCRIPT/set-sceb-bypass-mode.sh" 
#define SET_SCEB_CUTOFF_MODE_CMD "/usr/bin/bash /DSC/SCRIPT/set-sceb-cutoff-mode.sh" 


#define SET_SCEA_ACTIVE_FORWARDING 		"/usr/bin/bash /DSC/SCRIPT/set-scea-forword-mode.sh"
#define SET_SCEA_ACTIVE_BYPASS			"/usr/bin/bash /DSC/SCRIPT/set-scea-bypass-mode.sh"
#define SET_SCEA_ACTIVE_CUTOFF			"/usr/bin/bash /DSC/SCRIPT/set-scea-cutoff-mode.sh"
#define SET_SCEA_FAILURE_BYPASS			"/usr/bin/bash /DSC/SCRIPT/set-scea-onfail-bypass-mode.sh"
#define SET_SCEA_FAILURE_CUTOFF			"/usr/bin/bash /DSC/SCRIPT/set-scea-onfail-cutoff-mode.sh"
#define SET_SCEA_RECOVERY_OPERATIONAL 	"/usr/bin/bash /DSC/SCRIPT/set-scea-recovery-op.sh"
#define SET_SCEA_RECOVERY_NON_OPERATIONAL "/usr/bin/bash /DSC/SCRIPT/set-scea-recovery-nonop.sh"

#define SET_SCEB_ACTIVE_FORWARDING 		"/usr/bin/bash /DSC/SCRIPT/set-sceb-forword-mode.sh"
#define SET_SCEB_ACTIVE_BYPASS			"/usr/bin/bash /DSC/SCRIPT/set-sceb-bypass-mode.sh"
#define SET_SCEB_ACTIVE_CUTOFF			"/usr/bin/bash /DSC/SCRIPT/set-sceb-cutoff-mode.sh"
#define SET_SCEB_FAILURE_BYPASS			"/usr/bin/bash /DSC/SCRIPT/set-sceb-onfail-bypass-mode.sh"
#define SET_SCEB_FAILURE_CUTOFF			"/usr/bin/bash /DSC/SCRIPT/set-sceb-onfail-cutoff-mode.sh"
#define SET_SCEB_RECOVERY_OPERATIONAL 	"/usr/bin/bash /DSC/SCRIPT/set-sceb-recovery-op.sh"
#define SET_SCEB_RECOVERY_NON_OPERATIONAL "/usr/bin/bash /DSC/SCRIPT/set-sceb-recovery-nonop.sh"


void shellRunSetSceMode(IxpcQMsgType *rxIxpcMsg, int sysType, int mode, int stat)
{

	char cmd[128];
	int pos=0, pid=0;
	char	trcBuf_loc[1024];

	bzero(cmd,sizeof(cmd));

	/*sysType : 0:SCEA, 1:SECB, 2:ALL*/
	switch(sysType)
	{
		case 0:
		/*SCEA 쉘 명령어 설정 */
         pos += sprintf(trcBuf_loc+pos, "      SYSTEM = %s\n", "SCEA");

		 if(mode==0){  // ACTIVE MODE
			 pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "ACTIVE");
			 if(stat==0){ 
				 pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "FORWARDING");
				 sprintf(cmd, "%s", SET_SCEA_ACTIVE_FORWARDING);
			 }else if(stat==1){ 
				 pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "BYPASS");
				 sprintf(cmd, "%s", SET_SCEA_ACTIVE_BYPASS);
			 }else if(stat==2){ 
				 pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "CUTOFF");
				 sprintf(cmd, "%s", SET_SCEA_ACTIVE_CUTOFF);
			 }else{
				pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
				logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 	MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			 	return;
			 }

		 }else if(mode==1){  // FAILURE MODE
			pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "FAILURE");
			if(stat==0){ 
				pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "BYPASS");
				sprintf(cmd, "%s", SET_SCEA_FAILURE_BYPASS);
			}else if(stat==1){ 
				pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "CUTOFF");
				sprintf(cmd, "%s", SET_SCEA_FAILURE_CUTOFF);
			}else{
				pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
				logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 	MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
				return;
			}

		}else if(mode==2){  // RECOVERY MODE
			pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "RECOVERY");
			if(stat==0){ 
				pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "OPERATIONAL");
				sprintf(cmd, "%s", SET_SCEA_RECOVERY_OPERATIONAL);
			}else if(stat==1){ 
				pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "NON-OPERATIONAL");
				sprintf(cmd, "%s", SET_SCEA_RECOVERY_NON_OPERATIONAL);
			}else{
				pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
				logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 	MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
				return;
			}
		}else{
			pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "UNKNOWN");
			logPrint (trcErrLogId,FL,"[%s] UNKNOWN MODE (%d)\n", __FUNCTION__, mode);
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			return;
		}

		break;

		case 1:
			/*SCEB 쉘 명령어 설정 */

			pos += sprintf(trcBuf_loc+pos, "      SYSTEM = %s\n", "SCEB");
			if(mode==0){ // ACTIVE MODE
				pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "ACTIVE");
				if(stat==0){ 
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "FORWARDING");
					sprintf(cmd, "%s", SET_SCEB_ACTIVE_FORWARDING);
				}else if(stat==1){ 
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "BYPASS");
					sprintf(cmd, "%s", SET_SCEB_ACTIVE_BYPASS);
				}else if(stat==2){ 
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "CUTOFF");
					sprintf(cmd, "%s", SET_SCEB_ACTIVE_CUTOFF);
				}else{
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}

			}else if(mode==1){  // FAILURE MODE
				pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "FAILURE");
				if(stat==0){ 
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "BYPASS");
					sprintf(cmd, "%s", SET_SCEB_FAILURE_BYPASS);
				}else if(stat==1){ 
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "CUTOFF");
					sprintf(cmd, "%s", SET_SCEB_FAILURE_CUTOFF);
				}else{
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}

			}else if(mode==2){  // RECOVERY MODE
				pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "RECOVERY");
				if(stat==0){ pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "OPERATIONAL");
					sprintf(cmd, "%s", SET_SCEB_RECOVERY_OPERATIONAL);
				}else if(stat==1){ pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "NON-OPERATIONAL");
					sprintf(cmd, "%s", SET_SCEB_RECOVERY_NON_OPERATIONAL);
				}else{
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}
			}else{
				pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "UNKNOWN");
				logPrint (trcErrLogId,FL,"[%s] UNKNOWN MODE (%d)\n", __FUNCTION__, mode);
				MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
				return;
			}

			break;


		case 2:
			/* ALL 의 첫번째 설정 */
			pos += sprintf(trcBuf_loc+pos, "      SYSTEM = %s\n", "SCEA/SCEB");
			if(mode==0){
				pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "ACTIVE");
				if(stat==0){
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "FORWARDING");
					sprintf(cmd, "%s", SET_SCEA_ACTIVE_FORWARDING);
				}else if(stat==1){
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "BYPASS");
					sprintf(cmd, "%s", SET_SCEA_ACTIVE_BYPASS);
				}else if(stat==2){
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "CUTOFF");
					sprintf(cmd, "%s", SET_SCEA_ACTIVE_CUTOFF);
				}else{
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}

			}else if(mode==1){
				pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "FAILURE");
				if(stat==0){ 
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "BYPASS");
					sprintf(cmd, "%s", SET_SCEA_FAILURE_BYPASS);
				}else if(stat==1){ 
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "CUTOFF");
					sprintf(cmd, "%s", SET_SCEA_FAILURE_CUTOFF);
				}else{
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}

			}else if(mode==2){ 
				pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "RECOVERY");
				if(stat==0){
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "OPERATIONAL");
					sprintf(cmd, "%s", SET_SCEA_RECOVERY_OPERATIONAL);
				}else if(stat==1){
					pos += sprintf(trcBuf_loc+pos, "      STAT   = %s\n", "NON-OPERATIONAL");
					sprintf(cmd, "%s", SET_SCEA_RECOVERY_NON_OPERATIONAL);
				}else{
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}
			}else{
				pos += sprintf(trcBuf_loc+pos, "      MODE   = %s\n", "UNKNOWN");
				logPrint (trcErrLogId,FL,"[%s] UNKNOWN MODE (%d)\n", __FUNCTION__, mode);
				MMCResSnd(rxIxpcMsg,trcBuf_loc, -1, 0);
				return;
			}

			break;

		default:
			pos += sprintf(trcBuf_loc+pos, "      UNKOWN SYSTYPE\n");
			logPrint (trcErrLogId,FL,"[%s] UNKNOWN SYSTYPE (%d)\n", __FUNCTION__, stat);

	 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
			return;
	}


	if ((pid = fork()) < 0){
		logPrint (trcErrLogId,FL,"[%s] fork() fail....\n", __FUNCTION__);
		return;
	}

	if(pid == 0){
		/* ALL 의 첫번째 실행 */

#if 1
		system(cmd);
#else
		FILE *fp=NULL;
		const int _BUFSIZ = 1048;
		char buf[_BUFSIZ];
		fp = popen(cmd, "r");
		if(fp==NULL){
			sprintf(trcBuf_loc,"[%s] popen() err.(cmd=%s)\n",__FUNCTION__, cmd);
			trclib_writeLogErr (FL,trcBuf_loc);
			return -1;
		}
		while(fgets(buf, _BUFSIZ, fp) != NULL){;}

		if(fp)pclose(fp);

#endif
		
		/* set-sce-mode not all */
		if(sysType !=2){
			pos += sprintf(trcBuf_loc+pos, "      RESULT = SUCCESS\n");
			MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
		}
		exit(0);
	}

	/* mode-sce all */
	if(sysType == 2){
		if ((pid = fork()) < 0){
			logPrint (trcErrLogId,FL,"[%s] fork() fail....\n", __FUNCTION__);
			return;
		}

		if(pid == 0){
			/* ALL 의 두번째 실행 */
			sleep(1);	
			if(mode==0){
				if(stat==0){
					sprintf(cmd, "%s", SET_SCEB_ACTIVE_FORWARDING);
				}else if(stat==1){
					sprintf(cmd, "%s", SET_SCEB_ACTIVE_BYPASS);
				}else if(stat==2){
					sprintf(cmd, "%s", SET_SCEB_ACTIVE_CUTOFF);
				}else {
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}
			}else if(mode==1){
				if(stat==0){
					sprintf(cmd, "%s", SET_SCEB_FAILURE_BYPASS);
				}else if(stat==1){
					sprintf(cmd, "%s", SET_SCEB_FAILURE_CUTOFF);
				}else {
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}
			}else if(mode==2){
				if(stat==0){
					sprintf(cmd, "%s", SET_SCEB_RECOVERY_OPERATIONAL);
				}else if(stat==1){
					sprintf(cmd, "%s", SET_SCEB_RECOVERY_NON_OPERATIONAL);
				}else {
					pos += sprintf(trcBuf_loc+pos, "      UNKOWN STAT\n");
					logPrint (trcErrLogId,FL,"[%s] UNKNOWN STAT (%d)\n", __FUNCTION__, stat);

			 		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
					return;
				}
			}else{
				pos += sprintf(trcBuf_loc+pos, "      UNKOWN MODE\n");
				logPrint (trcErrLogId,FL,"[%s] UNKNOWN MODE (%d)\n", __FUNCTION__, stat);

	 			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
				return;
			}
			system(cmd);
			pos += sprintf(trcBuf_loc+pos, "      RESULT = SUCCESS\n");
			MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
			exit(0);
		}
	}
	return;
}

/*
   by sjjeon
   dis-sys-ver
*/

/* MODIFY : by june, 2010-10-03
 * DESC : 기존 함수에 version이 안나오는 경우가 발생하여 수정.
 */
void doDisSysVer(IxpcQMsgType *rxIxpcMsg)
{
    MMLReqMsgType   *rxReqMsg;
    char argSys[COMM_MAX_NAME_LEN];
    int pos= 0, i;
	char cmd[]="/DSC/SCRIPT/dis-sys-ver.sh";
	char fullCmd[BUFSIZ] = {0,};
	char outPath[]="/DSC/DATA/sys-ver.txt";
	char ver[4][32], buf[512];
	char trcBuf_loc[1024];
	FILE *fp=NULL;
	sprintf(fullCmd, "%s > %s", cmd, outPath);
	system(fullCmd);
	fp = fopen(outPath,"r");
	if(fp==NULL)
	{
		sprintf(trcBuf_loc,"[%s] fopen() err.(%s)\n",__FUNCTION__, outPath);
		trclib_writeLogErr (FL,trcBuf_loc);
		return;
	}

	bzero(buf,sizeof(buf));
	bzero(ver,sizeof(char)*32*4);

	i=0;
	while(fgets(buf,512, fp)!=NULL){
		if(i==4) break;
		strcpy(ver[i], buf);
		i++;
	}
	pclose(fp);
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    strcpy(argSys, rxReqMsg->head.para[0].paraVal);
	
	btrim(ver[0]);
	btrim(ver[1]);
	btrim(ver[2]);
	strcpy(ver[3], &g_pstSCEInfo->SCEDev[0].sysInfo.version[8]);
	btrim(ver[3]);

	sprintf(trcBuf,"[%s]ver[0]:%s, ver[1]:%s, ver[2]:%s, ver[3]:%s \n",__FUNCTION__,ver[0],ver[1],ver[2],ver[3]);
	        trclib_writeLogErr (FL,trcBuf);

	pos += sprintf(trcBuf_loc+pos, "    SYSTEM = %s\n", mySysName);
	pos += sprintf(trcBuf_loc+pos, "    RESULT = SUCCESS\n");
	pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
	pos += sprintf(trcBuf_loc+pos, "    %10s   %-20s \n", "SYS-NAME", "Version");
	pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
	pos += sprintf(trcBuf_loc+pos, "    %-10s   %-20s\n", "SUN_OS", (ver[0]));
	pos += sprintf(trcBuf_loc+pos, "    %-10s   %-20s\n", "MY-SQL", (ver[1]));
	pos += sprintf(trcBuf_loc+pos, "    %-10s   %-20s\n", "TAP [A/B]", (ver[2]));
	pos += sprintf(trcBuf_loc+pos, "    %-10s   %-20s\n", "SCE [A/B]", (ver[3]));
	pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);

	return;
}


void doDisFlowCnt(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char	argSys[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[1024];
    int 	pos= 0, i=0, cnt = 0, idx = 0;
	char	sceName[MAX_SCE_CNT][128];
	char	cmd[MAX_SCE_CNT][128];
	char	fileName[MAX_SCE_CNT][128];
	char	buf[1024];
	FILE	*fp = NULL;
	char	str1[128], str2[128], str3[128];
	unsigned int flow = 0;
	unsigned int actFlow[MAX_SCE_CNT] = {0, 0};

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	
	strcpy(argSys, rxReqMsg->head.para[0].paraVal);
	trclib_writeLogErr (FL,trcBuf);                                            

	if(!strcasecmp(argSys, "SCEA"))
	{
		i = 0; cnt = 0;
		sprintf(sceName[0], "SCEA");
		sprintf(fileName[0], "/tmp/scea_flow.txt");
		sprintf(cmd[0], "/DSC/SCRIPT/snmp-sce-flow.sh %s > %s", sceName[0], fileName[0]);
	}
	else if(!strcasecmp(argSys, "SCEB"))
	{
		i = 1; cnt = 1;
		sprintf(sceName[1], "SCEB");
		sprintf(fileName[1], "/tmp/sceb_flow.txt");
		sprintf(cmd[1], "/DSC/SCRIPT/snmp-sce-flow.sh %s > %s",sceName[1], fileName[1]);
	}
	else if(!strcasecmp(argSys, "ALL")||!strcasecmp(argSys,""))
	{
		i = 0; cnt = 1;
		sprintf(sceName[0], "SCEA");
		sprintf(sceName[1], "SCEB");
		sprintf(fileName[0], "/tmp/scea_flow.txt");
		sprintf(fileName[1], "/tmp/sceb_flow.txt");
		sprintf(cmd[0], "/DSC/SCRIPT/snmp-sce-flow.sh %s > %s",sceName[0],  fileName[0]);
		sprintf(cmd[1], "/DSC/SCRIPT/snmp-sce-flow.sh %s > %s",sceName[1],  fileName[1]);
	}
	else 
	{
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  Unknown System.(%s)\n\n",argSys );
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
	}

	for( idx = i; idx <= cnt; idx++ )
	{
		memset(buf, 0x00, sizeof(buf));
		system(cmd[idx]);
		if( (fp = fopen(fileName[idx], "r")) == NULL )
		{
			sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  SNMP GET FAIL.(%s)\n\n",argSys );
			MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
		}
		else
		{
			while( fgets(buf, sizeof(buf), fp) )
			{
				sscanf(buf, "%s %s %s %d", str1, str2, str3, &flow);
				actFlow[idx] += flow;
				flow = 0;
			}
		}
	}

	pos += sprintf(trcBuf_loc+pos, "    SYSTEM = %s\n", mySysName);
	pos += sprintf(trcBuf_loc+pos, "    RESULT = SUCCESS\n");
	pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
	pos += sprintf(trcBuf_loc+pos, "    %10s   %-20s \n", "SYS-NAME", "ACTIVE FLOWS");
	pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
	for(idx = i; idx <= cnt; idx++ ) 
	{
		pos += sprintf(trcBuf_loc+pos, "    %-10s   %-20d\n", sceName[idx], actFlow[idx]);
		pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
	}
	MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
	if(fp) fclose(fp);
	fp=NULL;
}/*End of doDisFlowCnt()*/


int getNametoHwIndex(const char *hwStr)
{
	int idx=0;

	if (hwStr==NULL) return -1;

	if (!strncasecmp(hwStr, "PWR1", 4)) idx = 1;
	else if (!strncasecmp(hwStr, "PWR2", 4)) idx = 2;
	else if (!strncasecmp(hwStr, "FAN1", 4)) idx = 3;
	else if (!strncasecmp(hwStr, "FAN2", 4)) idx = 4;
	else if (!strncasecmp(hwStr, "FAN3", 4)) idx = 5;
	else idx = -1;
	
	return idx;
}
#if 0
// by june, 20100312
void doClrAlm_SysMsg(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char	argSys[COMM_MAX_NAME_LEN];
	char	argHw[COMM_MAX_NAME_LEN];
	char	trcBuf_loc[1024];
    int 	pos= 0, idx = 0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	strcpy(argSys, rxReqMsg->head.para[0].paraVal);
	strcpy(argHw, rxReqMsg->head.para[1].paraVal);

	idx = getNametoHwIndex(argHw);
	if (!idx ) {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  Invalid parameter 2(%s)\n\n", argHw);
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
	}

	if(!strcasecmp(argSys, mySysName))
	{
		pos += sprintf(trcBuf_loc+pos, "    SYSTEM = %s\n", mySysName);
		pos += sprintf(trcBuf_loc+pos, "    RESULT = SUCCESS\n");
		pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
		pos += sprintf(trcBuf_loc+pos, "    %5s %5s %7s \n", "SYS-NAME", "H/W", "STATE");
		pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
		pos += sprintf(trcBuf_loc+pos, "     %5s %7s %7s \n", mySysName, argHw, "CLEAR" );
		pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
	}
	else 
	{
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  Unknown System.(%s)\n\n",argSys );
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
	}

}	/* End of doClrAlm_SysMsg() */
#endif

int findIndex(char *syntax_ori, char *idx)
{
	char *token;
	char search[]=" ";
	int len=strlen(idx);
	int i=0, cnt=0;
	char syntax[2048];

	//  if(strlen(syntax_ori) == 0|| strlen(idx)==0) return -1;
	if(syntax_ori == NULL|| idx == NULL) return -1;

	while(1){
		if(syntax_ori[i]!=' '){
			syntax_ori = &syntax_ori[i];
			break;
		}
		i++;
	}

	memset(syntax,0,sizeof(syntax));

	strncpy(syntax,syntax_ori,strlen(syntax_ori));

	/*첫번째 설정*/
	token = strtok((char *)syntax, (char *)search);
	//printf("%s\n",token);

	i=0;                                                                                                                                                             
	while(1){
		if(token[i]!=' '){
			token = &token[i];
			break;
		}
		i++;
	}

#if 0
	if(!strncasecmp(token,idx,strlen(idx))){
		//printf("find OK.. %s\n",token);
		return 1;
	}
#else

	for(i=0; i<len;i++){
		if(token[i]==idx[i]) cnt++;
		if(cnt == len) return 1;
	}
	cnt = 0;
#endif

	while(1) {
		token = strtok(NULL, search);                                                                                                                                
		if(token == NULL) break;                                                                                                                                     
		//printf("%s\n",token);                                                                                                                                      
#if 0                                                                                                                                                                
		if(!strncasecmp(token,idx,strlen(idx))){                                                                                                                     
			//printf("find OK...%s\n",token);                                                                                                                        
			return 1;                                                                                                                                                
		}                                                                                                                                                            
#else                                                                                                                                                                
		for(i=0; i<len;i++){                                                                                                                                         
			if(token[i]==idx[i]) cnt++;                                                                                                                              
			if(cnt == len) return 1;                                                                                                                                 
		}                                                                                                                                                            
		cnt = 0;                                                                                                                                                     

#endif                                                                                                                                                               
	}                                                                                                                                                                
	return 0;                                                                                                                                                        
}

int findCaseIndex(char *syntax_ori, char *idx)
{
	int i=0, size;
	char syntax[2048];

	if(syntax_ori == NULL|| idx == NULL) return -1;

	while(1){
		if(syntax_ori[i]!=' '){
			syntax_ori = &syntax_ori[i];
			break;
		}
		i++;
	}

	memset(syntax,0,sizeof(syntax));
	strncpy(syntax,syntax_ori,strlen(syntax_ori));

	size=strlen(syntax);

	for(i=0; i<size; i++){
		if(toupper(syntax[i])== toupper(idx[0]))
		{
			if(toupper(syntax[i+1])== toupper(idx[1]))
			{
				if(!strncasecmp(&syntax[i],idx,strlen(idx)))
				{
					//printf("find OK...%s\n",idx);                                                                                                                  
					return 1;                                                                                                                                        
				}                                                                                                                                                    
			}                                                                                                                                                        
		}                                                                                                                                                            
	}                                                                                                                                                                
	return 0;                                                                                                                                                        
}
