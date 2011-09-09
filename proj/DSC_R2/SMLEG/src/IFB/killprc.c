#include "ifb_proto.h"

extern IFB_ProcInfoContext	confProcTbl[SYSCONF_MAX_APPL_NUM];
int		oneProcFlag=0, fastFlag=0, noIFlag=0, samdQid;
char	oneProcName[32];

SFM_SysCommMsgType *loc_sadb;

pid_t getCM_AdapPid(void);


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	char	*iv_home;
	char	choice;
	int		procIndex, ret, rst;

	iv_home = getenv(IV_HOME);
	if(iv_home == NULL){
		fprintf(stderr, "[killprc_init] not found %s environment name\n", IV_HOME);
		return -1;
	}

    init_ver_shm();

	/* by june
	 * - 화면에 Operation Mode 표시 
	 */
	if (init_sadb_shm() < 0)
		return -1;

    fprintf(stdout, "\n");
	// login-name을 확인한다.
	if (ifb_checkLogin () < 0)
		return -1;
	fprintf(stdout, "\n");
	/* id, passwd check mml_passwd 팀 혌 */
	if(interact_w(iv_home) < 0)
		return -1;
	fprintf(stdout, "\n");
	// input argument 확인
	if (killprc_getArgs (ac, av) < 0)
		return -1;
	if (killprc_getSAMDqid () < 0)
		return -1;
	// sysconf 파일에서 등록된 process들의 list를 confProcTbl에 저장한다.
	//
	if (ifb_setConfProcTbl () < 0)
		return -1;
	// /proc를 검색하여 실행중인 process들의 정보를 confProcTbl에 저장한다.
	if (ifb_getProcStatus (1) < 0)
		return -1;

	if (oneProcFlag) {
		ret=strlen(oneProcName);
		rst=isdigit(oneProcName[ret-1]);

		if(rst) {
			// confProcTbl에서의 index를 찾는다. -> 대소문자 구분 안한다.
			if ((procIndex = ifb_getProcIndex (oneProcName)) < 0) {
				fprintf(stderr,"\n Not Registered process\n\n");
				return -1;
			}

			if (killprc_shutdownProc (procIndex) < 0)
				return -1;

			// 해당 process에 등록된 메시지 큐에 남아있는 메시지를 지운다.
			ifb_clearQ(confProcTbl[procIndex].msgQkey, 0);

			commlib_microSleep(3000);
			goto killprc_echoprint;
		} else {
			for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
				if (!strcmp (confProcTbl[procIndex].procName, ""))
					continue;

				if(!strncasecmp(confProcTbl[procIndex].procName,oneProcName,strlen(oneProcName))) {

					if (killprc_shutdownProc (procIndex) < 0)
						continue;

					// 해당 process에 등록된 메시지 큐에 남아있는 메시지를 지운다.
					ifb_clearQ(confProcTbl[procIndex].msgQkey, 0);

				}
				commlib_microSleep(10000);
			}
			goto killprc_echoprint;
		}
	}

	if(!noIFlag){
		fprintf(stderr,"\n Do you want shutdown whole Processes ? (y/n) [n] ");
		scanf("\n%c", &choice);
		if (!ifb_promptYesNo2(choice))
			exit(0);
		fprintf(stderr,"\n     Shutdown whole Processes now ? (y/n) [n] ");
		scanf("\n%c", &choice);
		if (!ifb_promptYesNo2(choice))
			exit(0);
	}

	// 등록된 모든 프로세스를 차례로 죽인다.
	// SAMD, IXPC 는 별도 처리한다. modified by uamyd 20110503
	// first, noti
	int ixpcIndex, samdIndex;
	for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
		if ( !strcmp (confProcTbl[procIndex].procName, "") ){ 
			continue;
		}
		if ( !strcmp (confProcTbl[procIndex].procName, "SAMD") ){
			samdIndex = procIndex;
		}
		if ( !strcmp (confProcTbl[procIndex].procName, "IXPC") ){
			ixpcIndex = procIndex;
		}
		killprc_notify2samd(procIndex);
	}

	commlib_microSleep(30000); //wait

	for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
		if ( !strcmp (confProcTbl[procIndex].procName, "") ){
			continue;
		}
		if( procIndex == samdIndex || procIndex == ixpcIndex ){
			continue;
		}
		if (killprc_shutdownProc (procIndex) < 0)
			continue;

		// 해당 process에 등록된 메시지 큐에 남아있는 메시지를 지운다.
		// 필요 없음 ifb_clearQ(confProcTbl[procIndex].msgQkey, 0);

		if (!fastFlag)
			commlib_microSleep(300000);
		else
			commlib_microSleep(10000);

	}

	commlib_microSleep(1000000); //SAMD 에서 전 프로세스에 대한 상태값을 전부 처리한 후에 보내도록 timer 설정
	killprc_shutdownProc (samdIndex);

	commlib_microSleep(1000000);
	killprc_shutdownProc (ixpcIndex);


