#include "samd.h"
#include "init_shm.h"
#include <conf.h>

#define PROC_NAME_LEN   128
#ifndef	APP_PATH
#define	APP_PATH		"/DSC/NEW"
#endif

extern int		ixpcQID, samdQID;
extern int		trcFlag, trcLogFlag;
extern long		oldSumOfPIDs, newSumOfPIDs;
extern char		iv_home[64], l_sysconf[256], trcBuf[4096], trcTmp[1024];
char			trcBuf2[4096];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
extern SFM_SysCommMsgType	*loc_sadb;
extern SFM_L3PD		*l3pd;
extern SAMD_ProcessInfo		ProcessInfo[SYSCONF_MAX_APPL_NUM];
extern SAMD_DaemonInfo      DaemonInfo[SYSCONF_MAX_DAEMON_NUM];
//extern SFM_SysIFBond        bond[MAX_BOND_NUM];
extern SFM_SysSts	        sysSts;
extern int		appCnt, daemonCnt;

extern T_keepalive			*keepalive;

//// 07.21 extern int duiaValue;
extern char l_sysconf2[256];
extern int alarm_flag, prev_alarm_flag;

/* DB Backup */
// db 삭제 extern DB_INFO_t	g_stDBInfo;
#if 0
MYSQL   sql_dscm, *conn_dscm;
MYSQL   sql_dsca, *conn_dsca;
MYSQL   sql_dscb, *conn_dscb;
#endif

extern void	strtoupper(char *buff);
extern char	*get_proc_version(char *procname);

int getListID(prpsinfo_t *ps_info);
pid_t getCM_AdapPid(void);

