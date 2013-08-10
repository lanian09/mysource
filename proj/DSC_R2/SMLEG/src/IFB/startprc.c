#include "ifb_proto.h"

extern IFB_ProcInfoContext	confProcTbl[SYSCONF_MAX_APPL_NUM];
int		oneProcFlag=0, noIFlag=0, samdQid = 0;
char	oneProcName[32];

pid_t getCM_AdapPid(void);

void handleChildProcess (void)
{                                                                                       
    int status;                                                                         
	// ���� ������ 
    while (wait3 (&status, WNOHANG, (struct rusage *)0) > 0);                           
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	char	*iv_home;
	int		procIndex,ret,rst;

	freopen ("/dev/null", "w", stdout);
	signal (SIGHUP, SIG_IGN);
//	signal (SIGCHLD, SIG_IGN);

	handleChildProcess ();

	if ((iv_home = getenv(IV_HOME)) == NULL) {
		fprintf(stderr, "[killprc_init] not found %s environment name\n", IV_HOME);
		return -1;
	}

	init_ver_shm();

	/* by june
	 * - ȭ�鿡 Operation Mode ǥ�� 
	 */
	if (init_sadb_shm() < 0)
		return -1;

	// login-name�� Ȯ���Ѵ�.
	if (ifb_checkLogin () < 0)
		return -1;

	/* id, passwd check mml_passwd �� �v */
	//if(interact_w(iv_home) < 0)
	//	return -1;

	// input argument Ȯ��
	if (startprc_getArgs (ac, av) < 0)
		return -1;

	if (startprc_getSAMDqid () < 0 )
		return -1;

	// sysconf ���Ͽ��� ��ϵ� process���� list�� confProcTbl�� �����Ѵ�.
	//
	if (ifb_setConfProcTbl () < 0)
		return -1;

	// /proc�� �˻��Ͽ� �������� process���� ������ confProcTbl�� �����Ѵ�.
	if (ifb_getProcStatus (1) < 0)
		return -1;

	if (oneProcFlag) {
		ret=strlen(oneProcName);
		rst=isdigit(oneProcName[ret-1]);

		if(rst) {
			// confProcTbl������ index�� ã�´�. -> ��ҹ��� ���� ���Ѵ�.
			if ((procIndex = ifb_getProcIndex (oneProcName)) < 0) {
				fprintf(stderr,"\n Not Registered process\n\n");
				return -1;
			}

			startprc_startupProc (procIndex);

			commlib_microSleep(3000);
			goto startprc_echoprint;
		} else {
			for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
				if (!strcmp (confProcTbl[procIndex].procName, ""))
					continue;

				if(!strncasecmp(confProcTbl[procIndex].procName,oneProcName,strlen(oneProcName))) {
					startprc_startupProc (procIndex);
				}
				commlib_microSleep(3000);
			}
			goto startprc_echoprint;
		}
	}

	// ��ϵ� ���μ��� ����Ʈ�� ����ϰ� Confirm�� ���� ���´�.
	if(!noIFlag)
		if (startprc_printProcListPrompt () < 0)
			return -1;

	// �⵿ �߰��� Ctl-C�� ���� ���� �� �� �ֵ���
	signal (SIGINT, startprc_interrupt);

	// ��ϵ� ��� ���μ����� ���ʷ� �⵿��Ų��.
	//
	for (procIndex=0; procIndex<SYSCONF_MAX_APPL_NUM; procIndex++) {
		if (!strcmp (confProcTbl[procIndex].procName, ""))
			continue;
		// ���μ����� �⵿��Ű�� ����� ����Ѵ�.
		// - �̹� running ���̸� ���̰� �ٽ� �������� ���´�.
		// - ������ �⵿�� pid�� confProcTbl�� ����ȴ�.
		startprc_startupProc (procIndex);
		/* MODIFY : BY JUNE
		 * DESC   : MMCR NIFO Creation & Reconstruction Time 
		 */
		if (!strcasecmp (confProcTbl[procIndex].procName, "MMCR"))
			sleep(3);
		else
			commlib_microSleep(300000);
	}