killprc_echoprint:
	commlib_microSleep(300000);
	// 프로세스들의 정보를 다시 기록하기 위해 clear한다.
	memset ((void*)confProcTbl, 0, sizeof(confProcTbl));

	// 프로세스의 상태를 다시 읽어 출력한다.
	ifb_setConfProcTbl ();
	ifb_getProcStatus (1);

	ifb_printProcStatus ();
    detatch_ver_shm();

	exit(0);

} //----- End of main -----//




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int killprc_getArgs (int ac, char *av[])
{
	int		a;
	char	*usage =
"  killprc [-b blockName] [-f] \n\
          -b blockName : kill process block name\n\
		  -v : kill all processes without interacting with user\n\
          -f : fast\n";

	while ((a = getopt(ac,av,"b:fv")) != EOF)
	{
		switch (a) {
			case 'b':
				oneProcFlag = 1;
				strcpy (oneProcName, optarg);
				break;

			case 'f':
				fastFlag = 1;
				break;

			case 'v':
				noIFlag = 1;
				break;

			default:
				noIFlag = 1;
				break;
#if 0
			default:
				fprintf(stderr,"%s",usage);
				return -1;
#endif
		}
	}

	if (optind < ac) {
		fprintf(stderr,"%s",usage);
		return -1;
	}

	return 1;

} //----- End of killprc_getArgs -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int killprc_shutdownProc(int index)
{
	pid_t	pid;

	/*	check whether process is die or not...	*/
	if(!confProcTbl[index].pid)
	{
		fprintf(stderr, "\n %s is not running\n\n", confProcTbl[index].procName);
		return -1;
	}

	if(!strcasecmp(confProcTbl[index].procName, "CM"))
	{
		if(kill(confProcTbl[index].pid, SIGKILL) < 0)
		{
			fprintf(stderr, "\n kill(%s[%lu]) errno(%d): %s\n\n", confProcTbl[index].procName, confProcTbl[index].pid, errno, strerror(errno));
			return -1;
		}

		while( (pid = getCM_AdapPid()) != 0)
		{
			if(kill(pid, SIGKILL) < 0)
			{
				fprintf(stderr, "\n kill(%s[%lu]) errno(%d): %s\n\n", confProcTbl[index].procName, confProcTbl[index].pid, errno, strerror(errno));
				return -1;
			}
		#ifdef DEBUG
			fprintf(stderr, "[%s] CM ADAPTER pid(%lu) Died!\n", __FUNCTION__, pid);
		#endif
		}

		fprintf(stderr, "\n SHUTDOWN PROCESS\n   PROCESS : %s\n   PID : %d\n COMPLETED\n\n", confProcTbl[index].procName, (int)confProcTbl[index].pid);
	}
	else if(!strcasecmp(confProcTbl[index].procName, "SMSERVER"))
	{
		if(kill(confProcTbl[index].pid, SIGKILL) < 0)
		{
			fprintf(stderr, "\n kill(%s[%lu]) errno(%d): %s\n\n", confProcTbl[index].procName, confProcTbl[index].pid, errno, strerror(errno));
			return -1;
		}

		fprintf(stderr, "\n SHUTDOWN PROCESS\n   PROCESS : %s\n   PID : %d\n COMPLETED\n\n", confProcTbl[index].procName, (int)confProcTbl[index].pid);
	}
	else
	{
		#if 0 /* 순서를 바꾼다. added by uamyd 20110427, 간혹 발생하는 '종료 후, masking 처리 전, samd에 의해 재기동되는 현상' 때문에 수정 */
		if(ifb_killProc(confProcTbl[index].pid) < 0)
			return -1;

		/*	It is informed to SAMD process that process can't restart again.	*/
		killprc_notify2samd(index);
		#endif

		killprc_notify2samd(index);
		if(ifb_killProc(confProcTbl[index].pid) < 0){
#ifdef DEBUG
		fprintf(stderr, "\n SHUTDOWN PROCESS\n   PROCESS : %s\n   PID : %d\n FAILED\n\n", 
				confProcTbl[index].procName, (int)confProcTbl[index].pid);
#endif
			return -1;
		}

		fprintf(stderr, "\n SHUTDOWN PROCESS\n   PROCESS : %s\n   PID : %d\n COMPLETED\n\n", confProcTbl[index].procName, (int)confProcTbl[index].pid);

		/*	프로세스가 여러개 떠 있을수 있으므로 더 있는지 확인해 죽인다.	*/
		while( (pid = ifb_getPid(confProcTbl[index].procName)) > 0)
		{
			ifb_killProc(pid);
			commlib_microSleep(50000);
		}
	}

	return 1;
} //----- End of killprc_shutdownProc -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int killprc_getSAMDqid (void)
{
	char	*env, fname[256], tmp[32];
	int		key;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr,"\n not found %s environment name\n\n", IV_HOME);
		return -1;
	}
	sprintf(fname,"%s/%s",env,SYSCONF_FILE);
	if (conflib_getNthTokenInFileSection (fname, "APPLICATIONS", "SAMD", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	if ((samdQid = msgget (key, IPC_CREAT|0666)) < 0) {
		fprintf(stderr,"\n SAMD msgget fail; key=0x%x, err=%d(%s)\n\n", key, errno, strerror(errno));
		return -1;
	}
	return 1;

} //----- End of killprc_getSAMDqid -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void killprc_notify2samd (int index)
{
	int		txLen;
	GeneralQMsgType			txMsg;
	IFB_KillPrcNotiMsgType	*notiMsg;

	txMsg.mtype = MTYPE_STATUS_REPORT;
	notiMsg = (IFB_KillPrcNotiMsgType*)txMsg.body;

	strcpy (notiMsg->processName, confProcTbl[index].procName);
	notiMsg->type = 0;
	notiMsg->Pid = confProcTbl[index].pid;
	txLen = sizeof(IFB_KillPrcNotiMsgType);

	if (msgsnd (samdQid, (char*)&txMsg, txLen, IPC_NOWAIT) < 0 ){ 
		fprintf(stderr,"\n SAMD msgsnd fail; to SAMD  err=%d(%s)\n\n", errno, strerror(errno));
		fprintf(stderr,"\n FAILED IN MASKING, Process : %s, PID : %d\n",
            confProcTbl[index].procName, (int)confProcTbl[index].pid);
	}

	return;

} //----- End of killprc_notify2samd -----//
/*
void killprc_sadbUpdate(int index)
{
    int     i;

    for(i = 0; i < loc_sadb->processCount; i++){
        if(!strcasecmp(loc_sadb->loc_process_sts[i].processName,
            confProcTbl[index].procName)){
            loc_sadb->loc_process_sts[i].status =
        }
    }
}
*/

pid_t getCM_AdapPid(void)
{
	int				fd;
	DIR				*dirp;
	char			*DAppArgs, tmpPSArgs[PRARGSZ], pathName[256];
	struct dirent	*direntp;
	prpsinfo_t		psInfo;
	pid_t			ret_pid;

	ret_pid	= 0;

	if( (dirp = opendir(PROC_DIR)) == NULL)
	{
		fprintf(stderr, "[%s] opendir fail; err=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}

	while( (direntp = readdir(dirp)) != NULL)
	{
		if(!strcasecmp(direntp->d_name, PARENT_PATH) || !strcasecmp(direntp->d_name, HOME_PATH))
			continue;

		sprintf(pathName, "%s/%s", PROC_DIR, direntp->d_name);

		if( (fd = open(pathName, O_RDONLY)) < 0)
			continue;
		if(ioctl(fd, PIOCPSINFO, &psInfo) < 0)
		{
			close(fd);
			continue;
		}
		close(fd);

	    DAppArgs    = NULL;
	    memset(tmpPSArgs, 0x00, PRARGSZ);
	    strcpy(tmpPSArgs, psInfo.pr_psargs);
	    if( (DAppArgs = strstr(tmpPSArgs, "-DADAPTER=")) == NULL)
			continue;
		else
		{
			ret_pid	= psInfo.pr_pid;
			break;
		}
	}
	closedir(dirp);
#ifdef DEBUG
	fprintf(stderr, "[%s] CM ADAPTER pid(%lu)\n", __FUNCTION__, ret_pid);
#endif

	return ret_pid;
}