int getLocalDupSts();
int findIndex(char *syntax, char *idex);
//int findCaseIndex(char *syntax, char *idx);
int getPidInfo(int pid, char *sttime);
//int findWord(char *buf, char *index);
void doDisSysVer(IxpcQMsgType *rxIxpcMsg);
//------------------------------------------------------------------------------
// /proc디렉토리 밑의 모든 프로세스 ID의 합을 구한다.
//------------------------------------------------------------------------------
long getSumOfPIDs (void)
{
	DIR		*dirp;
	long	totalPid=0;
	struct dirent   *direntp;

	if ((dirp = opendir (PROC_PATH)) == NULL) {
		sprintf(trcBuf,"[getSumOfPIDs] opendir err\n");
		trclib_writeLogErr (FL,trcBuf);
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
	int				i, fd, listID;
	DIR				*dirp;
	char			pathName[256];
	struct dirent	*direntp;
	prpsinfo_t		psInfo;

	if( (dirp = opendir(PROC_PATH)) == (DIR *)NULL)
	{
		sprintf(trcBuf, "[%s] opendir err\n", __FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	/** 모든 상태를 dead로 바꾼다 **/
	for(i = 0; i < loc_sadb->processCount; i++)
	{
		loc_sadb->loc_process_sts[i].status	= SFM_STATUS_DEAD;
		ProcessInfo[i].new_status			= SFM_STATUS_DEAD;
		ProcessInfo[i].runCnt				= 0;
		ProcessInfo[i].pid					= -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
			continue;

		sprintf(pathName, "%s/%s", PROC_PATH, direntp->d_name);

		if( (fd = open(pathName, O_RDONLY)) < 0)
		{
			if( (errno != EACCES) && (errno != ENOENT))
			{
				/*	Permission denied 는 skip하기 위해	*/
				sprintf(trcBuf, "[%s] open fail; (%s) err=%d[%s]\n", __FUNCTION__, pathName, errno, strerror(errno));
				trclib_writeLogErr(FL, trcBuf);
			}
			continue;
		}

		if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
		{
			sprintf(trcBuf, "[%s] ioctl err=%d[%s]\n", __FUNCTION__, errno, strerror(errno));
			trclib_writeLogErr(FL, trcBuf);
			close(fd);
			continue;
		}
		close(fd);

		if(strlen(psInfo.pr_psargs) == 0)
			continue;

		/*
		*	관리대상 프로세스인지 확인하고, 내부적으로 관리하는 해당 프로세스의 index가 return한다.
		*/
		if( (listID = getListID(&psInfo)) < 0)
			continue;

		loc_sadb->loc_process_sts[listID].status	= SFM_STATUS_ALIVE;

		ProcessInfo[listID].new_status				= SFM_STATUS_ALIVE;
		//ProcessInfo[listID].mask					= SFM_STATUS_ALIVE; // killprc로 죽인놈이 살아나면 mask가 자동으로 풀린다.
		ProcessInfo[listID].runCnt++;
		ProcessInfo[listID].pid						= psInfo.pr_pid;

		strftime (ProcessInfo[listID].startTime, 32, "%m-%d %H:%M", localtime((time_t*)&(psInfo.pr_start.tv_sec)));
		loc_sadb->loc_process_sts[listID].pid		= psInfo.pr_pid;
		loc_sadb->loc_process_sts[listID].uptime	= psInfo.pr_start.tv_sec;
	}
	closedir(dirp);

	return 1;
}

int getDaemonStatus(void)
{
	int		i, ret;
	char	command[256];

	/** 모든 상태를 dead로 바꾼다 **/
	for(i = 0; i < daemonCnt; i++)
	{
		loc_sadb->loc_process_sts[appCnt+i].status	= SFM_STATUS_DEAD;
		DaemonInfo[i].status						= SFM_STATUS_DEAD;
	}

	for(i = 0; i < daemonCnt; i++)
	{
		sprintf(command, "ps -ef | grep %s", DaemonInfo[i].procName);
		if( (ret = my_system(command)) == 0)
		{
			DaemonInfo[i].status						= SFM_STATUS_ALIVE;
			loc_sadb->loc_process_sts[appCnt+i].status	= SFM_STATUS_ALIVE;
		}
	}

	return 1;
}

//------------------------------------------------------------------------------
// 관리대상 프로세스인지 확인하고 해당 프로세스의 index를 return한다.
//------------------------------------------------------------------------------
int getListID(prpsinfo_t *ps_info)
{
	int		i;
    char    *DAppArgs, *DAppToken, tmpPSArgs[PRARGSZ];//, pName[256], buff[256], buff2[256];
	FILE	*fp;

	fp			= NULL;
    DAppToken   = NULL;
    DAppArgs    = NULL;

    memset(tmpPSArgs, 0x00, PRARGSZ);
    strcpy(tmpPSArgs, ps_info->pr_psargs);
    DAppArgs    = strstr(tmpPSArgs, "DAPP");
	for(i = 0; i < appCnt; i++)
	{
	    if(DAppArgs)
	    {
	        if( (DAppToken = strchr(DAppArgs, '=')) == NULL)
	        {
			#ifdef DEBUG
				sprintf(trcBuf, "[%s] check args(%s)\n", __FUNCTION__, DAppArgs);
				trclib_writeLogErr(FL, trcBuf);
			#endif
				return -1;
	        }
			else
				DAppToken++;

	        while( (char)*DAppToken == ' ')
	            DAppToken++;

	        strtok(DAppToken, " ");
	        if( !strcasecmp(DAppToken, loc_sadb->loc_process_sts[i].processName))
	        {
			#if 0
	            sprintf(trcBuf, "[%s] %s find\n", __FUNCTION__, DAppToken);
				trclib_writeLogErr(FL, trcBuf);
			#endif
				return i;
	        }
			else
			{
			#if 0
	            sprintf(trcBuf, "[%s] %s NOT find\n", __FUNCTION__, DAppToken);
				trclib_writeLogErr(FL, trcBuf);
			#endif
			}
	    }
		else if(!strcasecmp(ps_info->pr_fname, loc_sadb->loc_process_sts[i].processName))
			return i;
	}

	return -1;
}

//------------------------------------------------------------------------------
// 프로세스의 상태을 확인하여 죽은 상태로 변경된 놈을 자동으로 기동시킨다.
// - killprc로 죽여 mask로 설정된 경우는 살리지 않는다.
//------------------------------------------------------------------------------
void checkProcessStatus_OLD(void)
{
	int		i, pid,restartCnt;

	/*	전체 pid의 합이 같으면 한개씩 확인할 필요가 없다.		*/
#if 0 /*2003.07.07*/
	if( (newSumOfPIDs = getSumOfPIDs()) < 0)
		return;

	if(newSumOfPIDs == oldSumOfPIDs)
		return;
#endif /*2003.07.07*/
	//oldSumOfPIDs = newSumOfPIDs;  sjjeon

	/*	관리대상 프로세스들의 현재 상태 정보를 ProcessInfo table과 
		loc_sadb->loc_process_sts[i].status에 넣는다.	*/
	if(getProcessStatus() < 0)
		return;

	/* killprc 읒쓿溯� masking 처리를 위해서, 먼저 msgq를 수신해야만 한다.
	*  added by uamyd 20110427
	*/
	HandleRxMsg();

	/*	dead 상태로 변경된 놈들에 대해 killprc로 죽인 경우가 아니면 auto_restart 시킨다.	*/
	for(i = 0, restartCnt = 0; i < appCnt; i++)
	{

		if( (ProcessInfo[i].mask == SFM_ALM_MASKED) || 					// killprc로 죽인 경우.
			(ProcessInfo[i].new_status == SFM_STATUS_ALIVE)  ||			// 살아 있는 경우.
			(ProcessInfo[i].new_status == ProcessInfo[i].old_status)	// omp samp보고 추가.
			)
		{
			ProcessInfo[i].old_status = ProcessInfo[i].new_status;
			continue;
		}

		sprintf(trcBuf, "[%s] check down process %s is dead\n", 
										__FUNCTION__, ProcessInfo[i].procName);
		trclib_writeLogErr(FL, trcBuf);

		//if(runProcess(i, AUTO_FLOW) < 0)
		if((pid=runProcess(i, AUTO_FLOW))< 0)
		{
			sprintf(trcBuf, "[%s] process auto restart fail[%s]\n", 
										__FUNCTION__, ProcessInfo[i].procName);
		}
		else
		{
			sprintf(trcBuf,"[%s] process[%s] auto restarted\n", 
										__FUNCTION__, ProcessInfo[i].procName);
		}
		restartCnt++;
		trclib_writeLogErr(FL, trcBuf);
	}

	/* process 재기동이 한 건 이상 발생하면 FIMD로 alarm을 생성하도록 정보를 전송
	   added by uamyd 20110422
	*/
	if( restartCnt ){
		report_sadb2FIMD();
		getProcessStatus();
	}
}

void checkProcessStatus(void)
{
	int		i, pid, flag;

	/*	관리대상 프로세스들의 현재 상태 정보를 ProcessInfo table과 
		loc_sadb->loc_process_sts[i].status에 넣는다.	*/
	if(getProcessStatus() < 0)
		return;

	/*	dead 상태로 변경된 놈들에 대해 killprc로 죽인 경우가 아니면 auto_restart 시킨다.	*/
	for(i = 0, flag = 0 ; i < appCnt; i++)
	{
		if( (ProcessInfo[i].new_status == SFM_STATUS_ALIVE)  ||			// 살아 있는 경우.
			(ProcessInfo[i].new_status == ProcessInfo[i].old_status)	// omp samp보고 추가.
			)
		{
			//ProcessInfo[i].old_status = ProcessInfo[i].new_status;
			continue;
		}
		else {
			flag++;
			sprintf(trcBuf, "[%s] process=%s status dead\n", 
					__FUNCTION__, ProcessInfo[i].procName);
			trclib_writeLogErr(FL, trcBuf);
		}
	}

	if (flag){
		HandleRxMsg();
			
		for(i = 0 ; i < appCnt; i++)
		{
			if( (ProcessInfo[i].mask != SFM_ALM_MASKED) && 					// killprc로 죽인 경우.
				(ProcessInfo[i].new_status != SFM_STATUS_ALIVE) &&			// 살아 있는 경우.
				(ProcessInfo[i].new_status != ProcessInfo[i].old_status))	// omp samp보고 추가.
			{
				if((pid=runProcess(i, AUTO_FLOW))< 0) {
					sprintf(trcBuf, "[%s] process auto restart fail[%s]\n", 
							__FUNCTION__, ProcessInfo[i].procName);
				}
				else {
					sprintf(trcBuf,"[%s] process[%s] auto restarted\n", 
							__FUNCTION__, ProcessInfo[i].procName);
				}
				trclib_writeLogErr(FL, trcBuf);
			}
		}

		report_sadb2FIMD();
		getProcessStatus();
	}
}

/*	For using a shared memory watchdog function		*/
void checkKeepAlive(void)
{
	int		i;

	for(i = 0; i < appCnt; i++)
	{

		if(!strcasecmp(loc_sadb->loc_process_sts[i].processName, myAppName) ||
			!strcasecmp(loc_sadb->loc_process_sts[i].processName, "CAPD"))
			continue;

		if(loc_sadb->loc_process_sts[i].status != SFM_STATUS_ALIVE)
			continue;

		if(keepalive->cnt[i] == keepalive->oldcnt[i])
			(keepalive->retry[i])++;
		else
			keepalive->retry[i] = 0;

		keepalive->oldcnt[i] = keepalive->cnt[i];
		if(keepalive->retry[i] < KEEPALIVE_CHECK_TIME)
			continue;

		sprintf(trcBuf,"[%s] watchdog kill; proc= %s\n", __FUNCTION__, loc_sadb->loc_process_sts[i].processName);
		trclib_writeLogErr(FL,trcBuf);

		kill(ProcessInfo[i].pid, SIGKILL);

		keepalive->retry[i] = -10;

		sendWatchdogMsg2COND(i);

		// OMP-FIMD에서 장애메시지를 만들수 있도록 하기 위해 프로세스 상태정보를
		//	다시한번 읽어 ProcessInfo에 setting한 후 즉시 report한다.
		//
		getProcessStatus();
		report_sadb2FIMD();
	}

	return;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int runProcess(int procIdx, int isAuto)
{
	int	i, pid;
	struct stat	statBuff;

	/*	혹시 이미 다시 살아났는지 한번더 확인한다.	*/
	if( (pid = getProcessID(ProcessInfo[procIdx].procName)) > 1)
	{
		sprintf(trcBuf, "[%s] already restarted [%s:%d]\n", __FUNCTION__, ProcessInfo[procIdx].procName,pid);
		trclib_writeLogErr(FL, trcBuf);
		return pid;
	}

	/*	기동시킬 파일이 있는지 확인한다.	*/
	if(stat(ProcessInfo[procIdx].exeFile, &statBuff) < 0)
	{
		sprintf(trcBuf, "[%s] stat fail[%s]; err=%d(%s)\n", __FUNCTION__, ProcessInfo[procIdx].exeFile, errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	
	//printf("[%s:%s:%d] fork()\n",__FILE__,__FUNCTION__,__LINE__);

	if( (pid = fork()) < 0)
	{
		sprintf(trcBuf,"[%s] stat fail=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if(pid==0)
	{
	    /* RLEG0~4는 execl의 인자(소문자)를 그대로 받아서 sysconfig에 설정된 프로세스명(대문자)과 비교하기 때문에
		 * execl의 인자를 대문자로 변환해서 입력한다. added by dcham 2011.04.27 */
		strtoupper(ProcessInfo[procIdx].procName);
		execl(ProcessInfo[procIdx].exeFile, ProcessInfo[procIdx].procName, NULL);
		exit(0);
	}

	/*	auto_restart인 경우에는 새로 기동시킨 놈의 pid가 필요 없다.	*/
	if(isAuto == AUTO_FLOW){
		return pid;
	}


	/*	start-prc 명령인 경우에는 새로 기동시킨 놈의 pid를 결과 메시지에 출력하기
	 *	위해 필요하므로 pid를 찾아 return한다.
	 *	- 자신의 child 프로세스는 execl로 실행시키고 종료되었으므로 위에서 fork()에서
	 *	  넘어온 pid는 이미 사라졌다.
	 */
	for(i = 0; i < 5; i++)
	{
		commlib_microSleep(100000);
		if( (pid = getProcessID(ProcessInfo[procIdx].procName)) > 1)
			return pid;
	}

	return -1;
}


//------------------------------------------------------------------------------
// 특정 프로세스의 PID를 찾는다.
//------------------------------------------------------------------------------
int getProcessID(char *procName)
{
#ifndef __LINUX__
	int				fd;
	DIR				*dirp;
	FILE			*fp;
	char			pathName[256];//, pName[256];
	struct dirent	*direntp;
	prpsinfo_t		psInfo;
	char			*DAppArgs, *DAppToken, tmpPSArgs[PRARGSZ];//, buff[256], buff2[256];

	if( (dirp = opendir(PROC_PATH)) == NULL)
	{
		sprintf(trcBuf, "[getProcessID] opendir fail; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
			continue;

		sprintf(pathName, "%s/%s", PROC_PATH, direntp->d_name);

		if( (fd = open(pathName, O_RDONLY)) < 0)
			continue;

		if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
		{
			close(fd);
			continue;
		}
		close(fd);

		if(!strcasecmp(procName, "CM") || !strcasecmp(procName, "SMSERVER"))
		{
			fp			= NULL;
		    DAppToken   = NULL;
		    DAppArgs    = NULL;

		    memset(tmpPSArgs, 0x00, PRARGSZ);
		    strcpy(tmpPSArgs, psInfo.pr_psargs);
		    DAppArgs    = strstr(tmpPSArgs, "DAPP");
		    if(DAppArgs)
		    {
		        if( (DAppToken = strchr(DAppArgs, '=')) == NULL)
		        {
				//#ifdef DEBUG
					sprintf(trcBuf, "[%s] check args(%s)\n", __FUNCTION__, DAppArgs);
					trclib_writeLogErr(FL, trcBuf);
				//#endif
					closedir(dirp);
					return -1;
		        }
				else
					DAppToken++;

		        while( (char)*DAppToken == ' ')
		            DAppToken++;

		        strtok(DAppToken, " ");
		        if( !strcasecmp(DAppToken, procName))
		        {
				#if 0
		           printf("[%s] %s find %s, pid : %d\n", __FUNCTION__, DAppToken, procName, psInfo.pr_pid);
		           // sprintf(trcBuf, "[%s] %s find\n", __FUNCTION__, DAppToken);
					//trclib_writeLogErr(FL, trcBuf);
				#endif
					closedir(dirp);
					return psInfo.pr_pid;
		        }
		    }
			else
				continue;
		}
		else
		{
			if(!strcasecmp(procName, psInfo.pr_fname))
			{
			#ifdef DEBUG
				fprintf(stderr, "[SFMD:GET_PROC_ID] pid = %ld\n", psInfo.pr_pid);
			#endif
				closedir(dirp);
				return psInfo.pr_pid;
			}
		}
	}
	closedir(dirp);

	return -1;
#else
	int				fd, status, pid;
	char			*dname, fname[100], pname[128*4];
	DIR				*dirp;
	struct dirent	*direntp;
	struct stat		st;

	if( (dirp = opendir(PROC_PATH)) == (DIR*)NULL)
	{
		sprintf(trcBuf, "[getProcessStatus] opendir err\n");
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		dname = direntp->d_name;
		if(!isdigit(*dname))
			continue;
		pid = atoi(dname);

		sprintf(fname, "/proc/%d/cmdline", pid);

		/*	get the process owner	*/
		if ( (status = stat(fname, &st)) != 0)
			continue;

		if( (fd = open(fname, O_RDONLY)) < 0)
			continue;
		else
		{
			memset(pname, 0x00, PROC_NAME_LEN);
			if(read(fd, pname, PROC_NAME_LEN-1) < 0)
			{
				close(fd);
				continue;
			}
			else
			{
				close(fd);

				if(!strcasecmp(procName, pname))
				{
////					fprintf (stderr,"[SFMD:GET_PROC_ID] pid = %s(%d)\n",pname,pid);
					closedir (dirp);
					return pid;
				}
			}
		}
	}
	closedir(dirp);

	return -1;
#endif
}


VersionIndexTable vit[] = {
    {"IXPC",    SEQ_PROC_IXPC},
    {"SAMD",    SEQ_PROC_SAMD},
    {"MMCR",    SEQ_PROC_MMCR},
 //   {"STMM",    SEQ_PROC_STMM},
    {"CAPD",    SEQ_PROC_CAPD},
    {"PANA",    SEQ_PROC_ANA},
//    {"CDR",     SEQ_PROC_CDR},

    {"RLEG0",    SEQ_PROC_SESSANA0},
	// added by dcham 20110530 for SM connection 축소(5=>1)
    //{"RLEG1",    SEQ_PROC_SESSANA1},
    //{"RLEG2",    SEQ_PROC_SESSANA2},
    //{"RLEG3",    SEQ_PROC_SESSANA3},
    //{"RLEG4",    SEQ_PROC_SESSANA4},
//    {"WAP1ANA", SEQ_PROC_WAP1ANA},
//    {"UAWAPANA",SEQ_PROC_UAWAPANA},
//    {"WAP2ANA",  SEQ_PROC_MESVC},
    {"RDRCAPD", SEQ_PROC_RDRCAPD},  /* by yhshin,  2009.04.26 */
//  {"HTTPANA",  SEQ_PROC_KUNSVC},
  //  {"VODSANA",  SEQ_PROC_VODSANA},
   // {"WIPINWANA",  SEQ_PROC_WIPINWANA},
    //{"JAVANWANA",   SEQ_PROC_KVMANA},                                               
   // {"REANA",   SEQ_PROC_REANA},                                                    
   // {"CDR2", SEQ_PROC_CDR2},                                                        
   // {"VTANA", SEQ_PROC_VTANA},                                                      
  //  {"UDRGEN",   SEQ_PROC_UDRGEN},                                                  
  //  {"AAAIF",    SEQ_PROC_AAAIF},                                                   
  //  {"SDMD",     SEQ_PROC_SDMD},                                                    
  //  {"LOGM",     SEQ_PROC_LOGM},                                                    
    {"RANA",    SEQ_PROC_RANA},                                                     
 //   {"FBANA",   SEQ_PROC_FBANA},                                                    
  //  {"MEMD",    SEQ_PROC_MEM},                                                      
    {"RDRANA",  SEQ_PROC_RDRANA},   // ADD by jjinri 2009.04.24                     
    {"SMPP",    SEQ_PROC_SMPP},     /* by june,  2009.04.26 */                      
    {NULL, -1}                                                                      
                                                                                    
};


char *get_ver_str(char *procname) 
{   
    int         i;
    static char ver[10];
    
    memset(ver, 0x00, sizeof(ver));
    if(!strcasecmp(procname, "CM") || !strcasecmp(procname, "SMSERVER"))
    {   
        strcpy(ver, "-");
        return ver;
    }
            
    for(i = 0; vit[i].name != NULL; i++)
    {   
        if(!strncasecmp(procname, vit[i].name, strlen(vit[i].name)))
        {   
            get_version(vit[i].index, ver);
            if(strlen(ver) < 1)
                return NULL;
            
            return ver;
        }   
    }   
    
    return NULL;
}


void doDisPrcSts(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	int				i, ret, alive, dead;
	char			argSys[COMM_MAX_NAME_LEN], tmpbuf[128], version[10], *tmpVer;
	size_t			len_ver;

	alive	= 0;
	dead	= 0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	len_ver = sizeof(version);

	/* get input paramters */
	strcpy(argSys, rxReqMsg->head.para[0].paraVal);
#if 1 // by june	
	sprintf(trcBuf, "[%s] para0.val:%s\n", __FUNCTION__, rxReqMsg->head.para[0].paraVal);
	trclib_writeLogErr(FL, trcBuf);
#endif
	if(strlen(argSys) == 0)
		strcpy(argSys, mySysName);

	for(i = 0; i < strlen(argSys); i++)
		argSys[i] = toupper(argSys[i]);

	if(strcasecmp(mySysName, argSys))
	{
		/*	내시스템과 일치하지 않으면	*/
		for(i = 0; i < loc_sadb->lanCount; i++)
		{
			/*	시스템 이름과 일치하는 것이 있으면	*/
			if(!strncasecmp(loc_sadb->loc_lan_sts[i].target_SYSName, argSys, strlen(argSys)))
			{
				strcpy(rxIxpcMsg->head.dstSysName, argSys);
				MMCReqBypassSnd(rxIxpcMsg);
				return;
			}
		}
		sprintf(trcBuf, "\n    RESULT = FAIL\n    REASON = UNRECOGNIZED SYSTEM\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
		return;
	}
	else
	{
		if( (ret = getProcessStatus()) < 0)
		{
			sprintf(trcBuf, "\n    RESULT = FAIL\n    REASON = ERR GET PROCESS STATUS\n\n");
			MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
			return;
		}
		sprintf(trcBuf, "    SYSTEM = %s\n    RESULT = SUCCESS\n", mySysName);
		strcat(trcBuf,  "    =================================================\n");
		sprintf(trcTmp, "    Process    PID    STATUS    START-TIME   VERSION\n");
		strcat(trcBuf, trcTmp);
		sprintf(trcTmp, "    =================================================\n");
		strcat(trcBuf, trcTmp);
		for(i = 0; i < loc_sadb->processCount; i++)
		{
			strcpy(tmpbuf, ProcessInfo[i].procName);
			strtoupper(tmpbuf);

			memset(version, 0x00, len_ver);
			//if( (tmpVer = get_proc_version(ProcessInfo[i].procName)) == NULL)
			//yhshin if( (tmpVer = get_version(ProcessInfo[i].procName)) == NULL)
			//tmpVer = get_proc_version(ProcessInfo[i].procName);
			//get_version (i, &tmpVer);

			tmpVer = get_ver_str (ProcessInfo[i].procName);

			if (tmpVer[0] == NULL) 
				sprintf(version, "%s", "-");
			else if(strlen(tmpVer) >= 10)
			{
				strncpy(version, tmpVer, len_ver-1);
				version[len_ver] = 0x00;
			}
			else
				strcpy(version, tmpVer);

			if(loc_sadb->loc_process_sts[i].status == SFM_STATUS_ALIVE)
			{
				// 프로세스 복수개를 보여주지 않는다. sjjeon
				//if(ProcessInfo[i].runCnt == 1)
				{
					sprintf(trcTmp,"    %-10s %-6ld ALIVE     %-12s %-10s\n",
						tmpbuf, ProcessInfo[i].pid, ProcessInfo[i].startTime, version);
					strcat(trcBuf, trcTmp);
				}
				/*
				else
				{
					sprintf(trcTmp,"    %-10s %-6ld ALIVE(%d)  %-12s %-10s\n",
						tmpbuf, ProcessInfo[i].pid, ProcessInfo[i].runCnt, ProcessInfo[i].startTime, version);
					strcat(trcBuf, trcTmp);
				}
				*/
	
				alive += ProcessInfo[i].runCnt;
			}
			else
			{
				sprintf(trcTmp,"    %-10s -      DEAD      -            %-10s\n", tmpbuf, version);
				strcat(trcBuf, trcTmp);
				dead++;
			}
		}

		{
			int sm_pid,cm_pid, rv;
			char startTM[32];
			// SM PID, TIME 을 구한다.
			sm_pid = getProcessID("SMSERVER");
			rv = getPidInfo(sm_pid, startTM);
			if(rv >= 0){
				sprintf(trcTmp,"    %-10s %-6d ALIVE     %-12s %-10s\n","SM",sm_pid,startTM, version);
				alive += 1;
			}
			else{
				sprintf(trcTmp,"    %-10s -     DEAD      -            %-10s\n", "SM", version);
				dead +=1;
			}
			strcat(trcBuf, trcTmp);

			// CM PID, TIME 을 구한다.
			memset(startTM,0x00,sizeof(startTM));
			cm_pid = getProcessID("CM");
			rv = getPidInfo(cm_pid, startTM);

			if(rv >= 0){
				sprintf(trcTmp,"    %-10s %-6d ALIVE     %-12s %-10s\n","CM",cm_pid,startTM, version);
				alive += 1;
			}
			else{
				sprintf(trcTmp,"    %-10s -      DEAD      -            %-10s\n", "CM", version);
				dead +=1;
			}
			strcat(trcBuf, trcTmp);

		}
		sprintf(trcTmp, "    -----------------------------------------------\n");
		strcat(trcBuf, trcTmp);
		sprintf(trcTmp, "    TOTAL:%d (ALIVE:%d, DEAD:%d)\n", alive+dead, alive, dead);
		strcat(trcTmp,  "    -==============================================\n");
		strcat(trcBuf, trcTmp);
		MMCResSnd(rxIxpcMsg, trcBuf, 0, 0);
	}
}

/*
by sjjeon
port 정보로 구동 시간을 얻어온다.
*/
int getPidInfo(int pid, char *sttime)
{
	char trcBuf[1024];
	char pathName[128];
	prpsinfo_t    psInfo;
	int fd;

	bzero(pathName,sizeof(pathName));

	sprintf(pathName, "%s/%d", PROC_PATH,pid );

	if((fd = open(pathName, O_RDONLY)) < 0)
	{
		//if( (errno != EACCES) && (errno != ENOENT))
		{
			/*  Permission denied 는 skip하기 위해  */
			sprintf(trcBuf, "[%s] open fail; (%s) err=%d[%s]\n",
					__FUNCTION__, pathName, errno, strerror(errno));
			trclib_writeLogErr(FL, trcBuf);
		}

		return -1;
	}

	if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
	{
		sprintf(trcBuf, "[%s] ioctl err=%d[%s]\n", __FUNCTION__, errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		close(fd);
		return -1;
	}
	close(fd);

	strftime (sttime, 32, "%m-%d %H:%M", localtime((time_t*)&(psInfo.pr_start.tv_sec)));
	//strftime (startTime, 32, "%m-%d %H:%M", localtime((time_t*)&(psInfo.pr_start.tv_sec)));
	//printf("p_name: %s, p_id : %d, time : %s\n",psInfo.pr_fname, psInfo.pr_pid,startTime );
	return 0;

}


void doKillPrc(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType	*rxReqMsg;
	int				i, procIdx, ret, rst;
	
	char			argSysName[COMM_MAX_NAME_LEN], argProcName[COMM_MAX_NAME_LEN];
	char 			logbuf[256];

	procIdx		= 0;
	rxReqMsg	= (MMLReqMsgType*)rxIxpcMsg->body;

	/*	get input parameters	*/
	strcpy(argSysName, rxReqMsg->head.para[0].paraVal);
	strcpy(argProcName, rxReqMsg->head.para[1].paraVal);

	/*	대문자로 출력하기 위해		*/
	strtoupper(argSysName);
	strtoupper(argProcName);

	/* add by sjjeon 2009/08/26  : process 상태가 -1로 나오므로 pid를 얻어온다. 
	   관리대상 프로세스들의 현재 상태 정보를 ProcessInfo table과 loc_sadb->loc_process_sts[i].status에 넣는다.    */
	if(getProcessStatus() < 0)
		    return;

	/*	자신의 시스템 것이 아니면 해당 시스템으로 bypass한다.	*/
	if(strcasecmp(mySysName, argSysName))
	{
		for(i = 0; i < loc_sadb->lanCount; i++)
		{
			/*	시스템 이름과 일치하는 것이 있으면	*/
			/*	modify by mnpark - 20040112	*/
			if(!strncasecmp(loc_sadb->loc_lan_sts[i].target_SYSName, argSysName, strlen(argSysName)))
			{
				strcpy(rxIxpcMsg->head.dstSysName, argSysName);
				MMCReqBypassSnd(rxIxpcMsg);
				return;
			}
		}
		sprintf(logbuf, "\n      RESULT = FAIL\n      REASON = UNRECOGNIZED SYSTEM\n\n");
		MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
		return;
	}

	ret	= strlen(argProcName);
	rst	= isdigit(argProcName[ret-1]);

	if(rst)
	{
		/*	등록되지 않은 프로세스인지 확인한다.	*/
		for(i = 0; i < loc_sadb->processCount; i++)
		{
			if(!strcasecmp(ProcessInfo[i].procName, argProcName))
			{
				procIdx = i;
				break;
			}
		}

		if(i >= loc_sadb->processCount)
		{
			sprintf (logbuf, "\n     RESULT = FAIL\n      REASON = NOT EXIST APPNAME\n\n");
			MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
			return;
		}

		/*	samd, ixpc, mmcd는 죽이지 못한다.		*/
		if(!strcasecmp(argProcName, "SAMD") || !strcasecmp (argProcName, "IXPC") || !strcasecmp (argProcName, "MMCD"))
		{
			sprintf (logbuf, "\n      RESULT = FAIL\n      REASON = CAN'T KILL PROCESS\n\n");
			MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
			return;
		}

		/*	이미 죽어 있는지 확인한다.	*/
		if( (ProcessInfo[procIdx].pid < 2) || (ProcessInfo[procIdx].mask == SFM_ALM_MASKED))
		{
			sprintf(logbuf, "\n      RESULT = FAIL\n      REASON = NOT RUNNING STATE\n\n");
			MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
			return;
		}

		if(kill(ProcessInfo[procIdx].pid, SIGTERM) < 0)
		{
			if(kill(ProcessInfo[procIdx].pid, SIGKILL) < 0)
			{
				sprintf (logbuf, "\n      RESULT = FAIL\n      REASON = CAN'T KILL(%s)\n\n", strerror(errno));
				MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
				return;
			}
		}
		ProcessInfo[procIdx].pid = 0;	//yhshin
		ProcessInfo[procIdx].mask = SFM_ALM_MASKED;

		sprintf(logbuf, "\n      RESULT = SUCCESS\n\n");
		sprintf(trcTmp, "      SYSTEM  = %s\n      APPNAME = %s\n\n", argSysName, argProcName);
		strcat(logbuf, trcTmp);
		MMCResSnd(rxIxpcMsg, logbuf, 0, 0);

	}
	else
	{
		for(i = 0; i < loc_sadb->processCount; i++)
		{
			if(!strcmp(ProcessInfo[i].procName, ""))
				continue;

			if(strncasecmp(ProcessInfo[i].procName, argProcName, strlen(argProcName)))
 				continue;
 			else
				procIdx = i;

			if(i >= loc_sadb->processCount)
			{
				sprintf (logbuf, "\n     RESULT = FAIL\n      REASON = NOT EXIST APPNAME\n\n");
				MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
				return;
			}

			/*	SAMD, IXPC, MMCD는 죽이지 못한다.	*/
			if (!strcasecmp (argProcName, "SAMD") || !strcasecmp (argProcName, "IXPC") || !strcasecmp (argProcName, "MMCD"))
			{
				sprintf (logbuf, "\n      RESULT = FAIL\n      REASON = CAN'T KILL PROCESS\n\n");
				MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
				return;
			}

			/*	이미 죽어 있는지 확인한다.			*/
			if( (ProcessInfo[procIdx].pid < 2) || (ProcessInfo[procIdx].mask == SFM_ALM_MASKED))
			{
				sprintf (logbuf, "\n      RESULT = FAIL\n      REASON = NOT RUNNING STATE\n\n");
				MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
				return;
			}


			if(kill(ProcessInfo[procIdx].pid, SIGTERM) < 0)
			{
				if(kill(ProcessInfo[procIdx].pid, SIGKILL) < 0 )
				{
					sprintf (logbuf, "\n      RESULT = FAIL\n      REASON = CAN'T KILL(%s)\n\n", strerror(errno));
					MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
					return;
				}
			}

			ProcessInfo[procIdx].pid = 0;	//yhshin
			ProcessInfo[procIdx].mask = SFM_ALM_MASKED;

			sprintf(logbuf, "\n      RESULT = SUCCESS\n\n");
			sprintf(trcTmp, "      SYSTEM  = %s\n      APPNAME = %s\n\n", argSysName, argProcName);
			strcat(logbuf, trcTmp);
			MMCResSnd(rxIxpcMsg, logbuf, 0, 0);
		}
	}
}


void doRunPrc(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType	*rxReqMsg;
	int				i, procIdx;
	char			argSysName[COMM_MAX_NAME_LEN], argProcName[COMM_MAX_NAME_LEN];
	char			logbuf[256];

	procIdx = 0;
	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/*	get input parameters	*/
	strcpy(argSysName, rxReqMsg->head.para[0].paraVal);
	strcpy(argProcName, rxReqMsg->head.para[1].paraVal);

	/*	대문자로 출력하기 위해	*/
	strtoupper(argSysName);
	strtoupper(argProcName);

	/*	자신의 시스템 것이 아니면 해당 시스템으로 bypass한다.	*/
	if(strcasecmp(mySysName, argSysName))
	{
		for(i = 0; i < loc_sadb->lanCount; i++)
		{
			/*	시스템 이름과 일치하는 것이 있으면	*/
			if(!strncasecmp(loc_sadb->loc_lan_sts[i].target_SYSName, argSysName, strlen(argSysName)))
			{
				strcpy(rxIxpcMsg->head.dstSysName, argSysName);
				MMCReqBypassSnd(rxIxpcMsg);
				return;
			}
		}
		sprintf(logbuf, "\n      RESULT = FAIL\n      REASON = UNRECOGNIZED SYSTEM\n\n");
		MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
		return;
	}

	/*	등록되지 않은 프로세스인지 확인한다.	*/
	for(i = 0; i < loc_sadb->processCount; i++)
	{
		if(!strcasecmp(ProcessInfo[i].procName, argProcName))
		{
			procIdx = i;
			break;
		}
	}

	if(i >= loc_sadb->processCount)
	{
		sprintf (logbuf, "\n      RESULT = FAIL\n      REASON = NOT EXIST APPNAME\n\n");
		MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
		return;
	}

	/*	이미 살아 있는지 확인한다.	*/
	if(ProcessInfo[procIdx].pid > 0)
	{
		sprintf (logbuf, "\n      RESULT = FAIL\n      REASON = ALREADY RUNNING STATE\n\n");
		MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
		return;
	}

	if(runProcess(procIdx, NORMAL_FLOW) < 0 )
	{
		sprintf (logbuf, "\n      RESULT = FAIL\n      REASON = RUN FAIL\n\n");
		MMCResSnd(rxIxpcMsg, logbuf, -1, 0);
		return;
	}

	ProcessInfo[i].mask = SFM_ALM_MASKED;
	sprintf(logbuf, "\n      RESULT = SUCCESS\n\n");
	sprintf(trcTmp, "      SYSTEM  = %s\n      APPNAME = %s\n\n", argSysName, argProcName);
	strcat(logbuf, trcTmp);
	MMCResSnd(rxIxpcMsg, logbuf, 0, 0);

	return;
}


void doDisLoadSts(IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType		*rxReqMsg;
	char				argSys[COMM_MAX_NAME_LEN];
	unsigned long long	mem_tot, mem_use, mem_free;
	unsigned int		i, queUsage;
	char				logbuf[4098];

	mem_tot		= 0;
	mem_use		= 0;
	mem_free	= 0;

	rxReqMsg	= (MMLReqMsgType*)rxIxpcMsg->body;

	/*	get input paramters	*/
	strcpy(argSys, rxReqMsg->head.para[0].paraVal);

	for(i = 0; i < strlen(argSys); i++)
		argSys[i] = toupper(argSys[i]);
/*
	if(strcasecmp(mySysName, argSys))
	{
		//	내시스템과 일치하지 않으면
		for(i = 0; i < loc_sadb->lanCount; i++)
		{
			// 시스템 이름과 일치하는 것이 있으면
			if(!strncasecmp(loc_sadb->loc_lan_sts[i].target_SYSName, argSys, strlen(argSys)))
			{
				strcpy(rxIxpcMsg->head.dstSysName, argSys);
				MMCReqBypassSnd(rxIxpcMsg);
				return;
			}
		}
		sprintf(trcBuf, "\n  RESULT = FAIL\n  REASON = INPUT PARA ERR\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
		return;
	}
	else
	{
*/
	sprintf(logbuf, "    SYSTEM = %s\n    RESULT = SUCCESS\n", mySysName);
	strcat(logbuf, "    ===========================================================================\n");
	for(i = 0; i < loc_sadb->cpuCount; i++)
	{
		if(i == 0)
		{
			sprintf(trcTmp, "    CPU[%d] USED = %d.%d%%\n", i, (loc_sadb->cpu_usage[i]/10), (loc_sadb->cpu_usage[i]%10));
			strcat(logbuf, trcTmp);
		}
		else
		{
			sprintf(trcTmp, "    CPU[%d] USED = %d.%d%%\n", i, (loc_sadb->cpu_usage[i]/10), (loc_sadb->cpu_usage[i]%10));
			strcat(logbuf, trcTmp);
		}
	}

	sprintf(trcTmp, "    MEM USED    = %d.%d%%\n", (loc_sadb->mem_usage/10), (loc_sadb->mem_usage%10));
	strcat(logbuf, trcTmp);

	for(i = 0; i < loc_sadb->diskCount; i++)
	{
		//sjjeon
		sprintf(trcTmp, "    DISK [ %-10s] USED = %d.%d%%\n",
			loc_sadb->loc_disk_sts[i].diskName, 
			(loc_sadb->loc_disk_sts[i].disk_usage/10), (loc_sadb->loc_disk_sts[i].disk_usage%10));
		//sprintf(trcTmp, "    DISK [ %-10s] USED = %d.%d%%\n",
			//loc_sadb->loc_disk_sts[i].diskName, (loc_sadb->loc_disk_sts[i].disk_usage/10), (loc_sadb->loc_disk_sts[i].disk_usage%10));
		strcat(logbuf, trcTmp);
	}

	for(i = 0 ; i < loc_sadb->queCount; i++)
	{
		if( (!loc_sadb->loc_que_sts[i].cBYTES) || (!loc_sadb->loc_que_sts[i].qBYTES))
			queUsage = 0;
		else
			queUsage = ((loc_sadb->loc_que_sts[i].cBYTES*100)/loc_sadb->loc_que_sts[i].qBYTES);
		sprintf(trcTmp, "    QUEUE[ %-10s] LOAD = [%3d%%] , NUM = %5d , USED = %5d/%5d(kbytes)\n",
			loc_sadb->loc_que_sts[i].qNAME, queUsage, loc_sadb->loc_que_sts[i].qNUM, loc_sadb->loc_que_sts[i].cBYTES, loc_sadb->loc_que_sts[i].qBYTES);
		strcat(logbuf, trcTmp);
	}
	strcat(logbuf, "    ===========================================================================\n");
	MMCResSnd(rxIxpcMsg, logbuf, 0, 0);
/*
	}
*/
}



void doDisLanSts (IxpcQMsgType *rxIxpcMsg)
{
	int		i;
	struct in_addr ipAddr;
	char 	logbuf[4098];

	sprintf (logbuf, "    SYSTEM = %s\n", mySysName);
	strcat (logbuf, "    RESULT = SUCCESS\n");
	sprintf (trcTmp, "    ============================================\n");
	strcat (logbuf, trcTmp);
	sprintf (trcTmp, "    %-11s  %-16s  %-12s\n", "DESTINATION", "IP_ADDRESS", "STATUS");
	strcat (logbuf, trcTmp);
	strcat (logbuf, "    ============================================\n");

	for (i=0; i<loc_sadb->lanCount; i++)
	{
		ipAddr.s_addr = loc_sadb->loc_lan_sts[i].target_IPaddress;
		if (loc_sadb->loc_lan_sts[i].status == SFM_LAN_CONNECTED)
			sprintf (trcTmp, "    %-11s  %-16s  %-12s\n",
					loc_sadb->loc_lan_sts[i].target_SYSName,
					inet_ntoa(ipAddr),
					"CONNECTED");
		else
			sprintf (trcTmp, "    %-11s  %-16s  %-12s\n",
					loc_sadb->loc_lan_sts[i].target_SYSName,
					inet_ntoa(ipAddr),
					"DISCONNECTED");
		strcat (logbuf, trcTmp);
	}

	/* remote lan info : sjjeon*/
	for (i=0; i<loc_sadb->rmtlanCount; i++)
	{
		ipAddr.s_addr = loc_sadb->rmt_lan_sts[i].target_IPaddress;
		if (loc_sadb->rmt_lan_sts[i].status == SFM_LAN_CONNECTED)
			sprintf (trcTmp, "    %-11s  %-16s  %-12s\n",
					loc_sadb->rmt_lan_sts[i].target_SYSName,
					inet_ntoa(ipAddr),
					"CONNECTED");
		else
			sprintf (trcTmp, "    %-11s  %-16s  %-12s\n",
					loc_sadb->rmt_lan_sts[i].target_SYSName,
					inet_ntoa(ipAddr),
					"DISCONNECTED");
		strcat (logbuf, trcTmp);
	}


	strcat (logbuf, "    ============================================\n");
	MMCResSnd (rxIxpcMsg, logbuf, 0, 0);
	return;
}


void doDisSysSts(IxpcQMsgType *rxIxpcMsg){

	int i;
	char	disBuf2[2048];
    char	tempBuf2[1024], rcBuf[1024];
	char    fan[SFM_HW_MAX_FAN_NUM][9], power[SFM_HW_MAX_PWR_NUM][9], mirror[SFM_MAX_DEV_CNT][10];
	char	link[SFM_HW_MAX_LINK_NUM][10], cpu[SFM_HW_MAX_CPU_NUM][7], disk[SFM_HW_MAX_DISK_NUM][10];

	memset(tempBuf2, 0x00, sizeof(tempBuf2));
	memset(disBuf2, 0x00, sizeof(disBuf2));

	for(i=0; i<SFM_HW_MAX_FAN_NUM; i++){
		if(loc_sadb->sysSts.fanSts[i].status == 0){
			strcpy(fan[i], "NORMAL");
		}else strcpy(fan[i], "ABNORMAL");
	}
	for(i=0; i<SFM_HW_MAX_PWR_NUM; i++){
		if(loc_sadb->sysSts.pwrSts[i].status == 0){
			strcpy(power[i], "NORMAL");
		}else strcpy(power[i], "ABNORMAL");
	}
	for(i=0; i<SFM_MAX_DEV_CNT; i++){
		if(loc_sadb->loc_link_sts[i].status == 0){
			strcpy(mirror[i], "UP");
		}else if(loc_sadb->loc_link_sts[i].status == 1) {
			strcpy(mirror[i], "DOWN");
		}
	}
	for(i=0; i<SFM_HW_MAX_LINK_NUM; i++){
#if 0 /* MODIFY : BY JUNE, 2011-01-06
		 사실상 loc_sadb->sysSts.linkSt 은 사용 안하고있다. */
		if(loc_sadb->sysSts.linkSts[i].status == 0){
			strcpy(link[i], "UP");
		}else if(loc_sadb->sysSts.linkSts[i].status == 1)strcpy(link[i], "DOWN");
		//printf("link: %d\n", loc_sadb->sysSts.linkSts[i].status);
#endif
		if (loc_sadb->sysHW[i].status == 0) strcpy(link[i], "UP");
		else if (loc_sadb->sysHW[i].status == 1) strcpy(link[i], "DOWN");
		else strcpy(link[i], "NOT EQUIP");
	}
	for(i=0; i<SFM_HW_MAX_CPU_NUM; i++){
		if(loc_sadb->sysSts.cpuSts[i].status == 0){
			strcpy(cpu[i], "NORMAL");
		}else if(loc_sadb->sysSts.cpuSts[i].status == 1) strcpy(cpu[i], "ABNORMAL");
	}
	for(i=0; i<SFM_HW_MAX_DISK_NUM; i++){
		if(loc_sadb->sysSts.diskSts[i].status == 0){
			strcpy(disk[i], "NORMAL");
		}else if(loc_sadb->sysSts.diskSts[i].status == 1) strcpy(disk[i], "ABNORMAL");
	}

	sprintf(rcBuf,"    RESULT = SUCCESS       \n");
    sprintf(tempBuf2,"    SYSTEM = %s\n", mySysName);
	strcat (tempBuf2,"    ==============================================\n");
	strcat (tempBuf2,"      TYPE       STATUS       TYPE      SATUS    \n");
	strcat (tempBuf2,"    ==============================================\n");
	strcat (rcBuf, tempBuf2);
	strcat (trcBuf2, rcBuf);
#if 0 
	sprintf(disBuf2,"      CPU0        %-8s     CPU1      %-8s     \n", cpu[0], cpu[1]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      CPU2        %-8s     CPU3      %-8s     \n", cpu[2], cpu[3]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      DISK0       %-8s     DISK1     %-8s     \n", disk[0], disk[1]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      DISK2       %-8s     DISK3     %-8s     \n", disk[2], disk[3]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      FAN0        %-8s     FAN1      %-8s     \n", fan[0], fan[1]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      FAN2        %-8s     FAN3      %-8s     \n", fan[2], fan[3]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      FAN4        %-8s     FAN5      %-8s     \n", fan[4], fan[5]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      POWER_A     %-8s     POWER_B   %-8s     \n", power[0], power[1]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      UAWAP(eth0) %-8s     UDR(eth1) %-8s     \n", link[0], link[1]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      UAWAP(eth2) %-8s     UDR(eth3) %-8s     \n", link[2], link[3]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      DSCM(eth4)  %-8s     HB(eth5)  %-8s     \n", link[4], link[5]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      DSCM(eth6)  %-8s     HB(eth7)  %-8s     \n", link[6], link[7]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      MIRROR_A    %-8s     MIRROR_B  %-8s     \n", mirror[0], mirror[1]);
#else
	/*DISK 정보*/
	sprintf(disBuf2,"      DISK1       %-8s     DISK2     %-8s     \n", disk[0], disk[1]);
	strcat (trcBuf2, disBuf2);
	/*FAN 정보*/
	sprintf(disBuf2,"      FAN1        %-8s     FAN2      %-8s     \n", fan[0], fan[1]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      FAN3        %-8s\n", fan[2]);
	strcat (trcBuf2, disBuf2);
	/*POWER 정보*/
	sprintf(disBuf2,"      POWER_A     %-8s     POWER_B   %-8s     \n", power[0], power[1]);
	strcat (trcBuf2, disBuf2);
	/*Link 정보*/
#if 0
	sprintf(disBuf2,"      DSCM(eth0) %-8s     REP(eth1)  %-8s     \n", link[0], link[1]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"       H.B(eth2) %-8s     TAPA(eth3) %-8s     \n", link[2], link[3]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      DSCM(eth4) %-8s     TAPB(eth5) %-8s     \n", link[4], link[5]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"       H.B(eth6) %-8s     REP(eth7)  %-8s     \n", link[6], link[7]);
#else
	for (i=0 ; i < 12 ; i=i+2) {
		sprintf(disBuf2,"      %s %-8s     %s  %-8s     \n", loc_sadb->sysHW[i].StsName, link[i], loc_sadb->sysHW[i+1].StsName, link[i+1]);
		strcat (trcBuf2, disBuf2);
	}

#endif
	//strcat (trcBuf2, disBuf2);
#endif 
	sprintf (disBuf2,"    ==============================================\n");
	strcat (trcBuf2, disBuf2);

	MMCResSnd (rxIxpcMsg, trcBuf2, 0, 0);

	memset (trcBuf2, 0x00, sizeof(trcBuf2));

} /* End of doDisSysSts */

void doDisSysSts2(IxpcQMsgType *rxIxpcMsg){

	int i;
	char    disBuf2[2048];
    char    tempBuf2[2048], rcBuf[1024];
    char    fan[6][9], power[4][9], mirror[2][10], link[9][5], cpu[8][7], disk[4][10];

	for(i=0; i<6; i++){
        if(loc_sadb->sysSts.fanSts[i].status == 0){
            strcpy(fan[i], "NORMAL");
        }else strcpy(fan[i], "ABNORMAL");
    }
    for(i=0; i<4; i++){
        if(loc_sadb->sysSts.pwrSts[i].status == 0){
            strcpy(power[i], "NORMAL");
        }else strcpy(power[i], "ABNORMAL");
    }
    for(i=0; i<2; i++){
        if(loc_sadb->loc_link_sts[i].status == 0){
            strcpy(mirror[i], "UP");
        }else if(loc_sadb->loc_link_sts[i].status == 1) {
            strcpy(mirror[i], "DOWN");
		}
    }
    for(i=0; i<9; i++){
		if(loc_sadb->sysSts.linkSts[i].status == 0){
	        strcpy(link[i], "UP");
	    }else if(loc_sadb->sysSts.linkSts[i].status == 1)strcpy(link[i], "DOWN");
	}
    for(i=0; i<8; i++){
        if(loc_sadb->sysSts.cpuSts[i].status == 0){
            strcpy(cpu[i], "NORMAL");
        }else if(loc_sadb->sysSts.cpuSts[i].status == 1) strcpy(cpu[i], "ABNORMAL");
    }
    for(i=0; i<4; i++){
        if(loc_sadb->sysSts.diskSts[i].status == 0){
            strcpy(disk[i], "NORMAL");
        }else if(loc_sadb->sysSts.diskSts[i].status == 1) strcpy(disk[i], "ABNORMAL");
    }

	memset(tempBuf2, 0x00, sizeof(tempBuf2));
	sprintf(tempBuf2,"    SYSTEM = %s\n", mySysName);
	strcat (tempBuf2,"    ==============================================\n");
	strcat (tempBuf2,"      TYPE       STATUS       TYPE      SATUS    \n");
    strcat (tempBuf2,"    ==============================================\n");
    strcat (rcBuf, tempBuf2);
    strcat (trcBuf2, rcBuf);
    sprintf(disBuf2,"      CPU0        %-8s     CPU1      %-8s     \n", cpu[0], cpu[1]);
    strcat (trcBuf2, disBuf2);
    sprintf(disBuf2,"      CPU2        %-8s     CPU3      %-8s     \n", cpu[2], cpu[3]);
    strcat (trcBuf2, disBuf2);
    sprintf(disBuf2,"      CPU4        %-8s     CPU5      %-8s     \n", cpu[4], cpu[5]);
    strcat (trcBuf2, disBuf2);
    sprintf(disBuf2,"      CPU6        %-8s     CPU7      %-8s     \n", cpu[6], cpu[7]);
    strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      DISK0       %-8s     DISK1     %-8s     \n", disk[0], disk[1]);
    strcat (trcBuf2, disBuf2);
    sprintf(disBuf2,"      DISK2       %-8s     DISK3     %-8s     \n", disk[2], disk[3]);
    strcat (trcBuf2, disBuf2);
    sprintf(disBuf2,"      FAN0        %-8s     FAN1      %-8s     \n", fan[0], fan[1]);
    strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      FAN2        %-8s     FAN3      %-8s     \n", fan[2], fan[3]);
    strcat (trcBuf2, disBuf2);
    sprintf(disBuf2,"      FAN4        %-8s     FAN5      %-8s     \n", fan[4], fan[5]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      POWER_A     %-8s     POWER_B   %-8s     \n", power[0], power[1]);
    strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      POWER_C     %-8s     POWER_D   %-8s     \n", power[2], power[3]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      UAWAP(eth0) %-8s     UDR(eth1) %-8s     \n", link[0], link[1]);
    strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      UAWAP(eth2) %-8s     UDR(eth3) %-8s     \n", link[2], link[3]);
    strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      DSCM(eth4)  %-8s     HB(eth5)  %-8s     \n", link[4], link[5]);
    strcat (trcBuf2, disBuf2);
    sprintf(disBuf2,"      DSCM(eth6)  %-8s     HB(eth7)  %-8s     \n", link[6], link[7]);
	strcat (trcBuf2, disBuf2);
	sprintf(disBuf2,"      MIRROR_A    %-8s     MIRROR_B  %-8s     \n", mirror[0], mirror[1]);
	strcat (disBuf2,"    ==============================================\n");
    strcat (trcBuf2, disBuf2);

    MMCResSnd (rxIxpcMsg, trcBuf2, 0, 0);

    memset (trcBuf2, 0x00, sizeof(trcBuf2));

} /* End of doDisSysSts2 */

#if 0
int db_restore(char *backupdir)
{
	int		i = 0;
	char	cmd[1024] = {0,};
	char	backFile[256] = {0,};

	printf("db_restore() call...\n");
	for( i = 0; i < MAX_SYS_CNT; i++ )
	{
		memset(cmd, 0x00, sizeof(cmd));
		memset(backFile, 0x00, sizeof(backFile));
		sprintf(backFile, "%s/%s", backupdir, g_stDBInfo.backFile);
		strcpy(g_stDBInfo.backFile, backFile);

		printf("backup file : %s \n", g_stDBInfo.backFile);

		if( !strcmp(g_stDBInfo.sysName, "DSCM" ))
		{
			sprintf(cmd, "mysqldump -u%s -p%s -h%s %s > %s", g_stDBInfo.dbId,
					g_stDBInfo.dbPass,
					g_stDBInfo.dbIp,
					g_stDBInfo.dbName,
					g_stDBInfo.backFile);
			system(cmd);
			break;

		}
		else if( !strcmp(g_stDBInfo.sysName, "DSCA" ))
		{
			sprintf(cmd, "mysqldump -u%s -p%s -h%s %s %s %s > %s", g_stDBInfo.dbId,
					g_stDBInfo.dbPass,
					g_stDBInfo.dbIp,
					g_stDBInfo.dbName,
					g_stDBInfo.tblName[0],
					g_stDBInfo.tblName[1],
					g_stDBInfo.backFile);
			system(cmd);
			break;
		}
		else if( !strcmp(g_stDBInfo.sysName, "DSCB" ))
		{
			sprintf(cmd, "mysqldump -u%s -p%s -h%s %s %s %s > %s", g_stDBInfo.dbId,
					g_stDBInfo.dbPass,
					g_stDBInfo.dbIp,
					g_stDBInfo.dbName,
					g_stDBInfo.tblName[0],
					g_stDBInfo.tblName[1],
					g_stDBInfo.backFile);
			system(cmd);
			break;
		}
	}
	return 0;
}
#endif

#if 0 // LGT 에 의해 DB backup 제외 
int db_backup(char *backupdir)
{
	int		i = 0;
	char	cmd[1024] = {0,};
	char	backFile[256] = {0,};

	for( i = 0; i < MAX_SYS_CNT; i++ )
	{
		memset(cmd, 0x00, sizeof(cmd));
		memset(backFile, 0x00, sizeof(backFile));
		sprintf(backFile, "%s/%s", backupdir, g_stDBInfo.backFile);
//		strcpy(g_stDBInfo.backFile, backFile);

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
		else if( !strncmp(g_stDBInfo.sysName, "SCMA",4 ) || !strncmp(g_stDBInfo.sysName, "SCMB", 4))
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
	char	cmd[1024], curdir[256], ivhome[256], tmp[128];
	MMLReqMsgType	*rxReqMsg;
	int				i;
	char			option[COMM_MAX_NAME_LEN];
    struct tm       tmnow;
    time_t          now = time(NULL);
	int				ret = 0;/*, pid = -1;*/

    struct stat file_info; // stat() 함수 추가 07.15

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	/* get input parameters
	*/
	strcpy (option, rxReqMsg->head.para[0].paraVal);

	/* 대문자로 출력하기 위해
	*/
	for(i=0 ; i<strlen(option); i++) option[i] = toupper(option[i]);

    if(!localtime_r(&now, &tmnow)){
		sprintf(trcBuf, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(local_time_r) CALL FAIL\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
		return;
    }

	strcpy(ivhome, getenv(IV_HOME));
    strftime(tmp, sizeof(tmp), "%m%d", &tmnow);
    sprintf(oldpath, "%s/OLD/%s", ivhome, tmp);

#ifdef __LINUX__
    if(mkdir(oldpath, DEFFILEMODE) < 0){
#else
	if (stat(oldpath, &file_info) < 0 || S_ISDIR(file_info.st_mode) < 0 )
	{
    	if(mkdir(oldpath, 0755) < 0){
#endif
			if(errno != EEXIST){
				sprintf(trcBuf, "    RESULT = FAIL\n    REASON = BACKUP DIRECTORY CREATE FAIL\n\n");
				MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
				return;
			}
		}
    }

    if(!getcwd(curdir, sizeof(curdir))){
		sprintf(trcBuf, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(getcwd) CALL FAIL\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
		return;
    }

	sprintf(newpath, "%s/NEW", ivhome);
    if(chdir(newpath) < 0){
		sprintf(trcBuf, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(chdir) CALL FAIL\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
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
//printf("[%s:%s:%d] fork()\n",__FILE__,__FUNCTION__,__LINE__);
#if 0 // db 삭제 
		if( (pid = fork()) < 0 )
		{
			sprintf(trcBuf, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(fork) CALL FAIL\n\n");
			MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
			return ;
		}
		if( pid == 0 ) // child is db backup.
		{
			ret = db_backup(oldpath);
			if( ret == -1 )
			{
				sprintf(trcBuf, "    RESULT = FAIL\n    REASON = DB BACKUP FAIL\n\n");
				MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
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
		
		//if( (pid = fork()) < 0 )
		//{
		//	sprintf(trcBuf, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(fork) CALL FAIL\n\n");
		//	MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
	//		return ;
	//	}
	//	if( pid == 0 ) // child is db backup.
	//	{
	//		ret = db_backup(oldpath);
	//		if( ret == -1 )
	//		{
	//			sprintf(trcBuf, "    RESULT = FAIL\n    REASON = DB BACKUP FAIL\n\n");
	//			MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
	//			return ;
	//		}
//
//			exit(0);
//		}
		
			ret = db_backup(oldpath);
			if( ret == -1 )
			{
				sprintf(trcBuf, "    RESULT = FAIL\n    REASON = DB BACKUP FAIL\n\n");
				MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
				return ;
			}
    }

#endif
//	if( strcasecmp(option, "DB") != 0){
		if(my_system(cmd) < 0){
			sprintf(trcBuf, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(system) CALL FAIL\n\n");
			MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
			return;
		}
//	}

    if(chdir(curdir) < 0){
		sprintf(trcBuf, "    RESULT = FAIL\n    REASON = SYSTEM LIBRARY(chdir) CALL FAIL\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
		return;
    }

	if( ret == 0 )
	{
		memset(trcBuf, 0x00, sizeof(trcBuf));
		memset(trcTmp, 0x00, sizeof(trcTmp));
// db 삭제 		if( strcasecmp(option, "DB") != 0){
		sprintf (trcBuf, "    RESULT = SUCCESS\n    BACKUP FILE = %s\n", tarname);
//		}

#if 0 // db 삭제 
		if( !strcasecmp(option, "ALL") || !strcasecmp(option, "DB") ) {
			sprintf (trcTmp, "    DB BACKUP FILE = %s\n", g_stDBInfo.backFile);
			strcat(trcBuf, trcTmp);
		}
#endif 
		MMCResSnd (rxIxpcMsg, trcBuf, 0, 0);
		return;
	}
}

#if 0
// Dial Up Internet Access Service Type Display

void doDisDuiaType (IxpcQMsgType *rxIxpcMsg)
{
    sprintf (trcBuf, "    SYSTEM = %s\n", mySysName);
    strcat (trcBuf, "    RESULT = SUCCESS\n");
   	sprintf (trcTmp, "    ===================\n");
    strcat (trcBuf, trcTmp);
    sprintf (trcTmp, "    DUIA SERVICE_TYPE\n");
    strcat (trcBuf, trcTmp);
    strcat (trcBuf, "    ===================\n");

   	sprintf (trcTmp, "    TYPE = %d\n", loc_sadb->duia.svcType);
    strcat (trcBuf, trcTmp);

    strcat (trcBuf, "    ===================\n");
    MMCResSnd (rxIxpcMsg, trcBuf, 0, 0);
    return;
}

void doSetDuiaType (IxpcQMsgType *rxIxpcMsg)
{
    int     prevValue;
	FILE	*fp;


	MMLReqMsgType   *rxReqMsg;
	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	prevValue = loc_sadb->duia.svcType;

	loc_sadb->duia.svcType = atoi(rxReqMsg->head.para[0].paraVal);

	if ( ( fp = fopen(l_sysconf2, "w+" ) ) == NULL ){
		sprintf (trcBuf, "[duia] fopen fail[%s]; err=%d(%s)\n", "DUIA.conf", errno, strerror(errno));
	    trclib_writeLogErr (FL,trcBuf);
		return;
	}
	/*
	fprintf(fp, "# Set Dial Up Internet Access Service Type\n"
	            "[SERVICE_TYPE]\n"
	            "TYPE = %d\n", atoi(rxReqMsg->head.para[0].paraVal));
				*/
	fclose(fp);

    sprintf (trcBuf, "    SYSTEM = %s\n", mySysName);
    strcat (trcBuf, "    RESULT = SUCCESS\n");
    sprintf (trcTmp, "    ===========================================\n");
    strcat (trcBuf, trcTmp);
    sprintf (trcTmp, "    SERVICE_TYPE   PREV_VALUE   CURRENT_VALUE\n");
    strcat (trcBuf, trcTmp);
    strcat (trcBuf, "    ===========================================\n");

    sprintf (trcTmp, "%16s   %-10d   %-10d\n","", prevValue, atoi(rxReqMsg->head.para[0].paraVal));
    strcat (trcBuf, trcTmp);

    strcat (trcBuf, "    ===========================================\n");
    MMCResSnd (rxIxpcMsg, trcBuf, 0, 0);
    return;
}

void no_Transmitted_udr_check(void)
{
	time_t now, prev_now;
	struct tm   *pLocalTime, *pPrevTime;
	struct stat dumy;
	char dayBuf[5], dayBuf2[5], fname1[32], fname2[32], tmpBuf[1024];
	int check_cnt1 = 0, check_cnt2=0, txLen;
	GeneralQMsgType     txGenQMsg;
	IxpcQMsgType        *txIxpcMsg;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;

	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));
	memset(fname1, 0x00, sizeof(fname1));
	memset(fname2, 0x00, sizeof(fname2));

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, "DSCM");
    strcpy (txIxpcMsg->head.dstAppName, "COND");

	now = time(0);
	pLocalTime = (struct tm*)localtime((time_t*)&now);
	strftime (dayBuf, 5, "%m%d", pLocalTime);

	prev_now = now - (60*60)*24;
	pPrevTime = (struct tm*)localtime((time_t*)&prev_now);
	strftime (dayBuf2, 5, "%m%d", pPrevTime);

	sprintf(fname1, "%s/%s", AAA_FAIL, dayBuf);
	if (stat(fname1, &dumy) < 0){
		check_cnt1++;
	}

	sprintf(fname2, "%s/%s", AAA_FAIL, dayBuf2);
	if (stat(fname2, &dumy) < 0){
    	check_cnt2++;
    }

	prev_alarm_flag = alarm_flag;

	if ((check_cnt1 == 0  || check_cnt2 == 0) && alarm_flag == 0 ){
		if(check_cnt1 == 0 && check_cnt2 != 0){
			sprintf(tmpBuf, "[%s: %s]\n",
					mySysName, fname1);
		}else if(check_cnt1 != 0 && check_cnt2 == 0){
			 sprintf(tmpBuf, "[%s: %s]\n",
					 mySysName, fname2);

		}else{
			sprintf(tmpBuf, "[%s: %s,%s]\n",
				mySysName, fname1, dayBuf2);
		}
		sprintf(txIxpcMsg->body, tmpBuf);
//fprintf(stderr, "txIxpcMsg->body: %s\n", txIxpcMsg->body);
		txGenQMsg.mtype = MTYPE_NO_TRANSMITTED_ACT;
		alarm_flag = 1;
	}else if (check_cnt1 > 0 && check_cnt2 > 0){
		txGenQMsg.mtype = MTYPE_NO_TRANSMITTED_DACT;
		txIxpcMsg->body[0] = 0x00;
		alarm_flag = 0;
	}
//fprintf(stderr, "mtype: %d\n", txGenQMsg.mtype);
	txIxpcMsg->head.bodyLen = strlen(txIxpcMsg->body);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (prev_alarm_flag != alarm_flag ) {
		if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		    	sprintf(trcBuf,"[no_Transmitted_udr_check] msgsnd fail to COND; err=%d(%s)\n", errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
		} else{
			    if (trcFlag || trcLogFlag) {
				    sprintf(trcBuf,"[no_Transmitted_udr_check] send to COND\n");
				    trclib_writeLog (FL,trcBuf);
				}
		}
	}
}
#endif

void checkSdmdForPrcSts(void)
{
	//int		i, k, prcSts;
	int		sts = 0;

	sts	= getLocalDupSts();
	loc_sadb->loc_system_dup.myLocalDupStatus = sts;
	//loc_sadb->loc_system_dup.myLocalDupStatus = 3;
	if (sts == 3 /* FAULTED */) {
		sprintf(trcBuf,"[checkSdmdForPrcSts] sts=[%d]\n",loc_sadb->loc_system_dup.myLocalDupStatus); 
		trclib_writeLogErr (FL,trcBuf);
	}

}	/*	End of checkSdmdForPrcSts	*/

/*
sjjeon : 로컬 이중화 상태
1 : ACTIVE, 2 : STANDBY:  -1: INVALID STATUS, 3: FAULTED
*/
int getLocalDupSts(void)
{
	const int	MAXLINE	= 256;
	const int	ACTIVE	= 1;
	const int	STANDBY	= 2;
	const int   FAULTED = 3; //by dcham

	FILE	*fp = NULL;
	int		rv=0, retry_cnt=0;
	char	buff[MAXLINE];
	char	*mySysName;
	char	SVRNAME[10];
	char	cmd[128];
	char  	outfile[80];

	memset(SVRNAME, 0x00, sizeof(SVRNAME));
	memset(cmd, 0x00, sizeof(cmd));
	mySysName = getenv(MY_SYS_NAME);

	if(!strcasecmp(mySysName, "DSCA")||!strcasecmp(mySysName,"SCMA")) {
		strcpy(SVRNAME, "SCMA");
	} else if(!strcasecmp(mySysName, "DSCB")||!strcasecmp(mySysName,"SCMB")) {
		strcpy(SVRNAME, "SCMB");
	}
#ifdef _FILE_OPEN_
	sprintf(outfile, "/tmp/hastatus_check.txt");
	sprintf(cmd, "/opt/VRTS/bin/hastatus -sum | grep SCM_SG | grep %s > %s", SVRNAME,outfile);

#else
	// popen
	sprintf(cmd, "/opt/VRTS/bin/hastatus -sum | grep SCM_SG | grep %s", SVRNAME);
#endif

RETRY_CHECK:
	if(retry_cnt >0){
		sprintf(trcBuf, "[%s] hastatus check fail.(retry: %d, cmd : %s)\n", 
				__FUNCTION__, retry_cnt-1, cmd);
		trclib_writeLogErr(FL, trcBuf);
		rv = -1;
		goto OUT;
	}

#ifdef _FILE_OPEN_
	if(fp) fclose(fp);
	fp = NULL;

	my_system(cmd);
	fp = fopen(outfile,"r"); 
#else
	if(fp) pclose(fp);
	fp = NULL;

	fp = popen(cmd, "r");
#endif
	if(fp == NULL)
	{
		sprintf(trcBuf, "[%s] f(p)open() error.(cmd : %s)\n", __FUNCTION__, cmd);
		trclib_writeLogErr(FL, trcBuf);
		retry_cnt++;
		goto RETRY_CHECK;
	}

	rv = -1;
	while(fgets(buff, MAXLINE, fp) != NULL)
	{
		if(strstr(buff, "ONLINE") != NULL)
		{
			rv = ACTIVE;
			// buffer clear
			while(fgets(buff, MAXLINE, fp) != NULL){;};
			break;
		}
		else if((strstr(buff, "OFFLINE") != NULL) && (strstr(buff, "FAULTED") == NULL))
		{
			rv = STANDBY;
			// buffer clear
			while(fgets(buff, MAXLINE, fp) != NULL){;};
			break;
		}
		else if(strstr(buff, "FAULTED") != NULL) // by dcham
		{
			rv = FAULTED;
			// buffer clear
			while(fgets(buff, MAXLINE, fp) != NULL){;};
			break;
		}
	//	rv = FAULTED;
		sprintf(trcBuf, "[getLocalDupSts] rv=%d\n", rv);
		        trclib_writeLogErr(FL, trcBuf);
//		else
//			rv = -1;
	}

	if(rv <= 0){
		sprintf(trcBuf, "[%s] Duplication check error.(rv=%d, cmd : %s)\n", __FUNCTION__,rv, cmd);
		trclib_writeLogErr(FL, trcBuf);
		retry_cnt++;
		goto RETRY_CHECK;
	}

OUT:
#ifdef _FILE_OPEN_
	if(fp) fclose(fp);
#else
	if(fp) pclose(fp);
#endif
	return rv;
}

/*
 문장중에서 해당 Index를 찾는다.
 해당 Index가 존재하면 1, 존재하지 않으면 0 : sjjeon
 */
int findIndex(char *syntax, char *idex)
{
	char	*token;
	char	search[]=" ";

	if( (syntax == NULL) || (idex == NULL))
		return -1;

	/*첫번째 설정*/
	token = strtok((char*)syntax, (char*)search);
	//printf("%s\n",token);
//yhshin	if(!strncasecmp(token,idex,strlen(idex)))
	if(!strncasecmp(token,idex,strlen(token)))
	{
		//printf("find OK.. %s\n",token);
		return 1;
	}

	while(1)
	{
		token = strtok(NULL, search);
		if(token == NULL)
			break;
		//printf("%s\n",token);
		if(!strncasecmp(token,idex,strlen(idex)))
		{
			//printf("find OK...%s\n",token);
			return 1;
		}
	}

	return 0;
}

#if 0
/*
by sjjeon
대소문자는 구분하지 않는다.
1:OK
0:NOK
*/
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
#endif

#if 0
/*
by sjjeon
   String에서 해당 word만 찾는다.

0 : FAIL
1 : OK
 */
int findWord(char *buf, char *index)
{

	char *s,*e;
	char tmp[64];                                                                               

	// 초기에 Index를 찾는다.
	s  = strstr(buf,index);                                                                      

	while(s != NULL)                                                                             
	{                                                                                            
		bzero(tmp,sizeof(tmp));

		//e  = strstr(s, " \t");                                                                     
		//문자열 s에서 문자열 accept 내의 문자들중 하나가 처음 
		//발견되는 곳을 찾아서 발견된 s문자열의 포인터를 돌려준다.

		e  = strpbrk(s, " \t");                                                                     
		if(e ==NULL){                                                                            
			if(e!=strstr(s,"\0"))                                                                
			{                                                                                    
				//printf("last word : %s",s);                                                      
				return 1;                                                                        
			}                                                                                    
			return 0;// fail
		}                                                                                        

		strncpy(tmp, s, e-s);                                                                    
		tmp[e-s]='\0';                                                                           
//		printf("%s, %s\n",tmp, index);                                                                      
		
		// 비교한다.
		if(!strcasecmp(tmp,index)){                                                              
	//		printf("find word, %s\n", index);                                                               
			return 1;                                                                            
		}                                                                                        
	//	else{                                                                                  
	//	  printf("Not find word\n");                                                           
	//	}                                                                                      
		s  = strstr(e+1,index);                                                                  
	}                                                                                            
	return 0;                                                                                   

}            
#endif


void doDisDupSts(IxpcQMsgType *rxIxpcMsg)
{
	int i;
	char    tempBuf[1024], rcBuf[1024];
	char    mySts[10], youSts[10], heart[10], optName[2][10], procSts[17][10], bondName[3][10];

	/** intialize variable **/
	memset(tempBuf, 0x00, sizeof(tempBuf));

	sprintf(rcBuf, "    RESULT = SUCCESS \n");
	sprintf(tempBuf,"    SYSTEM = %s\n",mySysName);

// sjjeon : 추후에 이 기능이 필요하면 수정한다. 
	strcat (tempBuf,"    =============================================================================================\n");
	strcat (rcBuf, tempBuf);
	strcpy (trcBuf, rcBuf);
	strcat (tempBuf,"    SELF_STS    OPP_STS     HB_STS      UDR_SEQ      UAWAP1_LOGID    UAWAP2_LOGID    UAWAP3_LOGID\n");
	strcat (tempBuf,"    MIRROR_A    MIRROR_B    AAA_LINK    WAPGW_LINK   CAPD            RANA            PANA\n" );
	strcat (tempBuf,"    CDR         TRCDR       WAP1ANA     UAWAPANA     WAP2ANA         HTTPANA         UDRGEN\n");
	strcat (tempBuf,"    AAAIF       VODSANA     JAVANWANA   WIPINWANA    VTANA           PCDR            FBANA\n");
	strcat (tempBuf,"    =============================================================================================\n");

	strcat (rcBuf, tempBuf);
	strcpy (trcBuf, rcBuf);

	/** my local duplication system info **/

	//SELF_STS
	if(loc_sadb->loc_system_dup.myLocalDupStatus == 1){
		strcpy(mySts, "ACTIVE");
	}else if(loc_sadb->loc_system_dup.myLocalDupStatus == 2){
		strcpy(mySts, "STANDBY");
	}else strcpy(mySts, "UNKNOWN");

	//OPP_STS
	if(loc_sadb->loc_system_dup.yourDupStatus == 1){
		strcpy(youSts, "ACTIVE");
	}else if(loc_sadb->loc_system_dup.yourDupStatus == 2){
		strcpy(youSts, "STANDBY");
	}else strcpy(youSts, "UNKNOWN");

	//HB_STS
	if(loc_sadb->loc_system_dup.heartbeatAlarm == 1){
		strcpy(heart, "NORMAL");
	}else strcpy(heart, "ABNORMAL");

/*
	for(i=0; i<2; i++){
		if(loc_sadb->loc_link_sts[i].status == 0){
			strcpy(optName[i], "NORMAL");
		}else strcpy(optName[i], "ABNORMAL");
	}
   // 사용하지 않아 막는다. sjjeon
	for(i=0; i<SFM_MAX_PROC_CNT; i++){
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "CAPD")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[0], "NORMAL");
			}
			else strcpy(procSts[0], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "RANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[1], "NORMAL");
			}
			else strcpy(procSts[1], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "PANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[2], "NORMAL");
			}
			else strcpy(procSts[2], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "CDR")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[3], "NORMAL");
			}
			else strcpy(procSts[3], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "TRCDR")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[4], "NORMAL");
			}
			else strcpy(procSts[4], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "WAP1ANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[5], "NORMAL");
			}
			else strcpy(procSts[5], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "UAWAPANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[6], "NORMAL");
			}
			else strcpy(procSts[6], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "WAP2ANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[7], "NORMAL");
			}
			else strcpy(procSts[7], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "HTTPANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[8], "NORMAL");
			}
			else strcpy(procSts[8], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "UDRGEN")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[9], "NORMAL");
			}
			else strcpy(procSts[9], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "AAAIF")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[10], "NORMAL");
			}
			else strcpy(procSts[10], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "VODSANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[11], "NORMAL");
			}
			else strcpy(procSts[11], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "JAVANWANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[12], "NORMAL");
			}
			else strcpy(procSts[12], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "WIPINWANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[13], "NORMAL");
			}
			else strcpy(procSts[13], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "VTANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[14], "NORMAL");
			}
			else strcpy(procSts[14], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "PCDR")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[15], "NORMAL");
			}
			else strcpy(procSts[15], "ABNORMAL");
		}
		if(!strcasecmp(loc_sadb->sdmd_Prc_sts[i].processName, "FBANA")){
			if(loc_sadb->sdmd_Prc_sts[i].status == 0){
				strcpy(procSts[16], "NORMAL");
			}
			else strcpy(procSts[16], "ABNORMAL");
		}
	}

*/
	for(i=0; i < (MAX_BOND_NUM-1); i++){
		if(loc_sadb->IF_bond[i].status == 0){
			strcpy(bondName[i], "NORMAL");
		}else strcpy(bondName[i], "ABNORMAL");
	}
	sprintf(tempBuf,"    %-7s     %-7s     %-8s    %-10d   %-14d  %-14d  %-14d\n",
		mySts, youSts, heart, loc_sadb->loc_system_dup.uiUdrSequence, loc_sadb->loc_system_dup.RmtDbSts[0].uiTxnLogId,
		loc_sadb->loc_system_dup.RmtDbSts[1].uiTxnLogId,loc_sadb->loc_system_dup.RmtDbSts[2].uiTxnLogId
	);
	strcat(trcBuf, tempBuf);
	sprintf(tempBuf,"    %-8s    %-8s    %-8s    %-8s     %-8s        %-8s        %-8s\n",
		optName[0], optName[1], bondName[1], bondName[0], procSts[0], procSts[1], procSts[2]
	);
	strcat(trcBuf, tempBuf);
	sprintf(tempBuf,"    %-8s    %-8s    %-8s    %-8s     %-8s        %-8s        %-8s\n",
		procSts[3], procSts[4], procSts[5], procSts[6], procSts[7], procSts[8], procSts[9]
	);
	strcat(trcBuf, tempBuf);
	sprintf(tempBuf,"    %-8s    %-8s    %-8s    %-8s     %-8s        %-8s        %-8s\n",
		procSts[10], procSts[11], procSts[12], procSts[13], procSts[14], procSts[15], procSts[16]
	);
	strcat(trcBuf, tempBuf);
	strcat(trcBuf, "    =============================================================================================\n");

	MMCResSnd (rxIxpcMsg, trcBuf, 0, 0);
} /* End of doDisDupSts */


pid_t getCM_AdapPid(void)
{
	int				fd;
	int       cmCnt;
	DIR				*dirp;
	// *DAdpArgs 변수 추가 20100822 by dcham
	char			*DAppArgs, *DAdpArgs, tmpPSArgs[PRARGSZ], pathName[256];
	struct dirent	*direntp;
	prpsinfo_t		psInfo;
	pid_t			ret_pid;

	cmCnt= 0; // by dcham
	ret_pid	= 0;

	if( (dirp = opendir(PROC_PATH)) == NULL)
	{
		sprintf(trcBuf, "[%s] opendir fail; err=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
			continue;

		sprintf(pathName, "%s/%s", PROC_PATH, direntp->d_name);

		if( (fd = open(pathName, O_RDONLY)) < 0)
			continue;
		if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
		{
			close(fd);
			continue;
		}
		close(fd);

		DAppArgs    = NULL;
		DAdpArgs    = NULL;

		memset(tmpPSArgs, 0x00, PRARGSZ);
		strcpy(tmpPSArgs, psInfo.pr_psargs);
		// 검색 keyword 추가("-DAPP=CM") 20100822 by dcham
		//if( (DAppArgs = strstr(tmpPSArgs, "cm")) == NULL) 
		if( ((DAppArgs = strstr(tmpPSArgs, "-DADAPTER=")) == NULL) && ((DAdpArgs = strstr(tmpPSArgs, "=CM")) == NULL))
			continue;
		else // CM프로세스 4개 모두 감시 하도록 수정. 20100902 by dcham
		{

			cmCnt++;
			if(cmCnt == 4) {
				ret_pid	= psInfo.pr_pid;
				break;
			} 
		}
	}
	closedir(dirp);

#ifdef DEBUG
	sprintf(trcBuf, "[%s] CM ADAPTER pid(%lu)\n", __FUNCTION__, ret_pid);
	trclib_writeLogErr(FL, trcBuf);
#endif

	return ret_pid;
}

/*
pid_t getCM_AdapPid(void)
{
	int				fd;
	DIR				*dirp;
	char			*DAppArgs, tmpPSArgs[PRARGSZ], pathName[256];
	char            *DAdpArgs;//dcham
	struct dirent	*direntp;
	prpsinfo_t		psInfo;
	pid_t			ret_pid;

	ret_pid	= 0;

	if( (dirp = opendir(PROC_PATH)) == NULL)
	{
		sprintf(trcBuf, "[%s] opendir fail; err=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
			continue;

		sprintf(pathName, "%s/%s", PROC_PATH, direntp->d_name);

		if( (fd = open(pathName, O_RDONLY)) < 0)
			continue;
		if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
		{
			close(fd);
			continue;
		}
		close(fd);

	    DAppArgs    = NULL;
		DAdpArgs    = NULL; // by dcham

	    memset(tmpPSArgs, 0x00, PRARGSZ);
	    strcpy(tmpPSArgs, psInfo.pr_psargs);
	    //if( ((DAppArgs = strstr(tmpPSArgs, "-DADAPTER=")) == NULL) || ((DAdpArgs = strstr(tmpPSArgs, "-DAPP=CM")) == NULL))
		if( ((DAppArgs = strstr(tmpPSArgs, "-DADAPTER=")) == NULL) || ((DAdpArgs = strstr(tmpPSArgs, "=CM")) == NULL))
			continue;
		else
		{
			ret_pid	= psInfo.pr_pid;
			break;
		}
	}
	closedir(dirp);
#ifdef DEBUG
	sprintf(trcBuf, "[%s] CM ADAPTER pid(%lu)\n", __FUNCTION__, ret_pid);
	trclib_writeLogErr(FL, trcBuf);
#endif

	return ret_pid;
}
*/

/*
   by sjjeon
   dis-sys-ver
*/
void doDisSysVer(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char argSys[COMM_MAX_NAME_LEN];
	int /*sysType,*/ pos= 0, i=0; 
	char cmd[]="/DSC/NEW/STATUS/dis-sys-ver.sh";
	char ver[10][21], buf[512];

	FILE *pp=NULL;
	pp = popen(cmd,"r");
	if(pp==NULL)
	{    
		sprintf (trcBuf, "[%s] doDisSysVer popen is null!!!\n",__FUNCTION__); trclib_writeLogErr (FL,trcBuf);
		return ;
	}    

	bzero(buf,sizeof(buf));
	bzero(ver,sizeof(ver));

	while(fgets(buf,512, pp)!=NULL){
		if(i>10) continue;
		if(strlen(buf)>20) 
			strncpy(ver[i], buf, 20); 
		else 
			strcpy(ver[i], buf);
		//printf("Get Ver.. %s\n", buf);
		i++; 
	}    
	pclose(pp);

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	strcpy(argSys, rxReqMsg->head.para[0].paraVal);

	
	btrim(ver[0]);
	btrim(ver[1]);
	btrim(ver[2]);

	pos += sprintf(trcBuf+pos, "    SYSTEM = %s\n", mySysName);
	pos += sprintf(trcBuf+pos, "    RESULT = SUCCESS\n");
	pos += sprintf(trcBuf+pos, "   =====================================\n");
	pos += sprintf(trcBuf+pos, "    %10s   %-20s \n", "SYS-NAME", "Version");
	pos += sprintf(trcBuf+pos, "   =====================================\n");
	pos += sprintf(trcBuf+pos, "    %-10s   %-20s\n", "SUN_OS", ver[0]);
	pos += sprintf(trcBuf+pos, "    %-10s   %-20s\n", "MY-SQL", ver[1]);
	pos += sprintf(trcBuf+pos, "    %-11s  %-20s\n", "TIMESTEN DB", ver[2]);
	pos += sprintf(trcBuf+pos, "   =====================================\n");

	MMCResSnd(rxIxpcMsg, trcBuf, 0, 0);
	sprintf (trcBuf, "[%s] doDisSysVer popen is not null!!!\n",__FUNCTION__); trclib_writeLogErr (FL,trcBuf);

	return ;
}

// by june, 20100312
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
// by june, 20100312
void doClrAlm_SysMsg(IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg;
	char    argHw[COMM_MAX_NAME_LEN], tmpStr[8];
	char    trcBuf_loc[1024];
	int     pos= 0, hwIdx = 0, i, flag=0;

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	strcpy(argHw, rxReqMsg->head.para[0].paraVal);
#if 0	
	sprintf(trcBuf, "[%s] cmd:%s, para.cnt:%d\n"
			, __FUNCTION__, rxReqMsg->head.cmdName, rxReqMsg->head.paraCnt);
	trclib_writeLogErr(FL, trcBuf);
	sprintf(trcBuf, "[%s] para0[%s].val:%s, para1[%s].val:%s\n"
			, __FUNCTION__
			, rxReqMsg->head.para[0].paraName, rxReqMsg->head.para[0].paraVal
			, rxReqMsg->head.para[1].paraName, rxReqMsg->head.para[1].paraVal);
	trclib_writeLogErr(FL, trcBuf);
#endif
	if (!strcasecmp(argHw, "ALL")) {
		for(i=1 ; i<4 ; i++) {
			sprintf(tmpStr, "FAN%d", i);
			hwIdx = get_hwinfo_index (tmpStr);
			if (hwIdx>0)
				loc_sadb->sysHW[hwIdx].status = 0; flag = 1;
		}
		for(i=1 ; i<3 ; i++) {
			sprintf(tmpStr, "PWR%d", i);
			hwIdx = get_hwinfo_index (tmpStr);
			if (hwIdx>0)
				loc_sadb->sysHW[hwIdx].status = 0; flag = 1;
		}
	}
	else {
		hwIdx = get_hwinfo_index (argHw);
		if (hwIdx>0)
			loc_sadb->sysHW[hwIdx].status = 0; flag = 1;
	}

	if (flag == 1) {
		pos += sprintf(trcBuf_loc+pos, "    SYSTEM = %s\n", mySysName);
		pos += sprintf(trcBuf_loc+pos, "    RESULT = SUCCESS\n");
		pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
		pos += sprintf(trcBuf_loc+pos, "    %5s %5s %7s \n", "SYS-NAME", "H/W", "STATE");
		pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
		pos += sprintf(trcBuf_loc+pos, "     %5s %7s %7s \n", mySysName, argHw, "CLEAR" );
		pos += sprintf(trcBuf_loc+pos, "   =====================================\n");
		MMCResSnd(rxIxpcMsg, trcBuf_loc, 0, 0);
	}
	else {
		sprintf (trcBuf_loc, "\n      RESULT = FAIL\n      REASON =  Invalid parameter 2(%s)\n\n", argHw);
		MMCResSnd(rxIxpcMsg, trcBuf_loc, -1, 0);
	}

}   /* End of doClrAlm_SysMsg() */