startprc_echoprint:
	// ������ �⵿��Ų ���μ������� ������ �ٽ� ����ϱ� ���� clear�Ѵ�.
	memset ((void*)confProcTbl, 0, sizeof(confProcTbl));

	// ���μ����� ���¸� �ٽ� �о� ����Ѵ�.
	ifb_setConfProcTbl ();
	commlib_microSleep(1000000);

	ifb_getProcStatus (0);
	ifb_printProcStatus ();
    detatch_ver_shm();

	return 0;

} //----- End of main -----//




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int startprc_getArgs (int ac, char *av[])
{
	int		a;
	char	*usage =
"  startprc [-b blockName] \n\
          -b blockName : startup process block name\n\
          -v           : startup processes without interacting with user\n\n";

	while ((a = getopt(ac,av,"b:v")) != EOF)
	{
		switch (a) {
			case 'b':
				oneProcFlag = 1;
				strcpy (oneProcName, optarg);
				break;

			case 'v':
				noIFlag = 1;
				break;

			default:
				fprintf(stderr,"%s",usage);
				return -1;
		}
	}

	if (optind < ac) {
		fprintf(stderr,"%s",usage);
		return -1;
	}

	return 1;

} //----- End of startprc_getArgs -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void startprc_startupProc (int index)
{
	pid_t			pid;
	unsigned char	i, pname[32], SCMcommand[128];

	pid	= 0;

	// �̹� running ������ Ȯ��
	if(confProcTbl[index].pid)
	{
		fprintf(stderr,"\n %s is already running\n", confProcTbl[index].procName);
        return;
/* LG Request that when the process is already running, do not ask the question
   of process replacement, do not invoke the new process, return at once." - samuel 2006.09.26 */
/*
		fprintf(stderr," Replace with New Process ? (y/n) [n] ");
		if (!ifb_promptYesNo())
			return;
		// �������� ���δ�.
		if (ifb_killProc (confProcTbl[index].pid) < 0)
			return;
		fprintf(stderr," Process %s(pid=%d) was killed\n",
				confProcTbl[index].procName, (int)confProcTbl[index].pid);
*/
	}

	if(!strcasecmp(confProcTbl[index].procName, "SMSERVER"))
	{
		sprintf(SCMcommand, "sudo -u pcube %s --start", confProcTbl[index].exeName);
		if(system(SCMcommand) < 0)
		{
			fprintf(stderr, "[%s] system(%s) ERROR[errno(%d):%s]\n", __FUNCTION__, SCMcommand, errno, strerror(errno));
			return;
		}
	}
	else if(!strcasecmp(confProcTbl[index].procName, "CM"))
	{
		while( (pid = getCM_AdapPid()) != 0)
		{
			if(kill(pid, SIGKILL) < 0)
			{
				fprintf(stderr, "[%s] kill(%lu) ERROR[errno(%d):%s]\n", __FUNCTION__, pid, errno, strerror(errno));
				return;
			}
		#ifdef DEBUG
			fprintf(stderr, "[%s] CM ADAPTER pid(%lu) Died!\n", __FUNCTION__, pid);
		#endif
		}

		sprintf(SCMcommand, "sudo -u scmscm %s start", confProcTbl[index].exeName);
		if(system(SCMcommand) < 0)
		{
			fprintf(stderr, "[%s] system(%s) ERROR[errno(%d):%s]\n", __FUNCTION__, SCMcommand, errno, strerror(errno));
			return;
		}
	}
	else
	{
		if( (pid = fork()) < 0)
		{
			fprintf(stderr,"\n Can't Create New Process\n\n");
			return;
		}

		if(pid == 0)
		{
			// �ش� process�� ��ϵ� �޽��� ť�� �ִ� ���� �޽����� ������� �⵿ ��Ų��.
			//ifb_clearQ(confProcTbl[index].msgQkey, 0);

			// sysconf���� �빮�ڷ� ��ϵǾ� �ְ�, �������� �̸��� �ҹ��ڷ� �Ǿ� �ִ�.
//			for(i = 0; i < strlen(confProcTbl[index].procName); i++)
//				pname[i] = tolower(confProcTbl[index].procName[i]);

			pname[i] = 0;
			if(execl(confProcTbl[index].exeName, confProcTbl[index].procName, NULL) < 0)
			{
				fprintf(stderr,"\n Can't Execute New Process : %s (%s)\n\n",
				confProcTbl[index].procName, confProcTbl[index].exeName);
			}
			exit(0);
		}
	}

	startprc_notify2samd(index);

	fprintf(stderr,"\n STARTUP NEW PROCESS\n   PROCESS : %s\n   PID : %d\n COMPLETED\n\n",
		confProcTbl[index].procName, (int)pid);

	confProcTbl[index].pid = pid; // yhshin
	return;
} //----- End of startprc_startupProc -----//

void startprc_notify2samd (int index)
{
    int                     txLen;
    GeneralQMsgType         txMsg;
    IFB_KillPrcNotiMsgType  *notiMsg;
    
    txMsg.mtype = MTYPE_STATUS_REPORT;
    notiMsg = (IFB_KillPrcNotiMsgType*)txMsg.body;

    strcpy (notiMsg->processName, confProcTbl[index].procName);
    notiMsg->Pid = confProcTbl[index].pid;
    notiMsg->type = 1; //start-prc
    txLen = sizeof(IFB_KillPrcNotiMsgType);

    if ( msgsnd (samdQid, (char*)&txMsg, txLen, IPC_NOWAIT)< 0 ){
        fprintf(stderr,"\n SAMD msgsnd fail; to SAMD  err=%d(%s)\n\n", errno, strerror(errno));
    }

    return;

} //----- End of startprc_notify2samd -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int startprc_printProcListPrompt (void)
{
	int		index;

	fprintf(stderr,"============================================================\n");
	fprintf(stderr," PROCESS_NAME   WHERE \n");
	fprintf(stderr,"============================================================\n");

	for (index=0; index<SYSCONF_MAX_APPL_NUM; index++) {
		if (strcmp(confProcTbl[index].procName, ""))
			fprintf(stderr," %-12s   %s\n", confProcTbl[index].procName, confProcTbl[index].exeName);
	}
	fprintf(stderr,"============================================================\n");
	fprintf(stderr,"\n Do you want startup whole Processes ? (y/n) [n] ");
	if (!ifb_promptYesNo())
		exit(0);
	fprintf(stderr,"\n     Startup whole Processes now ? (y/n) [n] ");
	if (!ifb_promptYesNo())
		exit(0);
	return 1;

} //----- End of startprc_printProcListPrompt -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void startprc_interrupt (int signo)
{
	fprintf(stderr,"\n     Do you want Abort startup procedure ? (y/n) [n] ");
	if (!ifb_promptYesNo())
		exit(0);
	signal (SIGINT, startprc_interrupt);
	return;
} //----- End of startprc_interrupt -----//

int startprc_getSAMDqid (void) 
{       
    char    *env, fname[256], tmp[32];
    int     key;
    
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

} //----- End of startprc_getSAMDqid -----//

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
